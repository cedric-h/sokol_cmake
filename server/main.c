#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "../common/common.h"
#include "../bq_websocket/bq_websocket.h"
#include "../bq_websocket/bq_websocket_platform.h"

#define SOKOL_TIME_IMPL
#include "../sokol/sokol_time.h"

typedef struct {
    bqws_socket *ws;
    uint64_t last_hb_recv;
    bool timed_out;
} _srv_Client;

#define MAX_CLIENTS (128)
#define MAX_ENTS    (100)
/* internal server state */
static struct {
    _srv_Client clients[MAX_CLIENTS];
    uint64_t last_hb_send; 
    NetEnt ents[MAX_ENTS];
} _srv;

/* State tracked across all clients that only matters for a single tick */
typedef struct {
    int connection_count,
         timed_out_count;
    bool needs_hb;
} _srv_Frame;

/* call once per tick, handles client input */
void _srv_update_client(_srv_Client*, _srv_Frame*);

void _srv_make_map() {
    seed_rand(383, 3283, 0, 5848338);

    int i = 0;
    for (int x = 0; x < 10; x++)
    for (int y = 0; y < 10; y++) {
        _srv.ents[i].scale.x = 0.8f + 0.4f * randf();
        _srv.ents[i].scale.y = 0.8f + 0.4f * randf();
        _srv.ents[i].rot = PI_f * randf();
        Vec2 pos = vec2((float) x - 5.0f, (float) y - 5.0f);
        _srv.ents[i].pos.x = pos.x;
        _srv.ents[i].pos.y = pos.y;
        _srv.ents[i].art_hint = ArtHint_Stone;
        if (mag2(vec2(pos.x, pos.y)) < 3.0f) continue;
        i++;
    }
}

#ifdef EMBED_SERV
void srv_start() {
#else
int main() {
#endif
    _srv_make_map();

    bqws_pt_init(NULL);
    bqws_pt_server *sv = bqws_pt_listen(&(bqws_pt_listen_opts) { .port = 80 });

    stm_setup();

    int tick = 0;
    for (;;) {
        tick++;

        /* Accept new connections */
        bqws_socket *new_ws = bqws_pt_accept(sv, NULL, NULL);
        if (new_ws) {
            for (size_t i = 0; i < MAX_CLIENTS; i++) {
                if (_srv.clients[i].ws == NULL) {
                    bqws_server_accept(new_ws, NULL);
                    _srv.clients[i] = (_srv_Client) {
                        .last_hb_recv = stm_now(),
                        .ws = new_ws,
                    };
                    for (int i = 0; i < MAX_ENTS; i++) {
                        send_net_to_client_world_map_spawn(
                            (NetToClient_WorldMapSpawn) { .map_ent = _srv.ents[i] },
                            new_ws
                        );
                    }

                    /* Finish loading */
                    NetToClient_WorldSpawn res = {
                        .player_ent = (NetEnt) {
                            .art_hint = ArtHint_Player,
                            .scale.x = 1.0f,
                            .scale.y = 0.6f,
                        },
                    };
                    send_net_to_client_world_spawn(res, new_ws);

                    new_ws = NULL;
                    break;
                }
            }
            bqws_free_socket(new_ws);
        }

        _srv_Frame frame = {
            .connection_count = 0,
            .timed_out_count  = 0,
            .needs_hb = false,
         };
        if (stm_sec(stm_since(_srv.last_hb_send)) > 0.5f) {
            _srv.last_hb_send = stm_now();
            frame.needs_hb = true;
        }

        /* Update existing clients */
        for (size_t i = 0; i < MAX_CLIENTS; i++)
            if (_srv.clients[i].ws)
                _srv_update_client(&_srv.clients[i], &frame);

        /* tacky ASCII animation ensures you the server is updating */
        char anim = "|\\-/"[tick / 10 % 4];
        if (frame.timed_out_count > 0) {
            printf("\r[%c %d users just timed out %c]     ",
                    anim, frame.timed_out_count, anim);
        } else {
            /* nothing exciting is happening, we'll just tell you
               how many people are online ... */
            printf("\r[%c %d users online %c]             ",
                    anim, frame.connection_count, anim);
        }
        fflush(stdout);

        /* four ticks for every client tick */
        bqws_pt_sleep_ms(1000 / (60 / 4));
    }

    /* technically unreachable :( */
    // bqws_pt_free_server(sv);
    // bqws_pt_shutdown();
}

void _srv_apply_client_request(_srv_Client *client, NetToServer req);
/* keeps sockets in running order */
void _srv_update_client(_srv_Client *client, _srv_Frame *frame) {
    bqws_socket *ws = client->ws;
    /* update still needs to get called even if timed out,
       their socket still isn't fully closed */
    bqws_update(ws);

    if (bqws_is_closed(ws)) {
        /* Free the socket and slot */
        bqws_free_socket(ws);
        client->ws = NULL;
    }

    /* timing out only queues the close, so it still takes a couple ticks,
       long enough for us to display it on the status bar animation */
    if (client->timed_out) {
        frame->timed_out_count++;
        return;
    } else
        frame->connection_count++;

    /* send heartbeats and time out if we don't get a heartbeat */
    if (frame->needs_hb) {
        NetToClient_Heartbeat hb = { .ballast = 0 };
        send_net_to_client_heartbeat(hb, ws);
    }
    if (stm_sec(stm_since(client->last_hb_recv)) > 1.0f) {
        client->timed_out = true;
        bqws_queue_close(client->ws, BQWS_CLOSE_NO_REASON, NULL, 0);
    }

    /* prints text websocket messages to stdout,
       deserializes and applies the binary ones. */
    bqws_msg *msg;
    while ((msg = bqws_recv(ws)) != NULL) {
        if (msg->type == BQWS_MSG_TEXT) {
            printf("msg: %.*s\n", (int) msg->size, msg->data);
        } else if (msg->type == BQWS_MSG_BINARY) {
            uint8_t *data = (uint8_t *) msg->data;
            _srv_apply_client_request(client, unpack_net_to_server(data));
        } else {
            bqws_close(ws, BQWS_CLOSE_GENERIC_ERROR, NULL, 0);
        }
        bqws_free_msg(msg);
    }
}

void _srv_apply_client_request(_srv_Client *client, NetToServer nts) {
    switch (nts.kind) {
        case (NetToServerKind_Heartbeat):;
            client->last_hb_recv = stm_now();
            break;
    }
}

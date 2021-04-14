#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "../bq_websocket/bq_websocket.h"
#include "../bq_websocket/bq_websocket_platform.h"

#include "../sokol/sokol_time.h"
#include "../sokol/sokol_app.h"
#include "../sokol/sokol_gfx.h"
#include "../sokol/sokol_glue.h"
#include "../common/common.h"
#include "shaders.glsl.h"
#include "mui/microui.h"

typedef enum {
    EntProp_Active,
    EntProp_COUNT,
} EntProp;

typedef struct {
    /* specifies which of the following field groups are valid */
    uint16_t props[(EntProp_COUNT + 63)/64];

    float rot;
    Vec2 pos, scale;
    mu_Color color;
} Ent;

INLINE bool has_ent_prop(Ent *ent, EntProp prop) {
    return !!(ent->props[prop/64] & ((uint16_t)1 << (prop%64)));
}

INLINE bool take_ent_prop(Ent *ent, EntProp prop) {
    bool before = has_ent_prop(ent, prop);
    ent->props[prop/64] &= ~((uint16_t)1 << (prop%64));
    return before;
}

INLINE bool give_ent_prop(Ent *ent, EntProp prop) {
    bool before = has_ent_prop(ent, prop);
    ent->props[prop/64] |= (uint16_t)1 << (prop%64);
    return before;
}

#define MAX_ENTS    (1000)
#define NETBUF_SIZE (100)
static struct {
    bool spawned;
    struct {
        bqws_socket *ws;
        struct { bqws_msg *msg; NetToClient ntc; } msgbuf[NETBUF_SIZE];
        uint64_t last_hb_send, /* tracking   */
                 last_hb_recv; /* heartbeats */
    } net;
    Ent *player, ents[MAX_ENTS];
} state;

/* returns a pointer to some fresh Ent data stored in the state struct.
   The Entity will be set to active so its memory isn't reused, but other
   than that you can consider it zero-initialized. */
Ent *add_ent() {
    for (int i = 0; i < MAX_ENTS; i++)
        if (!state.ents[i].props[EntProp_Active]) {
            Ent *ent = &state.ents[i];
            *ent = (Ent) {0}; // clear existing memory
            ent->props[EntProp_Active] = true;
            return ent;
        }
    return NULL;
}

/* Use this function to iterate over all of the Ents in the game.
   ex:
        for (Ent *e = 0; e = ent_all_iter(e); )
            draw_ent(e);
*/
Ent *ent_all_iter(Ent *ent) {
    if (ent == NULL) ent = state.ents - 1;

    while (!has_ent_prop((++ent), EntProp_Active))
        if ((ent - state.ents) == MAX_ENTS)
            return NULL;

    if (has_ent_prop(ent, EntProp_Active)) return ent;
    return NULL;
}

#include "renderer.h"
#include "ui.h"

static void net_send_hb(void);
static void init(void) {
    seed_rand(2039583, 5505884, 485859, 3880);
    stm_setup();

    sg_setup(&(sg_desc) {
        .context = sapp_sgcontext()
    });

    bqws_pt_init(NULL);
    state.net.ws = bqws_pt_connect("ws://localhost:80", NULL, NULL, NULL);
    if (state.net.ws) net_send_hb();

    ui_init();
    rcx_init();
}

static void ui_test_window(mu_Context *ctx) {
    if (mu_begin_window(ctx, "Info", mu_rect(40, 40, 300, 200))) {
        mu_layout_row(ctx, 1, (int[]) { -1 }, 0);

        if (!state.spawned) mu_text(ctx, "loading ...");
        if (bqws_get_io_closed(state.net.ws)) {
            bqws_close_reason why = bqws_get_peer_close_reason(state.net.ws);
            if (why == BQWS_CLOSE_NO_REASON)
                mu_text(ctx, "server closed conn: timed out ..");
            else mu_text(ctx, "server closed conn");
        }

        static char server_msg[255];
        double hb_recv_sec = stm_sec(stm_since(state.net.last_hb_recv));
        sprintf(server_msg, "%.2fs since last server heartbeat", hb_recv_sec);
        mu_text(ctx, server_msg);
        
        if (hb_recv_sec > 1.0f) mu_text(ctx, "timed out?");

        mu_end_window(ctx);
    }
}

static void net_frame(void);
static void frame(void) {
    if (state.net.ws) net_frame();

    mu_begin(&_ui.ctx);
    ui_test_window(&_ui.ctx);
    mu_end(&_ui.ctx);

    /* if we have a player draw the game world surrounding it */
    if (state.spawned) {
        for (Ent *ent = 0; ent = ent_all_iter(ent); )
            rcx_rect(ent);
    }

    rcx_draw_commands(&_ui.ctx, sapp_widthf(), sapp_heightf());
}

static Ent *net_spawn_net_ent(NetEnt *net) {
    Ent *e = add_ent();
    e->rot = net->rot;
    e->pos = vec2(net->pos.x, net->pos.y);
    e->scale = vec2(net->scale.x, net->scale.y);
    switch (net->art_hint) {
    case (ArtHint_Player):;
        e->color = mu_color(0, 0, 0, 255);
        break;
    case (ArtHint_Stone):;
        uint8_t col = 70 + rand32() % 25;
        e->color = mu_color(20+col, col, col, 255);
        break;
    }
    return e;
}

static void net_handle_core_messages(void) {
    for (int i = 0; i < NETBUF_SIZE; i++) if (state.net.msgbuf[i].msg) {
        NetToClient *ntc = &state.net.msgbuf[i].ntc;

        switch (ntc->kind) {
        case (NetToClientKind_Heartbeat):;
            state.net.last_hb_recv = stm_now();
            break;
        case (NetToClientKind_WorldMapSpawn):;
            net_spawn_net_ent(&ntc->world_map_spawn.map_ent);
            break;
        case (NetToClientKind_WorldSpawn):;
            if (state.spawned) {
                puts("double spawn?");
                assert(false);
            }
            NetToClient_WorldSpawn *wo = &ntc->world_spawn;
            state.player = net_spawn_net_ent(&wo->player_ent);
            state.spawned = true;
            break;
        /* whoops, you're not one of ours! */
        default: goto SKIP_CONSUME;
        }

        bqws_free_msg(state.net.msgbuf[i].msg);
        state.net.msgbuf[i].msg = NULL;
        SKIP_CONSUME:;
    }
}

static void net_send_hb(void) {
    state.net.last_hb_send = stm_now();
    send_net_to_client_heartbeat(
        (NetToClient_Heartbeat) { .ballast = 0 },
        state.net.ws
    );
}

static void net_frame(void) {
    bqws_update(state.net.ws);
    bqws_msg *msg = NULL;
    while ((msg = bqws_recv(state.net.ws))) {
        if (msg->type == BQWS_MSG_TEXT) {
            printf("[Server]  %.*s", (int)msg->size, msg->data);
            bqws_free_msg(msg);
        } else if (msg->type == BQWS_MSG_BINARY) {
            NetToClient ntc = unpack_net_to_client((uint8_t *) msg->data);
            bool stored = false, space = false;
            for (int i = 0; i < NETBUF_SIZE; i++)
                if (state.net.msgbuf[i].msg == NULL) {
                    if (stored) {
                        space = true;
                        break;
                    } else {
                        state.net.msgbuf[i].ntc = ntc;
                        state.net.msgbuf[i].msg = msg;
                        stored = true;
                    }
                }
            if (!space) {
                printf("netbuf filled!");
                break;
            }
            /* freed when consumed in netbuf */
        } else {
            printf("unknown message type?");
            bqws_free_msg(msg);
        }
    }

    if (stm_sec(stm_since(state.net.last_hb_send)) > 0.5f)
        net_send_hb();

    net_handle_core_messages();
}

static void event(const sapp_event *ev) {
    if (ui_event(ev)) return;

    switch (ev->type) {
    case (SAPP_EVENTTYPE_KEY_DOWN):;
        #ifndef NDEBUG
        if (ev->key_code == SAPP_KEYCODE_ESCAPE)
            sapp_request_quit();
        #endif
        break;
    case SAPP_EVENTTYPE_MOUSE_UP:;
    case SAPP_EVENTTYPE_MOUSE_DOWN:;
    case SAPP_EVENTTYPE_MOUSE_MOVE:;
        /* uhhh make the player face the mouse? */
        break;
    };
}

static void cleanup(void) {
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .event_cb = event,
        .cleanup_cb = cleanup,
        .width = 800,
        .height = 600,
        .sample_count = 4,
        .gl_force_gles2 = true,
        .window_title = "Cave",
    };
}

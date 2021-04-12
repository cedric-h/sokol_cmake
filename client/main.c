#define _CRT_SECURE_NO_WARNINGS
#define LEN(arr) ((int) (sizeof arr / sizeof arr[0]))
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "../sokol/sokol_app.h"
#include "../sokol/sokol_gfx.h"
#include "../sokol/sokol_glue.h"
#include "../common/msvc_pragma.h"
#include "../common/math.h"
#include "shaders.glsl.h"
#include "mui/microui.h"

typedef struct {
    float rot;
    Vec2 pos, scale;
    mu_Color color;
} Ent;

/*
static struct {
    bqws_socket *ws;
    struct { bqws_msg *msg; NetToClient ntc; } netbuf[NETBUF_SIZE];
} state;*/

#include "renderer.h"
#include "ui.h"

static void init(void) {
    sg_setup(&(sg_desc) {
        .context = sapp_sgcontext()
    });
    ui_init();
    rcx_init();
}

static void ui_test_window(mu_Context *ctx) {
    if (mu_begin_window(ctx, "Window", mu_rect(40, 40, 300, 200))) {
        mu_text(ctx, "hi");
        mu_end_window(ctx);
    }
}

static void frame(void) {
    mu_begin(&_ui.ctx);
    ui_test_window(&_ui.ctx);
    mu_end(&_ui.ctx);

    Ent ent = {
        .scale = vec2_f(1.0f),
        .color = mu_color(0, 0, 0, 255),
    };
    rcx_rect(&ent);
    rcx_draw_commands(&_ui.ctx, sapp_widthf(), sapp_heightf());
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

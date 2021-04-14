#include "mui/atlas.inl"
#include <string.h>

#define BUFFER_SIZE 16384

static struct {
    float    tex_buf[BUFFER_SIZE *  8];
    float    vert_buf[BUFFER_SIZE *  8];
    uint8_t  color_buf[BUFFER_SIZE * 16];
    uint32_t index_buf[BUFFER_SIZE *  6];

    int buf_idx;

    sg_pipeline s_pip;
    sg_bindings s_bind;

    sg_buffer s_vbuf;
    sg_buffer s_vcol;
    sg_buffer s_vtex;
    sg_buffer s_ibuf;
} _rcx;

/* a uniform block with a model-view-projection matrix */
typedef struct {
    Mat4 mvp;
} params_t;

static void rcx_init(void) {
    sg_buffer_desc vbuf_desc = {.size=sizeof(_rcx.vert_buf), .usage=SG_USAGE_STREAM};
    sg_buffer_desc vcol_desc = {.size=sizeof(_rcx.color_buf),.usage=SG_USAGE_STREAM};
    sg_buffer_desc vtex_desc = {.size=sizeof(_rcx.tex_buf),  .usage=SG_USAGE_STREAM};

    sg_buffer_desc ibuf_desc = {
        .size = sizeof(_rcx.index_buf),
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .usage = SG_USAGE_STREAM
    };

    _rcx.s_vbuf = sg_make_buffer(&vbuf_desc);
    _rcx.s_vcol = sg_make_buffer(&vcol_desc);
    _rcx.s_vtex = sg_make_buffer(&vtex_desc);
    _rcx.s_ibuf = sg_make_buffer(&ibuf_desc);

    sg_image img = sg_make_image(&(sg_image_desc){
        .width = ATLAS_WIDTH,
        .height = ATLAS_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_R8,// RGBA8,
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST,
        .data.subimage[0][0] = SG_RANGE(atlas_texture),
    });

    /* define the resource bindings */
    _rcx.s_bind = (sg_bindings){
        .vertex_buffers[0] = _rcx.s_vbuf,
        .vertex_buffers[1] = _rcx.s_vtex,
        .vertex_buffers[2] = _rcx.s_vcol,
        .fs_images[0] = img,
        .index_buffer = _rcx.s_ibuf
    };

    /* create a shader (use vertex attribute locations) */
    sg_shader shd = sg_make_shader(mui_shader_desc(sg_query_backend()));

    /* create a pipeline object (default render state is fine) */
    _rcx.s_pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT32,
        .layout = {
            .attrs = {
                [0] = { .offset=0, .buffer_index = 0, .format=SG_VERTEXFORMAT_FLOAT2 },
                [1] = { .offset=0, .buffer_index = 1, .format=SG_VERTEXFORMAT_FLOAT2 },
                [2] = { .offset=0, .buffer_index = 2, .format=SG_VERTEXFORMAT_UBYTE4N }
            }
        },
        .colors[0].blend = {
            .enabled        = true,
            .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
            .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        }
    });
}

typedef struct {
    Vec2 tl, tr, bl, br;
} _rcx_Quad;

_rcx_Quad _rcx_quad_from_mu_rect(mu_Rect rt) {
    return (_rcx_Quad) {
        .tl = vec2((float) rt.x + (float) 0.0f, (float) rt.y + (float) 0.0f),
        .tr = vec2((float) rt.x + (float) rt.w, (float) rt.y + (float) 0.0f),
        .bl = vec2((float) rt.x + (float) 0.0f, (float) rt.y + (float) rt.h),
        .br = vec2((float) rt.x + (float) rt.w, (float) rt.y + (float) rt.h),
    };
}

static void _rcx_push_quad(_rcx_Quad quad, mu_Rect src, mu_Color color) {
    assert(_rcx.buf_idx < BUFFER_SIZE);

    int texvert_idx = _rcx.buf_idx *  8;
    int   color_idx = _rcx.buf_idx * 16;
    int element_idx = _rcx.buf_idx *  4;
    int   index_idx = _rcx.buf_idx *  6;
    _rcx.buf_idx++;

    /* update texture buffer */
    float x = src.x / (float) ATLAS_WIDTH;
    float y = src.y / (float) ATLAS_HEIGHT;
    float w = src.w / (float) ATLAS_WIDTH;
    float h = src.h / (float) ATLAS_HEIGHT;
    _rcx.tex_buf[texvert_idx + 0] = x;
    _rcx.tex_buf[texvert_idx + 1] = y;
    _rcx.tex_buf[texvert_idx + 2] = x + w;
    _rcx.tex_buf[texvert_idx + 3] = y;
    _rcx.tex_buf[texvert_idx + 4] = x;
    _rcx.tex_buf[texvert_idx + 5] = y + h;
    _rcx.tex_buf[texvert_idx + 6] = x + w;
    _rcx.tex_buf[texvert_idx + 7] = y + h;

    /* update vertex buffer */
    _rcx.vert_buf[texvert_idx + 0] = quad.tl.x;
    _rcx.vert_buf[texvert_idx + 1] = quad.tl.y;
    _rcx.vert_buf[texvert_idx + 2] = quad.tr.x;
    _rcx.vert_buf[texvert_idx + 3] = quad.tr.y;
    _rcx.vert_buf[texvert_idx + 4] = quad.bl.x;
    _rcx.vert_buf[texvert_idx + 5] = quad.bl.y;
    _rcx.vert_buf[texvert_idx + 6] = quad.br.x;
    _rcx.vert_buf[texvert_idx + 7] = quad.br.y;

    /* update color buffer */
    memcpy(_rcx.color_buf + color_idx +  0, &color, 4);
    memcpy(_rcx.color_buf + color_idx +  4, &color, 4);
    memcpy(_rcx.color_buf + color_idx +  8, &color, 4);
    memcpy(_rcx.color_buf + color_idx + 12, &color, 4);

    /* update index buffer */
    _rcx.index_buf[index_idx + 0] = element_idx + 0;
    _rcx.index_buf[index_idx + 1] = element_idx + 1;
    _rcx.index_buf[index_idx + 2] = element_idx + 2;
    _rcx.index_buf[index_idx + 3] = element_idx + 2;
    _rcx.index_buf[index_idx + 4] = element_idx + 3;
    _rcx.index_buf[index_idx + 5] = element_idx + 1;
}

static void rcx_rect(Ent *ent) {
    Vec2 center = {
        .x =  sapp_widthf() / 2.0f,
        .y = sapp_heightf() / 2.0f,
    };
    #define WORLD_SCALE (50.0f)

    Vec2 scale = mul2_f(ent->scale, WORLD_SCALE);
    Vec2 offset = add2(mul2_f(ent->pos, WORLD_SCALE), center);
    float rot = ent->rot - PI_f / 4.0f;
    _rcx_Quad quad = {
        .tl = add2(offset, mul2(vec2_rot(rot              ), scale)),
        .tr = add2(offset, mul2(vec2_rot(rot + PI_f / 2.0f), scale)),
        .bl = add2(offset, mul2(vec2_rot(rot - PI_f / 2.0f), scale)),
        .br = add2(offset, mul2(vec2_rot(rot - PI_f / 1.0f), scale)),
    };
    _rcx_push_quad(quad, atlas[ATLAS_WHITE], ent->color);
}

static void _rcx_draw_rect(mu_Rect rect, mu_Color color) {
    _rcx_Quad quad = _rcx_quad_from_mu_rect(rect);
    _rcx_push_quad(quad, atlas[ATLAS_WHITE], color);
}

static void _rcx_draw_text(const char *text, mu_Vec2 pos, mu_Color color) {
    mu_Rect dst = { pos.x, pos.y, 0, 0 };
    for (const char *p = text; *p; p++) {
        if ((*p & 0xc0) == 0x80) { continue; }
        int chr = mu_min((unsigned char) *p, 127);
        mu_Rect src = atlas[ATLAS_FONT + chr];
        dst.w = src.w;
        dst.h = src.h;
        _rcx_Quad quad = _rcx_quad_from_mu_rect(dst);
        _rcx_push_quad(quad, src, color);
        dst.x += dst.w;
    }
}


static void _rcx_draw_icon(int id, mu_Rect rect, mu_Color color) {
    mu_Rect src = atlas[id];
    int x = rect.x + (rect.w - src.w) / 2;
    int y = rect.y + (rect.h - src.h) / 2;
    _rcx_Quad quad = _rcx_quad_from_mu_rect(mu_rect(x, y, src.w, src.h));
    _rcx_push_quad(quad, src, color);
}


static int rcx_get_text_width(const char *text, int len) {
    int res = 0;
    for (const char *p = text; *p && len--; p++) {
        if ((*p & 0xc0) == 0x80) { continue; }
        int chr = mu_min((unsigned char) *p, 127);
        res += atlas[ATLAS_FONT + chr].w;
    }
    return res;
}


static int rcx_get_text_height(void) {
    return 18;
}

typedef enum {
    CMD_DRAW,
    CMD_CLIP
} cmd_type;

#define MAX_CMDS (1024)

typedef struct {
    cmd_type type;
    union {
        mu_Rect clip;
        struct {
            int start_buf_idx;
            int length;
        } draw;
    };
} _rcx_draw_cmd;

typedef struct {
    int cmd_idx;
    int start_buf_idx;
    _rcx_draw_cmd cmds[MAX_CMDS];
} _rcx_draw_fifo;

static void _rcx_draw_fifo_start(_rcx_draw_fifo* self) {
    self->cmd_idx = 0;
    self->start_buf_idx = 0;
}

static void _rcx_draw_fifo_queue_draw(_rcx_draw_fifo* self, int cur_buf_idx) {
    assert(self->cmd_idx < MAX_CMDS);
    self->cmds[self->cmd_idx++] = (_rcx_draw_cmd){
        .type = CMD_DRAW,
        .draw = {
            .start_buf_idx = self->start_buf_idx,
            .length = cur_buf_idx - self->start_buf_idx
        }
    };
    self->start_buf_idx = cur_buf_idx;
}

static void _rcx_draw_fifo_queue_clip(_rcx_draw_fifo* self, mu_Rect r) {
    assert(self->cmd_idx < MAX_CMDS);
    self->cmds[self->cmd_idx++] = (_rcx_draw_cmd){
        .type = CMD_CLIP,
        .clip = r
    };
}

static void rcx_draw_commands(mu_Context* ctx, float width, float height) {
    params_t vs_params;

    vs_params.mvp = ortho4x4(0.0f, width, height, 0.0f, -1.0f, 1.0f);

    /* default pass action */
    sg_pass_action pass_action = {
        .colors[0] = { .action = SG_ACTION_CLEAR, .value = { 0.25f, 0.25f, 0.3f, 1.0f } }
    };

    sg_begin_default_pass(&pass_action, (int) width, (int) height);
    sg_apply_pipeline(_rcx.s_pip);
    sg_apply_bindings(&_rcx.s_bind);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_mui_uniforms, &SG_RANGE(vs_params));

    /* render */
    _rcx_draw_fifo cmd_fifo;
    _rcx_draw_fifo_start(&cmd_fifo);

    mu_Command *cmd = NULL;
    while (mu_next_command(ctx, &cmd)) {
        switch (cmd->type) {
            case MU_COMMAND_TEXT:
                _rcx_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color); break;
            case MU_COMMAND_RECT:
                _rcx_draw_rect(cmd->rect.rect, cmd->rect.color); break;
            case MU_COMMAND_ICON:
                _rcx_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
            case MU_COMMAND_CLIP: {
                _rcx_draw_fifo_queue_draw(&cmd_fifo, _rcx.buf_idx);
                _rcx_draw_fifo_queue_clip(&cmd_fifo, cmd->clip.rect);
                break;
            }
        }
    }
    _rcx_draw_fifo_queue_draw(&cmd_fifo, _rcx.buf_idx);

    sg_update_buffer(_rcx.s_vbuf, &(sg_range) {
        .ptr = _rcx.vert_buf, 
        .size = _rcx.buf_idx*8*sizeof(float)
    });
    sg_update_buffer(_rcx.s_vtex, &(sg_range) {
        .ptr = _rcx.tex_buf, 
        .size = _rcx.buf_idx*8*sizeof(float)
    });
    sg_update_buffer(_rcx.s_vcol, &(sg_range) {
        .ptr = _rcx.color_buf, 
        .size = _rcx.buf_idx*16
    });
    sg_update_buffer(_rcx.s_ibuf, &(sg_range) {
        .ptr = _rcx.index_buf, 
        .size = _rcx.buf_idx*6*sizeof(int)
    });

    for (int i = 0; i < cmd_fifo.cmd_idx; i++) {
        const _rcx_draw_cmd* c = &cmd_fifo.cmds[i];
        switch(c->type) {
            case CMD_DRAW: {
                if (c->draw.length != 0) {
                    sg_draw(c->draw.start_buf_idx*6, c->draw.length*6, 1);
                }
                break;
            }
            case CMD_CLIP: {
                mu_Rect r = c->clip;
                sg_apply_scissor_rect(r.x, r.y, r.w, r.h, true);
                break;
            }
        }
    }
    _rcx.buf_idx = 0;

    sg_end_pass();
    sg_commit();
}

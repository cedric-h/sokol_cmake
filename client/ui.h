static struct {
    mu_Context ctx;
} _ui;

static int _ui_text_width(mu_Font font, const char *text, int len) {
    (void) font;
    if (len == -1) { len = (int) strlen(text); }
    return rcx_get_text_width(text, len);
}

static int _ui_text_height(mu_Font font) {
    (void) font;
    return rcx_get_text_height();
}

static void ui_init(void) {
    mu_init(&_ui.ctx);
    _ui.ctx.text_width = _ui_text_width;
    _ui.ctx.text_height = _ui_text_height;
}

static bool ui_event(const sapp_event *ev) {
    static const char key_map[512] = {
        [SAPP_KEYCODE_LEFT_SHIFT]       = MU_KEY_SHIFT,
        [SAPP_KEYCODE_RIGHT_SHIFT]      = MU_KEY_SHIFT,
        [SAPP_KEYCODE_LEFT_CONTROL]     = MU_KEY_CTRL,
        [SAPP_KEYCODE_RIGHT_CONTROL]    = MU_KEY_CTRL,
        [SAPP_KEYCODE_LEFT_ALT]         = MU_KEY_ALT,
        [SAPP_KEYCODE_RIGHT_ALT]        = MU_KEY_ALT,
        [SAPP_KEYCODE_ENTER]            = MU_KEY_RETURN,
        [SAPP_KEYCODE_BACKSPACE]        = MU_KEY_BACKSPACE,
    };

    switch (ev->type) {
    case SAPP_EVENTTYPE_MOUSE_DOWN:
        mu_input_mousedown(&_ui.ctx, (int)ev->mouse_x, (int)ev->mouse_y, (1<<ev->mouse_button));
        break;
    case SAPP_EVENTTYPE_MOUSE_UP:
        mu_input_mouseup(&_ui.ctx, (int)ev->mouse_x, (int)ev->mouse_y, (1<<ev->mouse_button));
        break;
    case SAPP_EVENTTYPE_MOUSE_MOVE:
        mu_input_mousemove(&_ui.ctx, (int)ev->mouse_x, (int)ev->mouse_y);
        break;
    case SAPP_EVENTTYPE_MOUSE_SCROLL:
        mu_input_scroll(&_ui.ctx, 0, (int)ev->scroll_y);
        break;
    case SAPP_EVENTTYPE_KEY_DOWN:
        mu_input_keydown(&_ui.ctx, key_map[ev->key_code & 511]);
        break;
    case SAPP_EVENTTYPE_KEY_UP:
        mu_input_keyup(&_ui.ctx, key_map[ev->key_code & 511]);
        break;
    case SAPP_EVENTTYPE_CHAR:
        {
            char txt[2] = { (char)(ev->char_code & 255), 0 };
            mu_input_text(&_ui.ctx, txt);
        }
        break;
    default:
        break;
    }
    return _ui.ctx.focus;
}

#include QMK_KEYBOARD_H

enum layer_names {
    _BASE,
    _COLEMAK,
    _FKEYS,
    _NAVNUM
};

// Mission Control on encoder click
#define MC_CTL LCTL(KC_UP)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_BASE] = LAYOUT(
    //  Row 0: empty (42-key style)
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,               KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
    //  Row 1
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSPC,
    //  Row 2
        MT(MOD_LCTL, KC_ESC), KC_A, KC_S, KC_D, KC_F, KC_G,              KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,
    //  Row 3
        SC_LSPO, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,               KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, SC_RSPC,
    //  Row 4: [4,0]=enc click, [4,1]-[4,5]=thumb, [9,0]-[9,4]=thumb, [9,5]=enc click
        MC_CTL,  KC_MINS, KC_EQL,  KC_LALT, LT(_NAVNUM, KC_SPC), KC_LGUI, CW_TOGG, KC_ENT, LT(_FKEYS, KC_DEL), KC_LBRC, KC_RBRC, MC_CTL
    ),

    [_COLEMAK] = LAYOUT(
        _______, _______, _______, _______, _______, _______,             _______, _______, _______, _______, _______, _______,
        _______, _______, _______, KC_F,    KC_P,    KC_G,                KC_J,    KC_L,    KC_U,    KC_Y,    KC_SCLN, _______,
        _______, _______, KC_R,    KC_S,    KC_T,    KC_D,                _______, KC_N,    KC_E,    KC_I,    KC_O,    _______,
        _______, _______, _______, _______, _______, _______,             KC_K,    _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______,             _______, _______, _______, _______, _______, _______
    ),

    [_FKEYS] = LAYOUT(
        _______, _______, _______, _______, _______, _______,             _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______,             _______, KC_F7,   KC_F8,   KC_F9,   _______, _______,
        _______, _______, _______, _______, _______, _______,             _______, KC_F4,   KC_F5,   KC_F6,   _______, _______,
        _______, _______, _______, _______, _______, _______,             _______, KC_F1,   KC_F2,   KC_F3,   _______, _______,
        _______, _______, _______, _______, _______, _______,             _______, _______, _______, KC_F10,  KC_F11,  _______
    ),

    [_NAVNUM] = LAYOUT(
        _______, _______, _______, _______, _______, _______,             _______, _______, _______, _______, _______, _______,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSLS,
        _______, _______, _______, _______, _______, _______,             KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, _______, _______,
        _______, _______, _______, _______, _______, _______,             KC_HOME, KC_PGDN, KC_PGUP, KC_END,  _______, _______,
        _______, _______, _______, _______, _______, TG(_COLEMAK),        _______, _______, _______, _______, _______, _______
    )
};

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_BASE]    = { ENCODER_CCW_CW(LCTL(KC_RGHT), LCTL(KC_LEFT)), ENCODER_CCW_CW(LCTL(KC_RGHT), LCTL(KC_LEFT)) },
    [_COLEMAK] = { ENCODER_CCW_CW(LCTL(KC_RGHT), LCTL(KC_LEFT)), ENCODER_CCW_CW(LCTL(KC_RGHT), LCTL(KC_LEFT)) },
    [_FKEYS]   = { ENCODER_CCW_CW(LCTL(KC_RGHT), LCTL(KC_LEFT)), ENCODER_CCW_CW(LCTL(KC_RGHT), LCTL(KC_LEFT)) },
    [_NAVNUM]  = { ENCODER_CCW_CW(LCTL(KC_RGHT), LCTL(KC_LEFT)), ENCODER_CCW_CW(LCTL(KC_RGHT), LCTL(KC_LEFT)) }
};
#endif

// Force native multitouch digitizer mode (macOS doesn't send PTP feature report)
extern bool digitizer_send_mouse_reports;
void keyboard_post_init_user(void) {
    digitizer_send_mouse_reports = false;
}

// 4-key reset combo: Q + Z + P + / held for 1 second -> QK_BOOT
// Q=[1,1], Z=[3,1], P=[6,4], /=[8,4]
static uint32_t reset_timer = 0;

void matrix_scan_user(void) {
    if (matrix_is_on(1, 1) && matrix_is_on(3, 1) && matrix_is_on(6, 4) && matrix_is_on(8, 4)) {
        if (reset_timer == 0) {
            reset_timer = timer_read32();
        } else if (timer_elapsed32(reset_timer) > 1000) {
            reset_keyboard();
        }
    } else {
        reset_timer = 0;
    }
}

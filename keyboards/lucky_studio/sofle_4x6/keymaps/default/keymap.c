#include QMK_KEYBOARD_H

enum layer_names {
    _BASE,
    _COLEMAK,
    _FKEYS,
    _NAVNUM
};

enum custom_keycodes {
    SPEED_UP = SAFE_RANGE,
    SPEED_DN
};

// Mission Control on encoder click
#define MC_CTL LCTL(KC_UP)

// Touchpad speed levels
static const uint8_t speed_levels[] = {2, 4, 6, 8, 12};
#define SPEED_LEVEL_COUNT (sizeof(speed_levels) / sizeof(speed_levels[0]))
static uint8_t speed_index = 1; // default index 1 = divisor 4

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_BASE] = LAYOUT(
    //  Row 0: number row
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,
    //  Row 1
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSLS,
    //  Row 2
        MT(MOD_LCTL, KC_ESC), KC_A, KC_S, KC_D, KC_F, KC_G,              KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,
    //  Row 3
        SC_LSPO, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,               KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, SC_RSPC,
    //  Row 4: [4,0]=enc click, [4,1]-[4,5]=thumb, [9,0]-[9,4]=thumb, [9,5]=no encoder
        MC_CTL,  KC_MINS, KC_EQL,  KC_LALT, LT(_NAVNUM, KC_SPC), KC_LGUI, CW_TOGG, KC_ENT, LT(_FKEYS, KC_DEL), KC_LBRC, KC_RBRC, KC_NO
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
        _______, _______, _______, _______, _______, _______,             _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______,             KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, _______, _______,
        _______, _______, _______, _______, _______, _______,             KC_HOME, KC_PGDN, KC_PGUP, KC_END,  _______, _______,
        _______, _______, _______, _______, _______, TG(_COLEMAK),        _______, _______, _______, _______, _______, _______
    )
};

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [_BASE]    = { ENCODER_CCW_CW(LCTL(KC_RGHT), LCTL(KC_LEFT)) },
    [_COLEMAK] = { ENCODER_CCW_CW(LCTL(KC_RGHT), LCTL(KC_LEFT)) },
    [_FKEYS]   = { ENCODER_CCW_CW(LCTL(KC_RGHT), LCTL(KC_LEFT)) },
    [_NAVNUM]  = { ENCODER_CCW_CW(SPEED_DN, SPEED_UP) }
};
#endif

// Touchpad gestures:
// 1-finger tap = left click, 1-finger tap+hold = left drag
// 2-finger tap = right click, 2-finger tap+hold = right drag
// 3-finger tap = middle click, 3-finger tap+hold = middle drag

// Touchpad speed cycling
extern uint8_t digitizer_mouse_speed_divisor;

// Momentum debug exports
extern int32_t  momentum_vel_h, momentum_vel_v;
extern int32_t  momentum_accum_h, momentum_accum_v;
extern bool     momentum_active;
extern int32_t  dbg_scroll_vel_h, dbg_scroll_vel_v;
extern int      dbg_state, dbg_contacts, dbg_tap_contacts;

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        switch (keycode) {
            case SPEED_UP:
                if (speed_index > 0) speed_index--;
                digitizer_mouse_speed_divisor = speed_levels[speed_index];
                return false;
            case SPEED_DN:
                if (speed_index < SPEED_LEVEL_COUNT - 1) speed_index++;
                digitizer_mouse_speed_divisor = speed_levels[speed_index];
                return false;
        }
    }
    return true;
}

// Debug: VIA custom value handler for momentum state
// data = [command_id, channel_id, value_id, ...]
void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    if (data[1] == 0xFE) {
        // Fill starting at data[2]
        data[2] = (uint8_t)dbg_state;
        data[3] = (uint8_t)dbg_contacts;
        data[4] = (uint8_t)dbg_tap_contacts;
        data[5] = dbg_scroll_vel_h & 0xFF;
        data[6] = (dbg_scroll_vel_h >> 8) & 0xFF;
        data[7] = dbg_scroll_vel_v & 0xFF;
        data[8] = (dbg_scroll_vel_v >> 8) & 0xFF;
        data[9] = momentum_vel_h & 0xFF;
        data[10] = (momentum_vel_h >> 8) & 0xFF;
        data[11] = momentum_vel_v & 0xFF;
        data[12] = (momentum_vel_v >> 8) & 0xFF;
        data[13] = momentum_active ? 1 : 0;
    }
}

// Touchpad: mouse fallback mode (macOS doesn't support Windows PTP protocol)

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

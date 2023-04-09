// Copyright 2021 Andrzej Kotulski (@akotulski)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H


// Trackball stuff.
#define TRACK _______
#ifdef USE_CUSTOM_TRACK_DRIVER
#include "pointing_device.h"
#include "../../pmw3360/pmw3360.h"

uint8_t track_mode = 0; // 0 Mousecursor; 1 arrowkeys/carret; 2 scrollwheel; 3 sound/brightness

enum TrackModes{
    kTrackBallCursorMode,
    kTrackBallCaretMode,
    kTrackBallScrollMode,
    kTrackBallSoundBrightnessMode,
};

uint8_t prev_track_mode = 0;
bool integration_mode = false;
int16_t cum_x = 0;
int16_t cum_y = 0;
int16_t sensor_x = 0;
int16_t sensor_y = 0;

// Thresholds help to move only horizontal or vertical. When accumulated distance reaches threshold, only move one discrete value in direction with bigger delta.
uint8_t	carret_threshold = 24;		 // higher means slower
uint16_t carret_threshold_inte = 340; // in integration mode higher threshold

#define regular_smoothscroll_factor 8
bool smooth_scroll = true;
uint8_t	scroll_threshold = 8 / regular_smoothscroll_factor;	// divide if started smooth
uint16_t scroll_threshold_inte = 1200 / regular_smoothscroll_factor;

uint16_t cursor_multiplier = 250;	// adjust cursor speed
uint16_t cursor_multiplier_inte = 20;
#define CPI_STEP 20

int16_t cur_factor;

enum TapDanceNames {
    kTapDanceTrackModeSwitch,
};

void TrackModeSwitchFinished (qk_tap_dance_state_t *state, void *user_data);
void TrackModeSwitchReset (qk_tap_dance_state_t *state, void *user_data);

#undef TRACK
#define TRACK TD(kTapDanceTrackModeSwitch)

#endif  // USE_CUSTOM_TRACK_DRIVER

// Layer names definition.
enum LayerNames {
    _BASE,
    _SYM,
    _FUNC,
    _NUMPAD,
    _MOUSE,
    _NAV,
};

enum Combos {
  _NAV_COMBO,
  _SYM_COMBO,
  _MOUSE_COMBO,
  _NUMPAD_COMBO,
  _LEFT_RESET_COMBO,
  _RIGHT_RESET_COMBO,
};

const uint16_t PROGMEM nav_combo[] = {KC_D, KC_F, COMBO_END};
const uint16_t PROGMEM sym_combo[] = {KC_J, KC_K, COMBO_END};
const uint16_t PROGMEM mouse_combo[] = {KC_K, KC_L, COMBO_END};
const uint16_t PROGMEM numpad_combo[] = {KC_S, KC_D, COMBO_END};
const uint16_t PROGMEM left_reset_combo[] = {KC_A, KC_S, KC_D, KC_F, COMBO_END};
const uint16_t PROGMEM right_reset_combo[] = {KC_J, KC_K, KC_L, KC_SCLN, COMBO_END};

combo_t key_combos[COMBO_COUNT] = {
  [_NAV_COMBO] = COMBO(nav_combo, MO(_NAV)),
  [_SYM_COMBO] = COMBO(sym_combo, MO(_SYM)),
  [_MOUSE_COMBO] = COMBO(mouse_combo, MO(_MOUSE)),
  [_NUMPAD_COMBO] = COMBO(numpad_combo, MO(_NUMPAD)),
  [_LEFT_RESET_COMBO] = COMBO(left_reset_combo, QK_BOOT),
  [_RIGHT_RESET_COMBO] = COMBO(right_reset_combo, QK_BOOT),
};


#define CTRLESC MT(MOD_LCTL, KC_ESCAPE)
#define MS_L KC_MS_BTN1
#define MS_R KC_MS_BTN2

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Base (qwerty)
     * +-----------------------------------------+                             +-----------------------------------------+
     * | TAB  |   q  |   w  |   e  |   r  |   t  |                             |   y  |   u  |   i  |   o  |   p  |      |
     * |------+------+------+------+------+------|                             |------+------+------+------+------+------|
     * | ESC  |   a  |   s  |   d  |   f  |   g  |                             |   h  |   j  |   k  |   l  |   ;  |   '  |
     * |------+------+------+------+------+------|                             |------+------+------+------+------+------|
     * | SHFT |   z  |   x  |   c  |   v  |   b  |                             |   n  |   m  |   ,  |   .  |   /  | SHFT |
     * +------+------+------+------+-------------+                             +-------------+------+------+------+------+
     *               |   -  |  =   | SPC  | CAPSW|                                           |   [  |   ]  |
     *               +-------------+-------------+                                           +-------------+
     *                                    +-------------+               +---------------------------+
     *                                    |  ALT | SYM  |               | LCLK | RCLK | ENTR | BSPC |
     *                                    |------+------|               |------+------+------+------|
     *                                    | WIN  |NUMPAD|
     *                                    +-------------+
     *
     *
     */
    [_BASE] = LAYOUT(
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,               KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSPC,
        CTRLESC, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,               KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,
        KC_LSPO, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,               KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSPC,
                          KC_MINS, KC_EQL,  KC_SPC,  MO(_FUNC),                               KC_LBRC, KC_RBRC,
                                            KC_LALT, KC_LGUI,            TRACK,   CW_TOGG, KC_DEL,    KC_ENT,
                                            _______, _______
    ),
    [_SYM] = LAYOUT(
        KC_GRV,  _______, KC_AMPR, KC_ASTR, _______, KC_BSLS,            _______, _______, _______, _______, _______, _______,
        _______, _______, KC_DLR,  KC_PERC, KC_CIRC, _______,            _______, _______, _______, _______, _______, _______,
        _______, _______, KC_EXLM, KC_AT,   KC_HASH, _______,            _______, _______, _______, _______, _______, _______,
                          _______, _______, _______, _______,                              _______, _______,
                                            _______, _______,            _______, _______, _______, _______,
                                            _______, _______
    ),
    [_NAV] = LAYOUT(
        _______, _______, _______, _______, _______, _______,            _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______,            KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, _______, _______,
        _______, _______, _______, _______, _______, _______,            KC_HOME, KC_PGDN, KC_PGUP, KC_END,  _______, _______,
                          _______, _______, _______, _______,                              _______, _______,
                                            _______, _______,            _______, _______, _______, _______,
                                            _______, _______
    ),
    [_NUMPAD] = LAYOUT(
        _______, _______, _______, _______, _______, _______,            _______, KC_7,    KC_8,    KC_9,    KC_PLUS, _______,
        _______, _______, _______, _______, _______, _______,            _______, KC_4,    KC_5,    KC_6,    KC_MINS,  KC_ASTR,
        _______, _______, _______, _______, _______, _______,            _______, KC_1,    KC_2,    KC_3,    KC_SLSH, _______,
                          _______, _______, _______, _______,                              KC_0,    KC_DOT,
                                            _______, _______,            _______, _______, _______, _______,
                                            _______, _______
    ),
    [_FUNC] = LAYOUT(
        _______, _______, _______, _______, _______, _______,            _______, KC_F7,   KC_F8,   KC_F9,    _______, _______,
        _______, _______, _______, _______, _______, _______,            _______, KC_F4,   KC_F5,   KC_F6,    _______, _______,
        _______, _______, _______, _______, _______, _______,            _______, KC_F1,   KC_F2,   KC_F3,    KC_F12,  _______,
                          _______, _______, _______, _______,                              KC_F10,  KC_F11,
                                            _______, _______,            _______, _______, _______, _______,
                                            _______, _______
    ),
    [_MOUSE] = LAYOUT(
        _______, _______, _______, KC_WH_U, _______, _______,            _______, _______, _______, _______, _______, _______,
        _______, _______, KC_WH_L, KC_WH_D, KC_WH_R, _______,            _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______,            _______, _______, _______, _______, _______, _______,
                          _______, _______, MS_L,    MS_R,                                 _______, _______,
                                            _______, _______,            _______, _______, _______, _______,
                                            _______, _______
    )
};

/***************************
 * Trackball handling
 **************************/

#ifdef USE_CUSTOM_TRACK_DRIVER

void pointing_device_init(void){
	if(is_keyboard_left()) {
		return;
    }
	pmw_init();
}

int max(int num1, int num2) { return (num1 > num2 ) ? num1 : num2; }
int min(int num1, int num2) { return (num1 > num2 ) ? num2 : num1; }

int8_t sign(int x) { return (x > 0) - (x < 0); }
int8_t CLAMP_HID(int value) { return value < -127 ? -127 : value > 127 ? 127 : value; }

void tap_code_fast(uint8_t code) {
	register_code(code);
	// Dont do this:
	// if (code == KC_CAPS) {
	//	 wait_ms(TAP_HOLD_CAPS_DELAY);
	// } else {
	//	 wait_ms(TAP_CODE_DELAY);
	// }
	unregister_code(code);
}

void tap_tb(uint8_t keycode0, uint8_t keycode1, uint8_t keycode2, uint8_t keycode3) {
	if(abs(cum_x) + abs(cum_y) >= cur_factor){
		if(abs(cum_x) > abs(cum_y)) {
			if(cum_x > 0) {
				for (int8_t i = 0; i <= (abs(cum_x) + abs(cum_y)) / cur_factor; i++) {
					tap_code_fast(keycode0);
					cum_x = max(cum_x - cur_factor, 0);
				}
				cum_y = 0;
			} else {
				for (int8_t i = 0; i <= (abs(cum_x) + abs(cum_y)) / cur_factor; i++) {
					tap_code_fast(keycode1);
					cum_x = min(cum_x + cur_factor, 0);
				}
				cum_y = 0;
			}
		} else {
			if(cum_y > 0) {
				for (int8_t i = 0; i <= (abs(cum_x) + abs(cum_y)) / cur_factor; i++) {
					tap_code_fast(keycode2);
					cum_y = max(cum_y - cur_factor, 0);
					}
				cum_x = 0;
			} else {
				for (int8_t i = 0; i <= (abs(cum_x) + abs(cum_y)) / cur_factor; i++) {
					tap_code_fast(keycode3);
					cum_y = min(cum_y + cur_factor, 0);
				}
				cum_x = 0;
			}
		}
	}
}

void handle_pointing_device_modes(void){
	report_mouse_t mouse_report = pointing_device_get_report();

	if (track_mode == kTrackBallCursorMode) {
		if (integration_mode)
			cur_factor = cursor_multiplier_inte;
		else
			cur_factor = cursor_multiplier;
		mouse_report.x = CLAMP_HID( sensor_x * cur_factor / 100);
		mouse_report.y = CLAMP_HID(-sensor_y * cur_factor / 100);
	} else {
		// accumulate movement until threshold reached
		cum_x += sensor_x;
		cum_y += sensor_y;
		if (track_mode == kTrackBallCaretMode) {
			if (integration_mode)
				cur_factor = carret_threshold_inte;
			else
				cur_factor = carret_threshold;
			tap_tb(KC_RIGHT, KC_LEFT, KC_UP, KC_DOWN);

		} else if(track_mode == kTrackBallScrollMode) {
				if (integration_mode)
					cur_factor = scroll_threshold_inte;
				else
					cur_factor = scroll_threshold;
				if(abs(cum_x) + abs(cum_y) >= cur_factor) {
					if(abs(cum_x) > abs(cum_y)) {
						mouse_report.h = sign(cum_x) * (abs(cum_x) + abs(cum_y)) / cur_factor;
					} else {
						mouse_report.v = sign(cum_y) * (abs(cum_x) + abs(cum_y)) / cur_factor;
					}
					cum_x = 0;
					cum_y = 0;
				}
		} else { // sound vol/brightness (3)
			cur_factor = carret_threshold;
			tap_tb(KC_BRIGHTNESS_UP, KC_BRIGHTNESS_DOWN, KC_AUDIO_VOL_UP, KC_AUDIO_VOL_DOWN);
		}
	}
	pointing_device_set_report(mouse_report);
	pointing_device_send();
}

void get_sensor_data(void) {
	if(!is_keyboard_master())
		return;
	report_pmw_t pmw_report = pmw_get_report();

	if (integration_mode) {
		sensor_x += pmw_report.x;
		sensor_y += pmw_report.y;
	} else {
		sensor_x = pmw_report.x;
		sensor_y = pmw_report.y;
	}
}

void pointing_device_task(void) {
#ifndef POLLING
	if ( is_keyboard_master() && integration_mode )
		handle_pointing_device_modes();
#else
	get_sensor_data();
	handle_pointing_device_modes();
#endif
}

#ifndef POLLING
	ISR(INT2_vect) {
		get_sensor_data();
		handle_pointing_device_modes();
	}
#endif

// Tap dance definitions.

void TrackModeSwitchFinished (qk_tap_dance_state_t *state, void *user_data) {
	switch (state->count) {
	case 2:
		track_mode = kTrackBallCaretMode;
        break;
    case 3:
		track_mode = kTrackBallScrollMode;
        break;
    case 4:
		track_mode = kTrackBallSoundBrightnessMode;
        break;
	case 1:
    default:
		track_mode = kTrackBallCursorMode;
    }
}

qk_tap_dance_action_t tap_dance_actions[] = {
    [kTapDanceTrackModeSwitch] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, TrackModeSwitchFinished, NULL),
};

void keyboard_post_init_user(void) {
  // Customise these values to desired behaviour
  debug_enable=true;
  debug_matrix=true;
  //debug_keyboard=true;
  //debug_mouse=true;
}
#endif  // USE_CUSTOM_TRACK_DRIVER

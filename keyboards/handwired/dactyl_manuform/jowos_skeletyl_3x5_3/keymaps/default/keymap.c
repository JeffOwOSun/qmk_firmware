// Copyright 2022 QMK
// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H

enum layers {
  LAYER_BASE,
  LAYER_NUM,
  LAYER_SYM,
  LAYER_NAV,
  LAYER_MSE,
  LAYER_SYS,
  LAYER_FUN,
};

enum custom_keycodes {
  LGUI_A = MT(MOD_LGUI, KC_A),
  LALT_S = MT(MOD_LALT, KC_S),
  LCTL_D = MT(MOD_LCTL, KC_D),
  LSFT_F = MT(MOD_LSFT, KC_F),
  RGUI_SC = MT(MOD_RGUI, KC_SCLN),
  RALT_L = MT(MOD_RALT, KC_L),
  RCTL_K = MT(MOD_RCTL, KC_K),
  RSFT_J = MT(MOD_RSFT, KC_J),
  ESC_SYS = LT(LAYER_SYS, KC_ESC),
  SPC_NAV = LT(LAYER_NAV, KC_SPC),
  TAB_MSE = LT(LAYER_MSE, KC_TAB),
  ENT_SYM = LT(LAYER_SYM, KC_ENT),
  BSPC_NUM = LT(LAYER_NUM, KC_BSPC),
  DEL_FUN = LT(LAYER_FUN, KC_DEL),
  KC_PC_UNDO = LCTL(KC_Z),
  KC_PC_REDO = LCTL(KC_Y),
  KC_PC_CUT = LCTL(KC_X),
  KC_PC_COPY = LCTL(KC_C),
  KC_PC_PASTE = LCTL(KC_V),
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [LAYER_BASE] = LAYOUT_split_3x5_3(
        KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                               KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,
        LGUI_A,  LALT_S,  LCTL_D,  LSFT_F,  KC_G,                               KC_H,    RSFT_J,  RCTL_K,  RALT_L,  RGUI_SC,
        KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,                               KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,
                                   ESC_SYS, SPC_NAV, TAB_MSE,         ENT_SYM,  BSPC_NUM,  DEL_FUN
    ),
    [LAYER_NUM] = LAYOUT_split_3x5_3(
    KC_LBRC,    KC_7,     KC_8,      KC_9,      KC_RBRC,                        _______, _______, _______, _______, _______,
    KC_QUOT,    KC_4,     KC_5,      KC_6,      KC_EQL,                         _______, KC_RSFT, KC_RCTL, KC_RALT, KC_RGUI, 
    KC_GRV,     KC_1,     KC_2,      KC_3,      KC_BSLS,                        _______, _______, _______, _______, _______,
                                     KC_DOT,    KC_0,  KC_MINS,        _______, _______, _______
    ),
    [LAYER_SYM] = LAYOUT_split_3x5_3(
    KC_LCBR,        KC_AMPR,        KC_ASTR,        KC_LPRN,        KC_RCBR,                        _______, _______, _______, _______, _______,
    KC_DQUO,        KC_DLR,         KC_PERC,        KC_CIRC,        KC_PLUS,                        _______, KC_RSFT, KC_RCTL, KC_RALT, KC_RGUI, 
    KC_TILD,        KC_EXLM,        KC_AT,          KC_HASH,        KC_PIPE,                        _______, _______, _______, _______, _______,
                                    KC_LPRN,    KC_RPRN,  KC_UNDS,        _______, _______, _______
    ),
    [LAYER_NAV] = LAYOUT_split_3x5_3(
    _______, _______, _______, _______, _______,                                 KC_PC_REDO,  KC_PC_PASTE,  KC_PC_COPY,  KC_PC_CUT,   KC_PC_UNDO,
    KC_LGUI, KC_LALT, KC_LCTL, KC_LSFT, _______,                                 KC_LEFT,        KC_DOWN,        KC_UP,          KC_RIGHT,       CAPS_WORD,
    _______, _______, _______, _______, _______,                                 KC_HOME,        KC_PGDN,      KC_PGUP,        KC_END,         KC_INSERT,
                               _______, _______, _______,                KC_ENT, KC_BSPC, KC_DEL
    ),
    [LAYER_MSE] = LAYOUT_split_3x5_3(
    _______, _______, _______, _______, _______,                                 KC_PC_REDO,  KC_PC_PASTE,  KC_PC_COPY,  KC_PC_CUT,   KC_PC_UNDO,
    KC_LGUI, KC_LALT, KC_LCTL, KC_LSFT, _______,                                 KC_MS_LEFT,  KC_MS_DOWN,   KC_MS_UP,    KC_MS_RIGHT, _______, 
    _______, _______, _______, _______, _______,                                 KC_MS_WH_LEFT,  KC_MS_WH_DOWN,  KC_MS_WH_UP,    KC_MS_WH_RIGHT, _______, 
                               _______, _______, _______,                KC_MS_BTN2, KC_MS_BTN1, KC_MS_BTN3
    ),
    [LAYER_SYS] = LAYOUT_split_3x5_3(
    _______, _______, _______, _______, _______,                                  KC_NO,        KC_NO,        KC_NO,        KC_NO,        KC_NO,
    KC_LGUI, KC_LALT, KC_LCTL, KC_LSFT, _______,                                  KC_MEDIA_PREV_TRACK,KC_AUDIO_VOL_DOWN,KC_AUDIO_VOL_UP,KC_MEDIA_NEXT_TRACK,KC_NO,
    _______, _______, _______, _______, _______,                                  KC_NO,        KC_NO,        KC_NO,        KC_NO,        KC_NO,
                               _______, _______, _______,                KC_MEDIA_STOP,  KC_MEDIA_PLAY_PAUSE, KC_AUDIO_MUTE
    ),
    [LAYER_FUN] = LAYOUT_split_3x5_3(
    KC_F12,  KC_F7,  KC_F8,  KC_F9,  KC_PSCR,                                  KC_NO,          KC_NO,          KC_NO,          KC_NO,          KC_NO, 
    KC_F11,  KC_F4,  KC_F5,  KC_F6,  KC_LSCR,                               KC_NO,          KC_RSFT,      KC_RCTL,       KC_RALT,        KC_RGUI,
    KC_F10,  KC_F1,  KC_F2,  KC_F3,  KC_PAUSE,                                    KC_NO,          KC_NO,          KC_NO,          KC_NO,          KC_NO,
                             KC_APP, KC_SPC, KC_TAB,                     _______, _______, _______
    ),
};

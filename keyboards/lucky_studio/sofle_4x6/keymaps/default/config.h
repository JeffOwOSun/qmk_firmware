#pragma once
#define TAPPING_TERM 200

// Touchpad sensitivity (default 400, range 50-1200)
#define POINTING_DEVICE_DEFAULT_CPI 200

// 3-finger swipe keycodes for macOS
#define DIGITIZER_SWIPE_UP_KC    LCTL(KC_UP)     // Mission Control
#define DIGITIZER_SWIPE_DOWN_KC  LCTL(KC_DOWN)   // App Exposé
#define DIGITIZER_SWIPE_LEFT_KC  LCTL(KC_LEFT)   // Left workspace
#define DIGITIZER_SWIPE_RIGHT_KC LCTL(KC_RIGHT)  // Right workspace

// Enable tap-to-click
#define DIGITIZER_REPORT_TAPS_AS_CLICKS

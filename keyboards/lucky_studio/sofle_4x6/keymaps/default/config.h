#pragma once
#define TAPPING_TERM 200

// Scroll speed (higher = slower, default 10)
#define DIGITIZER_SCROLL_DIVISOR 30

// 3-finger swipe keycodes for macOS
#define DIGITIZER_SWIPE_UP_KC    LCTL(KC_UP)     // Mission Control
#define DIGITIZER_SWIPE_DOWN_KC  LCTL(KC_DOWN)   // App Exposé
#define DIGITIZER_SWIPE_LEFT_KC  LCTL(KC_RIGHT)  // Right workspace (swipe left = go right)
#define DIGITIZER_SWIPE_RIGHT_KC LCTL(KC_LEFT)   // Left workspace (swipe right = go left)

// Natural scrolling (swipe down = scroll up)
#define DIGITIZER_SCROLL_INVERT

// Enable tap-to-click
#define DIGITIZER_REPORT_TAPS_AS_CLICKS

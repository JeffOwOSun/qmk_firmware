#pragma once
#define TAPPING_TERM 200

// Scroll speed (higher = slower, default 10)
// Higher divisor = smaller scroll increments = smoother feel on macOS
#define DIGITIZER_SCROLL_DIVISOR 120

// 3-finger swipe keycodes for macOS
#define DIGITIZER_SWIPE_UP_KC    LCTL(KC_UP)     // Mission Control
#define DIGITIZER_SWIPE_DOWN_KC  LCTL(KC_DOWN)   // App Exposé
#define DIGITIZER_SWIPE_LEFT_KC  LCTL(KC_RIGHT)  // Right workspace (swipe left = go right)
#define DIGITIZER_SWIPE_RIGHT_KC LCTL(KC_LEFT)   // Left workspace (swipe right = go left)

// Natural scrolling (swipe down = scroll up)
#define DIGITIZER_SCROLL_INVERT

// Tap-to-click tuning
#define DIGITIZER_MOUSE_TAP_DISTANCE 150           // ~1.5mm movement tolerance (default 25 = 0.25mm)
#define DIGITIZER_MOUSE_TAP_DETECTION_TIMEOUT 200   // 200ms window for double-tap
#define DIGITIZER_MOUSE_TAP_DURATION 50             // 50ms button hold per tap (default 1ms)
// DIGITIZER_REPORT_TAPS_AS_CLICKS disabled — clicks handled via mouse fallback only

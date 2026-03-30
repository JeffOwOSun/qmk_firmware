// Copyright 2025 George Norton (@george-norton)
// SPDX-License-Identifier: GPL-2.0-or-later

#if defined(POINTING_DEVICE_DRIVER_digitizer)

// We can fallback to reporting as a mouse for hosts which do not implement trackpad support.

#    include <stdlib.h>
#    include QMK_KEYBOARD_H
#    include "digitizer.h"

#    ifndef DIGITIZER_MOUSE_SPEED_DIVISOR
#        define DIGITIZER_MOUSE_SPEED_DIVISOR 4
#    endif
#    include "digitizer_mouse_fallback.h"
#    include "debug.h"
#    include "timer.h"
#    include "action.h"

#    ifndef DIGITIZER_MOUSE_TAP_DETECTION_TIMEOUT
#        define DIGITIZER_MOUSE_TAP_DETECTION_TIMEOUT 200
#    endif

#    ifndef DIGITIZER_MOUSE_TAP_DURATION
#        define DIGITIZER_MOUSE_TAP_DURATION 1
#    endif

#    ifndef DIGITIZER_MOUSE_TAP_DISTANCE
#        define DIGITIZER_MOUSE_TAP_DISTANCE 25
#    endif

#    ifndef DIGITIZER_SCROLL_DIVISOR
#        define DIGITIZER_SCROLL_DIVISOR 10
#    endif

#    ifndef DIGITIZER_MOUSE_SWIPE_TIMEOUT
#        define DIGITIZER_MOUSE_SWIPE_TIMEOUT 1000
#    endif

#    ifndef DIGITIZER_MOUSE_SWIPE_DISTANCE
#        define DIGITIZER_MOUSE_SWIPE_DISTANCE 500
#    endif

#    ifndef DIGITIZER_MOUSE_SWIPE_THRESHOLD
#        define DIGITIZER_MOUSE_SWIPE_THRESHOLD 300
#    endif

#    ifndef DIGITIZER_SWIPE_LEFT_KC
#        define DIGITIZER_SWIPE_LEFT_KC QK_MOUSE_BUTTON_3
#    endif

#    ifndef DIGITIZER_SWIPE_RIGHT_KC
#        define DIGITIZER_SWIPE_RIGHT_KC QK_MOUSE_BUTTON_4
#    endif

#    ifndef DIGITIZER_SWIPE_UP_KC
#        define DIGITIZER_SWIPE_UP_KC KC_LEFT_GUI
#    endif

#    ifndef DIGITIZER_SWIPE_DOWN_KC
#        define DIGITIZER_SWIPE_DOWN_KC KC_ESC
#    endif

#    ifndef DIGITIZER_MIN_CPI
#        define DIGITIZER_MIN_CPI 50
#    endif

#    ifndef DIGITIZER_MAX_CPI
#        define DIGITIZER_MAX_CPI 1200
#    endif

#    ifdef DIGITIZER_REPORT_TAPS_AS_CLICKS
bool digitizer_taps_as_clicks = true;
#    else
bool digitizer_taps_as_clicks = false;
#    endif

#define CLIP(X, A, B) (X<A ? A : X>B ? B : X)

// This variable indicates that we are sending mouse reports. It will be updated
// during USB enumeration if the host sends a feature report indicating it supports
// Microsofts Precision Trackpad protocol. This variable can also be modified by users
// to force reporting as a mouse or as a digitizer.
bool                  digitizer_send_mouse_reports = true;
uint8_t               digitizer_mouse_speed_divisor = DIGITIZER_MOUSE_SPEED_DIVISOR;
static report_mouse_t mouse_report                 = {};

// Momentum scrolling (Apple-style exponential decay with fractional accumulator)
int32_t  momentum_vel_h = 0;       // velocity in raw touchpad units/tick (Q8.8 fixed-point)
int32_t  momentum_vel_v = 0;
int32_t  momentum_accum_h = 0;     // fractional scroll accumulator
int32_t  momentum_accum_v = 0;
bool     momentum_active = false;
// Debug exports
int32_t  dbg_scroll_vel_h = 0, dbg_scroll_vel_v = 0;
int      dbg_state = 0, dbg_contacts = 0, dbg_tap_contacts = 0;

// Decay: 254/256 per tick at 300Hz ≈ Apple's 0.998/ms
// At 300Hz: 0.993^300 = 0.12 after 1 sec (velocity drops to 12%)
#ifndef DIGITIZER_MOMENTUM_DECAY_NUM
#    define DIGITIZER_MOMENTUM_DECAY_NUM 254
#endif
#define DIGITIZER_MOMENTUM_DECAY_SHIFT 8  // divide by 256
// Minimum velocity (Q8.8) before stopping
#ifndef DIGITIZER_MOMENTUM_MIN_VEL
#    define DIGITIZER_MOMENTUM_MIN_VEL 8   // very low threshold — momentum triggers easily
#endif

static report_mouse_t digitizer_get_mouse_report(report_mouse_t _mouse_report);
static uint16_t       digitizer_get_cpi(void);
static void           digitizer_set_cpi(uint16_t cpi);
static bool           digitizer_mouse_fallback_init(void);

const pointing_device_driver_t digitizer_pointing_device_driver = {.init = digitizer_mouse_fallback_init, .get_report = digitizer_get_mouse_report, .get_cpi = digitizer_get_cpi, .set_cpi = digitizer_set_cpi};

/**
 * @brief Initialize the pointing device driver. If this function does not return true, the pointing device
 * is not usable.
 *
 * @return report_mouse_t
 */
static bool digitizer_mouse_fallback_init(void)
{
    // TODO: Return false here, if we have a physical digitizer device and its initialization failed.
    return true;
}

/**
 * @brief Gets the current digitizer mouse report, the pointing device feature will send this is we
 * nave fallen back to mouse mode.
 *
 * @return report_mouse_t
 */
static report_mouse_t digitizer_get_mouse_report(report_mouse_t _mouse_report) {
    if (digitizer_send_mouse_reports) {
        report_mouse_t report = mouse_report;
        // Clear everything — buttons are set fresh each cycle by digitizer_update_mouse_report
        memset(&mouse_report, 0, sizeof(report_mouse_t));
        return report;
    }
    return _mouse_report;
}

static uint16_t mouse_cpi = 400;

/**
 * @brief Gets the CPI used by the digitizer mouse fallback feature.
 *
 * @return the current CPI value
 */
static uint16_t digitizer_get_cpi(void) {
    return mouse_cpi;
}

/**
 * @brief Sets the CPI used by the digitizer mouse fallback feature.
 *
 *  @param[in] the new CPI value
 */
static void digitizer_set_cpi(uint16_t cpi) {
    mouse_cpi = CLIP(cpi, DIGITIZER_MIN_CPI, DIGITIZER_MAX_CPI);
    digitizer_set_scale((mouse_cpi * 100) / DIGITIZER_MAX_CPI);
}

// The gesture detection state machine will transition between these states.
typedef enum { None, Down, MoveScroll, Tapped, DoubleTapped, Drag, Swipe, Finished } State;

static State state     = None;
static int   tap_count = 0;

/**
 * \brief Signals that a gesture is in progress so digitizer_update_mouse_report should be called,
 * even if no new digitizer data is available.
 * @return true if update_mouse_report should run.
 */
bool digitizer_update_gesture_state(void) {
    return tap_count || state != None || momentum_active;
}

/**
 * \brief Generate a mouse report from the digitizer report. This function implements
 * a state machine to detect gestures and handle them.
 * @param[in] report a new digitizer report
 */
void digitizer_update_mouse_report(report_digitizer_t *report) {
    static int      contact_start_time = 0;
    static int      contact_start_x    = 0;
    static int      contact_start_y    = 0;
    static int      tap_contacts       = 0;
    static int      last_contacts      = 0;
    static uint16_t last_x             = 0;
    static uint16_t last_y             = 0;
    static int32_t  scroll_vel_h       = 0;
    static int32_t  scroll_vel_v       = 0;
    const uint16_t  x                  = report->fingers[0].x;
    const uint16_t  y                  = report->fingers[0].y;
    const uint32_t  duration           = timer_elapsed32(contact_start_time);
    int             contacts           = 0;

    memset(&mouse_report, 0, sizeof(report_mouse_t));

    for (int i = 0; i < DIGITIZER_FINGER_COUNT; i++) {
        if (report->fingers[i].tip) {
            contacts++;
        }
    }

    // Kill momentum immediately when fingers touch
    if (contacts > 0 && momentum_active) {
        momentum_active = false;
        momentum_vel_h = 0;
        momentum_vel_v = 0;
        momentum_accum_h = 0;
        momentum_accum_v = 0;
    }

    switch (state) {
        case None: {
            if (contacts != 0) {
                state              = Down;
                contact_start_time = timer_read32();
                contact_start_x    = x;
                contact_start_y    = y;
                tap_contacts       = contacts;
            }
            break;
        }
        case Down: {
            const uint16_t distance_x = abs(contact_start_x - x);
            const uint16_t distance_y = abs(contact_start_y - y);
            tap_contacts              = MAX(contacts, tap_contacts);

            if (contacts == 0) {
                state              = Tapped;
                contact_start_time = timer_read32();
            } else if (contacts >= 3 && (distance_x > DIGITIZER_MOUSE_SWIPE_THRESHOLD || distance_y > DIGITIZER_MOUSE_SWIPE_THRESHOLD)) {
                state = Swipe;
            } else if (duration > DIGITIZER_MOUSE_TAP_DETECTION_TIMEOUT || distance_x > DIGITIZER_MOUSE_TAP_DISTANCE || distance_y > DIGITIZER_MOUSE_TAP_DISTANCE) {
                if (contacts >= 3) {
                    state = Swipe;
                } else {
                    state = MoveScroll;
                }
            }
            break;
        }
        case Drag:
        case MoveScroll: {
            if (contacts == 0) {
                state = None;
                // Start momentum from tracked scroll velocity (Q8.8 fixed-point)
                if (abs(scroll_vel_h) > DIGITIZER_MOMENTUM_MIN_VEL || abs(scroll_vel_v) > DIGITIZER_MOMENTUM_MIN_VEL) {
                    momentum_vel_h = scroll_vel_h;
                    momentum_vel_v = scroll_vel_v;
                    momentum_accum_h = 0;
                    momentum_accum_v = 0;
                    momentum_active = true;
                }
                scroll_vel_h = 0;
                scroll_vel_v = 0;
            } else if (contacts == 1 || (state == Drag && contacts <= 3)) {
                mouse_report.x = (x - last_x) / digitizer_mouse_speed_divisor;
                mouse_report.y = (y - last_y) / digitizer_mouse_speed_divisor;
                // Don't reset scroll velocity here — preserve it for momentum
                // when the last finger lifts. Only reset for Drag (not scroll).
                if (state == Drag) {
                    scroll_vel_h = 0;
                    scroll_vel_v = 0;
                }
            } else if (contacts == 3 && duration < DIGITIZER_MOUSE_SWIPE_TIMEOUT) {
                state = Swipe;
            } else {
                static int carry_h = 0;
                static int carry_v = 0;
                const int  raw_h   = x - last_x;
                const int  raw_v   = y - last_y;
                const int  h       = raw_h + carry_h;
                const int  v       = raw_v + carry_v;

                int scroll_h = h / DIGITIZER_SCROLL_DIVISOR;
                int scroll_v = v / DIGITIZER_SCROLL_DIVISOR;
                if (scroll_h > 1) scroll_h = 1;
                if (scroll_h < -1) scroll_h = -1;
                if (scroll_v > 1) scroll_v = 1;
                if (scroll_v < -1) scroll_v = -1;
                carry_h = h - scroll_h * DIGITIZER_SCROLL_DIVISOR;
                carry_v = v - scroll_v * DIGITIZER_SCROLL_DIVISOR;

                // EMA velocity in Q8.8: vel = (vel * 7 + raw * 256) / 8
                // Slower decay (87.5% old) so finger-lift zero frames don't kill it
                int32_t new_vel_h = (scroll_vel_h * 7 + raw_h * 256) / 8;
                int32_t new_vel_v = (scroll_vel_v * 7 + raw_v * 256) / 8;
                // Only update if moving (preserve velocity through zero-delta frames)
                if (raw_h != 0 || abs(new_vel_h) > abs(scroll_vel_h) / 2) scroll_vel_h = new_vel_h;
                if (raw_v != 0 || abs(new_vel_v) > abs(scroll_vel_v) / 2) scroll_vel_v = new_vel_v;

#ifdef DIGITIZER_SCROLL_INVERT
                mouse_report.h = scroll_h;
                mouse_report.v = -scroll_v;
#else
                mouse_report.h = scroll_h;
                mouse_report.v = scroll_v;
#endif
            }
            break;
        }
        case DoubleTapped:
        case Tapped: {
            tap_contacts = MAX(contacts, tap_contacts);
            if (contacts == 0 && last_contacts != contacts) {
                tap_count++;
                state              = DoubleTapped;
                contact_start_time = timer_read32();
            } else if (duration > DIGITIZER_MOUSE_TAP_DETECTION_TIMEOUT) {
                if (contacts > 0 && state == Tapped) {
                    state = Drag;
                } else {
                    tap_count++;
                    state = Finished;
                }
            }
            break;
        }
        case Swipe: {
            const int32_t distance_x = x - contact_start_x;
            const int32_t distance_y = y - contact_start_y;
            if (contacts == 0) {
                state = None;
            } else if (duration > DIGITIZER_MOUSE_SWIPE_TIMEOUT) {
                state = MoveScroll;
            } else if (digitizer_send_mouse_reports) {
                if (distance_x > DIGITIZER_MOUSE_SWIPE_DISTANCE && abs(distance_y) < DIGITIZER_MOUSE_SWIPE_THRESHOLD) {
                    // Swipe right
                    tap_code16(DIGITIZER_SWIPE_RIGHT_KC);
                    state = Finished;
                } else if (distance_x < -DIGITIZER_MOUSE_SWIPE_DISTANCE && abs(distance_y) < DIGITIZER_MOUSE_SWIPE_THRESHOLD) {
                    // Swipe left
                    tap_code16(DIGITIZER_SWIPE_LEFT_KC);
                    state = Finished;
                } else if (distance_y > DIGITIZER_MOUSE_SWIPE_DISTANCE && abs(distance_x) < DIGITIZER_MOUSE_SWIPE_THRESHOLD) {
                    // Swipe down
                    tap_code16(DIGITIZER_SWIPE_DOWN_KC);
                    state = Finished;
                } else if (distance_y < -DIGITIZER_MOUSE_SWIPE_DISTANCE && abs(distance_x) < DIGITIZER_MOUSE_SWIPE_THRESHOLD) {
                    // Swipe up
                    tap_code16(DIGITIZER_SWIPE_UP_KC);
                    state = Finished;
                }
            }
            break;
        }
        case Finished: {
            if (contacts == 0) {
                state = None;
            }
            break;
        }
    }
    // Simple tap handling: tap sets a flag that holds the button for one report cycle
    // then clears. Drag holds button while fingers are down.
    // Momentum scrolling: exponential decay with fractional accumulator
    if (momentum_active && contacts == 0) {
            // Accumulate velocity into fractional accumulator (Q8.8 -> raw units)
            momentum_accum_h += momentum_vel_h;
            momentum_accum_v += momentum_vel_v;

            // Convert accumulator to scroll events (divisor is in raw units, accum is Q8.8)
            int div_q8 = DIGITIZER_SCROLL_DIVISOR * 256;  // divisor in Q8.8
            int mh = momentum_accum_h / div_q8;
            int mv = momentum_accum_v / div_q8;
            if (mh > 1) mh = 1;
            if (mh < -1) mh = -1;
            if (mv > 1) mv = 1;
            if (mv < -1) mv = -1;
            momentum_accum_h -= mh * div_q8;
            momentum_accum_v -= mv * div_q8;

            if (mh != 0 || mv != 0) {
#ifdef DIGITIZER_SCROLL_INVERT
                mouse_report.h = mh;
                mouse_report.v = -mv;
#else
                mouse_report.h = mh;
                mouse_report.v = mv;
#endif
            }

            // Decay velocity: vel = vel * 254 / 256
            momentum_vel_h = (momentum_vel_h * DIGITIZER_MOMENTUM_DECAY_NUM) >> DIGITIZER_MOMENTUM_DECAY_SHIFT;
            momentum_vel_v = (momentum_vel_v * DIGITIZER_MOMENTUM_DECAY_NUM) >> DIGITIZER_MOMENTUM_DECAY_SHIFT;

            // Stop when velocity is negligible
            if (abs(momentum_vel_h) < DIGITIZER_MOMENTUM_MIN_VEL && abs(momentum_vel_v) < DIGITIZER_MOMENTUM_MIN_VEL) {
                momentum_active = false;
                momentum_vel_h = 0;
                momentum_vel_v = 0;
                momentum_accum_h = 0;
                momentum_accum_v = 0;
            }
    }

    static bool     tap_fire   = false;
    static uint32_t tap_fire_time = 0;

    if (tap_count > 0 && !tap_fire) {
        tap_fire = true;
        tap_fire_time = timer_read32();
        tap_count = 0;
    }
    if (tap_fire && timer_elapsed32(tap_fire_time) > DIGITIZER_MOUSE_TAP_DURATION) {
        tap_fire = false;
    }

    const bool button_pressed = tap_fire || (state == Drag);
    if (report->button1 || (tap_contacts == 1 && button_pressed)) {
        mouse_report.buttons |= 0x1;
    }
    if (report->button2 || (tap_contacts == 2 && button_pressed)) {
        mouse_report.buttons |= 0x2;
    }
    if (report->button3 || (tap_contacts == 3 && button_pressed)) {
        mouse_report.buttons |= 0x4;
    }
    last_contacts = contacts;
    last_x        = x;
    last_y        = y;
    // Debug exports
    dbg_state = state;
    dbg_contacts = contacts;
    dbg_tap_contacts = tap_contacts;
    dbg_scroll_vel_h = scroll_vel_h;
    dbg_scroll_vel_v = scroll_vel_v;
}
#endif

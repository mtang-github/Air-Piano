#ifndef TIMING_CONFIG_H
#define TIMING_CONFIG_H

#include "timing.h"

/*
 * Timing constants for the air piano.
 * All _MS values are in milliseconds, _US values in
 * microseconds, and _S values in seconds.
 */

/* Minimum note duration in nanoseconds */
#define MIN_NOTE_DURATION 50000000L

/* Maximum note duration in nanoseconds */
#define MAX_NOTE_DURATION (2L * BILLION)

/* 30 Hz period in nanoseconds for distance thread */
#define DISTANCE_PERIOD_NS 33333333L

/*
 * 10 µs trigger pulse in nanoseconds for triggering
 * ultrasonic distance sensor
 */
#define TRIG_PULSE_NS 10000L

/*
 * Echo timeout for ultrasonic distance sensor: 30 ms
 * in nanoseconds
 */
#define ECHO_TIMEOUT_NS 30000000L

/* 100 Hz period in nanoseconds for beam thread */
#define BEAM_PERIOD_NS 10000000L

/* 100 Hz frequency for beam thread */
#define BEAM_TICKS_PER_SEC 100u

/*
 * Print alignment warning after this many consecutive
 * broken beam ticks
 */
#define BEAM_ALIGNMENT_WARN_TICKS \
    (2u * BEAM_TICKS_PER_SEC)

/* 20 Hz period for the display thread */
#define DISPLAY_PERIOD_NS 50000000L

/* 10 Hz period for logging thread */
#define LOGGING_PERIOD_NS 100000000L

#endif /* TIMING_CONFIG_H */

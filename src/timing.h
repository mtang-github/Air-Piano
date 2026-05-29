#ifndef TIMING_H
#define TIMING_H

#include <time.h>
#include <stdint.h>
#include <limits.h>

#include "error_type.h"

/*
 * This file defines the basic timing-related
 * interface for the air piano.
 */

/* Integer type used for nanosecond durations. */
#define NS_TYPE int64_t

#define NS_MAX INT64_MAX
#define NS_MIN INT64_MIN

/* Type used for timepoints and durations. */
#define TIME_TYPE struct timespec

/* Initial value for TIME_TYPE. */
#define TIME_TYPE_INIT {0}

/* A constant for the number one billion */
#define BILLION ((int64_t)1000000000)

/*
 * Converts an ns duration to a TIME_TYPE and writes
 * it into the specified TIME_TYPE; returns SUCCESS
 * if successful, or an error code otherwise.
 */
ERR_TYPE ns_to_time(
    NS_TYPE duration_ns,
    TIME_TYPE *to_write
);

/*
 * Converts a TIME_TYPE to ns; writes non-SUCCESS
 * on failure.
 */
NS_TYPE time_to_ns(
    const TIME_TYPE * const time,
    ERR_TYPE *err_out
);

/*
 * Adds the specified duration in ns to the given
 * TIME_TYPE; returns error code on failure, SUCCESS
 * otherwise.
 */
ERR_TYPE time_add_ns(
    TIME_TYPE *to_add_to,
    NS_TYPE duration_ns
);

/*
 * Computes the difference between two TIME_TYPE
 * objects and outputs to a third; returns
 * non-SUCCESS on failure.
 */
ERR_TYPE time_diff(
    const TIME_TYPE * const left,
    const TIME_TYPE * const right,
    TIME_TYPE *out
);

/*
 * Writes the current timepoint into the specified
 * TIME_TYPE; returns SUCCESS if successful, or an error
 * code otherwise.
 */
ERR_TYPE get_current_time(TIME_TYPE *to_write);

/*
 * Sleeps for the specified amount of time in ns.
 * Defined in main.c.
 */
ERR_TYPE sleep_relative_ns(NS_TYPE sleep_time_ns);

/*
 * Sleeps until the specified time point. Defined
 * in main.c.
 */
ERR_TYPE sleep_absolute(
    const TIME_TYPE *sleep_until
);

#endif /* TIMING_H */

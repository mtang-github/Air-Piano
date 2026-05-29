#ifndef PRIORITY_H
#define PRIORITY_H

#include <stdint.h>

#include "error_type.h"

/*
 * Provides a utility function for setting the real-time
 * priority of the calling thread. Also contains
 * priority config for the air piano.
 */

#define BEAM_PRIO 90

#define DISTANCE_PRIO 70

#define DISPLAY_PRIO 60

#define CONSOLE_PRIO 40

#define LOGGING_PRIO 30

/*
 * Sets the real-time priority of the calling thread to
 * the given priority number; assumes POSIX priority
 * conventions. Returns ERR_BAD_ARG if an invalid
 * priority was given, SUCCESS otherwise.
 */
ERR_TYPE set_thread_priority(int32_t prio);

#endif
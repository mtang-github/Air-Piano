#ifndef THREAD_LAUNCHED_FLAGS_H
#define THREAD_LAUNCHED_FLAGS_H

#include <stdint.h>

/*
 * This file contains definitions for a space-efficient
 * flags type for thread launched-ness
 */

/*
 * The unsigned integer type used for storing thread
 * launched flags
 * 
 * layout: (msb) b d i _ _ _ l c (lsb)
 *     bit 7: beam thread
 *     bit 6: distance thread
 *     bit 5: display thread
 *     bit 4: unused
 *     bit 3: unused
 *     bit 2: unused
 *     bit 1: logging thread
 *     bit 0: console thread
 */
#define THREAD_LAUNCHED_FLAGS_TYPE uint8_t

/*
 * initial value for thread launched flags, means no
 * thread has been launched
 */
#define THREAD_LAUNCHED_FLAGS_INIT 0u

#define BEAM_THREAD_MASK (1u << 7)
#define DISTANCE_THREAD_MASK (1u << 6)
#define DISPLAY_THREAD_MASK (1u << 5)
#define LOGGING_THREAD_MASK (1u << 1)
#define CONSOLE_THREAD_MASK (1u << 0)

/*
 * Sets the beam thread as launched and returns the new
 * thread launched flags
 */
static inline
THREAD_LAUNCHED_FLAGS_TYPE set_beam_thread_launched(
    THREAD_LAUNCHED_FLAGS_TYPE flags
){
    return flags | BEAM_THREAD_MASK;
}

/*
 * Sets the distance thread as launched and returns the
 * new thread launched flags
 */
static inline
THREAD_LAUNCHED_FLAGS_TYPE set_distance_thread_launched(
    THREAD_LAUNCHED_FLAGS_TYPE flags
){
    return flags | DISTANCE_THREAD_MASK;
}

/*
 * Sets the display thread as launched and returns the
 * new thread launched flags
 */
static inline
THREAD_LAUNCHED_FLAGS_TYPE set_display_thread_launched(
    THREAD_LAUNCHED_FLAGS_TYPE flags
){
    return flags | DISPLAY_THREAD_MASK;
}

/*
 * Sets the logging thread as launched and returns the
 * new thread launched flags
 */
static inline
THREAD_LAUNCHED_FLAGS_TYPE set_logging_thread_launched(
    THREAD_LAUNCHED_FLAGS_TYPE flags
){
    return flags | LOGGING_THREAD_MASK;
}

/*
 * Sets the console thread as launched and returns the
 * new thread launched flags
 */
static inline
THREAD_LAUNCHED_FLAGS_TYPE set_console_thread_launched(
    THREAD_LAUNCHED_FLAGS_TYPE flags
){
    return flags | CONSOLE_THREAD_MASK;
}

/*
 * Returns non-zero if the beam thread was launched,
 * 0 otherwise
 */
static inline
THREAD_LAUNCHED_FLAGS_TYPE is_beam_thread_launched(
    THREAD_LAUNCHED_FLAGS_TYPE flags
){
    return flags & BEAM_THREAD_MASK;
}

/*
 * Returns non-zero if the distance thread was launched,
 * 0 otherwise
 */
static inline
THREAD_LAUNCHED_FLAGS_TYPE is_distance_thread_launched(
    THREAD_LAUNCHED_FLAGS_TYPE flags
){
    return flags & DISTANCE_THREAD_MASK;
}

/*
 * Returns non-zero if the display thread was launched,
 * 0 otherwise
 */
static inline
THREAD_LAUNCHED_FLAGS_TYPE is_display_thread_launched(
    THREAD_LAUNCHED_FLAGS_TYPE flags
){
    return flags & DISPLAY_THREAD_MASK;
}

/*
 * Returns non-zero if the logging thread was launched,
 * 0 otherwise
 */
static inline
THREAD_LAUNCHED_FLAGS_TYPE is_logging_thread_launched(
    THREAD_LAUNCHED_FLAGS_TYPE flags
){
    return flags & LOGGING_THREAD_MASK;
}

/*
 * Returns non-zero if the console thread was launched,
 * 0 otherwise
 */
static inline
THREAD_LAUNCHED_FLAGS_TYPE is_console_thread_launched(
    THREAD_LAUNCHED_FLAGS_TYPE flags
){
    return flags & CONSOLE_THREAD_MASK;
}

#endif /* THREAD_LAUNCHED_FLAGS_H */
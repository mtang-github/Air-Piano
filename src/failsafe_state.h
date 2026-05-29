#ifndef FAILSAFE_STATE_H
#define FAILSAFE_STATE_H

#include "timing.h"

/* This file defines the shared failsafe state */

/* The shared failsafe state. */
typedef struct {
    /*
     * 1 if we are currently in failsafe mode, 0
     * otherwise
     */
    uint8_t failsafe;
} Failsafe_State;

/*
 * Opaque pointer type for shared failsafe state that
 * includes concurrency
 */
typedef struct Failsafe_State_Shared
    *Failsafe_State_Shared_Handle;

/*
 * Initializes the shared failsafe state (mutex,
 * initial values). Outputs non-SUCCESS on error.
 * Should not be called twice concurrently.
 */
Failsafe_State_Shared_Handle failsafe_state_shared_init(
    ERR_TYPE * const err_out
);

/*
 * Sets the failsafe state to the given value. Error
 * if a value other than 0 or 1 is passed in. Returns
 * non-SUCCESS on error.
 */
ERR_TYPE failsafe_state_shared_set(
    Failsafe_State_Shared_Handle handle,
    uint8_t failsafe
);

/*
 * Outputs 1 if we are currently in failsafe, 0
 * otherwise. Returns non-SUCCESS on error (in case
 * of error, will output 1).
 */
ERR_TYPE failsafe_state_shared_get(
    Failsafe_State_Shared_Handle handle,
    uint8_t *failsafe_out
);

/*
 * Cleans up the specified shared failsafe state. Should
 * not call unless no thread could possibly be
 * holding the mutex, nor should it be called twice
 * concurrently. Returns non-SUCCESS on error.
 */
ERR_TYPE failsafe_state_shared_cleanup(
    Failsafe_State_Shared_Handle handle
);

#endif /* FAILSAFE_STATE_H */
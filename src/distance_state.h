#ifndef DISTANCE_STATE_H
#define DISTANCE_STATE_H

#include <stdint.h>

#include "notes.h"
#include "error_type.h"

/*
 * This file defines the shared state containing
 * distance (and thus note) information
 */

/*
 * Distance and note shared between the distance sensor
 * thread (writer) and the beam/display threads (readers).
 */
typedef struct {
    float distance_cm;

    /* the note index; INVALID_NOTE_INDEX if invalid */
    NOTE_INDEX_TYPE note_index;
} Distance_State;

/*
 * Opaque pointer type for shared distance state that
 * includes concurrency
 */
typedef struct Distance_State_Shared
    *Distance_State_Shared_Handle;

/*
 * Initializes the shared distance state (mutex,
 * initial values). Outputs non-SUCCESS on error.
 * Should not be called twice concurrently.
 */
Distance_State_Shared_Handle distance_state_shared_init(
    ERR_TYPE * const err_out
);

/*
 * Writes the distance state into the specified shared
 * distance state in a concurrent-safe manner.
 * Returns non-SUCCESS on error. 
 */
ERR_TYPE distance_state_shared_write(
    Distance_State_Shared_Handle handle,
    float distance_cm,
    NOTE_INDEX_TYPE note_index
);

/*
 * Copies the distance state from the specified shared
 * distance state in a concurrent-safe manner.
 * Returns non-SUCCESS on error.
 */
ERR_TYPE distance_state_shared_read(
    Distance_State_Shared_Handle handle,
    Distance_State * const out
);

/*
 * Cleans up the specified shared distance state. Should
 * not call unless no thread could possibly be
 * holding the mutex, nor should it be called twice
 * concurrently. Returns non-SUCCESS on error.
 */
ERR_TYPE distance_state_shared_cleanup(
    Distance_State_Shared_Handle handle
);

#endif /* DISTANCE_STATE_H */
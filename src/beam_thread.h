#ifndef BEAM_THREAD_H
#define BEAM_THREAD_H

#include "pin_io.h"
#include "distance_state.h"
#include "logging_state.h"
#include "failsafe_state.h"
#include "error_type.h"

/*
 * This file defines the interface for the beam thread,
 * which checks for beam breaks and plays notes.
 */

typedef struct {
    Pin_Io_Handle pin_io_handle;
    Distance_State_Shared_Handle
        distance_state_shared_handle;
    Failsafe_State_Shared_Handle
        failsafe_state_shared_handle;
    Logging_State_Shared_Handle
        logging_state_shared_handle;
    NS_TYPE note_duration;
    ERR_TYPE err_out;
} Beam_Thread_Args;

void *beam_thread_fn(void *arg);

#endif /* BEAM_THREAD_H */

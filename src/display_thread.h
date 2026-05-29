#ifndef DISPLAY_THREAD_H
#define DISPLAY_THREAD_H

#include "pin_io.h"
#include "distance_state.h"
#include "failsafe_state.h"
#include "logging_state.h"
#include "error_type.h"

/*
 * This file defines the interface for the display
 * thread, which displays note information in real-time
 * even without beam breaks.
 */

typedef struct {
    Pin_Io_Handle pin_io_handle;
    Distance_State_Shared_Handle
        distance_state_shared_handle;
    Failsafe_State_Shared_Handle
        failsafe_state_shared_handle;
    Logging_State_Shared_Handle
        logging_state_shared_handle;
    ERR_TYPE err_out;
} Display_Thread_Args;

void *display_thread_fn(void *arg);
#endif /* DISPLAY_THREAD_H */
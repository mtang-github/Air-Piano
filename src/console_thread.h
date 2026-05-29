#ifndef CONSOLE_THREAD_H
#define CONSOLE_THREAD_H

#include "logging_state.h"
#include "failsafe_state.h"

#ifdef EMU
#include "pin_io.h"
#include "distance_state.h"
#endif

/*
 * This file defines the interface for the console
 * thread, which is responsible for reading console
 * input into the system.
 */

/* Arguments passed to the console thread by pointer */
typedef struct {
    #ifdef EMU
    /* required for emulated version to toggle beam */
    Pin_Io_Handle pin_io_handle;
    /*
     * required for emulated version to change
     * distance
     */
    Distance_State_Shared_Handle
        distance_state_shared_handle;
    #endif

    Logging_State_Shared_Handle
        logging_state_shared_handle;
    Failsafe_State_Shared_Handle
        failsafe_state_shared_handle;
    ERR_TYPE err_out;
} Console_Thread_Args;

/*
 * Thread entry point: monitors console input for
 * the control thread to see.
 */
void *console_thread_fn(void *arg);

#endif /* CONSOLE_THREAD_H */
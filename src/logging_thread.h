#ifndef LOGGING_THREAD_H
#define LOGGING_THREAD_H

#include <stdio.h>

#include "logging_state.h"

/* Arguments passed to the logging thread by pointer */
typedef struct {
    Logging_State_Shared_Handle
        logging_state_shared_handle;
    FILE *log_file;
    ERR_TYPE err_out;
} Logging_Thread_Args;

/*
 * Thread entry point: outputs logs to stdout
 * concurrently to other threads
 */
void *logging_thread_fn(void *arg);

#endif /* LOGGING_THREAD_H */

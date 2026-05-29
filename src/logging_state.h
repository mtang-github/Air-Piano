#ifndef LOGGING_STATE_H
#define LOGGING_STATE_H

#include "error_type.h"
#include "timing.h"

/*
 * This file defines the interface to the logging
 * thread. Logging requests are provided as pointers
 * to null-terminated strings, which are stored in a
 * finite container until the logging thread runs.
 * Logs which overflow will be lost, but such events
 * will be recorded in the log.
 */

/* The max number of stdout messages stored */
#define MAX_MESSAGES 32

/* The max number of log entries stored */
#define MAX_LOG_ENTRIES 64

/*
 * Represents a log, which is not printed to console
 * but instead saved to file.
 */
typedef struct{
    const char *message;
    TIME_TYPE timestamp;
} Log_Entry;

/* Represents the state of the log queue */
typedef struct {
    /*
     * A list of messages to log; most recent near the
     * end of the array.
     */
    const char *messages[MAX_MESSAGES];
    /*
     * The current number of messages waiting to be
     * logged; a value of MAX_MESSAGES + 1
     * indicates overflow of unknown size.
     */
    uint8_t current_messages;

    /*
     * A list of logs to write to file; most recent
     * near the end of the array.
     */
    Log_Entry log_entries[MAX_LOG_ENTRIES];
    /*
     * The current number of log entries waiting to be
     * logged; a value of MAX_LOG_ENTRIES + 1
     * indicates overflow of unknown size.
     */
    uint8_t current_log_entries;
} Logging_State;

/* Opaque pointer type for shared logging state */
typedef struct Logging_State_Shared
    *Logging_State_Shared_Handle;

/*
 * Initializes the shared logging state. Outputs
 * non-SUCCESS on error. Should not be called twice
 * concurrently.
 */
Logging_State_Shared_Handle logging_state_shared_init(
    ERR_TYPE * const err_out
);

/*
 * Appends the given message to the logging state in
 * a concurrent-safe manner. Returns non-SUCCESS on
 * error, ERR_DATA_STRUCT_FULL if the message buffer
 * is full.
 */
ERR_TYPE logging_state_shared_message(
    Logging_State_Shared_Handle handle,
    const char * const msg
);

/*
 * Appends the given log entry to the logging state in
 * a concurrent-safe manner. Returns non-SUCCESS on
 * error, ERR_DATA_STRUCT_FULL if the log entry buffer
 * is full.
 */
ERR_TYPE logging_state_shared_entry(
    Logging_State_Shared_Handle handle,
    const char * const msg,
    const TIME_TYPE * const timestamp
);

/*
 * Copies the logging state to the given pointer and
 * clears the queues in the shared state in a
 * concurrent safe manner. Returns non-SUCCESS on
 * error.
 */
ERR_TYPE logging_state_shared_read(
    Logging_State_Shared_Handle handle,
    Logging_State * const out
);

/*
 * Cleans up the shared logging state. Returns
 * non-SUCCESS on error. Should not be called twice
 * concurrently.
 */
ERR_TYPE logging_state_shared_cleanup(
    Logging_State_Shared_Handle handle
);

#endif /* LOGGING_STATE_H */
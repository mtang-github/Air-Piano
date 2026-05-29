#include "logging_state.h"

#include <pthread.h>
#include <string.h>

#include "mutex_init.h"

/*
 * This file provides the implementation for the
 * opaque logging state type.
 */

/*
 * Definition for the logging state shared opaque type:
 * a shared logging state that includes concurrency
 */
struct Logging_State_Shared{
    /* the actual logging state */
    Logging_State logging_state;
    /*
     * 0 if not in use, non-zero otherwise; marked as
     * volatile since we may need to check this value
     * outside of the lock
     */
    volatile uint8_t in_use;
    /* mutex used to control access to this object */
    pthread_mutex_t mutex;
};

/*
 * The single instance of the shared logging state,
 * accessible only from this file
 */
static struct Logging_State_Shared logging_state_shared
    = {0};

/*
 * Clears the given logging state; no concurrency
 * guarantees whatsoever. Returns non-SUCCESS on error.
 */
static ERR_TYPE logging_state_clear(
    Logging_State * const to_clear
){
    ERR_TYPE err = SUCCESS;
    if(to_clear == NULL){
        err = ERR_NULL_PTR;
    }
    if(err == SUCCESS){
        (void)memset(
            to_clear,
            0,
            sizeof(*to_clear)
        );
    }
    return err;
}

/*
 * Initializes the shared logging state. Outputs
 * non-SUCCESS on error. Should not be called twice
 * concurrently.
 */
Logging_State_Shared_Handle logging_state_shared_init(
    ERR_TYPE * const err_out
){
    Logging_State_Shared_Handle ret = NULL;
    ERR_TYPE err = SUCCESS;

    /*
     * error if double init; this check cannot use the
     * mutex because it is likely uninitialized. The
     * cleanup procedure only clears in use after the
     * object is truly cleaned up.
     */
    if(logging_state_shared.in_use != 0u){
        err = ERR_CALLED_TWICE;
    }
    if(err == SUCCESS){
        /*
         * at this point, barring concurrent calls to
         * init, it is safe to proceed
         */
        err = logging_state_clear(
            &(logging_state_shared.logging_state)
        );
    }
    if(err == SUCCESS){
        err = mutex_init(&(logging_state_shared.mutex));
    }
    if(err == SUCCESS){
        logging_state_shared.in_use = 1u;
        ret = &logging_state_shared;
    }
    if(err_out != NULL){
        *err_out = err;
    }
    return ret;
}

/*
 * Appends the given message to the logging state in
 * a concurrent-safe manner. Returns non-SUCCESS on
 * error, ERR_DATA_STRUCT_FULL if the message buffer
 * is full.
 */
ERR_TYPE logging_state_shared_message(
    Logging_State_Shared_Handle handle,
    const char * const msg
){
    ERR_TYPE err = SUCCESS;
    int32_t retval = 0;
    uint8_t current_messages = 0u;

    if(handle == NULL){
        err = ERR_NULL_PTR;
    }
    if(msg == NULL){
        err = ERR_NULL_PTR;
    }

    if(err == SUCCESS){
        retval = pthread_mutex_lock(&(handle->mutex));
        if(retval != 0){
            err = ERR_BAD_SYSCALL;
        }
    }
    if(err == SUCCESS){
        /* if we are here, mutex locked successfully */

        /* ====================== *
         * START CRITICAL SECTION *
         * ====================== */

        current_messages
            = handle->logging_state.current_messages;
        if(current_messages >= MAX_MESSAGES){
            /* buffer is full */
            err = ERR_DATA_STRUCT_FULL;
            /* increment to +1 to signal lost msg */
            if(current_messages == MAX_MESSAGES){
                handle->logging_state.current_messages
                    = current_messages + 1;
            }
        }
        else{
            /* buffer is NOT full */
            handle->logging_state
                .messages[current_messages] = msg;
            handle->logging_state.current_messages
                = current_messages + 1;
        }

        /* ==================== *
         * END CRITICAL SECTION *
         * ==================== */

        retval = pthread_mutex_unlock(&(handle->mutex));
        if(retval != 0){
            /* may overwrite capacity reached */
            err = ERR_BAD_SYSCALL;
        }
    }

    return err;
}

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
){
    ERR_TYPE err = SUCCESS;
    int32_t retval = 0;
    uint8_t current_log_entries = 0u;

    if(handle == NULL){
        err = ERR_NULL_PTR;
    }
    if(msg == NULL){
        err = ERR_NULL_PTR;
    }

    if(err == SUCCESS){
        retval = pthread_mutex_lock(&(handle->mutex));
        if(retval != 0){
            err = ERR_BAD_SYSCALL;
        }
    }
    if(err == SUCCESS){
        /* if we are here, mutex locked successfully */

        /* ====================== *
         * START CRITICAL SECTION *
         * ====================== */

        current_log_entries
            = handle->logging_state.current_log_entries;
        if(current_log_entries >= MAX_LOG_ENTRIES){
            /* buffer is full */
            err = ERR_DATA_STRUCT_FULL;
            /* increment to +1 to signal lost msg */
            if(current_log_entries == MAX_LOG_ENTRIES){
                handle->logging_state.current_log_entries
                    = current_log_entries + 1;
            }
        }
        else{
            /* buffer is NOT full */
            handle->logging_state
                .log_entries[current_log_entries]
                    .message = msg;
            handle->logging_state
                .log_entries[current_log_entries]
                    .timestamp = *timestamp;
            handle->logging_state.current_log_entries
                = current_log_entries + 1;
        }

        /* ==================== *
         * END CRITICAL SECTION *
         * ==================== */

        retval = pthread_mutex_unlock(&(handle->mutex));
        if(retval != 0){
            /* may overwrite capacity reached */
            err = ERR_BAD_SYSCALL;
        }
    }

    return err;
}

/*
 * Copies the logging state to the given pointer and
 * clears the message queue in the shared state in a
 * concurrent safe manner. Returns non-SUCCESS on
 * error.
 */
ERR_TYPE logging_state_shared_read(
    Logging_State_Shared_Handle handle,
    Logging_State * const out
){
    ERR_TYPE err = SUCCESS;
    int32_t retval = 0;

    if(out == NULL){
        err = ERR_NULL_PTR;
    }
    if(handle == NULL){
        err = ERR_NULL_PTR;
    }
    else{
        if(handle->in_use == 0u){
            err = ERR_NOT_IN_USE;
        }
    }

    if(err == SUCCESS){
        retval = pthread_mutex_lock(
            &(handle->mutex)
        );
        if(retval != 0){
            err = ERR_BAD_SYSCALL;
        }
    }
    if(err == SUCCESS){
        /* if we are here, mutex locked successfully */

        /* ====================== *
         * START CRITICAL SECTION *
         * ====================== */

        /* copy out the data */
        (void)memcpy(
            out,
            &(handle->logging_state),
            sizeof(handle->logging_state)
        );

        /* reset the shared data */
        err = logging_state_clear(
            &(logging_state_shared.logging_state)
        );

        /* ==================== *
         * END CRITICAL SECTION *
         * ==================== */

        retval = pthread_mutex_unlock(&(handle->mutex));
        if(retval != 0){
            /* may overwrite failed clear */
            err = ERR_BAD_SYSCALL;
        }
    }

    return err;
}

/*
 * Cleans up the shared logging state. Returns
 * non-SUCCESS on error. Should not be called twice
 * concurrently.
 */
ERR_TYPE logging_state_shared_cleanup(
    Logging_State_Shared_Handle handle
){
    ERR_TYPE err = SUCCESS;
    int32_t retval = 0;
    
    if(handle == NULL){
        err = ERR_NULL_PTR;
    }
    else{
        if(handle->in_use == 0u){
            err = ERR_NOT_IN_USE;
        }
    }
    if(err == SUCCESS){
        err = logging_state_clear(
            &(logging_state_shared.logging_state)
        );
        retval = pthread_mutex_destroy(&(handle->mutex));
        /* must clean up last */
        handle->in_use = 0u;
        if(retval != 0){
            /* may overwrite failed clear */
            err = ERR_BAD_SYSCALL;
        }
    }
    return err;
}
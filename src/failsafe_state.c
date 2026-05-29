#include "failsafe_state.h"

#include <pthread.h>
#include <string.h>
#include <float.h>

#include "mutex_init.h"

/* This file implements the failsafe state object */

/* Definition for the Failsafe_State_Shared type */
struct Failsafe_State_Shared {
    /* The actual failsafe state data */
    Failsafe_State failsafe_state;
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
 * The single instance of the shared failsafe state,
 * accessible only from this file.
 */
static struct Failsafe_State_Shared
    failsafe_state_shared = {0};

/*
 * Initializes the shared failsafe state (mutex,
 * initial values). Outputs non-SUCCESS on error.
 * Should not be called twice concurrently.
 */
Failsafe_State_Shared_Handle failsafe_state_shared_init(
    ERR_TYPE * const err_out
){
    Failsafe_State_Shared_Handle ret = NULL;
    ERR_TYPE err = SUCCESS;

    /*
     * error if double init; this check cannot use the
     * mutex because it is likely uninitialized. The
     * cleanup procedure only clears in use after the
     * object is truly cleaned up.
     */
    if(failsafe_state_shared.in_use != 0u){
        err = ERR_CALLED_TWICE;
    }
    if(err == SUCCESS){
        /*
         * at this point, barring concurrent calls to
         * init, it is safe to proceed
         */
        failsafe_state_shared
            .failsafe_state.failsafe = 0u;
        err = mutex_init(&(failsafe_state_shared.mutex));
    }
    if(err == SUCCESS){
        failsafe_state_shared.in_use = 1u;
        ret = &failsafe_state_shared;
    }
    if(err_out != NULL){
        *err_out = err;
    }
    return ret;
}

/*
 * Sets the failsafe state to the given value. Error
 * if a value other than 0 or 1 is passed in. Returns
 * non-SUCCESS on error.
 */
ERR_TYPE failsafe_state_shared_set(
    Failsafe_State_Shared_Handle handle,
    uint8_t failsafe
){
    ERR_TYPE err = SUCCESS;
    int32_t retval = 0;

    if(handle == NULL){
        err = ERR_NULL_PTR;
    }
    else {
        if(handle->in_use == 0u){
            err = ERR_NOT_IN_USE;
        }
    }
    if(err == SUCCESS){
        if(!(failsafe == 0u || failsafe == 1u)){
            err = ERR_BAD_ARG;
        }
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

        handle->failsafe_state.failsafe = failsafe;

        /* ==================== *
         * END CRITICAL SECTION *
         * ==================== */

        retval = pthread_mutex_unlock(&(handle->mutex));
        if(retval != 0){
            err = ERR_BAD_SYSCALL;
        }
    }
    return err;
}

/*
 * Returns 1 if we are currently in failsafe, 0
 * otherwise. Outputs non-SUCCESS on error (and
 * returns 1).
 */
ERR_TYPE failsafe_state_shared_get(
    Failsafe_State_Shared_Handle handle,
    uint8_t *failsafe_out
){
    ERR_TYPE err = SUCCESS;
    int32_t retval = 0;

    if(failsafe_out == NULL){
        err = ERR_NULL_PTR;
    }
    if(handle == NULL){
        err = ERR_NULL_PTR;
    }
    else {
        if(handle->in_use == 0u){
            err = ERR_NOT_IN_USE;
        }
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

        *failsafe_out = handle->failsafe_state.failsafe;

        /* ==================== *
         * END CRITICAL SECTION *
         * ==================== */
        
        retval = pthread_mutex_unlock(&(handle->mutex));
        if(retval != 0){
            err = ERR_BAD_SYSCALL;
        }
    }

    if(err != SUCCESS){
        if(failsafe_out != NULL){
            *failsafe_out = 1u;
        }
    }

    return err;
}

/*
 * Cleans up the specified shared failsafe state. Should
 * not call unless no thread could possibly be
 * holding the mutex, nor should it be called twice
 * concurrently. Returns non-SUCCESS on error.
 */
ERR_TYPE failsafe_state_shared_cleanup(
    Failsafe_State_Shared_Handle handle
){
    ERR_TYPE err = SUCCESS;
    int32_t retval = 0;

    if(handle == NULL){
        err = ERR_NULL_PTR;
    }
    else {
        if(handle->in_use == 0u){
            err = ERR_NOT_IN_USE;
        }
    }

    if(err == SUCCESS){
        handle->failsafe_state.failsafe = 0u;

        /*
         * Have to be careful calling mutex destroy,
         * since it won't really work if another thread
         * is holding the mutex. Thus, this function
         * should only be called when all threads
         * have been joined and final cleanup is
         * occurring.
         */
        retval = pthread_mutex_destroy(&(handle->mutex));

        /* must clean up last */
        handle->in_use = 0u;

        if(retval != 0){
            err = ERR_BAD_SYSCALL;
        }
    }

    return err;
}
#include "distance_state.h"

#include <pthread.h>

#include "mutex_init.h"

/*
 * This file implements the distance state
 * object.
 */

/* Definition for the Distance_State_Shared type */
struct Distance_State_Shared {
    /* the actual distance state data */
    Distance_State distance_state;
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
 * The single instance of the shared distance state,
 * accessible only from this file.
 */
static struct Distance_State_Shared
    distance_state_shared = {0};

/*
 * Initializes the shared distance state (mutex,
 * initial values). Outputs non-SUCCESS on error.
 * Should not be called twice concurrently.
 */
Distance_State_Shared_Handle distance_state_shared_init(
    ERR_TYPE * const err_out
){
    Distance_State_Shared_Handle ret = NULL;
    ERR_TYPE err = SUCCESS;

    /*
     * error if double init; this check cannot use the
     * mutex because it is likely uninitialized. The
     * cleanup procedure only clears in use after the
     * object is truly cleaned up.
     */
    if(distance_state_shared.in_use != 0u){
        err = ERR_CALLED_TWICE;
    }
    if(err == SUCCESS){
        /*
         * at this point, barring concurrent calls to
         * init, it is safe to proceed
         */
        distance_state_shared.distance_state
            .distance_cm = 0.0f;
        distance_state_shared.distance_state
            .note_index = INVALID_NOTE_INDEX;

        err = mutex_init(&(distance_state_shared.mutex));
    }
    if(err == SUCCESS){
        distance_state_shared.in_use = 1u;
        ret = &distance_state_shared;
    }
    if(err_out != NULL){
        *err_out = err;
    }
    return ret;
}


/*
 * Writes the distance state into the specified shared
 * distance state in a concurrent-safe manner.
 * Returns non-SUCCESS on error. 
 */
ERR_TYPE distance_state_shared_write(
    Distance_State_Shared_Handle handle,
    float distance_cm,
    NOTE_INDEX_TYPE note_index
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

        handle->distance_state.distance_cm
            = distance_cm;
        handle->distance_state.note_index
            = note_index;

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
 * Copies the distance state from the specified shared
 * distance state in a concurrent-safe manner.
 * Returns non-SUCCESS on error.
 */
ERR_TYPE distance_state_shared_read(
    Distance_State_Shared_Handle handle,
    Distance_State * const out
){
    ERR_TYPE err = SUCCESS;
    int32_t retval = 0;

    if(out == NULL){
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

        *out = handle->distance_state;

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
 * Cleans up the specified shared distance state. Should
 * not call unless no thread could possibly be
 * holding the mutex, nor should it be called twice
 * concurrently. Returns non-SUCCESS on error.
 */
ERR_TYPE distance_state_shared_cleanup(
    Distance_State_Shared_Handle handle
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
        handle->distance_state
            .distance_cm = 0.0f;
        handle->distance_state
            .note_index = INVALID_NOTE_INDEX;

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
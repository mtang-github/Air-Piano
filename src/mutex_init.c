#include "mutex_init.h"

/*
 * This file provides different implementations for
 * mutex_init() depending on if we are emulating or
 * real-time
 */

/*
 * Initializes the provided mutex; returns non-SUCCESS
 * on error.
 */
ERR_TYPE mutex_init(pthread_mutex_t *to_init){
    ERR_TYPE err = SUCCESS;
    int32_t retval = 0;
    pthread_mutexattr_t mutex_attr = {0};

    if(to_init == NULL){
        err = ERR_NULL_PTR;
    }

    /* set up mutex attributes */
    if(err == SUCCESS){
        retval = pthread_mutexattr_init(&mutex_attr);
        if(retval != 0){
            err = ERR_BAD_SYSCALL;
        }
    }

    /* only set prio inherit if we are not emulated */
    #ifndef EMU
    if(err == SUCCESS){
        retval = pthread_mutexattr_setprotocol(
            &mutex_attr,
            PTHREAD_PRIO_INHERIT
        );
        if(retval != 0){
            err = ERR_BAD_SYSCALL;
        }
    }
    #endif

    if(err == SUCCESS){
        retval = pthread_mutexattr_settype(
            &mutex_attr,
            PTHREAD_MUTEX_NORMAL
        );
        if(retval != 0){
            err = ERR_BAD_SYSCALL;
        }
    }

    if(err == SUCCESS){
        retval = pthread_mutex_init(
            to_init,
            &mutex_attr
        );

        if(retval != 0){
            err = ERR_BAD_SYSCALL;
        }
    }

    /* must destroy attr on all paths */
    retval = pthread_mutexattr_destroy(&mutex_attr);
    if(err == SUCCESS && retval != 0){
        /* only set err if this was the only error */
        err = ERR_BAD_SYSCALL;
    }

    /* if unsuccessful, try to destroy the mutex */
    if(err != SUCCESS){
        /*
         * squash errors, since we already encountered
         * one if we are here
         */
        (void)pthread_mutex_destroy(to_init);
    }
    
    return err;
}
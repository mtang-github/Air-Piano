#include "timing.h"

#include <time.h>

/*
 * Converts an ns duration to a TIME_TYPE and writes
 * it into the specified TIME_TYPE; returns SUCCESS
 * if successful, or an error code otherwise.
 */
ERR_TYPE ns_to_time(
    NS_TYPE duration_ns,
    TIME_TYPE *to_write
){
    ERR_TYPE err = SUCCESS;
    if(to_write == NULL){
        err = ERR_NULL_PTR;
    }
    if(duration_ns < 0){
        err = ERR_BAD_ARG;
    }
    if(err == SUCCESS){
        to_write->tv_sec
            = (time_t)(duration_ns / BILLION);
        to_write->tv_nsec = duration_ns % BILLION;
    }
    return err;
}

/*
 * Converts a TIME_TYPE to ns; writes non-SUCCESS
 * on failure.
 */
NS_TYPE time_to_ns(
    const TIME_TYPE * const time,
    ERR_TYPE *err_out
){
    ERR_TYPE err = SUCCESS;
    NS_TYPE ns_sec = 0;
    NS_TYPE ns_total = 0;

    if(time == NULL){
        err = ERR_NULL_PTR;
    }

    /* tv_sec * BILLION fits in int64_t for any 32-bit time_t value */
    if(err == SUCCESS){
        ns_sec = (NS_TYPE)time->tv_sec * BILLION;
        
        if(time->tv_nsec > 0
            && ns_sec > (NS_MAX - time->tv_nsec)
        ){
            err = ERR_OVERFLOW;
        }
        else if(time->tv_nsec < 0
            && ns_sec < (NS_MIN - time->tv_nsec)
        ){
            err = ERR_OVERFLOW;
        }
        else{
            ns_total = ns_sec + time->tv_nsec;
        }
    }

    if (err_out != NULL) {
        *err_out = err;
    }
    return ns_total;
}

/*
 * Adds the specified duration in ns to the given
 * TIME_TYPE; returns error code on failure, SUCCESS
 * otherwise.
 */
ERR_TYPE time_add_ns(
    TIME_TYPE *to_add_to,
    NS_TYPE duration_ns
){
    ERR_TYPE err = SUCCESS;
    NS_TYPE secs_to_add = 0;
    NS_TYPE ns_to_add = 0;

    if(to_add_to == NULL){
        err = ERR_NULL_PTR;
    }
    if(duration_ns < 0){
        err = ERR_BAD_ARG;
    }

    if(err == SUCCESS){
        /* precalculate sec/ns split */
        secs_to_add = duration_ns / BILLION;
        ns_to_add = duration_ns % BILLION;

        /* add values directly */
        to_add_to->tv_sec += secs_to_add;
        to_add_to->tv_nsec += ns_to_add;

        /* check for nsec creating a carry */
        if(to_add_to->tv_nsec >= BILLION){
            to_add_to->tv_nsec -= BILLION;
            ++to_add_to->tv_sec;
        }
    }
    return err;
}

/*
 * Computes the difference between two TIME_TYPE
 * objects and outputs to a third; returns
 * non-SUCCESS on failure.
 */
ERR_TYPE time_diff(
    const TIME_TYPE * const left,
    const TIME_TYPE * const right,
    TIME_TYPE *out
){
    ERR_TYPE err = SUCCESS;
    TIME_TYPE diff = TIME_TYPE_INIT;
    if(left == NULL || right == NULL || out == NULL){
        err = ERR_NULL_PTR;
    }
    if(err == SUCCESS){
        diff.tv_sec = left->tv_sec - right->tv_sec;
        diff.tv_nsec = left->tv_nsec - right->tv_nsec;
        if (diff.tv_nsec < 0) {
            diff.tv_nsec += BILLION;
            --diff.tv_sec;
        }
        *out = diff;
    }
    
    return err;
}

/*
 * Writes the current timepoint into the specified
 * TIME_TYPE; returns SUCCESS on error, or an error
 * code otherwise
 */
ERR_TYPE get_current_time(TIME_TYPE *to_write){
    ERR_TYPE err = SUCCESS;
    int32_t retval = 0;
    if(to_write == NULL){
        err = ERR_NULL_PTR;
    }
    if(err == SUCCESS){
        retval = clock_gettime(
            CLOCK_MONOTONIC,
            to_write
        );

        if(retval != 0){
            err = ERR_BAD_SYSCALL;
        }
    }
    return err;
}
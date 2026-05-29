#include "priority.h"

#include <sched.h>
#include <pthread.h>

/* Implements the priority setting function */

/*
 * Sets the real-time priority of the calling thread to
 * the given priority number; assumes POSIX priority
 * conventions. Returns ERR_BAD_ARG if an invalid
 * priority was given, SUCCESS otherwise.
 */
ERR_TYPE set_thread_priority(int32_t prio){
    int32_t min_prio
        = sched_get_priority_min(SCHED_FIFO);
    int32_t max_prio
        = sched_get_priority_max(SCHED_FIFO);
    struct sched_param param = {0};
    int32_t retval = 0;

    if(prio > max_prio || prio < min_prio){
        return ERR_BAD_ARG;
    } 

    param.sched_priority = prio;
    retval = pthread_setschedparam(
        pthread_self(),
        SCHED_FIFO,
        &param
    );
    if(retval != 0){
        return ERR_BAD_SYSCALL;
    }
    return SUCCESS;
}
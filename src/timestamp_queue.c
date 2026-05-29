#include "timestamp_queue.h"

#include <string.h>
#ifdef EMU
#include <stdio.h>
#endif

/* define for verbose output */
// #define VERBOSE

#define MAX_QUEUE_INDEX_INCL \
    (MAX_BREAKS_PER_SECOND - 1u)

/*
 * Initializes the specified timestamp queue. Returns
 * non-SUCCESS on error.
 */
ERR_TYPE timestamp_queue_init(Timestamp_Queue *to_init){
    ERR_TYPE err = SUCCESS;
    TIME_TYPE zero_timestamp = TIME_TYPE_INIT;
    uint8_t i = 0u;

    if(to_init == NULL){
        err = ERR_NULL_PTR;
    }

    if(err == SUCCESS){
        /* set all timestamps to 0 */
        for(i = 0u; i < MAX_BREAKS_PER_SECOND; ++i){
            (void)memcpy(
                to_init->queue + i,
                &zero_timestamp,
                sizeof(zero_timestamp)
            );
        }

        /*
         * initialize head to point to last element,
         * so that on the first write, it writes to
         * first
         */
        to_init->head = MAX_QUEUE_INDEX_INCL;
    }

    return err;
}

/*
 * Appends a new break timestamp to the timestamp
 * queue and checks for violations. Returns non-SUCCESS
 * on error.
 */
ERR_TYPE timestamp_queue_append(
    Timestamp_Queue *queue,
    TIME_TYPE *timestamp,
    uint8_t *violation_out
){
    ERR_TYPE err = SUCCESS;
    TIME_TYPE diff = TIME_TYPE_INIT;
    NS_TYPE diff_ns = 0;
    uint8_t violation = 0u;
    uint8_t next_head = 0u;
    uint8_t tail = 0u;

    if(queue == NULL){
        err = ERR_NULL_PTR;
    }
    if(timestamp == NULL){
        err = ERR_NULL_PTR;
    }
    if(violation_out == NULL){
        err = ERR_NULL_PTR;
    }

    if(err == SUCCESS){
        /*
         * append the new timestamp, overwriting
         * prior data
         */
        next_head = queue->head + 1u;
        if(next_head > MAX_QUEUE_INDEX_INCL){
            next_head = 0u;
        }
        queue->head = next_head;
        (void)memcpy(
            queue->queue + next_head,
            timestamp,
            sizeof(*timestamp)
        );

        /*
         * check the timestamp against the tail to see
         * if the duration between the two is less than
         * one second; if so, flag a violation
         */
        tail = next_head + 1u;
        if(tail > MAX_QUEUE_INDEX_INCL){
            tail = 0u;
        }
        time_diff(
            timestamp,
            queue->queue + tail,
            &diff
        );
        diff_ns = time_to_ns(&diff, &err);
        /*
         * if diff is less than 1 sec,
         * that implies 10 breaks have occurred
         * in that time frame, which is a
         * violation
         */
        if(diff_ns < BILLION){
            #ifdef VERBOSE
            printf("diff: %lld\n", diff_ns);
            #endif
            violation = 1u;
        }
    }

    if(err != SUCCESS){
        violation = 1u;
    }
    if(violation_out != NULL){
        *violation_out = violation;
    }

    return err;
}

/*
 * Destroys the specified timestamp queue. Returns
 * non-SUCCESS on error.
 */
ERR_TYPE timestamp_queue_destroy(
    Timestamp_Queue *to_destroy
){
    ERR_TYPE err = SUCCESS;
    if(to_destroy == NULL){
        err = ERR_NULL_PTR;
    }

    if(err == SUCCESS){
        (void)memset(
            to_destroy,
            0,
            sizeof(*to_destroy)
        );
    }

    return err;
}
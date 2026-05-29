#ifndef TIMESTAMP_QUEUE_H
#define TIMESTAMP_QUEUE_H

#include "timing.h"

/*
 * This file provides the interface for the
 * timestamp queue, used for
 * detecting too-frequent beam breaks.
 */

/*
 * The maximum number of beam breaks tolerable by the
 * system until it presumes the sensor has gone bad.
 */
#define MAX_BREAKS_PER_SECOND 10u

/*
 * The timestamp queue, a data structure used to
 * detect too-frequent bream breaks. It is implemented
 * as a circular queue.
 */
typedef struct {
    /* The actual queue. */
    TIME_TYPE queue[MAX_BREAKS_PER_SECOND];

    /*
     * Indexes the current head of the circular queue;
     * the tail is assumed to be one ahead of the head
     */
    uint8_t head;
} Timestamp_Queue;

/*
 * Initializes the specified timestamp queue. Returns
 * non-SUCCESS on error.
 */
ERR_TYPE timestamp_queue_init(Timestamp_Queue *to_init);

/*
 * Appends a new break timestamp to the timestamp
 * queue and checks for violations. Returns non-SUCCESS
 * on error.
 */
ERR_TYPE timestamp_queue_append(
    Timestamp_Queue *queue,
    TIME_TYPE *timestamp,
    uint8_t *violation_out
);

/*
 * Destroys the specified timestamp queue. Returns
 * non-SUCCESS on error.
 */
ERR_TYPE timestamp_queue_destroy(
    Timestamp_Queue *to_destroy
);

#endif /* TIMESTAMP_QUEUE_H */
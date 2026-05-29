#ifndef MUTEX_INIT_H
#define MUTEX_INIT_H

#include <pthread.h>

#include "error_type.h"

/*
 * This file provides the interface for mutex_init(),
 * a helper function used initialize all mutexes used
 * by the air piano in the same way.
 */

/*
 * Initializes the provided mutex; returns non-SUCCESS
 * on error.
 */
ERR_TYPE mutex_init(pthread_mutex_t *to_init);

#endif
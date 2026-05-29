#include "distance_thread.h"

#include <time.h>
#include <stdint.h>
#include <sched.h>
#include <pthread.h>

#ifdef EMU
#include <stdio.h>
#endif

#include "stop.h"
#include "notes.h"
#include "timing_config.h"
#include "priority.h"

#define FAILSAFE_MIN_DIST_CM_INCL 0.0f
#define FAILSAFE_MAX_DIST_CM_INCL 500.0f

/*
 * Busy-waits until at least ns_delay nanoseconds have
 * elapsed since *start.
 */
static void busy_wait_ns(
    const struct timespec *start,
    long ns_delay
){
    struct timespec now;
    long elapsed;
    do {
        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsed = (now.tv_sec - start->tv_sec) * 1000000000L
                  + (now.tv_nsec - start->tv_nsec);
    } while(elapsed < ns_delay);
}

/*
 * Fires one HC-SR04 pulse and returns the measured
 * distance in cm, or -1.0f on timeout/error.
 */
static float measure_distance(
    Pin_Io_Handle pin_io_handle
){
    #ifdef EMU
    (void)pin_io_handle;
    (void)busy_wait_ns;
    return 10.0f;
    #else
    struct timespec t0, t1, t2, now;
    uint8_t val = 0u;
    long elapsed;

    /* 10 µs trigger pulse */
    pin_io_set(pin_io_handle, PIN_OUT_TRIG, PIN_ON);
    clock_gettime(CLOCK_MONOTONIC, &t0);
    busy_wait_ns(&t0, TRIG_PULSE_NS);
    pin_io_set(pin_io_handle, PIN_OUT_TRIG, PIN_OFF);

    /* wait for echo to go high */
    clock_gettime(CLOCK_MONOTONIC, &t1);
    do {
        pin_io_read(pin_io_handle, PIN_IN_ECHO, &val);
        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsed = (now.tv_sec - t1.tv_sec) * 1000000000L
                  + (now.tv_nsec - t1.tv_nsec);
        if(elapsed > ECHO_TIMEOUT_NS){
            return -1.0f;
        }
    } while(val == 0u);
    clock_gettime(CLOCK_MONOTONIC, &t1);

    /* wait for echo to go low */
    do {
        pin_io_read(pin_io_handle, PIN_IN_ECHO, &val);
        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsed = (now.tv_sec - t1.tv_sec) * 1000000000L
                  + (now.tv_nsec - t1.tv_nsec);
        if(elapsed > ECHO_TIMEOUT_NS){
            return -1.0f;
        }
    } while(val == 1u);
    clock_gettime(CLOCK_MONOTONIC, &t2);

    /* duration in µs → distance in cm */
    long pulse_ns = (t2.tv_sec - t1.tv_sec) * 1000000000L
                    + (t2.tv_nsec - t1.tv_nsec);
    float dist_cm = (float)pulse_ns / 1000.0f * 0.0343f / 2.0f;
    return dist_cm;
    #endif
}

/*
 * Distance sensor thread: measures distance at ~30 Hz
 * and writes results to shared_distance.
 */
void *distance_thread_fn(void *arg)
{
    Distance_Thread_Args *args
        = (Distance_Thread_Args*)arg;
    TIME_TYPE next             = TIME_TYPE_INIT;
    TIME_TYPE timestamp        = TIME_TYPE_INIT;
    float distance_cm          = 0.0f;
    NOTE_INDEX_TYPE note_index = 0u;
    NOTE_INDEX_TYPE new_note_index = 0u;

    /* set real-time priority */
    set_thread_priority(DISTANCE_PRIO);

    /* ensure trig starts low */
    pin_io_set(
        args->pin_io_handle,
        PIN_OUT_TRIG,
        PIN_OFF
    );

    /*
     * initialize the next time to now, so when we
     * add in the period later, it is accurate
     */
    get_current_time(&next);

    while(stop == 0)
    {
        get_current_time(&timestamp);
        logging_state_shared_entry(
            args->logging_state_shared_handle,
            "distance thread start measuring",
            &timestamp
        );

        distance_cm = measure_distance(args->pin_io_handle);

        get_current_time(&timestamp);
        logging_state_shared_entry(
            args->logging_state_shared_handle,
            "distance thread finished measuring",
            &timestamp
        );

        if(distance_cm < FAILSAFE_MIN_DIST_CM_INCL){
            failsafe_state_shared_set(
                args->failsafe_state_shared_handle,
                1u
            );
            logging_state_shared_message(
                args->logging_state_shared_handle,
                "FAILSAFE: negative distance reading "
                "or failed distance reading"
            );
        }
        if(distance_cm > FAILSAFE_MAX_DIST_CM_INCL){
            failsafe_state_shared_set(
                args->failsafe_state_shared_handle,
                1u
            );
            logging_state_shared_message(
                args->logging_state_shared_handle,
                "FAILSAFE: distance reading over max"
            );
        }

        if(distance_cm >= 0.0f 
            && distance_cm <= NOTE_DIST_MAX_CM
        ){
            new_note_index
                = note_from_distance_cm(distance_cm);
            if(new_note_index != note_index){
                note_index = new_note_index;

                get_current_time(&timestamp);
                logging_state_shared_entry(
                    args->logging_state_shared_handle,
                    "distance thread about to update note",
                    &timestamp
                );
            }
            #ifndef EMU
            distance_state_shared_write(
                args->distance_state_shared_handle,
                distance_cm,
                note_index
            );
            #else
            /* suppress unused if emu */
            (void)note_index;
            #endif
        }

        /* advance to next 30 Hz tick */
        time_add_ns(&next, DISTANCE_PERIOD_NS);

        /* sleep until next scheduled wakeup time */
        sleep_absolute(&next);
    }

    /* if we are here, stop has been flagged */

    #ifdef EMU
    printf("THREAD EXIT: distance\n");
    #endif

    return NULL;
}

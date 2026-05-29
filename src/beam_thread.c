#include "beam_thread.h"

#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <pthread.h>

#include "stop.h"
#include "notes.h"
#include "pwm.h"
#include "timing_config.h"
#include "priority.h"
#include "timestamp_queue.h"

/*
 * Implements the beam thread, responsible for detecting
 * beam breaks.
 */

/*
 * Define if we want verbose output bypassing logger,
 * otherwise comment out
 */
#define VERBOSE

/* Helper function that actually outputs a note. */
static ERR_TYPE output_note(
    Beam_Thread_Args *args,
    Distance_State *distance_state
){
    ERR_TYPE err = SUCCESS;
    uint8_t failsafe = 0u;

    if(args == NULL){
        err = ERR_NULL_PTR;
    }
    if(distance_state == NULL){
        err = ERR_NULL_PTR;
    }

    if(err == SUCCESS){
        err = failsafe_state_shared_get(
            args->failsafe_state_shared_handle,
            &failsafe
        );
    }

    if(!failsafe && err == SUCCESS){
        err = distance_state_shared_read(
            args->distance_state_shared_handle,
            distance_state
        );
    }
    if(!failsafe && err == SUCCESS){
        if(note_is_valid(distance_state->note_index)){
            #ifdef VERBOSE
            /*
             * cannot use logger for this;
             * dynamic strings
             */
            printf("BEAM BREAK: playing %s (%.1f cm)\n",
                note_get_name(distance_state->note_index),
                distance_state->distance_cm
            );
            fflush(stdout);
            #else
            logging_state_shared_message(
                args->logging_state_shared_handle,
                note_get_name(distance_state->note_index)
            );
            #endif
            pwm_play(distance_state->note_index);
            sleep_relative_ns(args->note_duration);
            pwm_stop();
        }
        else{
            logging_state_shared_message(
                args->logging_state_shared_handle,
                "BEAM BREAK: no valid note yet\n"
            );
        }
    }

    return err;
}

/*
 * Beam sensor thread: polls the IR break beam at 100 Hz.
 * On a falling edge (beam newly broken), plays the current
 * note for 0.3 seconds then stops.
 *
 * If the beam reads as continuously broken for >= 2 seconds
 * (emitter and receiver not aligned), prints a warning
 * once per second.
 *
 * The beam sensor is active-low: GPIO reads 0 when broken.
 * Runs at SCHED_FIFO priority 90 (highest).
 */
void *beam_thread_fn(void *arg){
    Beam_Thread_Args *args = (Beam_Thread_Args *)arg;
    uint8_t prev               = 1u;
    uint8_t curr               = 1u;
    TIME_TYPE next_time        = TIME_TYPE_INIT;
    TIME_TYPE timestamp        = TIME_TYPE_INIT;
    uint32_t broken_ticks      = 0u;
    uint32_t tick_difference   = 0u;
    uint8_t violation          = 0u;
    Timestamp_Queue timestamp_queue = {0};

    /* local use distance state */
    Distance_State distance_state = {0};

    /* set real-time priority */
    set_thread_priority(BEAM_PRIO);

    /*
     * initialize timestamp queue data structure; do it
     * before initializing time
     */
    timestamp_queue_init(&timestamp_queue);

    /*
     * initialize the next time to now, so when we
     * add in the period later, it is accurate
     */
    get_current_time(&next_time);

    while(stop == 0)
    {
        get_current_time(&timestamp);
        logging_state_shared_entry(
            args->logging_state_shared_handle,
            "beam thread checking pin in",
            &timestamp
        );
        
        pin_io_read(
            args->pin_io_handle,
            PIN_IN_BEAM,
            &curr
        );

        /* curr being 0 means the beam was broken */
        if(curr == 0u){
            broken_ticks++;

            /* alignment warning once per second after 2s */
            if(broken_ticks >= BEAM_ALIGNMENT_WARN_TICKS){
                tick_difference = broken_ticks
                    - BEAM_ALIGNMENT_WARN_TICKS;
                if((tick_difference % BEAM_TICKS_PER_SEC) == 0){
                    logging_state_shared_message(
                        args->logging_state_shared_handle,
                        "WARNING: beam not aligned "
                        "(check emitter/receiver)\n"
                    );                    
                }
            }

            /* falling edge within normal operation */
            else if(prev != 0u){
                /*
                 * if we are here, we are not handling
                 * alignment warning and broken_ticks
                 * has not reached
                 * BEAM_ALIGNMENT_WARN_TICKS
                 */

                /*
                 * check for failsafe from beam
                 * timestamps
                 */
                get_current_time(&timestamp);
                timestamp_queue_append(
                    &timestamp_queue,
                    &timestamp,
                    &violation
                );
                if(violation){
                    /* violation! flag failsafe mode */
                    failsafe_state_shared_set(
                        args->failsafe_state_shared_handle,
                        1u
                    );
                    logging_state_shared_message(
                        args->logging_state_shared_handle,
                        "FAILSAFE: beam break too "
                        "frequent\n"
                    );
                    get_current_time(&timestamp);
                    logging_state_shared_entry(
                        args->logging_state_shared_handle,
                        "beam break (violation)",
                        &timestamp
                    );
                }
                else{
                    get_current_time(&timestamp);
                    logging_state_shared_entry(
                        args->logging_state_shared_handle,
                        "beam break (about to play note)",
                        &timestamp
                    );

                    output_note(args, &distance_state);

                    get_current_time(&timestamp);
                    logging_state_shared_entry(
                        args->logging_state_shared_handle,
                        "beam break (finish playing note)",
                        &timestamp
                    );

                    /*
                     * update next time to prevent
                     * tick "buildup"
                     */
                    get_current_time(&next_time);
                }
            }
        }
        else{
            /*
             * if we are here, we read a 1 meaning the
             * beam was not broken
             */
            broken_ticks = 0u;
        }

        prev = curr;

        /* advance to next 100 Hz tick */
        time_add_ns(&next_time, BEAM_PERIOD_NS);

        /* sleep until next scheduled wakeup time */
        sleep_absolute(&next_time);
    }

    /* if we are here, stop has been flagged */
    pwm_stop();

    #ifdef EMU
    printf("THREAD EXIT: beam\n");
    #endif

    return NULL;
}
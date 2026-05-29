#include "display_thread.h"

#ifdef EMU
#include <stdio.h>
#endif

#include "priority.h"
#include "timing_config.h"
#include "stop.h"

/*
 * Implements the display thread, responsible for
 * updating the current note display.
 */

/* Helper function to actually update display */
static ERR_TYPE update_display(
    Display_Thread_Args *args,
    NOTE_INDEX_TYPE *current_index,
    Distance_State *distance_state
){
    ERR_TYPE err = SUCCESS;
    TIME_TYPE timestamp = TIME_TYPE_INIT;

    distance_state_shared_read(
        args->distance_state_shared_handle,
        distance_state
    );

    if(note_is_valid(distance_state->note_index)){
        if(distance_state->note_index < *current_index){
            /* shift light down one */
            pin_io_set(
                args->pin_io_handle,
                note_get_led_pin_id(*current_index),
                PIN_OFF
            );
            --(*current_index);
            pin_io_set(
                args->pin_io_handle,
                note_get_led_pin_id(*current_index),
                PIN_ON
            );

            get_current_time(&timestamp);
            logging_state_shared_entry(
                args->logging_state_shared_handle,
                "display thread updated led",
                &timestamp
            );
        }
        if(distance_state->note_index > *current_index){
            /* shift light up one */
            pin_io_set(
                args->pin_io_handle,
                note_get_led_pin_id(*current_index),
                PIN_OFF
            );
            ++(*current_index);
            pin_io_set(
                args->pin_io_handle,
                note_get_led_pin_id(*current_index),
                PIN_ON
            );

            get_current_time(&timestamp);
            logging_state_shared_entry(
                args->logging_state_shared_handle,
                "display thread updated led",
                &timestamp
            );
        }
    }

    return err;
}

/*
 * The display thread, which gets distance data and
 * updates the note leds.
 */
void *display_thread_fn(void *arg){
    Display_Thread_Args *args
        = (Display_Thread_Args *)arg;

    ERR_TYPE err = SUCCESS;
    TIME_TYPE next = TIME_TYPE_INIT;
    TIME_TYPE timestamp = TIME_TYPE_INIT;
    NOTE_INDEX_TYPE current_index = C4_INDEX;
    uint8_t prev_failsafe = 0u;
    uint8_t failsafe = 0u;

    /* local use distance state */
    Distance_State distance_state = {0};

    /* set real-time priority */
    set_thread_priority(DISPLAY_PRIO);

    /* initialize by setting first light on */
    if(stop == 0){
        pin_io_set(
            args->pin_io_handle,
            note_get_led_pin_id(current_index),
            PIN_ON
        );
    }

    /*
     * initialize the next time to now, so when we
     * add in the period later, it is accurate
     */
    get_current_time(&next);

    while(stop == 0){
        get_current_time(&timestamp);
        logging_state_shared_entry(
            args->logging_state_shared_handle,
            "display thread running",
            &timestamp
        );

        /* pull latest failsafe */
        prev_failsafe = failsafe;
        err = failsafe_state_shared_get(
            args->failsafe_state_shared_handle,
            &failsafe
        );

        if(!failsafe){
            /* not failsafe: normal operation */
            update_display(
                args,
                &current_index,
                &distance_state
            );
            if(prev_failsafe){
                /*
                 * first update not in failsafe;
                 * reset current light; put after
                 * normal update to avoid weird
                 * flickering
                 */
                pin_io_set(
                    args->pin_io_handle,
                    note_get_led_pin_id(current_index),
                    PIN_ON
                );
            }
        }
        if(failsafe && !prev_failsafe){
            /* first update of failsafe: turn off */
            pin_io_set(
                args->pin_io_handle,
                note_get_led_pin_id(current_index),
                PIN_OFF
            );
        }

        /* advance to next 20 Hz tick */
        time_add_ns(&next, DISPLAY_PERIOD_NS);

        /* sleep until next scheduled wakeup time */
        sleep_absolute(&next);
    }

    /* if we are here, stop has been flagged */

    args->err_out = err;

    #ifdef EMU
    printf("THREAD EXIT: display\n");
    #endif

    return NULL;
}
#include "console_thread.h"

#include <stdio.h>

#include "stop.h"
#include "priority.h"

/*
 * This file implements the console thread, which is
 * responsible for reading console input into the
 * system.
 */

#ifndef EMU
#define LEGAL_COMMAND_STR "{q, f, r}"
#else
#define LEGAL_COMMAND_STR \
    "{q, f, r, b, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9}"
#endif

/*
 * file-scope globals used for emulating toggling
 * button sensors
 */
#ifdef EMU
/* toggle state of the beam in pin */
static uint8_t toggle_state = 0u;

/* emulated distance change */
static ERR_TYPE set_distance(
    Distance_State_Shared_Handle handle,
    float distance_cm
){
    NOTE_INDEX_TYPE note_index
        = note_from_distance_cm(distance_cm);

    return distance_state_shared_write(
        handle,
        distance_cm,
        note_index
    );
}
#endif

/*
 * Helper function for switching on all the console
 * commands.
 */
ERR_TYPE handle_command(
    Console_Thread_Args *args,
    char command
){
    ERR_TYPE err = SUCCESS;
    uint8_t failsafe = 0u;

    switch(command){
        case '\n':
            /* do nothing */
            break;
        case 'q':
            stop = 1u;
            break;
        case 'f':
            err = failsafe_state_shared_get(
                args->failsafe_state_shared_handle,
                &failsafe
            );
            if(err == SUCCESS){
                if(failsafe == 0u){
                    err = failsafe_state_shared_set(
                        args->failsafe_state_shared_handle,
                        1u
                    );
                    if(err == SUCCESS){
                        (void)logging_state_shared_message(
                            args->logging_state_shared_handle,
                            "FAILSAFE: manual failsafe"
                        );
                    }
                }
                else{
                    (void)logging_state_shared_message(
                        args->logging_state_shared_handle,
                        "NOTE: The system is already in "
                        "failsafe mode"
                    );
                }
            }
            
            else{
                /* if we were unable to failsafe */
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "FATAL: console thread unable to "
                    "enter failsafe"
                );
                stop = 1u;
            }
            break;
        case 'r':
            err = failsafe_state_shared_get(
                args->failsafe_state_shared_handle,
                &failsafe
            );
            if(err == SUCCESS){
                if(failsafe == 1u){
                    err = failsafe_state_shared_set(
                        args->failsafe_state_shared_handle,
                        0u
                    );
                    if(err == SUCCESS){
                        (void)logging_state_shared_message(
                            args->logging_state_shared_handle,
                            "FAILSAFE: manual override"
                        );
                    }
                }
                else{
                    (void)logging_state_shared_message(
                        args->logging_state_shared_handle,
                        "NOTE: The system is not in "
                        "failsafe mode"
                    );
                }
            }
            
            else{
                /* if we were unable to exit failsafe */
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "FATAL: console thread unable to "
                    "exit failsafe"
                );
                stop = 1u;
            }
            break;

        #ifdef EMU
        /* 'b' simulates toggling the beam in */
        case 'b':
            toggle_state ^= 1u;
            err = pin_io_set(
                args->pin_io_handle,
                PIN_IN_BEAM,
                toggle_state
            );
            if(err != SUCCESS){
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: failed to toggle pin"
                );
                err = SUCCESS;
            }
            else{
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: toggled pin"
                );
            }
            break;
        case '0':
            err = set_distance(
                args->distance_state_shared_handle,
                0.0f
            );
            if(err != SUCCESS){
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: failed to set dist"
                );
                err = SUCCESS;
            }
            else{
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: dist set to 0.0 cm"
                );
            }
            break;
        case '1':
            err = set_distance(
                args->distance_state_shared_handle,
                10.0f
            );
            if(err != SUCCESS){
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: failed to set dist"
                );
                err = SUCCESS;
            }
            else{
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: dist set to 10.0 cm"
                );
            }
            break;
        case '2':
            err = set_distance(
                args->distance_state_shared_handle,
                20.0f
            );
            if(err != SUCCESS){
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: failed to set dist"
                );
                err = SUCCESS;
            }
            else{
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: dist set to 20.0 cm"
                );
            }
            break;
        case '3':
            err = set_distance(
                args->distance_state_shared_handle,
                30.0f
            );
            if(err != SUCCESS){
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: failed to set dist"
                );
                err = SUCCESS;
            }
            else{
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: dist set to 30.0 cm"
                );
            }
            break;
        case '4':
            err = set_distance(
                args->distance_state_shared_handle,
                40.0f
            );
            if(err != SUCCESS){
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: failed to set dist"
                );
                err = SUCCESS;
            }
            else{
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: dist set to 40.0 cm"
                );
            }
            break;
        case '5':
            err = set_distance(
                args->distance_state_shared_handle,
                50.0f
            );
            if(err != SUCCESS){
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: failed to set dist"
                );
                err = SUCCESS;
            }
            else{
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: dist set to 50.0 cm"
                );
            }
            break;
        case '6':
            err = set_distance(
                args->distance_state_shared_handle,
                60.0f
            );
            if(err != SUCCESS){
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: failed to set dist"
                );
                err = SUCCESS;
            }
            else{
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: dist set to 60.0 cm"
                );
            }
            break;
        case '7':
            err = set_distance(
                args->distance_state_shared_handle,
                70.0f
            );
            if(err != SUCCESS){
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: failed to set dist"
                );
                err = SUCCESS;
            }
            else{
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: dist set to 70.0 cm"
                );
            }
            break;
        case '8':
            err = set_distance(
                args->distance_state_shared_handle,
                80.0f
            );
            if(err != SUCCESS){
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: failed to set dist"
                );
                err = SUCCESS;
            }
            else{
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: dist set to 80.0 cm"
                );
            }
            break;
        case '9':
            err = set_distance(
                args->distance_state_shared_handle,
                90.0f
            );
            if(err != SUCCESS){
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: failed to set dist"
                );
                err = SUCCESS;
            }
            else{
                (void)logging_state_shared_message(
                    args->logging_state_shared_handle,
                    "CONSOLE: dist set to 90.0 cm"
                );
            }
            break;
        #endif

        default:
            /* eat error if logging fails */
            (void)logging_state_shared_message(
                args->logging_state_shared_handle,
                "CONSOLE: unrecognized command; "
                "try: " LEGAL_COMMAND_STR
            );
            break;
    } /* end switch */

    return err;
}

/*
 * Thread entry point: monitors console input for
 * the control thread to see.
 */
void *console_thread_fn(void *arg){
    #define BUFFER_SIZE 64
    static char buffer[BUFFER_SIZE];

    /* 1 if okay to read input, 0 otherwise */
    uint8_t read_flag = 1u;
    Console_Thread_Args *args
        = (Console_Thread_Args *)arg;
    ERR_TYPE err = SUCCESS;
    TIME_TYPE timestamp = TIME_TYPE_INIT;

    /* set real-time priority */
    set_thread_priority(CONSOLE_PRIO);

    while(stop == 0){
        read_flag = 1u;
        if(fgets(buffer, sizeof(buffer), stdin)
            == NULL
        ){
            /* EOF or interrupted by signal */
            buffer[0] = '\0';
            read_flag = 0u;
        }
        /*
         * only single character commands are accepted
         */
        if((stop == 0)
            && (read_flag == 1u)
            && (buffer[2] != '\0')
        ){
            /* eat error if logging fails */
            (void)logging_state_shared_message(
                args->logging_state_shared_handle,
                "CONSOLE: only single character "
                "commands are accepted: "
                LEGAL_COMMAND_STR
            );
            read_flag = 0u;
        }
        if((stop == 0) && (read_flag == 1u)){
            err = handle_command(args, buffer[0]);
            get_current_time(&timestamp);
            logging_state_shared_entry(
                args->logging_state_shared_handle,
                "console thread handled command",
                &timestamp
            );
        }

        /*
         * this thread loops immediately, since its
         * slowed down by user input
         */
    } /* end while */

    args->err_out = err;

    #ifdef EMU
    printf("THREAD EXIT: console\n");
    #endif

    return NULL;

    #undef BUFFER_SIZE
}

#include "logging_thread.h"

#include "timing_config.h"
#include "priority.h"
#include "stop.h"

/*
 * Prints out stdout logs; assumes that the provided
 * logging state has been updated already.
 * Returns any errors that may occur.
 */
static ERR_TYPE print_messages(
    Logging_Thread_Args *args,
    Logging_State *logging_state
){
    ERR_TYPE err = SUCCESS;
    /* flag for if some messages were detected lost */
    uint8_t logs_lost = 0u;
    /* for loop index variable */
    uint8_t i = 0u;

    if(args == NULL){
        err = ERR_NULL_PTR;
    }
    if(logging_state == NULL){
        err = ERR_NULL_PTR;
    }
    if(err == SUCCESS){
        /* check for missing logs */
        if(logging_state->current_messages
            > MAX_MESSAGES
        ){
            logging_state->current_messages
                = MAX_MESSAGES;
            logs_lost = 1u;
        }
        /* print out backlog */
        for(i = 0u;
            i < logging_state->current_messages;
            ++i
        ){
            printf(
                "%s\n",
                logging_state->messages[i]
            );
        }

        if(logs_lost == 1u){
            printf("WARNING: messages lost\n");
        }

        fflush(stdout);
    }

    return err;
}

/*
 * Prints out a single file log. Returns any errors
 * that may occur.
 */
static ERR_TYPE print_log_entry(
    Logging_Thread_Args *args,
    Log_Entry *log_entry,
    const TIME_TYPE * const base
){
    ERR_TYPE err = SUCCESS;
    TIME_TYPE relative_timestamp = TIME_TYPE_INIT;
    NS_TYPE relative_ns = 0;
    float relative_s = 0.0f;
    int32_t retval = 0;

    /* convert timestamp to relative */
    err = time_diff(
        &(log_entry->timestamp),
        base,
        &relative_timestamp
    );
    if(err == SUCCESS){
        relative_ns = time_to_ns(
            &relative_timestamp,
            &err
        );
    }
    if(err == SUCCESS){
        relative_s = (float)relative_ns;
        relative_s /= ((float)BILLION);
        retval = fprintf(
            args->log_file,
            "[%.8f] %s\n",
            relative_s,
            log_entry->message
        );
        if(retval < 0){
            err = ERR_BAD_SYSCALL;
        }
    }
    return err;
}

/*
 * Prints out file logs; assumes that the provided
 * logging state has been updated already.
 * Returns any errors that may occur.
 */
static ERR_TYPE print_log_entries(
    Logging_Thread_Args *args,
    Logging_State *logging_state,
    const TIME_TYPE * const base
){
    ERR_TYPE err = SUCCESS;
    /* flag for if some logs were detected lost */
    uint8_t logs_lost = 0u;
    /* for loop index variable */
    uint8_t i = 0u;

    if(args == NULL){
        err = ERR_NULL_PTR;
    }
    if(logging_state == NULL){
        err = ERR_NULL_PTR;
    }
    if(err == SUCCESS){
        /* check for missing logs */
        if(logging_state->current_log_entries
            > MAX_LOG_ENTRIES
        ){
            logging_state->current_log_entries
                = MAX_LOG_ENTRIES;
            logs_lost = 1u;
        }
        /* print out backlog */
        for(i = 0u;
            i < logging_state->current_log_entries;
            ++i
        ){
            err = print_log_entry(
                args,
                logging_state->log_entries + i,
                base
            );
            if(err != SUCCESS){
                /* do not fail due to i/o errors */
                printf("WARNING: failed to log entry\n");
                fflush(stdout);
                err = SUCCESS;
            }
        }

        if(logs_lost == 1u){
            printf("WARNING: log entries lost\n");
            fflush(stdout);
        }
        fflush(args->log_file);
    }

    return err;
}

/*
 * Prints out logs. Returns any errors
 * that may occur, but attempts to perform regardless.
 */
static ERR_TYPE print_all_logs(
    Logging_Thread_Args *args,
    Logging_State *logging_state,
    const TIME_TYPE * const base
){
    ERR_TYPE err1 = SUCCESS;
    ERR_TYPE err2 = SUCCESS;

    /*
     * read in the logs to print; automatically
     * clears it in the shared state
     */
    err1 = logging_state_shared_read(
        args->logging_state_shared_handle,
        logging_state
    );
    if(err1 == SUCCESS){
        err1 = print_messages(args, logging_state);
        if(err1 != SUCCESS){
            /* error outputting to stdout */
            printf(
                "WARNING: failed to print messages"
            );
            fflush(stdout);
        }
        err2 = print_log_entries(
            args,
            logging_state,
            base
        );
        if(err2 != SUCCESS){
            /* error outputting to stdout */
            printf(
                "WARNING: failed to print log entries"
            );
            fflush(stdout);
        }
        /*
         * no need to clear our local copy because
         * it will be overwritten in the next
         * iteration
         */
    }
    else{
        /* error due to logging state read */
        printf(
            "WARNING: failed to read logging state"
        );
        fflush(stdout);
        /*
         * do not end program because of this;
         * logging is not as important
         */
    }

    /* return the first non-SUCCESS code encountered */
    if(err1 == SUCCESS){
        err1 = err2;
    }
    return err1;
}

/*
 * Thread entry point: outputs logs to stdout
 * concurrently to other threads
 */
void *logging_thread_fn(void *arg){
    Logging_Thread_Args *args
        = (Logging_Thread_Args *)arg;
    Logging_State logging_state = {0};
    TIME_TYPE next = TIME_TYPE_INIT;
    TIME_TYPE timestamp = TIME_TYPE_INIT;

    /* used for logging timestamps */
    TIME_TYPE base = TIME_TYPE_INIT;

    ERR_TYPE last_err = SUCCESS;
    ERR_TYPE err = SUCCESS;

    /* set real-time priority */
    set_thread_priority(LOGGING_PRIO);

    /*
     * initialize the next time to now, so when we
     * add in the period later, it is accurate
     */
    get_current_time(&next);
    base = next;

    while(stop == 0){
        get_current_time(&timestamp);
        logging_state_shared_entry(
            args->logging_state_shared_handle,
            "logging thread running",
            &timestamp
        );

        err = print_all_logs(args, &logging_state, &base);
        if(err != SUCCESS){
            last_err = err;
        }
        
        /* advance to next 10 Hz tick */
        time_add_ns(&next, LOGGING_PERIOD_NS);

        /* sleep until next scheduled wakeup time */
        sleep_absolute(&next);
    }

    /* if we are here, stop has been flagged */

    /*
     * print out any final logs that might come from
     * the thread that signals stop
     */
    err = print_all_logs(args, &logging_state, &base);
    if(err != SUCCESS){
        last_err = err;
    }

    args->err_out = last_err;

    #ifdef EMU
    printf("THREAD EXIT: logging\n");
    #endif

    return NULL;
}
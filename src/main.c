#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#ifdef EMU
#include <sys/utsname.h>
#endif

#include "error_type.h"
#include "gpio_bbb.h"
#include "gpio_common.h"
#include "stop.h"
#include "distance_state.h"
#include "failsafe_state.h"
#include "logging_state.h"
#include "beam_thread.h"
#include "distance_thread.h"
#include "display_thread.h"
#include "logging_thread.h"
#include "console_thread.h"
#include "notes.h"
#include "timing.h"
#include "pin_io.h"
#include "thread_launched_flags.h"
#include "timing_config.h"

/*
 * The main thread of the air piano, handles setup and
 * cleanup
 */

/* Holds variables used by main thread */
typedef struct {
    /* cross-thread interfaces */
    Distance_State_Shared_Handle
        distance_state_shared_handle;
    Failsafe_State_Shared_Handle
        failsafe_state_shared_handle;
    Logging_State_Shared_Handle
        logging_state_shared_handle;
    
    /* hardware access */
    Pin_Io_Handle pin_io_handle;

    /* threads */
    pthread_t beam_tid;
    pthread_t distance_tid;
    pthread_t display_tid;
    pthread_t logging_tid;
    pthread_t console_tid;

    /* flags for which threads were launched */
    THREAD_LAUNCHED_FLAGS_TYPE thread_launched_flags;

    /* thread args */
    Beam_Thread_Args     beam_args;
    Distance_Thread_Args distance_args;
    Display_Thread_Args  display_args;
    Logging_Thread_Args  logging_args;
    Console_Thread_Args  console_args;

    /* used for logging */
    FILE *log_file;

    /* user input */
    NS_TYPE note_duration;
} Main_Struct;

/* The instance of the main struct */
static Main_Struct main_struct = {0};

/*
 * Installs SIGINT and SIGTERM handlers.
 * Returns SUCCESS or ERR_BAD_SYSCALL.
 */
static ERR_TYPE install_handlers(void)
{
    ERR_TYPE ret = SUCCESS;
    struct sigaction sa;
    /* from stop.h */
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    /* no SA_RESTART so sleeps can be interrupted */
    sa.sa_flags = 0;

    if(sigaction(SIGINT, &sa, NULL) != 0){
        ret = ERR_BAD_SYSCALL;
    }
    if(ret == SUCCESS
        && sigaction(SIGTERM, &sa, NULL) != 0
    ){
        ret = ERR_BAD_SYSCALL;
    }
    return ret;
}

/*
 * Sleeps for the specified amount of time in ns.
 * Implements timing.h.
 */
ERR_TYPE sleep_relative_ns(NS_TYPE sleep_time_ns){
    ERR_TYPE err = SUCCESS;
    TIME_TYPE delay = TIME_TYPE_INIT;

    if(sleep_time_ns < 0){
        err = ERR_NULL_PTR;
    }
    if(err == SUCCESS){
        err = ns_to_time(sleep_time_ns, &delay); 
    }
    if(err == SUCCESS){
        /* delay now contains our sleep duration */
        while(!stop
            && clock_nanosleep(
                    CLOCK_MONOTONIC,
                    0, /* 0 for relative sleep */
                    &delay,
                    &delay
                ) == -1
            && errno == EINTR
        ){
            /*
             * delay updated with remaining time by
             * nanosleep
             */
        }
    }
    return err;
}

/*
 * Sleeps until the specified time point. Implements
 * timing.h
 */
ERR_TYPE sleep_absolute(
    const TIME_TYPE *sleep_until
){
    ERR_TYPE err = SUCCESS;

    if(err == SUCCESS){
        /* delay now contains our sleep duration */
        while(!stop
            && clock_nanosleep(
                    CLOCK_MONOTONIC,
                    TIMER_ABSTIME, /* abs sleep */
                    sleep_until,
                    NULL /* no need to write remaining */
                ) == -1
            && errno == EINTR
        ){
            /* do nothing */
        }
    }
    return err;
}

#ifdef EMU
/* Prints the system and machine name */
ERR_TYPE print_uts_info(void)
{
    struct utsname info;    
    
    if (uname(&info) == -1) {    
        return ERR_UNAME;
    }    

    printf("System name: %s\n", info.sysname);    
    printf("Machine:     %s\n", info.machine);    

    return SUCCESS;
}
#endif

/* opens log file; returns non-SUCCESS on failure */
static ERR_TYPE open_log_file(FILE **log_file_out){
    ERR_TYPE err = SUCCESS;
    FILE *log_file = NULL;

    if(log_file_out == NULL){
        err = ERR_NULL_PTR;
    }
    if(err == SUCCESS){
        log_file = fopen("air_piano.log", "w");
        if(log_file == NULL){
            err = ERR_BAD_SYSCALL;
        }
    }
    if(err == SUCCESS){
        *log_file_out = log_file;
    }

    return err;
}

/*
 * queries user for note duration; returns non-SUCCESS
 * on failure
 */
static ERR_TYPE query_note_duration(
    NS_TYPE *note_duration_out
){
    #define BUFFER_SIZE 32

    /* char buffer used for user input */
    static char buffer[BUFFER_SIZE] = {0};
    /* using int32_t to match sscanf() */
    int32_t  num_inputs = 0;
    float    input_duration = 0.0f;
    NS_TYPE  note_duration = 0;
    uint8_t  is_input_valid = 0u;

    ERR_TYPE err = SUCCESS;

    if(note_duration_out == NULL){
        err = ERR_NULL_PTR;
    }
    while(err == SUCCESS && is_input_valid == 0u){
        printf("Enter note duration (s):\n");
        if(fgets(buffer, sizeof(buffer), stdin) == NULL){
            err = ERR_BAD_INPUT;
        }
        if(err == SUCCESS){
            num_inputs = sscanf(
                buffer,
                "%f",
                &input_duration
            );
            if(num_inputs != 1){
                (void)printf("Failed to parse input\n");
                is_input_valid = 0u;
            }
            else{
                note_duration = (NS_TYPE)
                    (input_duration * ((float)BILLION));
                if(note_duration < MIN_NOTE_DURATION
                    || note_duration > MAX_NOTE_DURATION
                ){
                    is_input_valid = 0u;
                    (void)printf(
                        "Minimum 0.05s, maximum 1s\n"
                    );
                }
                else{
                    is_input_valid = 1u;
                }
            }
        }
    } /* end while loop */

    if(err == SUCCESS){
        *note_duration_out = note_duration;
    }

    return err;

    #undef BUFFER_SIZE
}

/* closes log file; returns non-SUCCESS on failure */
static ERR_TYPE close_log_file(FILE **log_file){
    ERR_TYPE err = SUCCESS;
    int32_t retval = 0;

    if(log_file == NULL){
        err = ERR_NULL_PTR;
    }
    if(err == SUCCESS){
        if(*log_file == NULL){
            err = ERR_NULL_PTR;
        }
    }
    if(err == SUCCESS){
        retval = fclose(*log_file);
        if(retval != 0){
            err = ERR_BAD_SYSCALL;
        }
        *log_file = NULL;
    }
    
    return err;
}

/* launches all (other) threads */
static ERR_TYPE launch_threads(){
    int32_t retcode = 0;
    ERR_TYPE err = SUCCESS;

    /* set up arg structs */
    main_struct.beam_args.pin_io_handle
        = main_struct.pin_io_handle;
    main_struct.beam_args.distance_state_shared_handle
        = main_struct.distance_state_shared_handle;
    main_struct.beam_args.failsafe_state_shared_handle
        = main_struct.failsafe_state_shared_handle;
    main_struct.beam_args.logging_state_shared_handle
        = main_struct.logging_state_shared_handle;
    main_struct.beam_args.note_duration
        = main_struct.note_duration;
    
    main_struct.distance_args.pin_io_handle
        = main_struct.pin_io_handle;
    main_struct.distance_args
        .distance_state_shared_handle
            = main_struct.distance_state_shared_handle;
    main_struct.distance_args
        .failsafe_state_shared_handle
            = main_struct.failsafe_state_shared_handle;
    main_struct.distance_args
        .logging_state_shared_handle
            = main_struct.logging_state_shared_handle;

    main_struct.display_args.pin_io_handle
        = main_struct.pin_io_handle;
    main_struct.display_args
        .distance_state_shared_handle
            = main_struct.distance_state_shared_handle;
    main_struct.display_args
        .failsafe_state_shared_handle
            = main_struct.failsafe_state_shared_handle;
    main_struct.display_args
        .logging_state_shared_handle
            = main_struct.logging_state_shared_handle;

    main_struct.logging_args
        .logging_state_shared_handle
            = main_struct.logging_state_shared_handle;
    main_struct.logging_args.log_file
        = main_struct.log_file;

    main_struct.console_args
        .logging_state_shared_handle
            = main_struct.logging_state_shared_handle;
    main_struct.console_args
        .failsafe_state_shared_handle
            = main_struct.failsafe_state_shared_handle;
    #ifdef EMU
    main_struct.console_args.pin_io_handle
        = main_struct.pin_io_handle;
    main_struct.console_args
        .distance_state_shared_handle
            = main_struct.distance_state_shared_handle;
    #endif

    main_struct.thread_launched_flags
        = THREAD_LAUNCHED_FLAGS_INIT;

    /* launch all threads */

    if(err == SUCCESS){
        /* launch beam thread */
        retcode = pthread_create(
            &(main_struct.beam_tid),
            NULL,   /* attributes */
            beam_thread_fn,
            &(main_struct.beam_args)
        );
        if(retcode != 0){
            err = ERR_BAD_SYSCALL;
        }
        else{
            main_struct.thread_launched_flags
                = set_beam_thread_launched(
                    main_struct.thread_launched_flags
                );
        }
    }
    if(err == SUCCESS){
        /* launch distance thread */
        retcode = pthread_create(
            &(main_struct.distance_tid),
            NULL,   /* attributes */
            distance_thread_fn,
            &(main_struct.distance_args)
        );
        if(retcode != 0){
            err = ERR_BAD_SYSCALL;
        }
        else{
            main_struct.thread_launched_flags
                = set_distance_thread_launched(
                    main_struct.thread_launched_flags
                );
        }
    }
    if(err == SUCCESS){
        /* launch display thread */
        retcode = pthread_create(
            &(main_struct.display_tid),
            NULL,   /* attributes */
            display_thread_fn,
            &(main_struct.display_args)
        );
        if(retcode != 0){
            err = ERR_BAD_SYSCALL;
        }
        else{
            main_struct.thread_launched_flags
                = set_display_thread_launched(
                    main_struct.thread_launched_flags
                );
        }
    }
    if(err == SUCCESS){
        /* launch logging thread */
        retcode = pthread_create(
            &(main_struct.logging_tid),
            NULL,   /* attributes */
            logging_thread_fn,
            &(main_struct.logging_args)
        );
        if(retcode != 0){
            err = ERR_BAD_SYSCALL;
        }
        else{
            main_struct.thread_launched_flags
                = set_logging_thread_launched(
                    main_struct.thread_launched_flags
                );
        }
    }
    if(err == SUCCESS){
        /* launch console thread */
        retcode = pthread_create(
            &(main_struct.console_tid),
            NULL,   /* attributes */
            console_thread_fn,
            &(main_struct.console_args)
        );
        if(retcode != 0){
            err = ERR_BAD_SYSCALL;
        }
        else{
            main_struct.thread_launched_flags
                = set_console_thread_launched(
                    main_struct.thread_launched_flags
                );
        }
    }

    return err;
}

/*
 * Performs initialization for the air piano; returns
 * a non-SUCCESS error code on failure.
 */
static ERR_TYPE main_init(void){
    ERR_TYPE err = SUCCESS;

    /* install signal handler for ctrl-c */
    if(err == SUCCESS){
        err = install_handlers();
    }

    /* initialize fields of main struct */

    if(err == SUCCESS){
        main_struct.pin_io_handle = pin_io_init(&err);
    }
    if(err == SUCCESS){
        main_struct.distance_state_shared_handle
            = distance_state_shared_init(&err);
    }
    if(err == SUCCESS){
        main_struct.failsafe_state_shared_handle
            = failsafe_state_shared_init(&err);
    }
    if(err == SUCCESS){
        main_struct.logging_state_shared_handle
            = logging_state_shared_init(&err);
    }

    if(err == SUCCESS){
        err = open_log_file(&(main_struct.log_file));
    }
    if(err == SUCCESS){
        err = query_note_duration(
            &(main_struct.note_duration)
        );
    }

    #ifdef EMU
    if (err == SUCCESS){
        err = print_uts_info();
    }
    #endif

    if(err == SUCCESS){
        err = launch_threads();
    }

    return err;
}

/*
 * Performs cleanup for the air piano.
 * Tries to clean up as much as possible. Returns the
 * last error code received if any cleanup
 * functionality fails.
 */
static ERR_TYPE main_cleanup(void){
    ERR_TYPE err = SUCCESS;
    ERR_TYPE last_err = SUCCESS;
    int retcode = 0;

    /*
     * main thread has no job until stop signaled,
     * and other threads stop; wait for them
     */
    if(is_beam_thread_launched(
        main_struct.thread_launched_flags) != 0
    ){
        retcode = pthread_join(
            main_struct.beam_tid,
            NULL
        );
        if(retcode != 0){
            err = ERR_BAD_SYSCALL;
        }
        else{
            err = main_struct.beam_args.err_out;
        }
    }
    if(err != SUCCESS){
        last_err = err;
    }
    if(is_distance_thread_launched(
        main_struct.thread_launched_flags) != 0
    ){
        retcode = pthread_join(
            main_struct.distance_tid,
            NULL
        );
        if(retcode != 0){
            err = ERR_BAD_SYSCALL;
        }
        else{
            err = main_struct.distance_args.err_out;
        }
    }
    if(err != SUCCESS){
        last_err = err;
    }
    if(is_display_thread_launched(
        main_struct.thread_launched_flags) != 0
    ){
        retcode = pthread_join(
            main_struct.display_tid,
            NULL
        );
        if(retcode != 0){
            err = ERR_BAD_SYSCALL;
        }
        else{
            err = main_struct.display_args.err_out;
        }
    }
    if(err != SUCCESS){
        last_err = err;
    }
    if(is_logging_thread_launched(
        main_struct.thread_launched_flags) != 0
    ){
        retcode = pthread_join(
            main_struct.logging_tid,
            NULL
        );
        if(retcode != 0){
            err = ERR_BAD_SYSCALL;
        }
        else{
            err = main_struct.logging_args.err_out;
        }
    }
    if(err != SUCCESS){
        last_err = err;
    }
    if(is_console_thread_launched(
        main_struct.thread_launched_flags) != 0
    ){
        retcode = pthread_join(
            main_struct.console_tid,
            NULL
        );
        if(retcode != 0){
            err = ERR_BAD_SYSCALL;
        }
        else{
            err = main_struct.console_args.err_out;
        }
    }
    if(err != SUCCESS){
        last_err = err;
    }

    /* zero memory just in case (memset cannot fail) */
    main_struct.thread_launched_flags = 0u;
    (void)memset(
        &(main_struct.beam_args),
        0,
        sizeof(main_struct.beam_args)
    );
    (void)memset(
        &(main_struct.distance_args),
        0,
        sizeof(main_struct.distance_args)
    );
    (void)memset(
        &(main_struct.display_args),
        0,
        sizeof(main_struct.display_args)
    );
    (void)memset(
        &(main_struct.logging_args),
        0,
        sizeof(main_struct.logging_args)
    );
    (void)memset(
        &(main_struct.console_args),
        0,
        sizeof(main_struct.console_args)
    );

    /* clean up resources */
    
    err = distance_state_shared_cleanup(
        main_struct.distance_state_shared_handle
    );
    if(err != SUCCESS){
        last_err = err;
    }
    err = failsafe_state_shared_cleanup(
        main_struct.failsafe_state_shared_handle
    );
    if(err != SUCCESS){
        last_err = err;
    }
    err = logging_state_shared_cleanup(
        main_struct.logging_state_shared_handle
    );
    if(err != SUCCESS){
        last_err = err;
    }
    err = pin_io_cleanup(
        main_struct.pin_io_handle
    );
    if(err != SUCCESS){
        last_err = err;
    }
    err = close_log_file(
        &(main_struct.log_file)
    );
    if(err != SUCCESS){
        last_err = err;
    }

    return last_err;
}

/* Main thread for the air piano */
int main(void)
{
    int ret = 0;
    ERR_TYPE err = SUCCESS;

    if(err == SUCCESS){
        err = main_init();
        if(err != SUCCESS){
            printf("init err: %s\n", err_to_str(err));
            /*
             * signal stop just in case any threads
             * got launched
             */
            stop = 1u;
            ret = 1;
        }
    }

    /* both success and non-success path do cleanup */
    err = main_cleanup();
    if(err != SUCCESS){
        printf("cleanup err: %s\n", err_to_str(err));
        ret = 1;
    }

    #ifdef EMU
    printf("THREAD EXIT: main\n");
    #endif

    return ret;
}
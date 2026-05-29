#include "stop.h"

#include <stdio.h>

/*
 * This file provides definitions for the global stop
 * variable.
 */

/* definition of the stop variable */
volatile sig_atomic_t stop = 0;

/* The signal handler that flags stop */
void handle_sigint(int signo){
    (void)signo;
    printf(
        "SIGINT detected: you may need to "
        "press <Enter> to exit the program.\n"
        "It is recommended to exit this program "
        "with the command 'q'.\n"
    );
    stop = 1;
}
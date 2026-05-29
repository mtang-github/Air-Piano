#ifndef STOP_H
#define STOP_H

#include <signal.h>

/*
 * This file provides access to the global stop
 * variable, which indicates to any thread whether the
 * system has been shut down yet.
 */

/*
 * The global stop variable, indicating whether the
 * system has been shut down yet. 0 if false, non-zero
 * if true.
 */
extern volatile sig_atomic_t stop;

/* The signal handler that flags stop */
void handle_sigint(int signo);

#endif /* STOP_H */
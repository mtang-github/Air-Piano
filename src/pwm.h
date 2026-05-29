#ifndef PWM_H
#define PWM_H

#include "error_type.h"
#include "notes.h"

/*
 * This file contains the implementation-independent
 * interface for buzzer PWM for the air piano
 */

/*
 * Plays the given note on the buzzer.
 * Sets period and duty cycle then enables PWM output.
 */
ERR_TYPE pwm_play(NOTE_INDEX_TYPE note_index);

/* Disables PWM output (silence). */
ERR_TYPE pwm_stop(void);

#endif /* PWM_H */

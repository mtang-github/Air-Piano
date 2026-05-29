#include "pwm.h"    
    
#include <stddef.h>    
#include <stdio.h>    

#include "timing.h"
    
/* Emulator PWM: print PWM state changes to stdout */

/*
 * Plays the given note on the buzzer.
 * Sets period and duty cycle then enables PWM output.
 */
ERR_TYPE pwm_play(NOTE_INDEX_TYPE note_index){

    ERR_TYPE err = SUCCESS;
    NS_TYPE period_ns = 0;
    NS_TYPE duty_ns = 0;

    if(note_index > NOTE_INDEX_MAX_INCL){
        err = ERR_BAD_ARG;
    }

    if(err == SUCCESS){
        period_ns = note_get_period_ns(note_index);
        duty_ns = note_get_duty_ns(note_index);

        const char *note_name
            = note_get_name(note_index);
        (void)printf(
            "PWM toggled on: %s (%lld, %lld)\n",
            note_name,
            period_ns,
            duty_ns
        );
    }
    return err;
}

/* Disables PWM output (silence). */
ERR_TYPE pwm_stop(void){
    (void)printf("PWM toggled off\n");
    return SUCCESS;
}
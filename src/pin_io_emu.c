#include "pin_io.h"    
    
#include <stddef.h>    
#include <stdio.h>    
    
#include "gpio_common.h"    

/* Emulator pin-out: print LED state changes to stdout */

/*
 * Table associating PIN_ID_TYPE with a readable string
 */
static const char *const pin_id_str[] = {
    "Trigger Out",
    "LED C4 Out",
    "LED D4 Out",
    "LED E4 Out",
    "LED F4 Out",
    "LED G4 Out",
    "LED A4 Out",
    "LED B4 Out",
    "LED C5 Out",
    "Beam In",
    "Echo In"
};

struct Pin_Io {
    /* Emulated pin states */
    PIN_STATE_TYPE   pin_states[NUM_GPIO_PINS];
    struct Gpio_Map  gpio_map;
    uint8_t          in_use;
};

static struct Pin_Io pin_io_instance = {0};

/*
 * Initializes the pin driver, and returns a handle to
 * it. Sets err_out on failure. Returns NULL on error.
 */
Pin_Io_Handle pin_io_init(ERR_TYPE * const err_out)
{
    ERR_TYPE err = SUCCESS;
    Pin_Io_Handle ret = NULL;

    if(pin_io_instance.in_use != 0u){
        err = ERR_CALLED_TWICE;
    }

    /* query gpio pins */
    if (err == SUCCESS){
        err = query_gpio(&pin_io_instance.gpio_map);
    }

    if(err == SUCCESS){
        pin_io_instance.in_use = 1u;
        ret = &pin_io_instance;
    }

    if(err_out != NULL){
        *err_out = err;
    }
    return ret;
}

/*
 * Sets the given output pin to PIN_ON or PIN_OFF.
 * Returns non-SUCCESS on error, including if an input
 * pin id was provided.
 */
ERR_TYPE pin_io_set(
    Pin_Io_Handle  handle,
    PIN_ID_TYPE    pin_id,
    PIN_STATE_TYPE state
){
    ERR_TYPE err = SUCCESS;
    const char *pin_state_str = NULL;
    if(handle == NULL){
        err = ERR_NULL_PTR;
    }
    else {
        if(handle->in_use == 0u){
            err = ERR_NOT_IN_USE;
        }
    }

    /* case 1: emulate pin out set */
    if(err == SUCCESS
        && pin_id >= PIN_OUT_MIN_INCL
        && pin_id <= PIN_OUT_MAX_INCL
    ){
        if (state == PIN_OFF){
            pin_state_str = "Off";
        }
        else if (state == PIN_ON){
            pin_state_str = "On";
        }
        else{
            err = ERR_INVALID_PIN_STATE;
        }

        /* don't print out trig because spam */
        if(err == SUCCESS
            && pin_id != PIN_OUT_TRIG
        ){
            GPIO_TYPE gpio
                = handle->gpio_map.map_array[pin_id];
            handle->pin_states[pin_id] = state;
            printf(
                "Pin %s (GPIO %d) set to %s\n", 
                pin_id_str[pin_id], 
                gpio,
                pin_state_str
            );
        }
    }

    /* case 2: we are simulating changing in */
    else if(err == SUCCESS
        && pin_id >= PIN_IN_MIN_INCL
        && pin_id <= PIN_IN_MAX_INCL
    ){
        if (state == PIN_OFF){
            pin_state_str = "Off";
        }
        else if (state == PIN_ON){
            pin_state_str = "On";
        }
        else{
            err = ERR_INVALID_PIN_STATE;
        }

        if(err == SUCCESS){
            GPIO_TYPE gpio
                = handle->gpio_map.map_array[pin_id];
            handle->pin_states[pin_id] = state;
            printf(
                "Simulating pin %s (GPIO %d) "
                "set to %s\n", 
                pin_id_str[pin_id], 
                gpio,
                pin_state_str
            );
        }
    }

    else{
        /*
         * if we are here, either failed
         * or bad pin id
         */
        if(err == SUCCESS){
            /*
             * if its not because we failed,
             * a bad pin id was provided
             */
            err = ERR_INVALID_PIN_ID; 
        }
    }

    return err;
}

/*
 * Reads the current state of the given input pin.
 * Writes PIN_ON or PIN_OFF into state_out. Returns
 * non-SUCCESS on error, including if an output pin id
 * was provided.
 */
ERR_TYPE pin_io_read(
    Pin_Io_Handle          handle,
    PIN_ID_TYPE            pin_id,
    PIN_STATE_TYPE * const state_out
){
    ERR_TYPE err = SUCCESS;

    if(state_out == NULL){
        err = ERR_NULL_PTR;
    }
    if(handle == NULL){
        err = ERR_NULL_PTR;
    }
    else {
        if(handle->in_use == 0u){
            err = ERR_NOT_IN_USE;
        }
    }

    /* sanity check bounds */
    if(pin_id < PIN_IN_MIN_INCL
        || pin_id > PIN_IN_MAX_INCL
    ){
        err = ERR_INVALID_PIN_ID;
    }

    if(err == SUCCESS){
        *state_out = handle->pin_states[pin_id];
    }

    return err;
}

/* Cleans up the output pin driver. */
ERR_TYPE pin_io_cleanup(Pin_Io_Handle handle)
{
    ERR_TYPE err = SUCCESS;
    PIN_ID_TYPE i = 0u;

    if(handle == NULL){
        err = ERR_NULL_PTR;
    }
    else {
        if(handle->in_use == 0u){
            err = ERR_NOT_IN_USE;
        }
    }

    if(err == SUCCESS){
        /* clear all outputs */
        for(i = PIN_OUT_MIN_INCL;
            i <= PIN_OUT_MAX_INCL;
            ++i
        ){
            (void)printf(
                "Pin %s (GPIO %u) reset\n", 
                pin_id_str[i],
                handle->gpio_map.map_array[i]
            );
        };
        handle->in_use = 0u;
    }

    return err;
}
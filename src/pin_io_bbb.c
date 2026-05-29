#include "pin_io.h"

#include <stddef.h>
#include <stdio.h>

#include "gpio_common.h"
#include "gpio_bbb.h"

/*
 * BBB mmap pin I/O: LEDs via gpio_bbb_set/clear,
 * buttons via gpio_bbb_read (memory-mapped GPIO).
 */

struct Pin_Io {
    Gpio_Bbb_Handle  gpio_handle;
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

    /* init gpio handle */
    if(err == SUCCESS){
        pin_io_instance.gpio_handle = gpio_bbb_init(&err);
    }

    /* set output pins as output */
    {
        PIN_ID_TYPE i;
        for(i = PIN_OUT_MIN_INCL;
            i <= PIN_OUT_MAX_INCL && err == SUCCESS;
            ++i
        ){
            err = gpio_bbb_set_dir(
                pin_io_instance.gpio_handle,
                pin_io_instance.gpio_map.map_array[i],
                GPIO_BBB_OUTPUT
            );
        }
    }

    /* set input pins as input */
    if(err == SUCCESS){
        err = gpio_bbb_set_dir(
            pin_io_instance.gpio_handle,
            pin_io_instance.gpio_map.map_array[PIN_IN_BEAM],
            GPIO_BBB_INPUT
        );
    }
    if(err == SUCCESS){
        err = gpio_bbb_set_dir(
            pin_io_instance.gpio_handle,
            pin_io_instance.gpio_map.map_array[PIN_IN_ECHO],
            GPIO_BBB_INPUT
        );
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

    if(handle == NULL){
        err = ERR_NULL_PTR;
    }
    else {
        if(handle->in_use == 0u){
            err = ERR_NOT_IN_USE;
        }
    }
    /* sanity check bounds */
    if(pin_id > PIN_OUT_MAX_INCL){
        err = ERR_INVALID_PIN_ID;
    }
    if(state > PIN_STATE_MAX_INCL){
        err = ERR_INVALID_PIN_STATE;
    }

    if(err == SUCCESS){
        GPIO_TYPE gpio = handle->gpio_map.map_array[pin_id];
        if(state == PIN_ON){
            err = gpio_bbb_set(handle->gpio_handle, gpio);
        }
        else{
            err = gpio_bbb_clear(handle->gpio_handle, gpio);
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
    Pin_Io_Handle handle,
    PIN_ID_TYPE pin_id,
    PIN_STATE_TYPE * const state_out
){
    ERR_TYPE err = SUCCESS;
    uint8_t raw = 0u;

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
        err = gpio_bbb_read(
            handle->gpio_handle,
            handle->gpio_map.map_array[pin_id],
            &raw
        );
    }
    if(err == SUCCESS){
        *state_out = (raw == 0u) ? PIN_OFF : PIN_ON;
    }

    return err;
}

/*
 * Cleans up the output pin driver. This implementation
 * clears all outputs
 */
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
        /* clear outputs */
        for(i = PIN_OUT_MIN_INCL;
            i <= PIN_OUT_MAX_INCL;
            ++i
        ){
            err = gpio_bbb_clear(
                handle->gpio_handle,
                handle->gpio_map.map_array[i]
            );
        };

        /* unmap gpio */
        err = gpio_bbb_cleanup(handle->gpio_handle);
        handle->in_use = 0u;
    }

    #ifdef EMU
    printf("pin io cleanup\n");
    #endif

    return err;
}

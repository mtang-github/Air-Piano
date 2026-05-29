#include "gpio_common.h"
#include <stdio.h>
#include <string.h>
#include "error_type.h"

/*
 * A struct with enough space to store user input for
 * GPIO numbers. Only visible in this file.
 */
typedef struct {
    /*
     * unsigned int array used for temporary storage
     * after sscanf()
     */
    unsigned int gpio_input_array[NUM_GPIO_PINS];
} Gpio_Input;

/*
 * Given a struct with arrays containing IN and OUT
 * gpio pins, returns 1 if the gpio input is valid,
 * 0 otherwise.
 */
static uint8_t is_gpio_input_valid(
    const Gpio_Input * const gpio_input,
    ERR_TYPE * const err_out
){
    GPIO_TYPE i = 0u;
    GPIO_TYPE j = 0u;
    uint8_t ret = 1u;
    uint8_t found_duplicate = 0u;
    ERR_TYPE err = SUCCESS;

    /*
     * only possible error is if we get passed null;
     * if the pointer is wrongly sized, that's the
     * caller's problem
     */
    if(gpio_input == NULL){
        err = ERR_NULL_PTR;
    }

    if(err == SUCCESS){
        /*
         * check if each gpio value is small enough
         * to fit within GPIO_TYPE; necessary since
         * sscanf reads in unsigned ints
         */
        for(i = 0u; i < NUM_GPIO_PINS; ++i){
            if(gpio_input->gpio_input_array[i]
                > GPIO_TYPE_MAX_INCL
            ){
                ret = 0u;
                break;
            }
        }

        /*
         * Make sure there are no duplicates, as that
         * would mess up GPIO and semantically make no
         * sense
         */
        for(i = 0u; i < NUM_GPIO_PINS; ++i){
            j = (GPIO_TYPE)(i + 1u);
            while((j < NUM_GPIO_PINS)
                && (found_duplicate == 0u)
            ){
                if(gpio_input->gpio_input_array[i]
                    == gpio_input->gpio_input_array[j]
                ){
                    ret = 0u;
                    found_duplicate = 1u;
                }
                ++j;
            }
            if(found_duplicate != 0u){
                break;
            }
        }
    }
    else{ /* if err != SUCCESS */
        ret = 0u;
    }

    if(err_out != NULL){
        *err_out = err;
    }
    return ret;
}

/*
 * Prompts the user for GPIO numbers for the three pins:
 *   [0]  trig_out  (HC-SR04 trigger)
 *   [1]  C4 led
 *   [2]  D4 led
 *   [3]  E4 led
 *   [4]  F4 led
 *   [5]  G4 led
 *   [6]  A4 led
 *   [7]  B4 led
 *   [8]  C5 led
 *   [9]  beam_in   (IR break beam signal)
 *   [10] echo_in   (HC-SR04 echo)
 * Returns non-SUCCESS on error.
 */
ERR_TYPE query_gpio(struct Gpio_Map * const map_out){
    #define BUFFER_SIZE 128

    /* char buffer used for user input */
    static char buf[BUFFER_SIZE] = {0};
    static Gpio_Input gpio_input = {0};
    /* using int32_t to match sscanf() */
    int32_t num_inputs = 0;
    uint8_t i = 0u;
    uint8_t is_valid_pins = 0u;

    ERR_TYPE ret = SUCCESS;

    if(!map_out){
        ret = ERR_NULL_PTR;
    }

    while(ret == SUCCESS && (is_valid_pins == 0u)){
        memset(&gpio_input, 0, sizeof(gpio_input));
        (void)printf(
            "Enter GPIO pins ("
                "trig_out "
                "C4_led_out "
                "D4_led_out "
                "E4_led_out "
                "F4_led_out "
                "G4_led_out "
                "A4_led_out "
                "B4_led_out "
                "C5_led_out "
                "beam_in "
                "echo_in"
            "):\n"
        );
        if(fgets(buf, sizeof(buf), stdin) == NULL){
            ret = ERR_BAD_INPUT;
        }
        if(ret == SUCCESS){
            num_inputs = sscanf(
                buf,
                "%u %u %u %u %u %u %u %u %u %u %u",
                gpio_input.gpio_input_array + 0,
                gpio_input.gpio_input_array + 1,
                gpio_input.gpio_input_array + 2,
                gpio_input.gpio_input_array + 3,
                gpio_input.gpio_input_array + 4,
                gpio_input.gpio_input_array + 5,
                gpio_input.gpio_input_array + 6,
                gpio_input.gpio_input_array + 7,
                gpio_input.gpio_input_array + 8,
                gpio_input.gpio_input_array + 9,
                gpio_input.gpio_input_array + 10
            );
            if(num_inputs != NUM_GPIO_PINS){
                (void)printf("Failed to parse input\n");
                is_valid_pins = 0u;
            }
            else{
                is_valid_pins = is_gpio_input_valid(
                    &gpio_input,
                    &ret
                );
                if(ret == SUCCESS && (is_valid_pins == 0u)){
                    (void)printf(
                        "Input is invalid; check for "
                        "duplicates or values over %u\n",
                        GPIO_TYPE_MAX_INCL
                    );
                }
            }
        }
    } /* end while loop */

    /*
     * input is now gotten and validated; write output
     */
    if(ret == SUCCESS){
        for(i = 0u; i < NUM_GPIO_PINS; ++i){
            map_out->map_array[i]
                = (GPIO_TYPE)gpio_input.gpio_input_array[i];
        }
    }

    return ret;
}

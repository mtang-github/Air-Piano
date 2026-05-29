#ifndef GPIO_COMMON_H
#define GPIO_COMMON_H

#include <stdint.h>
#include "error_type.h"

/*
 * Shared GPIO utilities for all pin implementations.
 */

/* Unsigned integer type for a GPIO number */
#define GPIO_TYPE          uint8_t

#define GPIO_TYPE_MAX_INCL 127u

/* trig_out, C4-C5 LED out, beam_in, echo_in */
#define NUM_GPIO_PINS 11u

/* Maps logical pins to hardware GPIO numbers */
struct Gpio_Map {
    GPIO_TYPE map_array[NUM_GPIO_PINS];
};

/*
 * Prompts the user for GPIO numbers for all inputs and
 * outputs. Returns non-SUCCESS code on error.
 */
ERR_TYPE query_gpio(struct Gpio_Map * const map_out);

#endif /* GPIO_COMMON_H */

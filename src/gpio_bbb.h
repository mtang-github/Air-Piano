#ifndef GPIO_BBB_H
#define GPIO_BBB_H

#include <stdint.h>

#include "error_type.h"
#include "gpio_common.h"

/*
 * This file defines the interface for gpio interaction
 * on the bbb.
 */

#define GPIO_BBB_OUTPUT 0u
#define GPIO_BBB_INPUT  1u

/* Opaque pointer type for gpio bbb map */
typedef struct Gpio_Bbb *Gpio_Bbb_Handle;

/*
 * Initializes the gpio map. Should only ever be
 * called once. Returns handle if successful,
 * NULL on error.
 */
Gpio_Bbb_Handle gpio_bbb_init(
    ERR_TYPE * const err_out
);

/* Cleans up the gpio map. */
ERR_TYPE gpio_bbb_cleanup(Gpio_Bbb_Handle handle);

/*
 * Sets the direction of the specified gpio on the bbb,
 * where direction is 0 (output) or 1 (input).
 */
ERR_TYPE gpio_bbb_set_dir(
    Gpio_Bbb_Handle handle,
    GPIO_TYPE gpio,
    uint8_t dir // 0 = output, 1 = input
);

/* Sets an output pin to High */
ERR_TYPE gpio_bbb_set(
    Gpio_Bbb_Handle handle,
    GPIO_TYPE gpio
);

/* Sets an output pin to Low */
ERR_TYPE gpio_bbb_clear(
    Gpio_Bbb_Handle handle,
    GPIO_TYPE gpio
);

/* Reads the pin state */
ERR_TYPE gpio_bbb_read(
    Gpio_Bbb_Handle handle,
    GPIO_TYPE gpio,
    uint8_t* value
);

#endif /* GPIO_BBB_H */
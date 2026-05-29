#ifndef PIN_IO_H
#define PIN_IO_H

#include <stdint.h>

#include "error_type.h"

/*
 * Abstract interface for GPIO pins (LEDs and sensors).
 * Each platform provides its own implementation.
 */

/* Logical output pin IDs */
#define PIN_ID_TYPE        uint8_t

#define PIN_OUT_TRIG       0u
#define PIN_OUT_LED_C4     1u
#define PIN_OUT_LED_D4     2u
#define PIN_OUT_LED_E4     3u
#define PIN_OUT_LED_F4     4u
#define PIN_OUT_LED_G4     5u
#define PIN_OUT_LED_A4     6u
#define PIN_OUT_LED_B4     7u
#define PIN_OUT_LED_C5     8u

#define PIN_IN_BEAM        9u
#define PIN_IN_ECHO        10u

#define PIN_OUT_MIN_INCL   PIN_OUT_TRIG
#define PIN_OUT_MAX_INCL   PIN_OUT_LED_C5
#define PIN_OUT_NUM        (PIN_OUT_MAX_INCL - PIN_OUT_MIN_INCL + 1u)

#define PIN_IN_MIN_INCL    PIN_IN_BEAM
#define PIN_IN_MAX_INCL    PIN_IN_ECHO
#define PIN_IN_NUM         (PIN_IN_MAX_INCL - PIN_IN_MIN_INCL + 1u)

#define PIN_ID_MAX_INCL    PIN_IN_MAX_INCL

/* Pin state values */
#define PIN_STATE_TYPE     uint8_t
#define PIN_OFF            0u
#define PIN_ON             1u
#define PIN_STATE_MAX_INCL PIN_ON

/* Opaque handle for the pin driver */
typedef struct Pin_Io *Pin_Io_Handle;

/*
 * Initializes the pin driver, and returns a handle to
 * it. Sets err_out on failure. Returns NULL on error.
 */
Pin_Io_Handle pin_io_init(ERR_TYPE * const err_out);

/*
 * Sets the given output pin to PIN_ON or PIN_OFF.
 * Returns non-SUCCESS on error, including if an input
 * pin id was provided.
 */
ERR_TYPE pin_io_set(
    Pin_Io_Handle  handle,
    PIN_ID_TYPE    pin_id,
    PIN_STATE_TYPE state
);

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
);

/* Cleans up the output pin driver. */
ERR_TYPE pin_io_cleanup(Pin_Io_Handle handle);

#endif /* PIN_IO_H */

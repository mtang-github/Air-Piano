#ifndef ERROR_TYPE_H
#define ERROR_TYPE_H

#include <stdint.h>

/*
 * This file includes various definitions for a custom
 * error type, meant to be used to signal error
 * conditions from functions.
 */

/* The type used for passing error codes */
#define ERR_TYPE uint8_t

/* No error encountered */
#define SUCCESS          ((ERR_TYPE)0u)

/* The function should only be called once */
#define ERR_CALLED_TWICE ((ERR_TYPE)1u)

/* NULL was passed into the function inappropriately */
#define ERR_NULL_PTR     ((ERR_TYPE)2u)

/* Attempted to turn on/off an invalid pin */
#define ERR_INVALID_PIN_ID ((ERR_TYPE)3u)

/* Invalid or unexpected user input */
#define ERR_BAD_INPUT      ((ERR_TYPE)4u)

/* A sysfs GPIO file operation failed */
#define ERR_GPIO           ((ERR_TYPE)5u)

/* A syscall failed */
#define ERR_BAD_SYSCALL     ((ERR_TYPE)6u)

/* A bad value for pin state was given */
#define ERR_INVALID_PIN_STATE ((ERR_TYPE)7u)

/* GPIO number out of bounds */
#define ERR_INVALID_GPIO_NUMBER ((ERR_TYPE)8u)

/* Called structure was not in use */
#define ERR_NOT_IN_USE ((ERR_TYPE)9u)

/* Uname failure */
#define ERR_UNAME ((ERR_TYPE)10u)

/* Bad argument to function */
#define ERR_BAD_ARG ((ERR_TYPE)11u)

/* A fixed-size data structure has reached capacity */
#define ERR_DATA_STRUCT_FULL ((ERR_TYPE)12u)

/* A constant in the program is invalid */
#define ERR_BAD_CONSTANT ((ERR_TYPE)13u)

/* Overflow encountered */
#define ERR_OVERFLOW ((ERR_TYPE)14u)

/* Turns an error code into a human-readable string */
static inline const char *err_to_str(ERR_TYPE err){
    const char *str = "unknown error";
    if(err == SUCCESS){
        str = "success";
    }
    if(err == ERR_CALLED_TWICE){
        str = "called twice";
    }
    if(err == ERR_NULL_PTR){
        str = "null ptr";
    }
    if(err == ERR_INVALID_PIN_ID){
        str = "invalid pin id";
    }
    if(err == ERR_BAD_INPUT){
        str = "bad user input";
    }
    if(err == ERR_GPIO){
        str = "sysfs gpio failure";
    }
    if(err == ERR_BAD_SYSCALL){
        str = "syscall failure (perhaps sudo?)";
    }
    if(err == ERR_INVALID_PIN_STATE){
        str = "invalid pin state (neither on nor off)";
    }
    if(err == ERR_INVALID_GPIO_NUMBER){
        str = "invalid gpio number";
    }
    if(err == ERR_NOT_IN_USE){
        str = "not in use";
    }
    if(err == ERR_UNAME){
        str = "uname failure";
    }
    if(err == ERR_BAD_ARG){
        str = "bad function argument";
    }
    if(err == ERR_DATA_STRUCT_FULL){
        str = "a data structure was full";
    }
    if(err == ERR_BAD_CONSTANT){
        str = "a constant in the program is invalid";
    }
    if(err == ERR_OVERFLOW){
        str = "overflow encountered";
    }
    return str;
}

#endif

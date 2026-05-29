#include "gpio_bbb.h"

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>

/*
 * This file implements mmap gpio for the bbb.
 */

#define PAGE_SIZE 4096U

#define GPIO_NUM_MODULES 4

#define GPIO_0_BASE_PHYS 0x44E07000U
#define GPIO_1_BASE_PHYS 0x4804C000U
#define GPIO_2_BASE_PHYS 0x481AC000U
#define GPIO_3_BASE_PHYS 0x481AE000U

/* offsets for 32 bit regs within each gpio module */
#define GPIO_OE_OFFSET             0x134U
#define GPIO_DATA_IN_OFFSET        0x138U
#define GPIO_SET_DATA_OUT_OFFSET   0x194U
#define GPIO_CLEAR_DATA_OUT_OFFSET 0x190U

/* Definition for the Gpio_Bbb type */
struct Gpio_Bbb{
    /* fd for /dev/mem */
    int fd;

    /* map page base for modules 0, 1, 2, 3 */
    volatile uint8_t *map_bases[4];
    /* base + page offset for modules 0, 1, 2, 3 */
    volatile uint8_t *gpio_bases[4];

    /* 0 if not in use, non-zero otherwise */
    uint8_t in_use;
};

/*
 * The single instance of Gpio_Bbb, accessible only
 * from this file. Stores the pointers to the mmapped
 * locations. Initialized by gpio_bbb_init().
 */
static struct Gpio_Bbb gpio_bbb = {0};

/* Mutexes for protecting access to gpio modules */
static pthread_mutex_t gpio_module_mutex[4] = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER
};

/*
 * Initializes a specific gpio module.
 */
static ERR_TYPE gpio_bbb_init_module(
    Gpio_Bbb_Handle handle,
    uint8_t index,
    uint32_t base_phys
){
    ERR_TYPE ret = SUCCESS;
    uint32_t page_base = base_phys;
    page_base &= ~(PAGE_SIZE - 1U);
    uint32_t page_off  = base_phys - page_base;
    uint8_t *map_base = NULL;
    void *mmap_result = NULL;

    if(handle == NULL){
        ret = ERR_NULL_PTR;
    }
    if(ret == SUCCESS){
        if(index >= GPIO_NUM_MODULES){
            ret = ERR_INVALID_GPIO_NUMBER;
        }
    }

    if(ret == SUCCESS){
        mmap_result = mmap(
            NULL,        /* no preferred starting addr */
            PAGE_SIZE,   /* num bytes to map */
            PROT_READ | PROT_WRITE, /* mem protection flags */
            MAP_SHARED,  /* changes visible to other processes */
            gpio_bbb.fd, /* fd of file to map */
            (off_t)page_base    /* offset in file from where mapping starts */
        );
        if(mmap_result == MAP_FAILED){
            ret = ERR_BAD_SYSCALL;
            handle->map_bases[index] = NULL;
        }
        else{
            handle->map_bases[index]
                = (volatile uint8_t*)mmap_result;
        }
    }
    if(ret == SUCCESS){
        map_base = (uint8_t *)(handle->map_bases[index]
            + page_off);
        handle->gpio_bases[index] = map_base;
    }

    return ret;
}

/*
 * Initializes the gpio map. Should only ever be
 * called once. Returns handle if successful,
 * NULL on error.
 */
Gpio_Bbb_Handle gpio_bbb_init(
    ERR_TYPE * const err_out
){
    Gpio_Bbb_Handle ret = NULL;
    ERR_TYPE err = SUCCESS;

    if(gpio_bbb.in_use != 0u){
        err = ERR_CALLED_TWICE;
    }

    if(err == SUCCESS){
        gpio_bbb.fd = open(
            "/dev/mem",
            O_RDWR | O_SYNC
        );

        if(gpio_bbb.fd < 0) {
            err = ERR_BAD_SYSCALL;
        }
    }

    if(err == SUCCESS){
        err = gpio_bbb_init_module(
            &gpio_bbb,
            0,
            GPIO_0_BASE_PHYS
        );
    }

    if(err == SUCCESS){
        err = gpio_bbb_init_module(
            &gpio_bbb,
            1,
            GPIO_1_BASE_PHYS
        );
    }

    if(err == SUCCESS){
        err = gpio_bbb_init_module(
            &gpio_bbb,
            2,
            GPIO_2_BASE_PHYS
        );
    }

    if(err == SUCCESS){
        err = gpio_bbb_init_module(
            &gpio_bbb,
            3,
            GPIO_3_BASE_PHYS
        );
    }

    /* final check for success */
    if(err == SUCCESS){
        gpio_bbb.in_use = 1u;
        ret = &gpio_bbb;
    }
    else{
        (void)gpio_bbb_cleanup(&gpio_bbb);
    }

    if(err_out != NULL){
        *err_out = err;
    }
    return ret;
}

/* Helper to access 32-bit registers by offset */
static volatile uint32_t *reg32_by_offset(
    volatile uint8_t *base,
    uint32_t off
){
    return (volatile uint32_t*)(base + off);
}

/*
 * Helper to convert GPIO number to module number and
 * index 
 */
static ERR_TYPE gpio_to_module_index(
    GPIO_TYPE gpio,
    uint8_t * const module_out,
    uint8_t * const index_out
){
    ERR_TYPE ret = SUCCESS;
    if(gpio > GPIO_TYPE_MAX_INCL){
        ret = ERR_INVALID_GPIO_NUMBER;
    }
    if(module_out == NULL){
        ret = ERR_NULL_PTR;
    }
    if(index_out == NULL){
        ret = ERR_NULL_PTR;
    }
    if(ret == SUCCESS){
        *module_out = gpio / 32u;
        *index_out = gpio % 32u;
    }
    return ret;
}

/* Sets a pin to input or output */
ERR_TYPE gpio_bbb_set_dir(
    Gpio_Bbb_Handle handle,
    GPIO_TYPE gpio,
    uint8_t dir // 0 = output, 1 = input
){
    ERR_TYPE ret = SUCCESS;
    uint8_t module = 1u;
    uint8_t index = 1u;
    volatile uint8_t *base = NULL;
    volatile uint32_t *oe_reg = NULL;

    if(handle == NULL){
        ret = ERR_NULL_PTR;
    }
    
    if(ret == SUCCESS){
        ret = gpio_to_module_index(
            gpio,
            &module,
            &index
        );
    }

    if(ret == SUCCESS){
        base = handle->gpio_bases[module];
        oe_reg = reg32_by_offset(base, GPIO_OE_OFFSET);

        pthread_mutex_lock(&gpio_module_mutex[module]);

        uint32_t val = *oe_reg;
        if (dir == 1) {          
            val |= (1u << index);
        } else {        
            val &= ~(1u << index);
        }
        *oe_reg = val;

        pthread_mutex_unlock(&gpio_module_mutex[module]);
    }

    return ret;
}

/* Sets an output pin to High */
ERR_TYPE gpio_bbb_set(
    Gpio_Bbb_Handle handle,
    GPIO_TYPE gpio
){
    ERR_TYPE ret = SUCCESS;
    uint8_t module = 1u;
    uint8_t index = 1u;
    volatile uint8_t *base = NULL;
    volatile uint32_t *set_reg = NULL;

    if(handle == NULL){
        ret = ERR_NULL_PTR;
    }
    
    if(ret == SUCCESS){
        ret = gpio_to_module_index(gpio, &module, &index);
    }
    
    if(ret == SUCCESS){
        base = handle->gpio_bases[module];
        set_reg = reg32_by_offset(
            base,
            GPIO_SET_DATA_OUT_OFFSET
        );

        /* atomic */
        *set_reg = (1u << index);
    }

    return ret;
}

/* Sets an output pin to Low */
ERR_TYPE gpio_bbb_clear(
    Gpio_Bbb_Handle handle,
    GPIO_TYPE gpio
){
    ERR_TYPE ret = SUCCESS;
    uint8_t module = 1u;
    uint8_t index = 1u;
    volatile uint8_t *base = NULL;
    volatile uint32_t *clear_reg = NULL;

    if(handle == NULL){
        ret = ERR_NULL_PTR;
    }

    if(ret == SUCCESS){
        ret = gpio_to_module_index(
            gpio,
            &module,
            &index
        );
    }

    if(ret == SUCCESS){
        base = handle->gpio_bases[module];
        clear_reg = reg32_by_offset(
            base,
            GPIO_CLEAR_DATA_OUT_OFFSET
        );

        /* atomic */
        *clear_reg = (1u << index);
    }

    return ret;
}

/* Reads the pin state */
ERR_TYPE gpio_bbb_read(
    Gpio_Bbb_Handle handle,
    GPIO_TYPE gpio,
    uint8_t *value
){
    ERR_TYPE ret = SUCCESS;
    uint8_t module = 1u;
    uint8_t index = 1u;
    volatile uint8_t *base = NULL;
    volatile uint32_t *data_in = NULL;

    if(handle == NULL){
        ret = ERR_NULL_PTR;
    }
    if(value == NULL){
        ret = ERR_NULL_PTR;
    }

    if(ret == SUCCESS){
        ret = gpio_to_module_index(
            gpio,
            &module,
            &index
        );
    }
    
    if(ret == SUCCESS){
        base = handle->gpio_bases[module];
        data_in = reg32_by_offset(
            base,
            GPIO_DATA_IN_OFFSET
        );

        /* atomic */
        *value = ((*data_in >> index) & 1u);
    }

    return ret;
}

/* Cleans up the gpio map. */
ERR_TYPE gpio_bbb_cleanup(Gpio_Bbb_Handle handle){
    ERR_TYPE ret = SUCCESS;
    uint8_t i = 0u;
    volatile uint8_t *map_base = NULL;

    if(handle == NULL){
        ret = ERR_NULL_PTR;
    }

    if(handle && handle->in_use){
        /* unmap all modules */
        for(i = 0u; i < GPIO_NUM_MODULES; ++i){
            map_base = handle->map_bases[i];

            /*
             * if combine with &&, static analyzer
             * complains about side effects due to
             * volatile
             */
            if((map_base != NULL)){
                if(map_base != MAP_FAILED){
                    (void)munmap((void*)map_base, PAGE_SIZE);
                }
            }
        }

        /* close fd only after we are done unmapping */
        if(handle->fd >= 0) {
            (void)close(handle->fd);
        }

        handle->in_use = 0u;
    }

    #ifdef EMU
    printf("gpio bbb cleanup\n");
    #endif

    return ret;
}
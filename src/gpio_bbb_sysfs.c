#include "gpio_bbb.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/*
 * Sysfs-based GPIO implementation for the BBB.
 * Implements the same interface as gpio_bbb.c but uses
 * /sys/class/gpio/ instead of mmap. This avoids bus
 * errors from accessing GPIO modules whose clocks have
 * not been enabled by the kernel.
 */

#define MAX_PATH 64

struct Gpio_Bbb {
    uint8_t in_use;
};

static struct Gpio_Bbb gpio_instance = {0};

/*
 * Write a string to a sysfs file. Returns SUCCESS or
 * ERR_GPIO.
 */
static ERR_TYPE sysfs_write(
    const char *path,
    const char *val
){
    int fd = open(path, O_WRONLY);
    if(fd < 0){
        return ERR_GPIO;
    }
    ssize_t len = (ssize_t)strlen(val);
    ssize_t written = write(fd, val, (size_t)len);
    close(fd);
    return (written == len) ? SUCCESS : ERR_GPIO;
}

/*
 * Export a GPIO pin via sysfs (ignore error if
 * already exported).
 */
static void gpio_export(GPIO_TYPE gpio)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%u", gpio);
    /* ignore return — EBUSY just means already exported */
    (void)sysfs_write("/sys/class/gpio/export", buf);
}

/*
 * Initializes the gpio map. Should only ever be
 * called once. Returns handle if successful,
 * NULL on error.
 */
Gpio_Bbb_Handle gpio_bbb_init(ERR_TYPE * const err_out)
{
    ERR_TYPE err = SUCCESS;
    Gpio_Bbb_Handle ret = NULL;

    if(gpio_instance.in_use != 0u){
        err = ERR_CALLED_TWICE;
    }

    if(err == SUCCESS){
        gpio_instance.in_use = 1u;
        ret = &gpio_instance;
    }

    if(err_out != NULL){
        *err_out = err;
    }
    return ret;
}

/*
 * Sets the direction of the specified gpio on the bbb,
 * where direction is 0 (output) or 1 (input).
 */
ERR_TYPE gpio_bbb_set_dir(
    Gpio_Bbb_Handle handle,
    GPIO_TYPE gpio,
    uint8_t dir
){
    char path[MAX_PATH];

    if(handle == NULL){
        return ERR_NULL_PTR;
    }

    gpio_export(gpio);

    snprintf(path, sizeof(path),
        "/sys/class/gpio/gpio%u/direction", gpio);
    return sysfs_write(path,
        (dir == GPIO_BBB_INPUT) ? "in" : "out");
}

/* Sets an output pin to High */
ERR_TYPE gpio_bbb_set(
    Gpio_Bbb_Handle handle,
    GPIO_TYPE gpio
){
    char path[MAX_PATH];
    if(handle == NULL){
        return ERR_NULL_PTR;
    }
    snprintf(path, sizeof(path),
        "/sys/class/gpio/gpio%u/value", gpio);
    return sysfs_write(path, "1");
}

/* Sets an output pin to Low */
ERR_TYPE gpio_bbb_clear(
    Gpio_Bbb_Handle handle,
    GPIO_TYPE gpio
){
    char path[MAX_PATH];
    if(handle == NULL){
        return ERR_NULL_PTR;
    }
    snprintf(path, sizeof(path),
        "/sys/class/gpio/gpio%u/value", gpio);
    return sysfs_write(path, "0");
}

/* Reads the pin state */
ERR_TYPE gpio_bbb_read(
    Gpio_Bbb_Handle handle,
    GPIO_TYPE gpio,
    uint8_t *value
){
    char path[MAX_PATH];
    char buf[4];
    int fd;
    ssize_t n;

    if(handle == NULL || value == NULL){
        return ERR_NULL_PTR;
    }

    snprintf(path, sizeof(path),
        "/sys/class/gpio/gpio%u/value", gpio);
    fd = open(path, O_RDONLY);
    if(fd < 0){
        return ERR_GPIO;
    }
    n = read(fd, buf, sizeof(buf) - 1u);
    close(fd);
    if(n <= 0){
        return ERR_GPIO;
    }
    buf[n] = '\0';
    *value = (buf[0] == '1') ? 1u : 0u;
    return SUCCESS;
}

/* Cleans up the gpio map. */
ERR_TYPE gpio_bbb_cleanup(Gpio_Bbb_Handle handle)
{
    if(handle == NULL){
        return ERR_NULL_PTR;
    }
    handle->in_use = 0u;

    #ifdef EMU
    printf("gpio bbb cleanup\n");
    #endif

    return SUCCESS;
}
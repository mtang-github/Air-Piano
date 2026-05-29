#include "pwm.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/* Sysfs path for the buzzer PWM channel */
#define PWM_PATH "/sys/class/pwm/pwmchip0/pwm0"

static ERR_TYPE pwm_write(
    const char *file,
    const char *val
){
    char path[128];
    int fd;
    ssize_t len, written;

    snprintf(path, sizeof(path), "%s/%s", PWM_PATH, file);
    fd = open(path, O_WRONLY);
    if(fd < 0){
        return ERR_GPIO;
    }
    len     = (ssize_t)strlen(val);
    written = write(fd, val, (size_t)len);
    close(fd);
    return (written == len) ? SUCCESS : ERR_GPIO;
}

ERR_TYPE pwm_play(NOTE_INDEX_TYPE note_index)
{
    char buf[32];
    ERR_TYPE err = SUCCESS;
    int64_t period_ns = 0;
    int64_t duty_ns = 0;

    if(note_index > NOTE_INDEX_MAX_INCL){
        return ERR_BAD_ARG;
    }

    period_ns = note_get_period_ns(note_index);
    duty_ns = note_get_duty_ns(note_index);

    /*
     * disable before changing period to avoid sysfs
     * rejection
     */
    (void)pwm_write("enable", "0");

    /*
     * zero duty_cycle before period so sysfs never
     * sees duty >= period
     */
    (void)pwm_write("duty_cycle", "0");

    snprintf(buf, sizeof(buf), "%lld", period_ns);
    err = pwm_write("period", buf);
    if(err != SUCCESS) return err;

    snprintf(buf, sizeof(buf), "%lld", duty_ns);
    err = pwm_write("duty_cycle", buf);
    if(err != SUCCESS) return err;

    return pwm_write("enable", "1");
}

ERR_TYPE pwm_stop(void)
{
    return pwm_write("enable", "0");
}

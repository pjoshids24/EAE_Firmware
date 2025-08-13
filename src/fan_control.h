#ifndef FAN_CONTROL_H
#define FAN_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    FAN_OFF,
    FAN_SPEED_CONTROL,
    FAN_MAX,
    FAN_FAULT
} fanState_t;

typedef struct {
    float pwmDutyCycle;
    bool isEnabled;
    fanState_t fanState;
} fanStatus_t;

bool fan_init(void);
void fan_updateSpeed(float current_value);
bool fan_setMaxSpeed(void);
void fan_enable(bool enable);
fanStatus_t fan_getStatus(void);

bool pwm_init(void);
bool pwm_setDutyCycle(float dutyCycle);
bool pwm_start(void);
bool pwm_stop(void);

#endif
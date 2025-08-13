#ifndef PUMP_CONTROL_H
#define PUMP_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    PUMP_OFF,
    PUMP_SPEED_CONTROL,
    PUMP_MAX,
    PUMP_FAULT,
} pumpState_t;

typedef struct {
    float pwmDutyCycle;
    bool isEnabled;
    pumpState_t pumpState;
} pumpStatus_t;

bool pump_init(void);
void pump_updateSpeed(float current_value);
bool pump_setMaxSpeed(void);
void pump_enable(bool enable);
pumpStatus_t pump_getStatus(void);

#endif
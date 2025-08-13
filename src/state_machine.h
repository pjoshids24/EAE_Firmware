#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdint.h>
#include <stdbool.h>
#include "temp_sensor.h"
#include "dio_manager.h"
#include "pump_control.h"
#include "fan_control.h"
#include "pid_controller.h"
#include "can_manager.h"

typedef enum {
    SM_INIT = 0U,
    SM_OFF = 1U,
    SM_STANDBY = 2U,
    SM_COOLING = 3U,
    SM_CRITICAL_TEMP = 4U,
    SM_FAULT = 5U,
    SM_NUM_STATES = 6U
} smState_t;

typedef union {
    uint8_t faultInfo;
    struct {
        bool noFault : 1;
        bool faultInitialization : 1;
        bool faultLowCoolant : 1;
        bool faultCriticalTemp : 1;
        bool faultFan : 1;
        bool faultPump : 1;
        bool faultTempInvalid : 1;
    } faultFlags_t;
} smFault_t;

typedef struct {
    tempReading_t temperature;
    ignitionState_t ignitionSwitch;
    levelState_t coolantLevel;
    fanStatus_t fanStatus;
    pumpStatus_t pumpStatus;
    bool initializationStatus;
    bool systemFault;
    smFault_t smFault;
} smInputs_t;

void sm_update(void);
smState_t sm_getCurrentState(void);

#endif
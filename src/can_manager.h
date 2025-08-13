#ifndef CAN_MANAGER_H
#define CAN_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "dio_manager.h"

#define CAN_MSG_TEMP_STATUS         0x100
#define CAN_MSG_PUMP_STATUS         0x101
#define CAN_MSG_FAN_STATUS          0x102
#define CAN_MSG_SYSTEM_STATUS       0x103
#define CAN_MSG_PID_PARAMS          0x104
#define CAN_MSG_IGNITION_STATUS     0x105
#define CAN_MSG_COOLANT_LEVEL       0x106
#define CAN_MSG_SETPOINT_CMD        0x200
#define CAN_MSG_PID_TUNE_CMD        0x201
#define CAN_MSG_SYSTEM_CMD          0x202

typedef struct {
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
    bool extended;
    bool remote;
} canFrame_t;

typedef struct {
    uint32_t baudrate;
} canConfig_t;

typedef struct {
    uint32_t txCount;
    uint32_t rxCount;
    uint32_t errorCount;
    uint32_t busOffCount;
} canStats_t;

typedef enum {
    CAN_STATUS_OK = 0,
    CAN_STATUS_ERROR,
    CAN_STATUS_BUS_OFF,
    CAN_STATUS_PASSIVE,
    CAN_STATUS_TIMEOUT
} canStatus_t;

typedef void (*canRxCallback_t)(const canFrame_t* frame);

bool canManager_init(const canConfig_t* config);
canStatus_t canManager_sendFrame(const canFrame_t* frame);
canStatus_t canManager_receiveFrame(canFrame_t* frame);
void canManager_setRxCallback(canRxCallback_t callback);
const canStats_t* canManager_getStats(void);
void canManager_processMessages(void);
void canManager_periodicSend(void);
canStatus_t canManager_sendSystemStatus(uint8_t systemState, ignitionState_t ignition, levelState_t coolantLevel, uint8_t faultCode);
canStatus_t canManager_sendTempStatus(float temperature, uint8_t status);
canStatus_t canManager_sendPumpStatus(float dutyCycle, uint8_t state, bool enabled);
canStatus_t canManager_sendFanStatus(float dutyCycle, uint8_t state, bool enabled);
canStatus_t canManager_sendPidParams(float kp, float ki, float kd, float setpoint);
void canManager_floatToBytes(float value, uint8_t* data);
float canManager_bytesToFloat(const uint8_t* data);

#endif
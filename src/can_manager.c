#include "can_manager.h"
#include "temp_sensor.h"
#include "pump_control.h"
#include "fan_control.h"
#include "state_machine.h"
#include "pid_controller.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAIN_LOOP_DELAY (100U)
#define CAN_TX_INTERVAL_MS (1000U)
#define CAN_RX_QUEUE_SIZE 32

static canConfig_t canConfig;
static canStats_t canStats;
static canRxCallback_t rxCallback = NULL;
static canFrame_t rxQueue[CAN_RX_QUEUE_SIZE];
static uint8_t rxQueueHead = 0;
static uint8_t rxQueueTail = 0;
static uint8_t rxQueueCount = 0;

static void canManager_handleSetpointCmd(const canFrame_t* frame);
static void canManager_handlePidTuneCmd(const canFrame_t* frame);
static void canManager_handleSystemCmd(const canFrame_t* frame);
static bool canManager_queueRxFrame(const canFrame_t* frame);
static bool canManager_dequeueRxFrame(canFrame_t* frame);
void can_rx_callback(const canFrame_t* frame);

/*
 * Initialize CAN manager
 */
bool canManager_init(const canConfig_t* config)
{
    bool initResult = false;

    if (config == NULL) {
        canConfig.baudrate = 250000;
    } else {
        canConfig = *config;
    }
    
    memset(&canStats, 0, sizeof(canStats));
    
    rxQueueHead = 0;
    rxQueueTail = 0;
    rxQueueCount = 0;
    
    initResult = true;
    
    canManager_setRxCallback(can_rx_callback);
    
    return initResult;
}

/*
 * Send CAN frame
 */
canStatus_t canManager_sendFrame(const canFrame_t* frame)
{
    if (frame == NULL) {
        return CAN_STATUS_ERROR;
    }
    
    if (frame->dlc > 8) {
        return CAN_STATUS_ERROR;
    }
    
    printf("CAN TX: ID=0x%03X, DLC=%d, Data=", frame->id, frame->dlc);
    for (int i = 0; i < frame->dlc; i++) {
        printf("%02X ", frame->data[i]);
    }
    printf("\n");
    
    canStats.txCount++;
    
    return CAN_STATUS_OK;
}

/*
 * Receive CAN frame (non-blocking)
 */
canStatus_t canManager_receiveFrame(canFrame_t* frame)
{
    if (frame == NULL) {
        return CAN_STATUS_ERROR;
    }
    
    if (canManager_dequeueRxFrame(frame)) {
        canStats.rxCount++;
        return CAN_STATUS_OK;
    }
    
    return CAN_STATUS_TIMEOUT;
}

/*
 * Register callback for received messages
 */
void canManager_setRxCallback(canRxCallback_t callback)
{
    rxCallback = callback;
}

/*
 * Get CAN statistics
 */
const canStats_t* canManager_getStats(void)
{
    return &canStats;
}

/*
 * Send temperature status message
 */
canStatus_t canManager_sendTempStatus(float temperature, uint8_t status)
{
    canFrame_t frame;
    
    frame.id = CAN_MSG_TEMP_STATUS;
    frame.dlc = 5;
    frame.extended = false;
    frame.remote = false;
    
    canManager_floatToBytes(temperature, &frame.data[0]);
    frame.data[4] = status;
    
    return canManager_sendFrame(&frame);
}

/*
 * Send pump status message
 */
canStatus_t canManager_sendPumpStatus(float dutyCycle, uint8_t state, bool enabled)
{
    canFrame_t frame;
    
    frame.id = CAN_MSG_PUMP_STATUS;
    frame.dlc = 6;
    frame.extended = false;
    frame.remote = false;
    
    canManager_floatToBytes(dutyCycle, &frame.data[0]);
    frame.data[4] = state;
    frame.data[5] = enabled ? 1 : 0;
    
    return canManager_sendFrame(&frame);
}

/*
 * Send fan status message
 */
canStatus_t canManager_sendFanStatus(float dutyCycle, uint8_t state, bool enabled)
{
    canFrame_t frame;
    
    frame.id = CAN_MSG_FAN_STATUS;
    frame.dlc = 6;
    frame.extended = false;
    frame.remote = false;
    
    canManager_floatToBytes(dutyCycle, &frame.data[0]);
    frame.data[4] = state;
    frame.data[5] = enabled ? 1 : 0;
    
    return canManager_sendFrame(&frame);
}

/*
 * Send system status message
 */
canStatus_t canManager_sendSystemStatus(uint8_t systemState, ignitionState_t ignition, levelState_t coolantLevel, uint8_t faultCode)
{
    canFrame_t frame;
    
    frame.id = CAN_MSG_SYSTEM_STATUS;
    frame.dlc = 4;
    frame.extended = false;
    frame.remote = false;
    
    frame.data[0] = systemState;
    frame.data[1] = ignition;
    frame.data[2] = coolantLevel;
    frame.data[3] = faultCode;
    
    return canManager_sendFrame(&frame);
}

/*
 * Send PID parameters message
 */
canStatus_t canManager_sendPidParams(float kp, float ki, float kd, float setpoint)
{
    canFrame_t frame;
    
    frame.id = CAN_MSG_PID_PARAMS;
    frame.dlc = 8;
    frame.extended = false;
    frame.remote = false;
    
    uint16_t kp_scaled = (uint16_t)(kp * 1000);
    uint16_t ki_scaled = (uint16_t)(ki * 1000);
    uint16_t kd_scaled = (uint16_t)(kd * 1000);
    uint16_t setpoint_scaled = (uint16_t)(setpoint * 10);
    
    frame.data[0] = (uint8_t)(kp_scaled & 0xFF);
    frame.data[1] = (uint8_t)(kp_scaled >> 8);
    frame.data[2] = (uint8_t)(ki_scaled & 0xFF);
    frame.data[3] = (uint8_t)(ki_scaled >> 8);
    frame.data[4] = (uint8_t)(kd_scaled & 0xFF);
    frame.data[5] = (uint8_t)(kd_scaled >> 8);
    frame.data[6] = (uint8_t)(setpoint_scaled & 0xFF);
    frame.data[7] = (uint8_t)(setpoint_scaled >> 8);
    
    return canManager_sendFrame(&frame);
}

/*
 * Send periodic CAN status messages
 */
void canManager_periodicSend(void)
{
    static uint32_t lastCanTxTime = 0;
    static uint32_t currentTime = 0;
    float kp, ki ,kd;
    
    currentTime += MAIN_LOOP_DELAY;
    
    if (currentTime - lastCanTxTime >= CAN_TX_INTERVAL_MS) {
        lastCanTxTime = currentTime;

        tempReading_t tempReading = tempSensor_readValue();
        pumpStatus_t pumpStatus = pump_getStatus();
        fanStatus_t fanStatus = fan_getStatus();

        pid_getGains(&kp, &ki, &kd);

        canManager_sendTempStatus(tempReading.temperatureCelsius, tempReading.status);

        float setpoint = pid_getSetpoint();
        canManager_sendPidParams(kp, ki, kd, setpoint);

        canManager_sendPumpStatus(pumpStatus.pwmDutyCycle, pumpStatus.pumpState, pumpStatus.isEnabled);
        canManager_sendFanStatus(fanStatus.pwmDutyCycle, fanStatus.fanState, fanStatus.isEnabled);
    }
}

/*
 * Process received CAN messages
 */
void canManager_processMessages(void)
{
    canFrame_t frame;
    
    while (canManager_receiveFrame(&frame) == CAN_STATUS_OK) {
        printf("CAN RX: ID=0x%03X, DLC=%d, Data=", frame.id, frame.dlc);
        for (int i = 0; i < frame.dlc; i++) {
            printf("%02X ", frame.data[i]);
        }
        printf("\n");
        
        switch (frame.id) {
            case CAN_MSG_SETPOINT_CMD:
                canManager_handleSetpointCmd(&frame);
                break;
                
            case CAN_MSG_PID_TUNE_CMD:
                canManager_handlePidTuneCmd(&frame);
                break;
                
            case CAN_MSG_SYSTEM_CMD:
                canManager_handleSystemCmd(&frame);
                break;
                
            default:
                break;
        }
        
        if (rxCallback != NULL) {
            rxCallback(&frame);
        }
    }
}

/*
 * Convert float to CAN data bytes (little endian)
 */
void canManager_floatToBytes(float value, uint8_t* data)
{
    union {
        float f;
        uint8_t bytes[4];
    } converter;
    
    converter.f = value;
    
    data[0] = converter.bytes[0];
    data[1] = converter.bytes[1];
    data[2] = converter.bytes[2];
    data[3] = converter.bytes[3];
}

/*
 * Convert CAN data bytes to float (little endian)
 */
float canManager_bytesToFloat(const uint8_t* data)
{
    union {
        float f;
        uint8_t bytes[4];
    } converter;
    
    converter.bytes[0] = data[0];
    converter.bytes[1] = data[1];
    converter.bytes[2] = data[2];
    converter.bytes[3] = data[3];
    
    return converter.f;
}

/*
 * Handle setpoint command message
 */
static void canManager_handleSetpointCmd(const canFrame_t* frame)
{
    if (frame->dlc >= 4) {
        float newSetpoint = canManager_bytesToFloat(&frame->data[0]);
        
        if (!pid_setSetpoint(newSetpoint)) {
            printf("CAN: Failed to update setpoint\n");
        }
    }
}

/*
 * Handle PID tuning command message
 */
static void canManager_handlePidTuneCmd(const canFrame_t* frame)
{
    if (frame->dlc >= 8) {
        uint16_t kp_scaled = ((uint16_t)frame->data[1] << 8) | frame->data[0];
        uint16_t ki_scaled = ((uint16_t)frame->data[3] << 8) | frame->data[2];
        uint16_t kd_scaled = ((uint16_t)frame->data[5] << 8) | frame->data[4];
        
        float kp = (float)kp_scaled / 1000.0f;
        float ki = (float)ki_scaled / 1000.0f;
        float kd = (float)kd_scaled / 1000.0f;
        
        if (!pid_setGains(kp, ki, kd)) {
            printf("CAN: Failed to update PID gains\n");
        }
    }
}

/*
 * Handle system command message
 */
static void canManager_handleSystemCmd(const canFrame_t* frame)
{
    if (frame->dlc >= 1) {
        uint8_t command = frame->data[0];
        
        switch (command) {
            case 0x01:
                printf("CAN: System reset command received\n");
                pid_reset();
                break;
                
            case 0x02:
                printf("CAN: Emergency stop command received\n");
                pump_enable(false);
                fan_enable(false);
                break;
                
            case 0x03:
                printf("CAN: System enable command received\n");
                pump_enable(true);
                fan_enable(true);
                break;
                
            default:
                printf("CAN: Unknown system command: 0x%02X\n", command);
                break;
        }
    }
}

/*
 * CAN receive callback function
 */
void can_rx_callback(const canFrame_t* frame)
{
    printf("CAN RX Callback: ID=0x%03X, DLC=%d\n", frame->id, frame->dlc);
}

/*
 * Queue received frame for processing
 */
static bool canManager_queueRxFrame(const canFrame_t* frame)
{
    if (rxQueueCount >= CAN_RX_QUEUE_SIZE) {
        return false;
    }
    
    rxQueue[rxQueueHead] = *frame;
    rxQueueHead = (rxQueueHead + 1) % CAN_RX_QUEUE_SIZE;
    rxQueueCount++;
    
    return true;
}

/*
 * Dequeue received frame for processing
 */
static bool canManager_dequeueRxFrame(canFrame_t* frame)
{
    if (rxQueueCount == 0) {
        return false;
    }
    
    *frame = rxQueue[rxQueueTail];
    rxQueueTail = (rxQueueTail + 1) % CAN_RX_QUEUE_SIZE;
    rxQueueCount--;
    
    return true;
}
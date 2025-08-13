#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "state_machine.h"

#define PID_LEVEL_PUMP (40.0f)

/* Forward declarations for state functions */
void sm_init_entry(void);
void sm_init_handler(void);
void sm_init_transition(void);
void sm_init_exit(void);

void sm_off_entry(void);
void sm_off_handler(void);
void sm_off_transition(void);
void sm_off_exit(void);

void sm_standby_entry(void);
void sm_standby_handler(void);
void sm_standby_transition(void);
void sm_standby_exit(void);

void sm_cooling_entry(void);
void sm_cooling_handler(void);
void sm_cooling_transition(void);
void sm_cooling_exit(void);

void sm_critical_temp_entry(void);
void sm_critical_temp_handler(void);
void sm_critical_temp_transition(void);
void sm_critical_temp_exit(void);

void sm_fault_entry(void);
void sm_fault_handler(void);
void sm_fault_transition(void);
void sm_fault_exit(void);

typedef void (*state_func_ptr)(void);

typedef struct {
    state_func_ptr entry;
    state_func_ptr handler;
    state_func_ptr transition;
    state_func_ptr exit;
} stateFunctions_t;

static const stateFunctions_t state_map[SM_NUM_STATES] = {
    [SM_INIT] = {
        .entry = sm_init_entry,
        .handler = sm_init_handler,
        .transition = sm_init_transition,
        .exit = sm_init_exit
    },
    [SM_OFF] = {
        .entry = sm_off_entry,
        .handler = sm_off_handler,
        .transition = sm_off_transition,
        .exit = sm_off_exit
    },
    [SM_STANDBY] = {
        .entry = sm_standby_entry,
        .handler = sm_standby_handler,
        .transition = sm_standby_transition,
        .exit = sm_standby_exit
    },
    [SM_COOLING] = {
        .entry = sm_cooling_entry,
        .handler = sm_cooling_handler,
        .transition = sm_cooling_transition,
        .exit = sm_cooling_exit
    },
    [SM_CRITICAL_TEMP] = {
        .entry = sm_critical_temp_entry,
        .handler = sm_critical_temp_handler,
        .transition = sm_critical_temp_transition,
        .exit = sm_critical_temp_exit
    },
    [SM_FAULT] = {
        .entry = sm_fault_entry,
        .handler = sm_fault_handler,
        .transition = sm_fault_transition,
        .exit = sm_fault_exit
    }
};

static smInputs_t smInstance;
static smInputs_t* smInputs = &smInstance;
static smState_t nextState;
static smState_t previousState = SM_INIT;
static smState_t currentState = SM_INIT;
static float pidOutput;

/*
 * Update system inputs from sensors and switches
 */
static void sm_updateInputs(void)
{
    smInputs->temperature = tempSensor_readValue();
    
    smInputs->ignitionSwitch = dioManager_readIgnition();
    
    smInputs->coolantLevel = dioManager_readLevel();
    
    smInputs->pumpStatus = pump_getStatus();
    smInputs->fanStatus = fan_getStatus();
    
    smInputs->systemFault = (smInputs->temperature.status == TEMP_INVALID) || 
                        (smInputs->coolantLevel == LEVEL_LOW)||
                        (smInputs->pumpStatus.pumpState == PUMP_FAULT)||
                        (smInputs->fanStatus.fanState == FAN_FAULT)||
                        (smInputs->temperature.status == TEMP_CRITICAL_HIGH);

    if (smInputs->temperature.status == TEMP_INVALID) {
        smInputs->smFault.faultFlags_t.faultTempInvalid = true;
    }
    if (smInputs->coolantLevel == LEVEL_LOW) {
        smInputs->smFault.faultFlags_t.faultLowCoolant = true;
    }
    if (smInputs->temperature.status == TEMP_CRITICAL_HIGH) {
        smInputs->smFault.faultFlags_t.faultCriticalTemp = true;
    }
    if (smInputs->pumpStatus.pumpState == PUMP_FAULT) {
        smInputs->smFault.faultFlags_t.faultPump = true;
    }
    if (smInputs->fanStatus.fanState == FAN_FAULT) {
        smInputs->smFault.faultFlags_t.faultFan = true;
    }
}

/*
 * Initialize all system components
 */
static bool sm_systemInitialization(void)
{
    bool initResult = true;
    
    if (!tempSensor_init()) {
        printf("ERROR: Failed to initialize temperature sensor\n");
        initResult = false;
    } else {
        printf("Temperature sensor initialized\n");
    }

    if (!pid_init()) {
        printf("ERROR: Failed to initialize PID controller\n");
        initResult = false;
    } else {
        printf("PID controller initialized\n");
    }
    
    if (!dioManager_init()) {
        printf("ERROR: Failed to initialize DIO manager\n");
        initResult = false;
    } else {
        printf("DIO manager initialized\n");
    }
    
    if (!pump_init()) {
        printf("ERROR: Failed to initialize pump control\n");
        initResult = false;
    } else {
        printf("Pump control initialized\n");
    }
    
    if (!fan_init()) {
        printf("ERROR: Failed to initialize fan control\n");
        initResult = false;
    } else {
        printf("Fan control initialized\n");
    }

    canConfig_t canConfig;
    canConfig.baudrate = 250000;
    
    if (!canManager_init(&canConfig)) {
        printf("ERROR: Failed to initialize CAN manager\n");
        initResult = false;
    } else {
        printf("CAN initialized\n");
    }
    
    if (initResult) {
        printf("System initialization complete\n");
    } else {
        printf("System initialization FAILED\n");
    }
    
    return initResult;
}

/*
 * Main state machine update function
 */
void sm_update(void)
{
    sm_updateInputs();
    
    if (previousState != currentState) {
        if (state_map[previousState].exit != NULL) {
            state_map[previousState].exit();
        }
        if (state_map[currentState].entry != NULL) {
            state_map[currentState].entry();
        }
        canManager_sendSystemStatus(currentState, smInputs->ignitionSwitch, smInputs->coolantLevel, smInputs->smFault.faultInfo);
    }
    
    if (state_map[currentState].handler != NULL) {
        state_map[currentState].handler();
    }
    
    if (state_map[currentState].transition != NULL) {
        state_map[currentState].transition();
    }
    
    if (nextState != currentState) {
        previousState = currentState;
        currentState = nextState;
    }
}

/*
 * Get current state machine state
 */
smState_t sm_getCurrentState(void)
{
    return currentState;
}

/*
 * SM_INIT State Functions
 */
void sm_init_entry(void)
{
    smInputs->initializationStatus = sm_systemInitialization();
    smInputs->smFault.faultFlags_t.faultFan = !smInputs->initializationStatus;
}

void sm_init_handler(void)
{
}

void sm_init_transition(void)
{
    if ((smInputs->coolantLevel == LEVEL_LOW) || 
        (smInputs->temperature.status == TEMP_INVALID)||(!smInputs->initializationStatus)) {
        nextState = SM_FAULT;
        return;
    }
    if (smInputs->ignitionSwitch == IGNITION_ON) {
        nextState = SM_STANDBY;
        return;
    }
    else if (smInputs->ignitionSwitch == IGNITION_OFF) {
        nextState = SM_OFF;
        return;
    }
    else {
        nextState = SM_FAULT;
        return;
    }
}

void sm_init_exit(void)
{
}

/*
 * SM_OFF State Functions
 */
void sm_off_entry(void)
{
    fan_enable(false);
    pump_enable(false);
}

void sm_off_handler(void)
{
}

void sm_off_transition(void)
{
    if ((smInputs->coolantLevel == LEVEL_LOW) || 
        (smInputs->temperature.status == TEMP_INVALID)||(smInputs->systemFault)) {
        nextState = SM_FAULT;
        return;
    }
    if (smInputs->ignitionSwitch == IGNITION_ON) {
        nextState = SM_STANDBY;
        return;
    }
    else if (smInputs->ignitionSwitch == IGNITION_OFF) {
        return;
    }
    else {
        nextState = SM_FAULT;
        return;
    }
}

void sm_off_exit(void)
{
}

/*
 * SM_STANDBY State Functions
 */
void sm_standby_entry(void)
{
    pid_reset();
    pump_enable(true);
    fan_enable(true);
}

void sm_standby_handler(void)
{
}

void sm_standby_transition(void)
{
    if ((smInputs->coolantLevel == LEVEL_LOW) || 
        (smInputs->temperature.status == TEMP_INVALID)||(smInputs->systemFault)) {
        nextState = SM_FAULT;
        return;
    }
    if (smInputs->ignitionSwitch == IGNITION_OFF) {
        nextState = SM_OFF;
        return;
    }
    nextState = SM_COOLING;
}

void sm_standby_exit(void)
{
}

/*
 * SM_COOLING State Functions
 */
void sm_cooling_entry(void)
{
}

void sm_cooling_handler(void)
{
    pidOutput = pid_compute(smInputs->temperature.temperatureCelsius);
    
    if (pidOutput < PID_LEVEL_PUMP) {
        pump_updateSpeed(pidOutput);
        fan_updateSpeed(0);
    } else {
        pump_updateSpeed(pidOutput);
        fan_updateSpeed(pidOutput - PID_LEVEL_PUMP);
    }
}

void sm_cooling_transition(void)
{
    if ((smInputs->coolantLevel == LEVEL_LOW) || 
        (smInputs->temperature.status == TEMP_INVALID)||(smInputs->systemFault)) {
        nextState = SM_FAULT;
        return;
    }
    
    if (smInputs->ignitionSwitch == IGNITION_OFF) {
        nextState = SM_OFF;
        return;
    }
    if (smInputs->temperature.status == TEMP_CRITICAL_HIGH) {
        nextState = SM_CRITICAL_TEMP;
    }
}

void sm_cooling_exit(void)
{
}

/*
 * SM_CRITICAL_TEMP State Functions
 */
void sm_critical_temp_entry(void)
{
    pid_reset();
    pump_setMaxSpeed();
    fan_setMaxSpeed();
}

void sm_critical_temp_handler(void)
{
}

void sm_critical_temp_transition(void)
{
    if ((smInputs->coolantLevel == LEVEL_LOW) || 
        (smInputs->temperature.status == TEMP_INVALID)||(smInputs->systemFault)) {
        nextState = SM_FAULT;
        return;
    }
    
    if (smInputs->temperature.status == TEMP_CRITICAL_HIGH) {
        nextState = SM_FAULT;
        return;
    }
    if (smInputs->temperature.status <= TEMP_HIGH) {
        nextState = SM_COOLING;
        return;
    }

    if (smInputs->ignitionSwitch == IGNITION_OFF) {
        nextState = SM_OFF;
        return;
    }
}

void sm_critical_temp_exit(void)
{
    printf("Exiting CRITICAL_TEMP state\n");
}

/*
 * SM_FAULT State Functions
 */
void sm_fault_entry(void)
{
    pump_setMaxSpeed();
    fan_setMaxSpeed();
    canManager_sendSystemStatus(SM_FAULT, smInputs->ignitionSwitch, smInputs->coolantLevel, smInputs->smFault.faultInfo);
}

void sm_fault_handler(void)
{
}

void sm_fault_transition(void)
{
    if ((smInputs->coolantLevel != LEVEL_LOW) && 
        (smInputs->temperature.status != TEMP_INVALID)&&(!smInputs->systemFault)) {
        if (smInputs->ignitionSwitch == IGNITION_ON) {
            nextState = SM_STANDBY;
            return;
        }
        else if (smInputs->ignitionSwitch == IGNITION_OFF) {
            return;
        }
    }
}

void sm_fault_exit(void)
{
    pid_reset();
}
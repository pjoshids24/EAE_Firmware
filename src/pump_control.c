#include "pump_control.h"

static pumpStatus_t pumpStatus;

#define PUMP_MIN_DUTY_CYCLE     (0.0F)
#define PUMP_MAX_DUTY_CYCLE     (100.0F)
#define PUMP_PWM_TO_DUTY_FACTOR (1.0F)

/*
 * Initializes the pump control module and PID controller
 */
bool pump_init(void)
{
    bool initResult = false;

    initResult = pwm_init();
    
    if (initResult) {
        pumpStatus.pwmDutyCycle = PUMP_MIN_DUTY_CYCLE;
        pumpStatus.isEnabled = false;
        pumpStatus.pumpState = PUMP_OFF;
        
        pwm_stop();
    }

    return initResult;
}

/*
 * Updates the pump speed based on the PID controller output and safety conditions
 */
void pump_updateSpeed(float controlOutput)
{
    if ((!pumpStatus.isEnabled)) {
        pwm_stop();
        pumpStatus.pumpState = PUMP_OFF;
        pumpStatus.pwmDutyCycle = PUMP_MIN_DUTY_CYCLE;
        return;
    }

    pumpStatus.pwmDutyCycle = PUMP_PWM_TO_DUTY_FACTOR * controlOutput;

    if (pumpStatus.pwmDutyCycle > PUMP_MAX_DUTY_CYCLE) {
        pumpStatus.pwmDutyCycle = PUMP_MAX_DUTY_CYCLE;
    } else if (pumpStatus.pwmDutyCycle < PUMP_MIN_DUTY_CYCLE) {
        pumpStatus.pwmDutyCycle = PUMP_MIN_DUTY_CYCLE;
    }

    pwm_setDutyCycle(pumpStatus.pwmDutyCycle);
    pumpStatus.pumpState = PUMP_SPEED_CONTROL;
}

/*
 * Set pump to maximum speed
 */
bool pump_setMaxSpeed(void)
{
    pumpStatus.pwmDutyCycle = PUMP_MAX_DUTY_CYCLE;
    pumpStatus.pumpState = PUMP_MAX;
    
    pwm_setDutyCycle(pumpStatus.pwmDutyCycle);
    
    return true;
}

/*
 * Get pump status
 */
pumpStatus_t pump_getStatus(void)
{
    return pumpStatus;
}

/*
 * Enables or disables the pump control
 */
void pump_enable(bool enable)
{
    pumpStatus.isEnabled = enable;
    if (enable) {
        pwm_start();
    } else {
        pumpStatus.pumpState = PUMP_OFF;
        pumpStatus.pwmDutyCycle = PUMP_MIN_DUTY_CYCLE;
        pwm_stop();
    }
}
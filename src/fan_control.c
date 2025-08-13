#include "fan_control.h"

static fanStatus_t fanStatus;

#define FAN_MIN_DUTY_CYCLE 0.0f
#define FAN_MAX_DUTY_CYCLE 100.0f
#define FAN_PWM_TO_DUTY_FACTOR (1.0f)

/*
 * Initializes the fan control module and PID controller
 */
bool fan_init(void)
{
    bool initResult = false;

    initResult = pwm_init();
    
    if (initResult) {
        fanStatus.pwmDutyCycle = FAN_MIN_DUTY_CYCLE;
        fanStatus.isEnabled = false;
        fanStatus.fanState = FAN_OFF;
        
        pwm_stop();
    }
    
    return initResult;
}

/*
 * Updates the fan speed based on the PID controller output and safety conditions
 */
void fan_updateSpeed(float controlOutput)
{
    if ((!fanStatus.isEnabled)) {
        pwm_stop();
        fanStatus.fanState = FAN_OFF;
        fanStatus.pwmDutyCycle = FAN_MIN_DUTY_CYCLE;
        return;
    }

    fanStatus.pwmDutyCycle = FAN_PWM_TO_DUTY_FACTOR * controlOutput;

    if (fanStatus.pwmDutyCycle > FAN_MAX_DUTY_CYCLE) {
        fanStatus.pwmDutyCycle = FAN_MAX_DUTY_CYCLE;
    } else if (fanStatus.pwmDutyCycle < FAN_MIN_DUTY_CYCLE) {
        fanStatus.pwmDutyCycle = FAN_MIN_DUTY_CYCLE;
    }

    pwm_setDutyCycle(fanStatus.pwmDutyCycle);
    fanStatus.fanState = FAN_SPEED_CONTROL;
}

/*
 * Set fan to maximum speed
 */
bool fan_setMaxSpeed(void)
{
    fanStatus.pwmDutyCycle = FAN_MAX_DUTY_CYCLE;
    fanStatus.fanState = FAN_MAX;
    
    pwm_setDutyCycle(fanStatus.pwmDutyCycle);
    
    return true;
}

/*
 * Get fan status
 */
fanStatus_t fan_getStatus(void)
{
    return fanStatus;
}

/*
 * Enables or disables the fan control
 */
void fan_enable(bool enable)
{
    fanStatus.isEnabled = enable;
    if (enable) {
        pwm_start();
    } else {
        fanStatus.fanState = FAN_OFF;
        fanStatus.pwmDutyCycle = FAN_MIN_DUTY_CYCLE;
        pwm_stop();
    }
}

/*
 * Initialize PWM hardware
 */
bool pwm_init(void)
{
    return true;
}

/*
 * Start PWM output
 */
bool pwm_start(void)
{
    return true;
}

/*
 * Stop PWM output
 */
bool pwm_stop(void)
{
    return true;
}

/*
 * Set PWM duty cycle
 */
bool pwm_setDutyCycle(float dutyCycle)
{
    return true;
}
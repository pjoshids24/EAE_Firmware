#include "pid_controller.h"
#include <string.h>
#include <math.h>

static pidController_t pidInstance = 
{
    {
    PID_DEFAULT_KP,
    PID_DEFAULT_KI,
    PID_DEFAULT_KD,
    PID_DEFAULT_SETPOINT,
    PID_DEFAULT_OUTPUT_MIN,
    PID_DEFAULT_OUTPUT_MAX
    },
    0.0f,
    0.0f,
    0.0f
};
static pidController_t* pid = &pidInstance;

/*
 * Initialize PID controller
 */
bool pid_init(void)
{
    if (pid == NULL) {
        return false;
    }

    pid->prevError = 0.0f;
    pid->integral = 0.0f;
    pid->output = 0.0f;

    return true;
}

/*
 * Set PID controller setpoint
 */
bool pid_setSetpoint(float setpoint)
{
    pid->config.setpoint = setpoint;
    return true;
}

/*
 * Get current PID controller setpoint
 */
float pid_getSetpoint(void)
{
    return pid->config.setpoint;
}

/*
 * Update PID controller gains
 */
bool pid_setGains(float kp, float ki, float kd)
{
    pid->config.kp = kp;
    pid->config.ki = ki;
    pid->config.kd = kd;
    
    return true;
}

/*
 * Get PID controller gains
 */
bool pid_getGains(float* kp, float* ki, float* kd)
{
    *kp = pid->config.kp;
    *ki = pid->config.ki;
    *kd = pid->config.kd;
    return true;
}

/*
 * Update PID controller output limits
 */
bool pid_setOutputLimits(float outputMin, float outputMax)
{
    if (outputMin >= outputMax) {
        return false;
    }
    
    pid->config.outputMin = outputMin;
    pid->config.outputMax = outputMax;
    
    float max_integral = (outputMax - outputMin) / (2.0f * pid->config.ki);
    if (pid->integral > max_integral) {
        pid->integral = max_integral;
    } else if (pid->integral < -max_integral) {
        pid->integral = -max_integral;
    }
    
    return true;
}

/*
 * Compute PID controller output
 */
float pid_compute(float sampledValue)
{
    float error, proportional, derivative;

    error = pid->config.setpoint - sampledValue;
    proportional = pid->config.kp * error;
    derivative = pid->config.kd * (error - pid->prevError);
    pid->integral += error;
    float max_integral = fabs(pid->config.outputMax - pid->config.outputMin) / (2.0f * fabs(pid->config.ki));
    if (pid->integral > max_integral) {
        pid->integral = max_integral;
    } else if (pid->integral < -max_integral) {
        pid->integral = -max_integral;
    }

    float integral = pid->config.ki * pid->integral;

    derivative = pid->config.kd * (error - pid->prevError);

    float output = proportional + integral + derivative;

    if (output > pid->config.outputMax) {
        output = pid->config.outputMax;
    } else if (output < pid->config.outputMin) {
        output = pid->config.outputMin;
    }

    pid->prevError = error;

    pid->output = output;

    return output;
}

/*
 * Reset PID controller internal states
 */
void pid_reset(void)
{
    pid->prevError = 0.0f;
    pid->integral = 0.0f;
    pid->output = 0.0f;
}

/*
 * Get PID controller output (last computed value)
 */
float pid_getOutput(void)
{
    return pid->output;
}
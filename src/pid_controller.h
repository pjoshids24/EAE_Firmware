#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>

#define PID_DEFAULT_KP          (1.0F)
#define PID_DEFAULT_KI          (0.1F)
#define PID_DEFAULT_KD          (0.01F)
#define PID_DEFAULT_SETPOINT    (25.0F)
#define PID_DEFAULT_OUTPUT_MIN  (-100.0F)
#define PID_DEFAULT_OUTPUT_MAX  (100.0F)

typedef struct {
    float kp;
    float ki;
    float kd;
    float setpoint;
    float outputMin;
    float outputMax;
} pidConfig_t;

typedef struct {
    pidConfig_t config;
    float prevError;
    float integral;
    float output;
} pidController_t;

bool pid_init(void);
bool pid_setSetpoint(float setpoint);
float pid_getSetpoint(void);
bool pid_setGains(float kp, float ki, float kd);
bool pid_getGains(float* kp, float* ki, float* kd);
bool pid_setOutputLimits(float outputMin, float outputMax);
float pid_compute(float process_value);
void pid_reset(void);
float pid_getOutput(void);

#endif
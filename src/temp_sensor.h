#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TEMP_OK = 0,
    TEMP_HIGH = 1,
    TEMP_CRITICAL_HIGH = 2,
    TEMP_INVALID = 3
} tempStatus_t;

typedef struct {
    float temperatureCelsius;
    tempStatus_t status;
} tempReading_t;

bool tempSensor_init(void);
tempReading_t tempSensor_readValue(void);

#endif
#include "temp_sensor.h"
#include <stdio.h>
#include <math.h>

static uint16_t adc_readChannel(uint8_t channel);

static tempStatus_t currentStatus;

#define ADC_RESOLUTION_MAX     (4095U)
#define ADC_RESOLUTION_MIN     (0U)
#define ADC_VREF_VOLTS         (3.3F)
#define TEMP_SENSOR_ADC_CHANNEL     (1U)
#define TEMP_SENSOR_SLOPE          (0.1F)
#define TEMP_SENSOR_OFFSET         (-50.0F)
#define TEMP_HIGH_THRESHOLD         (60.0F)
#define TEMP_CRITICAL_THRESHOLD    (80.0F)

/*
 * Initialize temperature sensor module
 */
bool tempSensor_init(void)
{
    bool initResult = false;
    
    initResult = true;
    
    if (initResult) {
        currentStatus = TEMP_OK;
    } else {
        currentStatus = TEMP_INVALID;
    }
    
    return initResult;
}

/*
 * Read temperature sensor value
 */
tempReading_t tempSensor_readValue(void)
{
    tempReading_t result = {0.0F, TEMP_INVALID};
    uint16_t adcValue;
    float voltage;
    
    adcValue = adc_readChannel(TEMP_SENSOR_ADC_CHANNEL);
    
    if ((adcValue <= ADC_RESOLUTION_MIN) || (adcValue >= ADC_RESOLUTION_MAX)) {
        result.status = TEMP_INVALID;
        return result;
    }
    
    voltage = ((float)adcValue / (float)ADC_RESOLUTION_MAX) * ADC_VREF_VOLTS;
    
    result.temperatureCelsius = (voltage * TEMP_SENSOR_SLOPE) + TEMP_SENSOR_OFFSET;
    
    if (result.temperatureCelsius > TEMP_HIGH_THRESHOLD) {
        if (result.temperatureCelsius > TEMP_CRITICAL_THRESHOLD) {
            result.status = TEMP_CRITICAL_HIGH;
        } else {
            result.status = TEMP_HIGH;
        }
    } else {
        result.status = TEMP_OK;
    }
    
    currentStatus = result.status;
    
    return result;
}

/*
 * ADC hardware abstraction layer read function
 */
static uint16_t adc_readChannel(uint8_t channel)
{
    (void)channel;
    
    static uint16_t simulatedValue = 2048U;
    
    simulatedValue += (simulatedValue > 2100U) ? (uint16_t)(-10) : (uint16_t)10;
    
    return simulatedValue;
}
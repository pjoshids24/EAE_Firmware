#ifndef DIO_MANAGER_H
#define DIO_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#define DIO_DEFAULT_IGNITION_PIN        GPIO_PIN_2
#define DIO_DEFAULT_LEVEL_SWITCH_PIN    GPIO_PIN_5

typedef enum {
    GPIO_STATE_LOW = 0U,
    GPIO_STATE_HIGH = 1U,
    GPIO_STATE_UNKNOWN = 2U
} gpioState_t;

typedef enum {
    GPIO_PIN_0 = 0U,
    GPIO_PIN_1 = 1U,
    GPIO_PIN_2 = 2U,
    GPIO_PIN_3 = 3U,
    GPIO_PIN_4 = 4U,
    GPIO_PIN_5 = 5U,
    MAX = 32U
} gpioPin_t;

typedef enum {
    IGNITION_OFF = 0,
    IGNITION_ON,
    IGNITION_UNKNOWN
} ignitionState_t;

typedef enum {
    LEVEL_LOW = 0,
    LEVEL_NORMAL,
    LEVEL_UNKNOWN
} levelState_t;

typedef struct {
    gpioPin_t ignitionPin;
    gpioPin_t levelPin;
} dioConfig_t;

typedef struct {
    ignitionState_t ignitionState;
    levelState_t levelState;
} dioStatus_t;

bool dioManager_init(void);
ignitionState_t dioManager_readIgnition(void);
levelState_t dioManager_readLevel(void);

#endif
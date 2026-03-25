#pragma once
#include <stdint.h>

typedef uint8_t pin_t;

typedef enum
{
    PIN_MODE_INPUT,
    PIN_MODE_OUTPUT,
    PIN_MODE_ANALOG,
    PIN_MODE_INPUT_PULLUP,
    PIN_MODE_INPUT_PULLDOWN,
    PIN_MODE_MAX_COUNT
} pin_mode_t;

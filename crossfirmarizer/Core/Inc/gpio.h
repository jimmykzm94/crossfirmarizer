#pragma once

#include "pins.h"
#include <stdint.h>

// FIXME the gpio mode works for stm32, so this is not generic, need fix
// #define GPIO_MODE_INPUT 0
// #define GPIO_MODE_INPUT_PULLUP 1
// #define GPIO_MODE_INPUT_PULLDOWN 2
// #define GPIO_MODE_OUTPUT 3
// #define GPIO_MODE_OUTPUT_OPENDRAIN 4
// #define GPIO_MODE_ALTERNATE 5
// #define GPIO_MODE_ANALOG 6

void gpio_init(pin_t pin, pin_mode_t mode);
void gpio_deinit(pin_t pin);
void gpio_write(pin_t pin, uint8_t value);
uint8_t gpio_read(pin_t pin);

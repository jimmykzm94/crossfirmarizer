#pragma once

#include "pins.h"
#include <stdint.h>

void gpio_init(pin_t pin, pin_mode_t mode);
void gpio_deinit(pin_t pin);
void gpio_write(pin_t pin, uint8_t value);
uint8_t gpio_read(pin_t pin);

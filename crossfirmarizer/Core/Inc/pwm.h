#pragma once

#include "pins.h"
#include <stdint.h>

void pwm_config(pin_t pin, uint16_t period_us);
void pwm_write(pin_t pin, uint16_t value);

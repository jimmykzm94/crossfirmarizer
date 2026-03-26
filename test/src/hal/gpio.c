#include "simulation_state.h"
#include <stdio.h>

extern simulation_state_t simulation_state;

FILE *fp;

void gpio_init(pin_t pin, pin_mode_t mode)
{
    // potentially, when pin is given by specific product
    // then we will use pin function to get actual pins
}
void gpio_deinit(pin_t pin)
{
    // nothing
}
void gpio_write(pin_t pin, uint8_t value)
{
    simulation_state.io_output_pins[pin].pin = pin;
    simulation_state.io_output_pins[pin].value = value;
}
uint8_t gpio_read(pin_t pin)
{
    return simulation_state.io_input_pins[pin].value;
}

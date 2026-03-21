#include "gpio.h"
#include "pins.h"
#include <avr/io.h>

void gpio_init(pin_t pin, pin_mode_t mode)
{
    if (pin >= PIN_MAX_COUNT) return;

    const pin_map_t *mapping = &pins[pin];

    switch (mode)
    {
    case PIN_MODE_INPUT:
        *mapping->ddr &= ~(1 << mapping->pin_num);
        *mapping->port &= ~(1 << mapping->pin_num); // Disable pull-up
        break;
    case PIN_MODE_OUTPUT:
        *mapping->ddr |= (1 << mapping->pin_num);
        break;
    // PIN_MODE_ANALOG is not handled for AVR in this basic implementation
    case PIN_MODE_ANALOG:
    default:
        break;
    }
}

void gpio_deinit(pin_t pin)
{
    // Nothing to do for GPIO deinit on AVR
}

void gpio_write(pin_t pin, uint8_t value)
{
    if (pin >= PIN_MAX_COUNT) return;

    const pin_map_t *mapping = &pins[pin];

    if (value)
    {
        *mapping->port |= (1 << mapping->pin_num);
    }
    else
    {
        *mapping->port &= ~(1 << mapping->pin_num);
    }
}

uint8_t gpio_read(pin_t pin)
{
    if (pin >= PIN_MAX_COUNT) return 0;

    const pin_map_t *mapping = &pins[pin];

    return (*mapping->pin_reg & (1 << mapping->pin_num)) ? 1 : 0;
}


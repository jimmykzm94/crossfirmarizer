#include "gpio.h"
#include <sam.h>

void gpio_init(pin_t pin, pin_mode_t mode)
{

    if (pin >= PIN_MAX_COUNT)
        return;

    uint8_t port_number = PIN_GET_PORT(pin);
    uint32_t pin_number = PIN_GET_PIN(pin);

    switch (mode)
    {
    case PIN_MODE_INPUT:
        PORT_REGS->GROUP[port_number].PORT_DIRCLR = (1 << pin_number);
        // Enable digital input buffer
        PORT_REGS->GROUP[port_number].PORT_PINCFG[pin_number] |= PORT_PINCFG_INEN_Msk;
        break;

    case PIN_MODE_INPUT_PULLUP:
        PORT_REGS->GROUP[port_number].PORT_DIRCLR = (1 << pin_number);
        // Enable digital input buffer
        PORT_REGS->GROUP[port_number].PORT_PINCFG[pin_number] |= PORT_PINCFG_INEN_Msk;
        PORT_REGS->GROUP[0].PORT_PINCFG[pin_number] |= PORT_PINCFG_PULLEN_Msk;
        PORT_REGS->GROUP[0].PORT_OUTSET = (1 << pin_number); // Configures it as Pull-UP
        break;

    case PIN_MODE_OUTPUT:
        PORT_REGS->GROUP[port_number].PORT_DIRSET = (1 << pin_number);
        // Disable digital input buffer to save power
        PORT_REGS->GROUP[port_number].PORT_PINCFG[pin_number] &= ~PORT_PINCFG_INEN_Msk;
        break;

    case PIN_MODE_ANALOG:
        PORT_REGS->GROUP[port_number].PORT_DIRCLR = (1 << pin_number);
        // Disable digital input buffer for analog pins
        PORT_REGS->GROUP[port_number].PORT_PINCFG[pin_number] &= ~PORT_PINCFG_INEN_Msk;
        PORT_REGS->GROUP[port_number].PORT_PINCFG[pin_number] |= PORT_PINCFG_PMUXEN_Msk;
        // Check even or odd
        if (pin_number & 1)
        {
            PORT_REGS->GROUP[port_number].PORT_PMUX[pin_number >> 1] &= ~PORT_PMUX_PMUXO_Msk;
            PORT_REGS->GROUP[port_number].PORT_PMUX[pin_number >> 1] |= PORT_PMUX_PMUXO(PORT_PMUX_PMUXO_B_Val);
        }
        else
        {
            PORT_REGS->GROUP[port_number].PORT_PMUX[pin_number >> 1] &= ~PORT_PMUX_PMUXE_Msk;
            PORT_REGS->GROUP[port_number].PORT_PMUX[pin_number >> 1] |= PORT_PMUX_PMUXE(PORT_PMUX_PMUXE_B_Val);
        }
        break;

    default:
        break;
    }
}

void gpio_deinit(pin_t pin)
{
    if (pin >= PIN_MAX_COUNT)
        return;

    uint8_t port_number = PIN_GET_PORT(pin);
    uint32_t pin_number = PIN_GET_PIN(pin);

    // Reset to default state: Input, digital buffer disabled, no pull-up/down
    PORT_REGS->GROUP[port_number].PORT_DIRCLR = pin_number;
    PORT_REGS->GROUP[port_number].PORT_PINCFG[pin_number] = 0;
}

void gpio_write(pin_t pin, uint8_t value)
{
    if (pin >= PIN_MAX_COUNT)
        return;

    uint8_t port_number = PIN_GET_PORT(pin);
    uint32_t pin_number = PIN_GET_PIN(pin);

    if (value)
    {
        PORT_REGS->GROUP[port_number].PORT_OUTSET = (1 << pin_number);
    }
    else
    {
        PORT_REGS->GROUP[port_number].PORT_OUTCLR = (1 << pin_number);
    }
}

uint8_t gpio_read(pin_t pin)
{
    if (pin >= PIN_MAX_COUNT)
        return 0;

    uint8_t port_number = PIN_GET_PORT(pin);
    uint32_t pin_number = PIN_GET_PIN(pin);

    // Read from the IN register
    uint32_t in_value = PORT_REGS->GROUP[port_number].PORT_IN;
    return (in_value >> pin_number) & 1;
}

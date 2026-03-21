#include "gpio.h"

void gpio_init(pin_t pin, pin_mode_t mode)
{
    GPIO_TypeDef *gpio_port = pins[pin].port;
    uint16_t pin_number = pins[pin].pin;

    // Enable GPIO Clock
    if (gpio_port == GPIOA) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    else if (gpio_port == GPIOB) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    else if (gpio_port == GPIOC) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    // Configure mode and pull-up/pull-down
    gpio_port->MODER &= ~(3U << (pin_number * 2)); // Clear mode bits
    gpio_port->PUPDR &= ~(3U << (pin_number * 2)); // Clear pull-up/pull-down bits

    // switch (mode)
    // {
    //     case GPIO_MODE_INPUT:
    //         // Input mode, no pull
    //         break;
    //     case GPIO_MODE_INPUT_PULLUP:
    //         gpio_port->PUPDR |= (1U << (pin_number * 2)); // Pull-up
    //         break;
    //     case GPIO_MODE_INPUT_PULLDOWN:
    //         gpio_port->PUPDR |= (2U << (pin_number * 2)); // Pull-down
    //         break;
    //     case GPIO_MODE_OUTPUT:
    //         gpio_port->MODER |= (1U << (pin_number * 2)); // Output mode
    //         break;
    //     case GPIO_MODE_OUTPUT_OPENDRAIN:
    //         gpio_port->MODER |= (1U << (pin_number * 2)); // Output mode
    //         gpio_port->OTYPER |= (1U << pin_number); // Open-drain
    //         break;
    //     case GPIO_MODE_ALTERNATE:
    //         gpio_port->MODER |= (2U << (pin_number * 2)); // Alternate function mode
    //         break;
    //     case GPIO_MODE_ANALOG:
    //         gpio_port->MODER |= (3U << (pin_number * 2)); // Analog mode
    //         break;
    // }

    switch(mode)
    {
        case PIN_MODE_INPUT:
            // Input mode, no pull
            break;
        case PIN_MODE_OUTPUT:
            gpio_port->MODER |= (1U << (pin_number * 2)); // Output mode
            break;
        default:
            break;
    }
}

void gpio_deinit(pin_t pin)
{
    // Reset pin to default state (input, no pull)
    pins[pin].port->MODER &= ~(3U << (pins[pin].pin * 2)); // Input mode
    pins[pin].port->PUPDR &= ~(3U << (pins[pin].pin * 2));     // No pull
}

void gpio_write(pin_t pin, uint8_t value)
{
    if (value) pins[pin].port->BSRR = (1U << pins[pin].pin);
    else pins[pin].port->BSRR = (1U << (pins[pin].pin + 16));
}

uint8_t gpio_read(pin_t pin)
{
    return (pins[pin].port->IDR & (1U << pins[pin].pin)) ? 1 : 0;
}

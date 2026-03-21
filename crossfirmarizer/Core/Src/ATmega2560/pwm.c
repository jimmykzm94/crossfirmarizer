#include "pwm.h"
#include "pins.h"
#include <avr/io.h>

void pwm_config(pin_t pin, uint16_t period_us)
{
    if (pin == D11)
    {
        // Set OC1A as output
        DDRB |= (1 << PB5);
        
        // Configure Timer1 for Fast PWM mode with ICR1 as TOP
        // Clear OC1A on compare match, set OC1A at BOTTOM (non-inverting mode)
        TCCR1A = (1 << WGM11) | (1 << COM1A1);
        TCCR1B = (1 << WGM13) | (1 << WGM12);

        // Set prescaler to 8
        TCCR1B |= (1 << CS11);

        // Set the period
        // TOP = (period_us * F_CPU_MHz) / prescaler - 1
        // TOP = (period_us * 16) / 8 - 1 = period_us * 2 - 1
        uint16_t top = period_us * 2 - 1;
        ICR1 = top;
    }
    else if (pin == D6)
    {
        // Set OC4A as output
        DDRH |= (1 << PH3);
    
        // Configure Timer4 for Fast PWM mode with ICR4 as TOP
        // Clear OC4A on compare match, set OC4A at BOTTOM (non-inverting mode)
        TCCR4A = (1 << WGM41) | (1 << COM4A1);
        TCCR4B = (1 << WGM43) | (1 << WGM42);

        // Set prescaler to 8
        TCCR4B |= (1 << CS41);

        // Set the period
        uint16_t top = period_us * 2 - 1;
        ICR4 = top;
    }
}

void pwm_write(pin_t pin, uint16_t value)
{
    if (pin == D11) {
        // The value is assumed to be scaled to the period.
        // For simplicity, we assume value is from 0 to ICR1.
        OCR1A = value;
    } else if (pin == D6) {
        // The value is assumed to be scaled to the period.
        // For simplicity, we assume value is from 0 to ICR4.
        OCR4A = value;
    }
}

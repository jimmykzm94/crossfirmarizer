/*
 * timer.c
 *
 *  Created on: Mar 15, 2026
 *      Author: jimmy
 */

#include "timer.h"
#include <avr/io.h>
#include "gpio.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stddef.h>
#include "command.h"

static TimerCallback tim2_delay_callback = NULL;
static TimerCallback tim4_input_capture_callback = NULL;

static volatile uint32_t echo_time_us_4 = 0;
static volatile uint8_t capture_state_4 = 0; // 0: idle, 1: waiting for falling edge
static volatile uint16_t capture_start_4 = 0;
static volatile uint32_t overflow_count_4 = 0;
static volatile uint32_t timeout_ms_4 = 0;

void timer_delay_init(timer_type_t timer_type, TimerCallback callback)
{
    if (timer_type == TIMER_2)
    {
        tim2_delay_callback = callback;
        // Timer2 setup for delay
        // CTC mode (WGM21 = 1)
        TCCR2A = (1 << WGM21);
        TCCR2B = 0;
        // Enable Compare Match A interrupt
        TIMSK2 |= (1 << OCIE2A);
    }
}

void timer_delay_start_us(uint32_t us)
{
    // Timer2 is an 8-bit timer. With a prescaler of 8, 1 tick = 0.5 us (at 16MHz).
    // Maximum non-blocking delay is 128 us.
    uint32_t ticks = us * 2;
    
    if (ticks > 0 && ticks <= 256)
    {
        OCR2A = ticks - 1;
        TCNT2 = 0;
        TIFR2 = (1 << OCF2A); // Clear any pending compare match flag
        TCCR2B = (1 << CS21);  // Start timer with prescaler 8
    }
    else
    {
        // Fallback for delays larger than 128us
        for (uint32_t i = 0; i < us; i++) {
            _delay_us(1);
        }
        if (tim2_delay_callback != NULL)
        {
            tim2_delay_callback(0);
        }
    }
}

void timer_capture_init(timer_type_t timer_type, pin_t pin, uint32_t timeout, TimerCallback callback)
{
    if (timer_type == TIMER_4 && pin == D49)
    {
        tim4_input_capture_callback = callback;
        timeout_ms_4 = timeout / 1000;

        // Configure echo pin as input
        gpio_init(pin, PIN_MODE_INPUT);

        // Configure Timer4 for input capture
        // Normal mode, stopped initially
        TCCR4A = 0;
        TCCR4B = 0;
        
        // Enable Input Capture and Overflow interrupts
        TIMSK4 |= (1 << ICIE4) | (1 << TOIE4);
        // command_send(CMD_HELLO, "ECHO4", 5);
    }
}

void timer_capture_start(timer_type_t timer_type)
{
    if (timer_type == TIMER_4)
    {
        capture_state_4 = 0;
        overflow_count_4 = 0;

        // Capture on rising edge
        TCCR4B |= (1 << ICES4);
        // Clear interrupt flags
        TIFR4 = (1 << ICF4) | (1 << TOV4);
        // Reset counter
        TCNT4 = 0;
        // Start timer with prescaler 8
        TCCR4B |= (1 << CS41);
    }
}

ISR(TIMER2_COMPA_vect)
{
    // Stop Timer2
    TCCR2B &= ~((1 << CS22) | (1 << CS21) | (1 << CS20));
    
    if (tim2_delay_callback != NULL)
    {
        tim2_delay_callback(0);
    }
}

ISR(TIMER4_CAPT_vect)
{
    if (capture_state_4 == 0)
    {
        // Rising edge captured
        capture_start_4 = ICR4;
        // Set to capture on falling edge
        TCCR4B &= ~(1 << ICES4);
        capture_state_4 = 1;
    }
    else
    {
        // Falling edge captured
        uint16_t capture_end = ICR4;
        echo_time_us_4 = (capture_end - capture_start_4) / 2; // Each tick is 0.5 us (16MHz clock / 8 prescaler).
        // Stop timer
        TCCR4B &= ~((1 << CS42) | (1 << CS41) | (1 << CS40));
        if (tim4_input_capture_callback != NULL)
        {
            tim4_input_capture_callback(echo_time_us_4);
        }
    }
}

ISR(TIMER4_OVF_vect)
{
    overflow_count_4++;
    // Timer4 with prescaler 8 overflows every 32.768ms
    uint32_t elapsed_ms = overflow_count_4 * 33;

    if (elapsed_ms >= timeout_ms_4)
    {
        // Timer overflowed, meaning a timeout
        // Stop timer
        TCCR4B &= ~((1 << CS42) | (1 << CS41) | (1 << CS40));
        if (tim4_input_capture_callback != NULL)
        {
            tim4_input_capture_callback(0); // Indicate timeout
        }
    }
}

timer_type_t timer_get_timer_for_pin(pin_t pin) {
    // This mapping is specific to ATmega2560
    switch(pin) {
        case D49: // ICP4
            return TIMER_4;
        default:
            return -1; // Not a valid timer pin
    }
}

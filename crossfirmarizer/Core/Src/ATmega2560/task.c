#include "task.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static volatile uint32_t task_ms = 0;
static volatile bool systick_flag = false;

static void setup_systick(void)
{
    // Configure Timer0 for 1ms interrupts
    // CTC mode, prescaler 64
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS01) | (1 << CS00);
    // (16000000 / 64 / 1000) - 1 = 249
    OCR0A = 249;
    // Enable compare match interrupt
    TIMSK0 |= (1 << OCIE0A);
}

void task_init(void)
{
    setup_systick();
    sei(); // Enable global interrupts
}

uint32_t task_get_ms(void)
{
    uint32_t ms;
    // Atomically read the value of task_ms
    cli();
    ms = task_ms;
    sei();
    return ms;
}

bool task_systick_happened(void)
{
    if (systick_flag)
    {
        systick_flag = false;
        return true;
    }
    return false;
}

// Timer0 Compare Match ISR
ISR(TIMER0_COMPA_vect)
{
    task_ms++;
    systick_flag = true;
}

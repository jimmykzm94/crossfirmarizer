#include "app.h"

int main(void)
{
    app_init();
    while (1)
    {
        app_background();
    }
}

// #include "gpio.h"

// // #include "timer.h"
// // #include "pins.h"
// #include <stdint.h>
// #include <util/delay.h>
// #include <avr/interrupt.h>

// int val = 0;
// volatile int ready = 0;
// volatile int done = 0;
// void callback(uint32_t v)
// {
//     done = 1;
// }

// // Timer2 Compare Match A Interrupt Service Routine
// volatile uint16_t ms_counter = 0;
// ISR(TIMER2_COMPA_vect)
// {
//     ms_counter++;

//     // Check if 500 ms have passed
//     if (ms_counter >= 1000)
//     {
//         PORTB ^= (1 << PB7); // Toggle built-in LED (Pin 13)
//         ms_counter = 0;      // Reset counter for the next cycle
//     }
// }

// void timer2_init()
// {
//     // 1. Configure Timer2 for CTC mode (Clear Timer on Compare Match)
//     TCCR2A = (1 << WGM21);
//     TCCR2B = 0; // Clear TCCR2B initially

//     // 2. Set Compare Match value for a 1 us interval
//     // Formula: (Target Time / (Prescaler / Clock)) - 1
//     // (0.001 s / (64 / 16000000 Hz)) - 1 = 250 - 1 = 249
//     // OCR2A = 249;

//     // prescaler 8, 1 tick 0.5us
//     uint32_t ticks = 1000 * 2;
//     OCR2A = ticks - 1;

//     // 3. Enable the Output Compare Match A Interrupt
//     TIMSK2 = (1 << OCIE2A);

//     // 4. Start the timer with a Prescaler of 64
//     TCCR2B |= (1 << CS21);
// }

// int main(void)
// {
//     gpio_init(PIN_LED, PIN_MODE_OUTPUT);
//     timer2_init();
//     sei();
//     // timer_delay_init(TIMER_2, callback);
//     // ready = 1;
//     // done = 0;
//     while (1)
//     {
//         // delay_us(1000000);
//         // val ^= 1;
//         // gpio_write(PIN_LED, val);
//     }
// }

// #define F_CPU 16000000UL
// #include <avr/io.h>
// #include <avr/interrupt.h>
// #include <stdlib.h>

// // State Machine
// typedef enum
// {
//     READY,
//     TRIG_HIGH,
//     WAIT_RISING,
//     WAIT_FALLING
// } sys_state_t;
// volatile sys_state_t state = READY;
// volatile uint16_t capture_start = 0;

// // Blocking UART Functions
// void uuart_init(uint32_t baud)
// {
//     UCSR0A = (1 << U2X0);
//     uint32_t ubrr = F_CPU / 8 / baud - 1;
//     UBRR0H = (ubrr >> 8);
//     UBRR0L = ubrr;
//     UCSR0B = (1 << TXEN0);                  // Enable TX
//     UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data
// }

// void uuart_tx(char c)
// {
//     while (!(UCSR0A & (1 << UDRE0)))
//         ; // Block until buffer is empty
//     UDR0 = c;
// }

// void uuart_print(const char *str)
// {
//     while (*str)
//         uuart_tx(*str++);
// }

// void uuart_print_num(uint32_t num)
// {
//     char buf[10];
//     ultoa(num, buf, 10);
//     uuart_print(buf);
// }

// // Timer 2 ISR: Stops the 10us trigger pulse, starts Timer 4 capture
// ISR(TIMER2_COMPA_vect)
// {
//     PORTL &= ~(1 << PL2); // Set D47 (Trig) LOW
//     TCCR2B = 0;           // Stop Timer 2
//     TIMSK2 &= ~(1 << OCIE2A);

//     // Enable Timer 4 Input Capture (Rising Edge)
//     TIFR4 = (1 << ICF4);                 // Clear pending flags
//     TCCR4B = (1 << ICES4) | (1 << CS41); // Rising edge, Prescaler 8 (1 tick = 0.5us)
//     TIMSK4 = (1 << ICIE4);               // Enable capture interrupt

//     state = WAIT_RISING;
// }

// // Timer 4 ISR: Records echo width and sends UART
// ISR(TIMER4_CAPT_vect)
// {
//     if (state == WAIT_RISING)
//     {
//         capture_start = ICR4;
//         TCCR4B &= ~(1 << ICES4); // Switch to Falling Edge
//         TIFR4 = (1 << ICF4);     // Clear flag
//         state = WAIT_FALLING;
//     }
//     else if (state == WAIT_FALLING)
//     {
//         uint16_t capture_end = ICR4;

//         // Stop capture
//         TIMSK4 &= ~(1 << ICIE4);
//         TCCR4B = 0;

//         // Calculate distance
//         // Prescaler 8 @ 16MHz = 0.5us per tick.
//         // Distance (cm) = (Time_us) / 58 = (Ticks * 0.5) / 58 = Ticks / 116
//         uint16_t ticks = capture_end - capture_start; // Overflows handled automatically by 16-bit math
//         uint32_t distance_cm = ticks / 116;

//         // Blocking UART call inside ISR as requested
//         uuart_print("Distance: ");
//         uuart_print_num(distance_cm);
//         uuart_print(" cm\r\n");

//         state = READY;
//     }
// }

// int main(void)
// {
//     uuart_init(115200);

//     // Pin Configuration
//     DDRL |= (1 << PL2); // D47 (Trig) as Output
//     PORTL &= ~(1 << PL2);

//     DDRL &= ~(1 << PL0); // D49 (Echo / ICP4) as Input

//     sei();

//     while (1)
//     {
//         if (state == READY)
//         {
//             PORTL |= (1 << PL2); // Set D47 (Trig) HIGH

//             // Start Timer 2 for 10us delay
//             // Prescaler 8 (0.5us per tick) -> 20 ticks = 10us
//             TCNT2 = 0;
//             OCR2A = 19;
//             TCCR2A = (1 << WGM21);  // CTC Mode
//             TIFR2 = (1 << OCF2A);   // Clear flag
//             TIMSK2 = (1 << OCIE2A); // Enable compare interrupt
//             TCCR2B = (1 << CS21);   // Start timer, prescaler 8

//             state = TRIG_HIGH;

//             // Wait roughly 60ms before allowing the next ping to prevent echo overlap
//             // (Using simple loop delay to keep main loop logic flat)
//             for (volatile uint32_t i = 0; i < 50000; i++)
//                 ;
//         }
//     }

//     return 0;
// }
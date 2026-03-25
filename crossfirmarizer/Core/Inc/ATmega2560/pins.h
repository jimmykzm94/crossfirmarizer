#pragma once

#ifdef TARGET_ATMEGA2560

#include "pin_mode.h"
#include <avr/io.h>

typedef enum
{
    // arduino analog pins
    A0,  // PF0 (ADC0)
    A1,  // PF1 (ADC1)
    A2,  // PF2 (ADC2)
    A3,  // PF3 (ADC3)
    A4,  // PF4 (ADC4)
    A5,  // PF5 (ADC5)
    A6,  // PF6 (ADC6)
    A7,  // PF7 (ADC7)
    A8,  // PK0 (ADC8 / PCINT16)
    A9,  // PK1 (ADC9 / PCINT17)
    A10, // PK2 (ADC10 / PCINT18)
    A11, // PK3 (ADC11 / PCINT19)
    A12, // PK4 (ADC12 / PCINT20)
    A13, // PK5 (ADC13 / PCINT21)
    A14, // PK6 (ADC14 / PCINT22)
    A15, // PK7 (ADC15 / PCINT23)

    // arduino digital pins
    D0,  // PE0 (RXD0 / PCINT8)
    D1,  // PE1 (TXD0)
    D2,  // PE4 (PWM / INT4)
    D3,  // PE5 (PWM / INT5)
    D4,  // PG5 (PWM)
    D5,  // PE3 (PWM / AIN1)
    D6,  // PH3 (OC4A)
    D7,  // PH4 (OC4B)
    D8,  // PH5 (OC4C)
    D9,  // PH6 (OC2B)
    D10, // PB4 (PWM / PCINT4)
    D11, // PB5 (OC1A / PCINT5)
    D12, // PB6 (OC1B / PCINT6)
    D13, // PB7 (PWM / PCINT7 / LED)
    D14, // PJ1 (TXD3 / PCINT10)
    D15, // PJ0 (RXD3 / PCINT9)
    D16, // PH1 (TXD2)
    D17, // PH0 (RXD2)
    D18, // PD3 (TXD1 / INT3)
    D19, // PD2 (RXD1 / INT2)
    D20, // PD1 (SDA / INT1)
    D21, // PD0 (SCL / INT0)
    D22, // PA0 (AD0)
    D23, // PA1 (AD1)
    D24, // PA2 (AD2)
    D25, // PA3 (AD3)
    D26, // PA4 (AD4)
    D27, // PA5 (AD5)
    D28, // PA6 (AD6)
    D29, // PA7 (AD7)
    D30, // PC7 (A15)
    D31, // PC6 (A14)
    D32, // PC5 (A13)
    D33, // PC4 (A12)
    D34, // PC3 (A11)
    D35, // PC2 (A10)
    D36, // PC1 (A9)
    D37, // PC0 (A8)
    D38, // PD7 (T0)
    D39, // PG2 (ALE)
    D40, // PG1 (RD)
    D41, // PG0 (WR)
    D42, // PL7
    D43, // PL6
    D44, // PL5 (PWM)
    D45, // PL4 (PWM)
    D46, // PL3 (PWM)
    D47, // PL2 (T5)
    D48, // PL1 (ICP5)
    D49, // PL0 (ICP4)
    D50, // PB3 (MISO / PCINT3)
    D51, // PB2 (MOSI / PCINT2)
    D52, // PB1 (SCK / PCINT1)
    D53, // PB0 (SS / PCINT0)

    // helpful pins
    PIN_LED, // PB7 (D13)

    // size
    PIN_MAX_COUNT
} pin_arduino_mega_t;

typedef struct
{
    volatile uint8_t *port;
    volatile uint8_t *ddr;
    volatile uint8_t *pin_reg;
    uint8_t pin_num;
} pin_map_t;

extern const pin_map_t pins[PIN_MAX_COUNT];

#endif

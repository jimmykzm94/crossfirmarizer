/*
 * timer.c
 *
 *  Created on: Mar 15, 2026
 *      Author: jimmy
 */

#include "timer.h"
#include "stm32f446xx.h"
#include "gpio.h"
#include "pins.h"
#include <stddef.h>

// Keep track of callbacks for each timer
static TimerCallback tim1_input_capture_callback = NULL;
static TimerCallback tim2_delay_callback = NULL;

volatile uint32_t echo_time_us = 0;
volatile uint8_t capture_state = 0; // 0: idle, 1: waiting for falling edge
volatile uint32_t capture_start = 0;

void timer_delay_init(timer_type_t timer_type, TimerCallback callback)
{
    tim2_delay_callback = callback;
    if (timer_type == TIMER_2)
    {
        // Enable TIM2 clocks
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
        
        // Assuming 16MHz PCLK1, this gives 1MHz timer clock (1us per tick)
        TIM2->PSC = 16 - 1;
        TIM2->CR1 |= TIM_CR1_OPM; // One-pulse mode
        TIM2->DIER |= TIM_DIER_UIE; // Update interrupt enable
        TIM2->SR &= ~TIM_SR_UIF; // Clear flag
        NVIC_EnableIRQ(TIM2_IRQn);
    }
}

void timer_delay_start_us(uint32_t us)
{
    TIM2->ARR = us;
    TIM2->CR1 |= TIM_CR1_CEN; // Start the timer
}

void timer_capture_init(timer_type_t timer_type, pin_t pin, uint32_t timeout, TimerCallback callback)
{
    tim1_input_capture_callback = callback;
    if (timer_type == TIMER_1)
    {
        // Enable TIM1 clocks
        RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

        // Assuming 16MHz PCLK2 for TIM1
        TIM1->PSC = 16 - 1; // 1MHz clock

        if (timeout > 0xFFFF)
        {
            timeout = 0xFFFF;
        }
        // timeout is in us, since PSC is 16-1.
        TIM1->ARR = timeout;

        // Configure PA8 for alternate function TIM1_CH1
        GPIOA->MODER &= ~(3U << (8 * 2));
        GPIOA->MODER |= (2U << (8 * 2));
        // Set alternate function to AF1 (TIM1) for PA8
        GPIOA->AFR[1] &= ~(0xFU << ((8 - 8) * 4));
        GPIOA->AFR[1] |= (1U << ((8 - 8) * 4));

        // Configure TIM1_CH1 for input capture
        TIM1->CCMR1 |= TIM_CCMR1_CC1S_0; // CC1 channel is configured as input, IC1 is mapped on TI1
        TIM1->DIER |= TIM_DIER_CC1IE;    // Capture/Compare 1 interrupt enable

        // Enable Update interrupt for the timeout detection
        TIM1->DIER |= TIM_DIER_UIE;

        NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
        NVIC_EnableIRQ(TIM1_CC_IRQn);
    }
}

void timer_capture_start(timer_type_t timer_type)
{
    echo_time_us = 0;
    capture_state = 0;
    if (timer_type == TIMER_1)
    {
        // Configure for rising edge
        TIM1->CCER &= ~TIM_CCER_CC1P;
        TIM1->SR &= ~TIM_SR_UIF;     // Clear any pending update interrupt (timeout)
        TIM1->SR &= ~TIM_SR_CC1IF;   // Clear capture flag
        TIM1->CCER |= TIM_CCER_CC1E; // Enable capture on CH1
        TIM1->CNT = 0;
        TIM1->CR1 |= TIM_CR1_CEN;    // Start timer
    }
}

timer_type_t timer_get_timer_for_pin(pin_t pin)
{
    // This mapping is specific to STM32F446RE
    if (pins[pin].port == GPIOA && pins[pin].pin == 8)
    {
        return TIMER_1;
    }
    return -1; // Not a valid timer pin
}

void TIM1_CC_IRQHandler(void) {
    if (TIM1->SR & TIM_SR_CC1IF) {
        if (capture_state == 0) { // Rising edge
            capture_start = TIM1->CCR1;
            // Configure for falling edge
            TIM1->CCER |= TIM_CCER_CC1P;
            capture_state = 1;
        } else { // Falling edge
            uint32_t capture_end = TIM1->CCR1;
            echo_time_us = capture_end - capture_start;
            TIM1->CR1 &= ~TIM_CR1_CEN; // Stop timer
            TIM1->CCER &= ~TIM_CCER_CC1E; // Disable capture
            capture_state = 0;

            if (tim1_input_capture_callback != NULL)
                tim1_input_capture_callback(echo_time_us);
        }
        TIM1->SR &= ~TIM_SR_CC1IF; // Clear interrupt flag
    }
}

void TIM1_UP_TIM10_IRQHandler(void) {
    if (TIM1->SR & TIM_SR_UIF) {
        TIM1->SR &= ~TIM_SR_UIF; // Clear interrupt flag
        
        // Sensor timeout reached
        TIM1->CR1 &= ~TIM_CR1_CEN;    // Stop timer
        TIM1->CCER &= ~TIM_CCER_CC1E; // Disable capture
        capture_state = 0;
        
        if (tim1_input_capture_callback != NULL)
            tim1_input_capture_callback(0); // Report timeout
    }
}

void TIM2_IRQHandler(void)
{
    if (TIM2->SR & TIM_SR_UIF)
    {
        TIM2->SR &= ~TIM_SR_UIF; // Clear interrupt flag
        if (tim2_delay_callback != NULL)
        {
            tim2_delay_callback(0); // Call callback, parameter is ignored.
        }
    }
}

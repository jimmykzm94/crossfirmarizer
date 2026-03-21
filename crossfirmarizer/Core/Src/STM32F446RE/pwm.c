#include "pins.h"
#include "stm32f446xx.h"
#include "pwm.h"

// This implementation configures timers for PWM with a configurable period in microseconds.
// It assumes a 16MHz timer clock, and sets the prescaler to get a 1MHz timer clock (1 tick = 1us).

void pwm_config(pin_t pin, uint16_t period_us)
{
    if (pin >= PIN_MAX_COUNT || period_us == 0) return;

    // Enable GPIO port clock
    if (pins[pin].port == GPIOA)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    }
    else if (pins[pin].port == GPIOB)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    }
    else if (pins[pin].port == GPIOC)
    {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    }

    // Configure GPIO for alternate function
    pins[pin].port->MODER &= ~(3U << (pins[pin].pin * 2));
    pins[pin].port->MODER |= (2U << (pins[pin].pin * 2));

    uint16_t arr_value = period_us - 1;

    // Configure timer for PWM
    if (pins[pin].port == GPIOA && pins[pin].pin == 5) // PA5 -> TIM2_CH1
    {
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
        TIM2->PSC = 16 - 1; // 1MHz from 16MHz
        TIM2->ARR = arr_value;
        TIM2->CCMR1 &= ~TIM_CCMR1_OC1M;
        TIM2->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // PWM mode 1
        TIM2->CCER |= TIM_CCER_CC1E;
        TIM2->CR1 |= TIM_CR1_CEN;
        pins[pin].port->AFR[0] &= ~(0xF << (pins[pin].pin * 4));
        pins[pin].port->AFR[0] |= (1U << (pins[pin].pin * 4)); // AF1 for TIM2
    }
    else if (pins[pin].port == GPIOA && pins[pin].pin == 7) // PA7 -> TIM3_CH2
    {
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
        TIM3->PSC = 16 - 1;
        TIM3->ARR = arr_value;
        TIM3->CCMR1 &= ~TIM_CCMR1_OC2M;
        TIM3->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2; // PWM mode 1
        TIM3->CCER |= TIM_CCER_CC2E;
        TIM3->CR1 |= TIM_CR1_CEN;
        pins[pin].port->AFR[0] &= ~(0xF << (pins[pin].pin * 4));
        pins[pin].port->AFR[0] |= (2U << (pins[pin].pin * 4)); // AF2 for TIM3
    }
    else if (pins[pin].port == GPIOB && pins[pin].pin == 3) // PB3 -> TIM2_CH2
    {
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
        TIM2->PSC = 16 - 1;
        TIM2->ARR = arr_value;
        TIM2->CCMR1 &= ~TIM_CCMR1_OC2M;
        TIM2->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2; // PWM mode 1
        TIM2->CCER |= TIM_CCER_CC2E;
        TIM2->CR1 |= TIM_CR1_CEN;
        pins[pin].port->AFR[0] &= ~(0xF << (pins[pin].pin * 4));
        pins[pin].port->AFR[0] |= (1U << (pins[pin].pin * 4)); // AF1 for TIM2
    }
    else if (pins[pin].port == GPIOB && pins[pin].pin == 4) // PB4 -> TIM3_CH1
    {
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
        TIM3->PSC = 16 - 1;
        TIM3->ARR = arr_value;
        TIM3->CCMR1 &= ~TIM_CCMR1_OC1M;
        TIM3->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // PWM mode 1
        TIM3->CCER |= TIM_CCER_CC1E;
        TIM3->CR1 |= TIM_CR1_CEN;
        pins[pin].port->AFR[0] &= ~(0xF << (pins[pin].pin * 4));
        pins[pin].port->AFR[0] |= (2U << (pins[pin].pin * 4)); // AF2 for TIM3
    }
    else if (pins[pin].port == GPIOB && pins[pin].pin == 5) // PB5 -> TIM3_CH2
    {
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
        TIM3->PSC = 16 - 1;
        TIM3->ARR = arr_value;
        TIM3->CCMR1 &= ~TIM_CCMR1_OC2M;
        TIM3->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2; // PWM mode 1
        TIM3->CCER |= TIM_CCER_CC2E;
        TIM3->CR1 |= TIM_CR1_CEN;
        pins[pin].port->AFR[0] &= ~(0xF << (pins[pin].pin * 4));
        pins[pin].port->AFR[0] |= (2U << (pins[pin].pin * 4)); // AF2 for TIM3
    }
    else if (pins[pin].port == GPIOB && pins[pin].pin == 6) // PB6 -> TIM4_CH1
    {
        RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
        TIM4->PSC = 16 - 1;
        TIM4->ARR = arr_value;
        TIM4->CCMR1 &= ~TIM_CCMR1_OC1M;
        TIM4->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // PWM mode 1
        TIM4->CCER |= TIM_CCER_CC1E;
        TIM4->CR1 |= TIM_CR1_CEN;
        pins[pin].port->AFR[0] &= ~(0xF << (pins[pin].pin * 4));
        pins[pin].port->AFR[0] |= (2U << (pins[pin].pin * 4)); // AF2 for TIM4
    }
    else if (pins[pin].port == GPIOB && pins[pin].pin == 10) // PB10 -> TIM2_CH3
    {
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
        TIM2->PSC = 16 - 1;
        TIM2->ARR = arr_value;
        TIM2->CCMR2 &= ~TIM_CCMR2_OC3M;
        TIM2->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2; // PWM mode 1
        TIM2->CCER |= TIM_CCER_CC3E;
        TIM2->CR1 |= TIM_CR1_CEN;
        pins[pin].port->AFR[1] &= ~(0xF << ((pins[pin].pin - 8) * 4));
        pins[pin].port->AFR[1] |= (1U << ((pins[pin].pin - 8) * 4)); // AF1 for TIM2
    }
    else if (pins[pin].port == GPIOC && pins[pin].pin == 7) // PC7 -> TIM3_CH2
    {
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
        TIM3->PSC = 16 - 1;
        TIM3->ARR = arr_value;
        TIM3->CCMR1 &= ~TIM_CCMR1_OC2M;
        TIM3->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2; // PWM mode 1
        TIM3->CCER |= TIM_CCER_CC2E;
        TIM3->CR1 |= TIM_CR1_CEN;
        pins[pin].port->AFR[0] &= ~(0xF << (pins[pin].pin * 4));
        pins[pin].port->AFR[0] |= (2U << (pins[pin].pin * 4)); // AF2 for TIM3
    }
}

void pwm_write(pin_t pin, uint16_t value)
{
    if (pin >= PIN_MAX_COUNT) return;

    if (pins[pin].port == GPIOA && pins[pin].pin == 5) { TIM2->CCR1 = value; }
    else if (pins[pin].port == GPIOA && pins[pin].pin == 7) { TIM3->CCR2 = value; }
    else if (pins[pin].port == GPIOB && pins[pin].pin == 3) { TIM2->CCR2 = value; }
    else if (pins[pin].port == GPIOB && pins[pin].pin == 4) { TIM3->CCR1 = value; }
    else if (pins[pin].port == GPIOB && pins[pin].pin == 5) { TIM3->CCR2 = value; }
    else if (pins[pin].port == GPIOB && pins[pin].pin == 6) { TIM4->CCR1 = value; }
    else if (pins[pin].port == GPIOB && pins[pin].pin == 10) { TIM2->CCR3 = value; }
    else if (pins[pin].port == GPIOC && pins[pin].pin == 7) { TIM3->CCR2 = value; }
}

#include "pwm.h"
#include "pins.h"
#include "sam.h"
#include <stddef.h>

#include "command.h"
#include <stdio.h>
#include <string.h>

typedef struct
{
    pin_t pin;
    tc_registers_t *tc;
    uint8_t port_group;
    uint8_t gclk_id;
} pwm_map_t;

static const pwm_map_t pwm_maps[] = {
    {PIN_EXT1_PIN8_PWM_NEG, TC6_REGS, MUX_PB03F_TC6_WO1, GCLK_CLKCTRL_ID_TC6_TC7_Val},
    {PIN_EXT2_PIN8_PWM_NEG, TC4_REGS, MUX_PA23F_TC4_WO1, GCLK_CLKCTRL_ID_TC4_TC5_Val},
    {PIN_EXT3_PIN8_PWM_NEG, TC2_REGS, MUX_PA13E_TC2_WO1, GCLK_CLKCTRL_ID_TC2_TC3_Val},
};

static const pwm_map_t *get_pwm_map(pin_t pin)
{
    for (uint8_t i = 0; i < sizeof(pwm_maps) / sizeof(pwm_map_t); i++)
    {
        if (pwm_maps[i].pin == pin)
        {
            return &pwm_maps[i];
        }
    }
    return NULL;
}

void pwm_config(pin_t pin, uint16_t period_us)
{
    const pwm_map_t *map = get_pwm_map(pin);
    if (!map)
        return;

    // 1. Instantly apply pre-calculated MUX from pins.h
    uint8_t port_num = PIN_GET_PORT(map->pin);
    uint8_t pin_num = PIN_GET_PIN(map->pin);
    if (pin_num % 2 == 0)
    {
        PORT_REGS->GROUP[port_num].PORT_PMUX[pin_num >> 1] &= ~PORT_PMUX_PMUXE_Msk;
        PORT_REGS->GROUP[port_num].PORT_PMUX[pin_num >> 1] |= PORT_PMUX_PMUXE(map->port_group);
    }
    else
    {
        PORT_REGS->GROUP[port_num].PORT_PMUX[pin_num >> 1] &= ~PORT_PMUX_PMUXO_Msk;
        PORT_REGS->GROUP[port_num].PORT_PMUX[pin_num >> 1] |= PORT_PMUX_PMUXO(map->port_group);
    }
    PORT_REGS->GROUP[port_num].PORT_PINCFG[pin_num] = PORT_PINCFG_PMUXEN_Msk;

    // 2. Enable APBC Clock for the specific TC
    if (map->tc == TC2_REGS)
        PM_REGS->PM_APBCMASK |= PM_APBCMASK_TC2_Msk;
    else if (map->tc == TC4_REGS)
        PM_REGS->PM_APBCMASK |= PM_APBCMASK_TC4_Msk;
    else if (map->tc == TC6_REGS)
        PM_REGS->PM_APBCMASK |= PM_APBCMASK_TC6_Msk;

    // 3. Enable GCLK
    GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN_Msk |
                              GCLK_CLKCTRL_GEN_GCLK0 |
                              GCLK_CLKCTRL_ID(map->gclk_id);
    while (GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk)
        ;

    // Reset TC
    map->tc->COUNT16.TC_CTRLA |= TC_CTRLA_SWRST_Msk;
    while (map->tc->COUNT16.TC_STATUS & TC_STATUS_SYNCBUSY_Msk)
        ;

    // 4. Configure TC in Match PWM mode
    map->tc->COUNT16.TC_CTRLA = TC_CTRLA_MODE_COUNT16 |
                                TC_CTRLA_WAVEGEN_MPWM |  // Switch to Match PWM
                                TC_CTRLA_PRESCALER_DIV4; // 8MHz / 4 = 2MHz clock

    // Set the period
    map->tc->COUNT16.TC_CC[0] = period_us * 2;
    while (map->tc->COUNT16.TC_STATUS & TC_STATUS_SYNCBUSY_Msk)
        ;

    // 5. Enable TC
    map->tc->COUNT16.TC_CTRLA |= TC_CTRLA_ENABLE_Msk;
    while (map->tc->COUNT16.TC_STATUS & TC_STATUS_SYNCBUSY_Msk)
        ;
}

void pwm_write(pin_t pin, uint16_t value)
{
    const pwm_map_t *map = get_pwm_map(pin);
    if (!map)
        return;

    map->tc->COUNT16.TC_CC[1] = value;
    while (map->tc->COUNT16.TC_STATUS & TC_STATUS_SYNCBUSY_Msk)
        ;
}
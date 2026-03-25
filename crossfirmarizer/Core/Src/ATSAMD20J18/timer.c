#include "timer.h"
#include "sam.h"
#include "pins.h"
#include <stddef.h>
#include <stdbool.h>

#define EXT_COUNT 3

static TimerCallback delay_callback = NULL;
static TimerCallback capture_callback[EXT_COUNT] = {NULL, NULL, NULL};
static volatile bool awaiting_falling_edge[EXT_COUNT] = {false, false, false};
static pin_t current_capture_pin[EXT_COUNT] = {PIN_MAX_COUNT, PIN_MAX_COUNT, PIN_MAX_COUNT};
static volatile uint16_t capture_start_count[EXT_COUNT] = {0, 0, 0};

// static tc_registers_t *get_tc_instance(uint8_t timer)
// {
//     if (timer <= 7)
//     {
//         return (TC0_REGS + (timer * 0x400));
//     }
//     return NULL;
// }

void timer_delay_init(timer_type_t timer_type, TimerCallback callback)
{
    // FIXME For now use timer 2
    if (timer_type == TIMER_2)
    {
        delay_callback = callback;

        // Enable TC7 bus clock
        PM_REGS->PM_APBCMASK |= PM_APBCMASK_TC7_Msk;

        // Enable GCLK for TC7
        GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN_Msk |
                                  GCLK_CLKCTRL_GEN_GCLK0 |
                                  GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TC6_TC7_Val);
        while (GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk)
            ;

        // Configure TC7 for one-shot mode
        TC7_REGS->COUNT16.TC_CTRLA = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_PRESCALER_DIV1;
        TC7_REGS->COUNT16.TC_CTRLBSET = TC_CTRLBSET_ONESHOT_Msk;
        TC7_REGS->COUNT16.TC_INTENSET = TC_INTENSET_OVF_Msk;

        // Enable TC7 interrupt
        NVIC_EnableIRQ(TC7_IRQn);
    }
}

void timer_delay_start_us(uint32_t us)
{
    uint32_t ticks = (SystemCoreClock / 1000000) * us - 1;

    TC7_REGS->COUNT16.TC_CC[0] = ticks;
    while (TC7_REGS->COUNT16.TC_STATUS & TC_STATUS_SYNCBUSY_Msk)
        ;

    TC7_REGS->COUNT16.TC_CTRLA |= TC_CTRLA_ENABLE_Msk;
    while (TC7_REGS->COUNT16.TC_STATUS & TC_STATUS_SYNCBUSY_Msk)
        ;
}

void TC7_Handler(void)
{
    if ((TC7_REGS->COUNT16.TC_INTFLAG & TC_INTFLAG_OVF_Msk) &&
        (TC7_REGS->COUNT16.TC_INTENSET & TC_INTENSET_OVF_Msk))
    {

        TC7_REGS->COUNT16.TC_INTFLAG = TC_INTFLAG_OVF_Msk; // Clear flag
        if (delay_callback)
        {
            delay_callback(0);
        }
    }
}

void timer_capture_init(timer_type_t timer_type, pin_t pin, uint32_t timeout, TimerCallback callback)
{
    (void)timeout; // Unused in this implementation

    // FIXME For now use timer 2, IRQ for EXTx only
    if (timer_type != TIMER_2)
        return;

    uint8_t ext_idx;
    uint8_t extint_chan;
    uint8_t port_group;
    
    if (pin == PIN_EXT1_PIN9_IRQ) {
        ext_idx = 0;
        extint_chan = 4;
        port_group = 1; // PB04
    } else if (pin == PIN_EXT2_PIN9_IRQ) {
        ext_idx = 1;
        extint_chan = 14;
        port_group = 1; // PB14
    } else if (pin == PIN_EXT3_PIN9_IRQ) {
        ext_idx = 2;
        extint_chan = 8;
        port_group = 0; // PA28
    } else {
        return;
    }

    capture_callback[ext_idx] = callback;
    current_capture_pin[ext_idx] = pin;

    // --- EIC Configuration ---
    // 1. Enable GCLK for EIC
    GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN_Msk |
                              GCLK_CLKCTRL_GEN_GCLK0 |
                              GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_EIC_Val);
    while (GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk)
        ;

    uint8_t pin_num = PIN_GET_PIN(pin);

    // 2. Configure Pin Mux for EIC
    // EIC is Function A (PMUX = 0)
    if (pin_num % 2 == 0) {
        PORT_REGS->GROUP[port_group].PORT_PMUX[pin_num >> 1] = 
            (PORT_REGS->GROUP[port_group].PORT_PMUX[pin_num >> 1] & ~PORT_PMUX_PMUXE_Msk) | PORT_PMUX_PMUXE(0);
    } else {
        PORT_REGS->GROUP[port_group].PORT_PMUX[pin_num >> 1] = 
            (PORT_REGS->GROUP[port_group].PORT_PMUX[pin_num >> 1] & ~PORT_PMUX_PMUXO_Msk) | PORT_PMUX_PMUXO(0);
    }
    PORT_REGS->GROUP[port_group].PORT_PINCFG[pin_num] |= PORT_PINCFG_PMUXEN_Msk | PORT_PINCFG_INEN_Msk;

    // 3. Configure EIC
    EIC_REGS->EIC_CTRL &= ~EIC_CTRL_ENABLE_Msk; // Disable EIC to configure
    while (EIC_REGS->EIC_STATUS & EIC_STATUS_SYNCBUSY_Msk)
        ;

    uint8_t config_idx = extint_chan / 8;
    uint8_t config_shift = 4 * (extint_chan % 8);

    // Clear existing sense config, then set to BOTH edges
    EIC_REGS->EIC_CONFIG[config_idx] &= ~(0xF << config_shift);
    EIC_REGS->EIC_CONFIG[config_idx] |= (EIC_CONFIG_SENSE0_BOTH_Val << config_shift);

    EIC_REGS->EIC_INTENSET = (1 << extint_chan);

    EIC_REGS->EIC_CTRL |= EIC_CTRL_ENABLE_Msk;
    while (EIC_REGS->EIC_STATUS & EIC_STATUS_SYNCBUSY_Msk)
        ;

    // 4. Enable EIC Interrupt
    NVIC_EnableIRQ(EIC_IRQn);

    // --- TC5 Configuration ---
    // 1. Enable TC5 Bus Clock
    PM_REGS->PM_APBCMASK |= PM_APBCMASK_TC5_Msk;

    // 2. Enable GCLK for TC5
    GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN_Msk |
                              GCLK_CLKCTRL_GEN_GCLK0 |
                              GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_TC4_TC5_Val);
    while (GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk)
        ;

    // Disable TC5 before configuring
    TC5_REGS->COUNT16.TC_CTRLA &= ~TC_CTRLA_ENABLE_Msk;
    while (TC5_REGS->COUNT16.TC_STATUS & TC_STATUS_SYNCBUSY_Msk)
        ;

    // 3. Configure TC5
    // Assuming GCLK0 is 8MHz. DIV8 prescaler results in 1 tick = 1 microsecond.
    TC5_REGS->COUNT16.TC_CTRLA = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_PRESCALER_DIV8;

    // Start TC5 in free-running mode to allow concurrent pin captures
    TC5_REGS->COUNT16.TC_CTRLA |= TC_CTRLA_ENABLE_Msk;
    while (TC5_REGS->COUNT16.TC_STATUS & TC_STATUS_SYNCBUSY_Msk)
        ;
}

void timer_capture_start(timer_type_t timer_type)
{
    if (timer_type != TIMER_2)
        return;
    for (int i = 0; i < EXT_COUNT; i++) {
        awaiting_falling_edge[i] = false;
    }
    // The EIC interrupt handles the rest automatically
}

void EIC_Handler(void)
{
    // Capture flags and clear immediately to prevent double-triggering
    uint32_t intflags = EIC_REGS->EIC_INTFLAG;
    EIC_REGS->EIC_INTFLAG = intflags;

    // Request read sync to safely read the free-running TC5
    TC5_REGS->COUNT16.TC_READREQ = TC_READREQ_RREQ_Msk | 0x10;
    while (TC5_REGS->COUNT16.TC_STATUS & TC_STATUS_SYNCBUSY_Msk)
        ;
    uint16_t current_count = TC5_REGS->COUNT16.TC_COUNT;

    for (int i = 0; i < EXT_COUNT; i++) {
        pin_t pin = current_capture_pin[i];
        if (pin == PIN_MAX_COUNT) continue;

        uint8_t extint_chan = (i == 0) ? 4 : (i == 1) ? 14 : 8;

        if (intflags & (1 << extint_chan)) {
            // Dynamically read the state of whichever pin triggered the interrupt
            uint8_t port_num = PIN_GET_PORT(pin);
            uint8_t pin_num = PIN_GET_PIN(pin);
            uint32_t pin_val = PORT_REGS->GROUP[port_num].PORT_IN & (1 << pin_num);

            if (pin_val) {
                // Rising edge
                awaiting_falling_edge[i] = true;
                capture_start_count[i] = current_count;
            } else {
                // Falling edge
                if (awaiting_falling_edge[i]) {
                    // 16-bit math automatically handles timer wrap-around safely
                    uint32_t elapsed = (uint32_t)(current_count - capture_start_count[i]) & 0xFFFF;
                    if (capture_callback[i]) {
                        capture_callback[i](elapsed);
                    }
                    awaiting_falling_edge[i] = false;
                }
            }
        }
    }
}

timer_type_t timer_get_timer_for_pin(pin_t pin)
{
    if (pin == PIN_EXT1_PIN9_IRQ || pin == PIN_EXT2_PIN9_IRQ || pin == PIN_EXT3_PIN9_IRQ)
    {
        return TIMER_2;
    }
    return (timer_type_t)-1;
}
#include "uart.h"
#include <sam.h>

#define RX_BUFFER_MASK (RX_BUFFER_SIZE - 1)
#define TX_BUFFER_MASK (TX_BUFFER_SIZE - 1)

static volatile uint8_t rx_buf[RX_BUFFER_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

static volatile uint8_t tx_buf[TX_BUFFER_SIZE];
static volatile uint16_t tx_head = 0;
static volatile uint16_t tx_tail = 0;

// 8MHz Generic Clock
const uint32_t B_9600 = 64276;
const uint32_t B_115200 = 50436;

void uart_init(uint32_t baudrate)
{
    // 0. Change to 8MHz clock
    // FIXME: Change clock somewhere else
    SYSCTRL_REGS->SYSCTRL_OSC8M &= ~SYSCTRL_OSC8M_PRESC_Msk;
    SystemCoreClock = 8000000;

    // 1. Pin Muxing for PA24 (TX) and PA25 (RX) - Virtual COM Port [cite: 314, 315]
    // Both use Peripheral Function C (index 2) for SERCOM3
    PORT_REGS->GROUP[0].PORT_PMUX[24 >> 1] = PORT_PMUX_PMUXE(2) | PORT_PMUX_PMUXO(2);
    PORT_REGS->GROUP[0].PORT_PINCFG[24] |= PORT_PINCFG_PMUXEN_Msk;
    PORT_REGS->GROUP[0].PORT_PINCFG[25] |= PORT_PINCFG_PMUXEN_Msk;

    // 2. Enable APB clock for SERCOM3
    PM_REGS->PM_APBCMASK |= PM_APBCMASK_SERCOM3_Msk;

    // 3. Enable GCLK for SERCOM3
    GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN_Msk |
                              GCLK_CLKCTRL_GEN_GCLK0 |
                              GCLK_CLKCTRL_ID_SERCOM3_CORE;
    while (GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk)
        ;

    // 4. Reset SERCOM3
    SERCOM3_REGS->USART_INT.SERCOM_CTRLA = SERCOM_USART_INT_CTRLA_SWRST_Msk;
    while (SERCOM3_REGS->USART_INT.SERCOM_STATUS & SERCOM_USART_INT_STATUS_SYNCBUSY_Msk)
        ;

    // 5. Configure CTRLA
    SERCOM3_REGS->USART_INT.SERCOM_CTRLA =
        SERCOM_USART_INT_CTRLA_MODE_USART_INT_CLK | // Internal clock
        SERCOM_USART_INT_CTRLA_RXPO_PAD3 |          // RX on PAD[3] (PA25) [cite: 314, 315]
        SERCOM_USART_INT_CTRLA_TXPO_PAD1 |          // TXPO_PAD1 means TX on PAD[2] (PA24) [cite: 314, 315]
        SERCOM_USART_INT_CTRLA_DORD_LSB;            // LSB first

    // 6. Configure CTRLB (8 data bits, 1 stop bit, no parity)
    SERCOM3_REGS->USART_INT.SERCOM_CTRLB =
        SERCOM_USART_INT_CTRLB_RXEN_Msk |
        SERCOM_USART_INT_CTRLB_TXEN_Msk |
        SERCOM_USART_INT_CTRLB_CHSIZE_8_BIT;
    while (SERCOM3_REGS->USART_INT.SERCOM_STATUS & SERCOM_USART_INT_STATUS_SYNCBUSY_Msk)
        ;

    // 7. Set Baud Rate
    if (baudrate == 9600)
        SERCOM3_REGS->USART_INT.SERCOM_BAUD = B_9600;
    else
        SERCOM3_REGS->USART_INT.SERCOM_BAUD = B_115200;

    // 8. Enable RX Interrupts in SERCOM & NVIC
    SERCOM3_REGS->USART_INT.SERCOM_INTENSET = SERCOM_USART_INT_INTENSET_RXC_Msk;
    NVIC_EnableIRQ(SERCOM3_IRQn);

    // 9. Enable SERCOM3
    SERCOM3_REGS->USART_INT.SERCOM_CTRLA |= SERCOM_USART_INT_CTRLA_ENABLE_Msk;
    while (SERCOM3_REGS->USART_INT.SERCOM_STATUS & SERCOM_USART_INT_STATUS_SYNCBUSY_Msk)
        ;
}

void uart_write(const uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        uint16_t next_head = (tx_head + 1) & TX_BUFFER_MASK;

        // If buffer is full, drop the remaining bytes and break rather than freezing
        if (next_head == tx_tail)
            break;

        tx_buf[tx_head] = data[i];
        tx_head = next_head;
    }

    SERCOM3_REGS->USART_INT.SERCOM_INTENSET = SERCOM_USART_INT_INTENSET_DRE_Msk;
}

uint8_t uart_read(void)
{
    if (rx_head == rx_tail)
        return 0;

    uint8_t data = rx_buf[rx_tail];
    // rx_tail = (rx_tail + 1) % RX_BUFFER_SIZE;
    rx_tail = (rx_tail + 1) & RX_BUFFER_MASK;
    return data;
}

int uart_available(void)
{
    // return (RX_BUFFER_SIZE + rx_head - rx_tail) % RX_BUFFER_SIZE;
    return (rx_head - rx_tail) & RX_BUFFER_MASK;
}

int uart_tx_busy(void)
{
    return (tx_head != tx_tail) ||
           !(SERCOM3_REGS->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_TXC_Msk);
}

void SERCOM3_Handler(void)
{
    if (SERCOM3_REGS->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_RXC_Msk)
    {
        uint8_t data = SERCOM3_REGS->USART_INT.SERCOM_DATA;
        // uint16_t next_head = (rx_head + 1) % RX_BUFFER_SIZE;
        uint16_t next_head = (rx_head + 1) & RX_BUFFER_MASK;

        if (next_head != rx_tail)
        {
            rx_buf[rx_head] = data;
            rx_head = next_head;
        }
    }

    if ((SERCOM3_REGS->USART_INT.SERCOM_INTENSET & SERCOM_USART_INT_INTENSET_DRE_Msk) &&
        (SERCOM3_REGS->USART_INT.SERCOM_INTFLAG & SERCOM_USART_INT_INTFLAG_DRE_Msk))
    {
        if (tx_head != tx_tail)
        {
            SERCOM3_REGS->USART_INT.SERCOM_DATA = tx_buf[tx_tail];
            // tx_tail = (tx_tail + 1) % TX_BUFFER_SIZE;
            tx_tail = (tx_tail + 1) & TX_BUFFER_MASK;
        }
        else
        {
            SERCOM3_REGS->USART_INT.SERCOM_INTENCLR = SERCOM_USART_INT_INTENCLR_DRE_Msk;
        }
    }
}
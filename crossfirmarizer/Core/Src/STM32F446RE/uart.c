#include "uart.h"
#include "stm32f446xx.h"

static uint8_t rx_buffer[RX_BUFFER_SIZE];
static int rx_buffer_head = 0;
static int rx_buffer_tail = 0;

void uart_init(uint32_t baudrate)
{
    // Enable clock for GPIOA and UART2
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // Configure PA2 and PA3 for alternate function
    GPIOA->MODER |= GPIO_MODER_MODER2_1 | GPIO_MODER_MODER3_1;
    GPIOA->AFR[0] |= (7 << GPIO_AFRL_AFSEL2_Pos) | (7 << GPIO_AFRL_AFSEL3_Pos);

    // Configure UART2
    // 8-N-1, 115200 baud
    // Assuming a 16MHz HSI clock
    USART2->BRR = (16000000 / 115200);
    USART2->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;

    // Enable USART2 interrupt
    NVIC_SetPriority(USART2_IRQn, 0);
    NVIC_EnableIRQ(USART2_IRQn);
}

void uart_write(const uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        while (!(USART2->SR & USART_SR_TXE))
            ;
        USART2->DR = data[i];
    }
}

uint8_t uart_read(void)
{
    while (rx_buffer_head == rx_buffer_tail)
    {
        // Wait for data
    }
    uint8_t c = rx_buffer[rx_buffer_tail];
    rx_buffer_tail = (rx_buffer_tail + 1) % RX_BUFFER_SIZE;
    return c;
}

void USART2_IRQHandler(void)
{
    if (USART2->SR & USART_SR_RXNE)
    {
        uint8_t c = USART2->DR;
        int next_head = (rx_buffer_head + 1) % RX_BUFFER_SIZE;
        if (next_head != rx_buffer_tail)
        {
            rx_buffer[rx_buffer_head] = c;
            rx_buffer_head = next_head;
        }
    }
}
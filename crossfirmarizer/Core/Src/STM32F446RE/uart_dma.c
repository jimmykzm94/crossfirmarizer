#include "uart_dma.h"
#include "stm32f4xx.h"

// RX Buffer
static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint16_t rx_read_pos = 0;

// TX Ring Buffer
static uint8_t tx_buffer[TX_BUFFER_SIZE];
static volatile uint16_t tx_write_pos = 0;
static volatile uint16_t tx_read_pos = 0;
static volatile uint16_t current_tx_len = 0;

static void _uart_dma_start_tx_from_buffer(void)
{
    // If the buffer is empty or a transfer is already in progress, do nothing
    if (tx_read_pos == tx_write_pos || (DMA1_Stream6->CR & DMA_SxCR_EN))
    {
        return;
    }

    uint16_t len;
    // Calculate the length of the continuous block to send
    if (tx_write_pos > tx_read_pos)
    {
        // Data is in a single continuous block
        len = tx_write_pos - tx_read_pos;
    }
    else
    {
        // Data wraps around, send the block from read_pos to the end of the buffer
        len = TX_BUFFER_SIZE - tx_read_pos;
    }

    // Save the length of this transfer
    current_tx_len = len;

    // Configure DMA stream
    DMA1->HIFCR |= DMA_HIFCR_CTCIF6; // Clear TX complete flag
    DMA1_Stream6->M0AR = (uint32_t)&tx_buffer[tx_read_pos];
    DMA1_Stream6->NDTR = len;
    DMA1_Stream6->CR |= DMA_SxCR_EN; // Start transmission
}

void uart_dma_init(uint32_t baudrate)
{
    // 1. Enable Clocks: GPIOA, USART2, DMA1
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_DMA1EN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // 2. Configure GPIO (PA2 = TX, PA3 = RX as Alternate Function 7)
    GPIOA->MODER &= ~((3U << (2 * 2)) | (3U << (3 * 2)));
    GPIOA->MODER |= ((2U << (2 * 2)) | (2U << (3 * 2)));
    GPIOA->AFR[0] |= (7U << (2 * 4)) | (7U << (3 * 4));

    // 3. Configure USART2
    USART2->CR1 &= ~USART_CR1_UE; // Disable USART for config

    // Calculate Baudrate with rounding. Assumes 16MHz APB1 Clock.
    uint32_t apb1_clk = 16000000;
    USART2->BRR = (apb1_clk + (baudrate / 2)) / baudrate;

    USART2->CR3 |= USART_CR3_DMAT | USART_CR3_DMAR; // Enable DMA TX/RX
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE;     // Enable TX/RX
    USART2->CR1 |= USART_CR1_UE;                    // Enable USART

    // 4. Configure DMA1 Stream 5 for RX (Peripheral to Memory, Circular)
    DMA1_Stream5->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream5->CR & DMA_SxCR_EN);

    DMA1_Stream5->PAR = (uint32_t)&USART2->DR;
    DMA1_Stream5->M0AR = (uint32_t)rx_buffer;
    DMA1_Stream5->NDTR = RX_BUFFER_SIZE;
    DMA1_Stream5->CR = (4U << 25) |    // Channel 4
                       DMA_SxCR_MINC | // Memory increment mode
                       DMA_SxCR_CIRC;  // Circular mode

    DMA1_Stream5->CR |= DMA_SxCR_EN; // Enable RX DMA

    // 5. Configure DMA1 Stream 6 for TX (Memory to Peripheral)
    DMA1_Stream6->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream6->CR & DMA_SxCR_EN);

    DMA1_Stream6->PAR = (uint32_t)&USART2->DR;
    DMA1_Stream6->CR = (4U << 25) |     // Channel 4
                       DMA_SxCR_MINC |  // Memory increment
                       DMA_SxCR_DIR_0 | // Memory-to-peripheral direction
                       DMA_SxCR_TCIE;   // Transfer complete interrupt

    // 6. Enable DMA TX Interrupt in NVIC
    NVIC_SetPriority(DMA1_Stream6_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}

void uart_dma_send(const uint8_t *data, uint16_t length)
{
    // Copy data to the ring buffer
    for (uint16_t i = 0; i < length; i++)
    {
        // Wait if the buffer is full.
        // A more robust implementation might return an error or false.
        while (((tx_write_pos + 1) % TX_BUFFER_SIZE) == tx_read_pos);

        tx_buffer[tx_write_pos] = data[i];
        tx_write_pos = (tx_write_pos + 1) % TX_BUFFER_SIZE;
    }

    // Enter critical section to check and start DMA transfer
    __disable_irq();
    // If DMA is not currently active, start a new transfer
    if (!(DMA1_Stream6->CR & DMA_SxCR_EN))
    {
        _uart_dma_start_tx_from_buffer();
    }
    __enable_irq();
}

uint16_t uart_dma_rx_available(void)
{
    // NDTR counts down, so the write index is the buffer size minus NDTR
    uint16_t rx_write_pos = RX_BUFFER_SIZE - DMA1_Stream5->NDTR;

    if (rx_write_pos >= rx_read_pos)
    {
        return rx_write_pos - rx_read_pos;
    }
    else
    {
        // Buffer wrapped around
        return RX_BUFFER_SIZE - rx_read_pos + rx_write_pos;
    }
}

uint16_t uart_dma_rx_read(uint8_t *dest, uint16_t len)
{
    uint16_t available = uart_dma_rx_available();

    // Only read what is available
    if (len > available)
    {
        len = available;
    }

    for (uint16_t i = 0; i < len; i++)
    {
        dest[i] = rx_buffer[rx_read_pos];
        rx_read_pos = (rx_read_pos + 1) % RX_BUFFER_SIZE; // Advance and wrap
    }

    return len;
}

void DMA1_Stream6_IRQHandler(void)
{
    // Check for Transfer Complete interrupt
    if (DMA1->HISR & DMA_HISR_TCIF6)
    {
        // Clear the interrupt flag
        DMA1->HIFCR |= DMA_HIFCR_CTCIF6;

        // Update the read position by the number of bytes just transferred
        if (current_tx_len > 0)
        {
            tx_read_pos = (tx_read_pos + current_tx_len) % TX_BUFFER_SIZE;
            current_tx_len = 0; // Clear the length
        }

        // Start the next transfer if there is more data in the buffer
        _uart_dma_start_tx_from_buffer();
    }
}

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define RX_BUFFER_SIZE 256
#define TX_BUFFER_SIZE 256

// Initializes the UART hardware and DMA streams
void uart_dma_init(uint32_t baudrate);

// Queues data for non-blocking DMA transfer
void uart_dma_send(const uint8_t *data, uint16_t length);

// Returns the number of unread bytes currently in the circular buffer
uint16_t uart_dma_rx_available(void);

// Reads up to 'len' bytes into 'dest' buffer. Returns actual bytes read.
uint16_t uart_dma_rx_read(uint8_t *dest, uint16_t len);
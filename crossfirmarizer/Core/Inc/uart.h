#pragma once
#include <stdint.h>

#define RX_BUFFER_SIZE 256
#define TX_BUFFER_SIZE 256

void uart_init(void);
void uart_write(const uint8_t *data, uint16_t len);
uint8_t uart_read(void);
int uart_available(void);
int uart_tx_busy(void);

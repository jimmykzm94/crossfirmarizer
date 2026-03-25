#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define BAUD 115200
#define MYUBRR (F_CPU/8/BAUD-1)

static volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
static volatile uint8_t rx_buffer_head = 0;
static volatile uint8_t rx_buffer_tail = 0;

static volatile uint8_t tx_buffer[TX_BUFFER_SIZE];
static volatile uint8_t tx_buffer_head = 0;
static volatile uint8_t tx_buffer_tail = 0;

void uart_init(uint32_t baudrate)
{
    // Enable double speed mode
    UCSR0A |= (1 << U2X0);

    // Set baud rate
    UBRR0H = (unsigned char)(MYUBRR>>8);
    UBRR0L = (unsigned char)MYUBRR;

    // Enable receiver and transmitter, and receive complete interrupt
    UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);

    // Set frame format: 8data, 1stop bit
    UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);

    // Enable global interrupts
    sei();
}

void uart_write(const uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        uint8_t next_head = (tx_buffer_head + 1);
        if (next_head != tx_buffer_tail)
        {
            tx_buffer[tx_buffer_head] = data[i];
            tx_buffer_head = next_head;
        }
    }
    // Enable UDRE interrupt
    UCSR0B |= (1 << UDRIE0);
}

uint8_t uart_read(void)
{
    uint8_t c = rx_buffer[rx_buffer_tail];
    rx_buffer_tail = (rx_buffer_tail + 1);
    return c;
}

ISR(USART0_RX_vect)
{
    uint8_t c = UDR0;
    uint8_t next_head = (rx_buffer_head + 1);
    if (next_head != rx_buffer_tail)
    {
        rx_buffer[rx_buffer_head] = c;
        rx_buffer_head = next_head;
    }
}

ISR(USART0_UDRE_vect)
{
    if (tx_buffer_head == tx_buffer_tail)
    {
        // Buffer is empty, disable UDRE interrupt
        UCSR0B &= ~(1 << UDRIE0);
    }
    else
    {
        // Get a character from the buffer and send it
        UDR0 = tx_buffer[tx_buffer_tail];
        tx_buffer_tail = (tx_buffer_tail + 1);
    }
}

int uart_available(void)
{
    return (uint8_t)(rx_buffer_head - rx_buffer_tail);
}

int uart_tx_busy(void)
{
    // Check if the UDRE interrupt is enabled, or if the buffer is not empty
    return (UCSR0B & (1 << UDRIE0)) || (tx_buffer_head != tx_buffer_tail);
}

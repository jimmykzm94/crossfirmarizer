#include "command.h"
#include "uart.h"

// TODO use #define for UART normal or DMA

uint8_t rx_byte;
uint8_t encoded_buf[MAX_ENCODED_LEN];
uint16_t buf_idx = 0;
Packet current_packet;
uint8_t response_buf[MAX_ENCODED_LEN];
size_t response_len;

void command_hw_init(void)
{
    uart_init();
}
void command_send(uint8_t cmd, const uint8_t *payload, size_t payload_len)
{
    response_len = create_packet(cmd, payload, payload_len, response_buf);
    uart_write(response_buf, response_len);
}
void command_receive(void (*command_handler)(Packet))
{
    if (uart_available() > 0)
    {
        rx_byte = uart_read();

        if (rx_byte == 0x00)
        {
            if (buf_idx > 0)
            {
                if (parse_packet(encoded_buf, buf_idx, &current_packet))
                {
                    command_handler(current_packet);
                }
            }
            buf_idx = 0;
        }
        else
        {
            if (buf_idx < MAX_ENCODED_LEN)
            {
                encoded_buf[buf_idx++] = rx_byte;
            }
            else
            {
                buf_idx = 0;
            }
        }
    }
}
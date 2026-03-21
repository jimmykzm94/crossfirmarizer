#include "command.h"
#include "uart_dma.h"
#include "serial_packet.h"

// TODO use #define for UART normal or DMA
uint8_t rx_byte;
uint8_t encoded_buf[MAX_ENCODED_LEN];
uint16_t buf_idx = 0;
Packet current_packet;
uint8_t response_buf[MAX_ENCODED_LEN];
size_t response_len;

void command_hw_init(void)
{
    uart_dma_init(115200);
}
void command_send(uint8_t cmd, const uint8_t *payload, size_t payload_len)
{
    response_len = create_packet(cmd, payload, payload_len, response_buf);
    uart_dma_send(response_buf, response_len);
}
void command_receive(void (*command_handler)(Packet))
{
    // Non-blocking: Drain the DMA circular buffer if bytes exist
    while (uart_dma_rx_available() > 0)
    {
        uart_dma_rx_read(&rx_byte, 1);

        if (rx_byte == 0x00)
        {
            // End of Frame detected
            if (buf_idx > 0)
            {
                // Attempt to parse the complete frame
                if (parse_packet(encoded_buf, buf_idx, &current_packet))
                {
                    // Valid packet received! Route the command.
                    command_handler(current_packet);
                }
            }
            // Reset buffer index for the next incoming frame
            buf_idx = 0;
        }
        else
        {
            // Collect incoming bytes
            if (buf_idx < MAX_ENCODED_LEN)
            {
                encoded_buf[buf_idx++] = rx_byte;
            }
            else
            {
                // Error: Buffer overflow. Frame is too large or missing a 0x00 delimiter.
                // Reset index to resync on the next 0x00.
                buf_idx = 0;
            }
        }
    }
}

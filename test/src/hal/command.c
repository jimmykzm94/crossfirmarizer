#include "command.h"
#include "simulation_state.h"
#include <stdio.h>

// FIXME command.c is consider application layer, for firmware code need use #ifdef to use UART or DMA

uint8_t rx_byte;
uint8_t encoded_buf[MAX_ENCODED_LEN];
uint16_t buf_idx = 0;
Packet current_packet;
uint8_t response_buf[MAX_ENCODED_LEN];
size_t response_len;

extern simulation_state_t simulation_state;
FILE *fp;

void command_hw_init(void)
{
    // nothing
}
void command_send(uint8_t cmd, const uint8_t *payload, size_t payload_len)
{
    response_len = create_packet(cmd, payload, payload_len, response_buf);
    uint32_t *count = &simulation_state.response.count;
    simulation_state.response.cmd[*count].cmd_id = cmd;
    switch (cmd)
    {
    case CMD_GETTER:
        simulation_state.response.cmd[*count].setter.pin = payload[0];
        simulation_state.response.cmd[*count].setter.value = payload[1];
        (*count)++;
        break;
    default:
        break;
    }
}
void command_receive(void (*command_handler)(Packet))
{
    for (int i = 0; i < simulation_state.command.count; i++)
    {
        switch (simulation_state.command.cmd[i].cmd_id)
        {
        case CMD_PIN_MODE:
            current_packet.cmd = CMD_PIN_MODE;
            current_packet.len = 2;
            current_packet.payload[0] = simulation_state.command.cmd[i].pin_mode.pin;
            current_packet.payload[1] = simulation_state.command.cmd[i].pin_mode.mode;
            break;
        case CMD_SETTER:
            current_packet.cmd = CMD_SETTER;
            current_packet.len = 2;
            current_packet.payload[0] = simulation_state.command.cmd[i].setter.pin;
            current_packet.payload[1] = simulation_state.command.cmd[i].setter.value;
            break;
        case CMD_GETTER:
            current_packet.cmd = CMD_GETTER;
            current_packet.len = 1;
            current_packet.payload[0] = simulation_state.command.cmd[i].getter.pin;
            break;
        default:
            break;
        };

        command_handler(current_packet);
    }
}

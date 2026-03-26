#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_PAYLOAD 32
#define FRAME_DELIMITER 0x00

// FIXME remove checksum from future because app dont need it
typedef struct
{
    uint8_t cmd;
    uint8_t len;
    uint8_t payload[MAX_PAYLOAD];
    uint8_t checksum;
} Packet;

bool parse_packet(const uint8_t *encoded_buffer, size_t encoded_len, Packet *pkt);
size_t create_packet(uint8_t cmd, const uint8_t *payload, size_t payload_len, uint8_t *output);
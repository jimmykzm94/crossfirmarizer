#include "serial_packet.h"

// Decodes COBS data. Returns the length of the decoded data, or 0 on error.
static size_t cobs_decode(const uint8_t *input, size_t length, uint8_t *output)
{
    size_t read_index = 0;
    size_t write_index = 0;
    uint8_t code;
    uint8_t i;

    while (read_index < length)
    {
        code = input[read_index];
        if (read_index + code > length && code != 1)
            return 0; // Malformed
        read_index++;

        for (i = 1; i < code; i++)
        {
            output[write_index++] = input[read_index++];
        }

        if (code != 0xFF && read_index != length)
        {
            output[write_index++] = 0x00;
        }
    }
    return write_index;
}

static size_t cobs_encode(const uint8_t *input, size_t length, uint8_t *output)
{
    size_t read_index = 0;
    size_t write_index = 1; // Start after the first code byte
    uint8_t code_index = 0; // Index of the current code byte
    uint8_t code = 1;       // Code starts at 1

    while (read_index < length)
    {
        if (input[read_index] == 0)
        {
            output[code_index] = code;
            code_index = write_index++;
            code = 1;
            read_index++;
        }
        else
        {
            output[write_index++] = input[read_index++];
            code++;
            if (code == 0xFF)
            {
                output[code_index] = code;
                code_index = write_index++;
                code = 1;
            }
        }
    }
    output[code_index] = code; // Write final code byte
    output[write_index++] = FRAME_DELIMITER; // Frame delimiter
    return write_index;
}

// Parses the decoded byte buffer
bool parse_packet(const uint8_t *encoded_buffer, size_t encoded_len, Packet *pkt)
{
    uint8_t decoded[64]; // Ensure this is large enough for CMD + LEN + PAYLOAD + CHK
    uint32_t decoded_len = cobs_decode(encoded_buffer, encoded_len, decoded);

    if (decoded_len < 3)
        return false; // Minimum size: CMD, LEN, CHK

    pkt->cmd = decoded[0];
    pkt->len = decoded[1];

    if (decoded_len != (uint32_t)(3 + pkt->len))
        return false; // Length mismatch

    uint8_t calc_chk = pkt->cmd ^ pkt->len;
    for (uint8_t i = 0; i < pkt->len; i++)
    {
        pkt->payload[i] = decoded[2 + i];
        calc_chk ^= pkt->payload[i];
    }

    pkt->checksum = decoded[2 + pkt->len];
    return pkt->checksum == calc_chk;
}

size_t create_packet(uint8_t cmd, const uint8_t *payload, size_t payload_len, uint8_t *output)
{
    if (payload_len > MAX_PAYLOAD)
        return 0; // Payload too large

    uint8_t buffer[3 + MAX_PAYLOAD]; // CMD + LEN + PAYLOAD + CHK
    buffer[0] = cmd;
    buffer[1] = payload_len;

    uint8_t checksum = cmd ^ payload_len;
    for (size_t i = 0; i < payload_len; i++)
    {
        buffer[2 + i] = payload[i];
        checksum ^= payload[i];
    }
    buffer[2 + payload_len] = checksum;

    return cobs_encode(buffer, 3 + payload_len, output);
}

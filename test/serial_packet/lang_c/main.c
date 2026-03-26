#include "serial_packet.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    if (argc < 3)
    {
        perror("Wrong arguments!\n First argument is \"encode\" or \"decode\"\n Second argument: data\n");
        return -1;
    }

    if (strcmp(argv[1], "encode") == 0)
    { 
        int values[20];
        int count = 0;
        char *endptr = (char *)argv[2];
        // Parse until the end of the string is reached
        while (*endptr != '\0')
        {
            values[count] = strtol(endptr, &endptr, 10);
            count++;
        }

        // Set payload
        uint8_t payload[20];
        uint8_t encoded_buf[40];
        for (int i = 0; i < count; i++)
        {
            payload[i] = values[i];
        }

        size_t len = create_packet(payload[0], &payload[1], count - 1, encoded_buf);
        for (int i = 0; i < len; i++)
        {
            printf("%03d ", encoded_buf[i]);
        }
        printf("\n");
    }
    else if (strcmp(argv[1], "decode") == 0)
    {
        int values[20];
        int count = 0;
        char *endptr = (char *)argv[2];
        // Parse until the end of the string is reached
        while (*endptr != '\0')
        {
            values[count] = strtol(endptr, &endptr, 10);
            count++;
        }

        // Set payload
        uint8_t encoded_buf[20];
        Packet packet;
        for (int i = 0; i < count; i++)
        {
            encoded_buf[i] = values[i];
        }
        if (parse_packet(encoded_buf, count - 1, &packet))
        {
            printf("%03d ", packet.cmd);
            for (int i = 0; i < packet.len; i++)
            {
                printf("%03d ", packet.payload[i]);
            }
            printf("\n");
        }
    }
    else
    {
        perror("Wrong mode of test! \"encode\" or \"decode\"\n");
        return -1;
    }


    return 0;
}

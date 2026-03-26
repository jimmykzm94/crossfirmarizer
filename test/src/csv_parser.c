#include "csv_parser.h"
#include "simulation_state.h"
#include "command.h"
#include <stdio.h>
#include <string.h>

extern simulation_state_t simulation_state;

// Parse and store structure
int parse_input_csv(const char* file_path) {
    // Open file
    printf("Parsing input CSV: %s\n", file_path);
    FILE* f = fopen(file_path, "r");
    if (f == NULL) {
        perror("Error opening input file\n");
        return -1;
    }
    
    uint32_t *count = &simulation_state.command.count;

    char line[1024];
    while (fgets(line, sizeof(line), f))
    {
        if (strncmp(line, "IO,", 3) == 0)
        {
            int pin;
            int value;
            if (sscanf(line, "IO,pin=%d,value=%d", &pin, &value) != 2)
            {
                perror("Error parsing IO input line\n");
                return -1;
            }
            simulation_state.io_input_pins[pin].pin = pin;
            simulation_state.io_input_pins[pin].value = value;
            (*count)++;
        }
        else if (strncmp(line, "CMD_PIN_MODE,", 13) == 0)
        {
            if (sscanf(line, "CMD_PIN_MODE,pin=%d,mode=%d", &simulation_state.command.cmd[*count].pin_mode.pin, &simulation_state.command.cmd[*count].pin_mode.mode) != 2)
            {
                perror("Error parsing CMD_PIN_MODE line\n");
                return -1;
            }
            simulation_state.command.cmd[*count].cmd_id = CMD_PIN_MODE;
            (*count)++;
        }
        else if (strncmp(line, "CMD_SETTER,", 11) == 0)
        {
            if (sscanf(line, "CMD_SETTER,pin=%d,value=%d", &simulation_state.command.cmd[*count].setter.pin, &simulation_state.command.cmd[*count].setter.value) != 2)
            {
                perror("Error parsing CMD_SETTER line\n");
                return -1;
            }
            simulation_state.command.cmd[*count].cmd_id = CMD_SETTER;
            (*count)++;
        }
        else if (strncmp(line, "CMD_GETTER,", 11) == 0)
        {
            if (sscanf(line, "CMD_GETTER,pin=%d", &simulation_state.command.cmd[*count].getter.pin) != 1)
            {
                perror("Error parsing CMD_GETTER line\n");
                return -1;
            }
            simulation_state.command.cmd[*count].cmd_id = CMD_GETTER;
            (*count)++;
        }
    }

    return 0;
}

// Write structure to CSV
int write_output_csv(const char* file_path) {
    printf("Writing output CSV: %s\n", file_path);

    // from io_output_pins to CSV
    FILE* f = fopen(file_path, "w");
    if (f == NULL) {
        perror("Error opening output file");
        return -1;
    }

    for (int i = 0; i < MAX_COUNT; i++)
    {
        fprintf(f, "IO,pin=%d,value=%d\n", simulation_state.io_output_pins[i].pin, simulation_state.io_output_pins[i].value);
    }

    for (int i = 0; i < simulation_state.response.count; i++)
    {
        switch (simulation_state.response.cmd[i].cmd_id)
        {
        case CMD_GETTER:
            fprintf(f, "CMD_GETTER,pin=%d,value=%d\n", simulation_state.response.cmd[i].setter.pin, simulation_state.response.cmd[i].setter.value);
            break;
        default:
            break;
        }
    }

    return 0;
}

#ifndef SIMULATION_STATE_H
#define SIMULATION_STATE_H

#include <stdint.h>
#include "pin_mode.h"

#define MAX_COUNT 128

typedef struct
{
    pin_t pin;
    uint8_t value;
    pin_mode_t mode;
} io_t;

typedef struct
{
    uint8_t pin;
    uint8_t mode;
} cmd_pin_mode_t;

typedef struct
{
    uint8_t pin;
    uint8_t value;
} cmd_setter_t;

typedef struct
{
    uint8_t pin;
} cmd_getter_t;

typedef struct
{
    uint8_t cmd_id;
    union
    {
        cmd_pin_mode_t pin_mode;
        cmd_setter_t setter;
        cmd_getter_t getter;
    };
} cmd_t;

typedef struct
{
    cmd_t cmd[MAX_COUNT];
    uint32_t count;
} command_t;

typedef struct {
    io_t io_input_pins[MAX_COUNT];
    io_t io_output_pins[MAX_COUNT];
    command_t command;
    command_t response;
} simulation_state_t;

#endif // SIMULATION_STATE_H

#include "app.h"
#include "task.h"
#include "command.h"
#include "gpio.h"
#include "i2c.h"
#include "pins.h"
#include "pwm.h"
#include "serial_packet.h"
#include "ultrasonic.h"
#include "version.h"

// Forward declaration of task functions
static void app_pin_monitor_task(void);

// Task scheduler structure
typedef struct
{
    void (*task_func)(void);
    uint32_t interval;
    uint32_t last_run;
} AppTask;

// Enum for task indices
enum
{
    TASK_PIN_MONITOR,
#ifdef ULTRASONIC_SENSOR_ENABLED
    TASK_ULTRASONIC,
#endif
    NUM_TASKS
};

// Create the task list
AppTask tasks[NUM_TASKS] = {
    [TASK_PIN_MONITOR] = {app_pin_monitor_task, 100, 0},
#ifdef ULTRASONIC_SENSOR_ENABLED
    [TASK_ULTRASONIC] = {ultrasonic_read_task, 500, 0} // Default 500ms
#endif
};

volatile uint8_t pin_listening[PIN_MAX_COUNT] = {};
volatile uint8_t pin_last_state[PIN_MAX_COUNT];

void command_handler(Packet current_packet)
{

    switch (current_packet.cmd)
    {
    case CMD_PIN_MODE:
        if (current_packet.len >= 2)
        {
            uint8_t pin = current_packet.payload[0];
            uint8_t mode = current_packet.payload[1];

            if (pin < PIN_MAX_COUNT)
            {
                gpio_init(pin, mode);
            }
        }
        break;
    case CMD_SETTER:
        if (current_packet.len >= 2)
        {
            uint8_t pin = current_packet.payload[0];
            uint8_t value = current_packet.payload[1];
            if (pin < PIN_MAX_COUNT)
            {
                gpio_write(pin, value);
            }
        }
        break;
    case CMD_GETTER:
        if (current_packet.len >= 1)
        {
            uint8_t value = 0;
            uint8_t pin = current_packet.payload[0];
            if (pin < PIN_MAX_COUNT)
            {
                value = gpio_read(pin);
            }

            uint8_t response_payload[2] = {current_packet.payload[0], value};
            command_send(CMD_GETTER, response_payload, sizeof(response_payload));
        }
        break;
    case CMD_GPIO_LISTEN:
        if (current_packet.len >= 2)
        {
            uint8_t pin = current_packet.payload[0];
            uint8_t enable = current_packet.payload[1];
            if (pin < PIN_MAX_COUNT)
            {
                pin_listening[pin] = enable;
                if (enable)
                    pin_last_state[pin] = 0xFF; // Force initial update
            }
        }
        break;
    case CMD_PWM_CONFIG:
        if (current_packet.len >= 3)
        {
            uint8_t pin = current_packet.payload[0];
            uint16_t period_us = current_packet.payload[1] | (current_packet.payload[2] << 8);
            if (pin < PIN_MAX_COUNT)
            {
                pwm_config(pin, period_us);
            }
        }
        break;
    case CMD_PWM_WRITE:
        if (current_packet.len >= 3)
        {
            uint8_t pin = current_packet.payload[0];
            uint16_t value = current_packet.payload[1] | (current_packet.payload[2] << 8);
            if (pin < PIN_MAX_COUNT)
            {
                pwm_write(pin, value);
            }
        }
        break;
    case CMD_HELLO:
        // send back a Hello response
        {
            uint8_t response_payload[] = "Hello from CrossFirmarizer!";
            command_send(CMD_HELLO, response_payload, sizeof(response_payload) - 1);
        }
        break;
    case CMD_I2C_CONFIG:
        if (current_packet.len >= 3)
        {
            uint8_t bus = current_packet.payload[0];
            uint8_t sda_pin = current_packet.payload[1];
            uint8_t scl_pin = current_packet.payload[2];
            i2c_init(bus, scl_pin, sda_pin);
        }
        break;
    case CMD_I2C_WRITE:
        if (current_packet.len >= 2)
        {
            uint8_t bus = current_packet.payload[0];
            uint8_t address = current_packet.payload[1];
            uint8_t *data = &current_packet.payload[2];
            int len = current_packet.len - 2;
            i2c_write(bus, address, data, len);
        }
        break;
    case CMD_I2C_READ:
        if (current_packet.len >= 3)
        {
            uint8_t bus = current_packet.payload[0];
            uint8_t address = current_packet.payload[1];
            uint8_t size = current_packet.payload[2];

            if (size > 32)
                size = 32; // Bound read size to prevent buffer overflow

            uint8_t response_payload[64];
            response_payload[0] = bus;
            response_payload[1] = address;

            i2c_read(bus, address, &response_payload[2], size);

            command_send(CMD_I2C_READ, response_payload, size + 2);
        }
        break;
    case CMD_ULTRASONIC_CONFIG:
#ifdef ULTRASONIC_SENSOR_ENABLED
        if (current_packet.len >= 4)
        {
            uint8_t trig_pin = current_packet.payload[0];
            uint8_t echo_pin = current_packet.payload[1];
            uint16_t interval = current_packet.payload[2] | (current_packet.payload[3] << 8);
            ultrasonic_init(trig_pin, echo_pin, interval);
            tasks[TASK_ULTRASONIC].interval = interval;
        }
#endif
        break;
    case CMD_ULTRASONIC_READ:
#ifdef ULTRASONIC_SENSOR_ENABLED
    {
        float distance = ultrasonic_get_last_reading();
        uint32_t distance_x100 = (uint32_t)(distance * 100.0f);

        uint8_t response_payload[4];
        response_payload[0] = (distance_x100 >> 24) & 0xFF;
        response_payload[1] = (distance_x100 >> 16) & 0xFF;
        response_payload[2] = (distance_x100 >> 8) & 0xFF;
        response_payload[3] = distance_x100 & 0xFF;

        command_send(CMD_ULTRASONIC_READ, response_payload, sizeof(response_payload));
    }
#endif
    break;
    case CMD_FW_VERSION:
    {
        uint8_t response_payload[2] = {VERSION_MAJOR, VERSION_MINOR};
        command_send(CMD_FW_VERSION, response_payload, sizeof(response_payload));
    }
    break;
    default:
        break;
    }
}

static void app_pin_monitor_task()
{
    // Check monitored pins
    for (int i = 0; i < PIN_MAX_COUNT; i++)
    {
        if (pin_listening[i])
        {
            uint8_t val = 0;
            val = gpio_read(i);

            if (val != pin_last_state[i])
            {
                pin_last_state[i] = val;
                uint8_t report_payload[2] = {i, val};
                command_send(CMD_GPIO_LISTEN, report_payload, 2);
            }
        }
    }
}

void app_init(void)
{
    command_hw_init();
    task_init();
}

void app_background(void)
{
    command_receive(command_handler);

    if (task_systick_happened())
    {
        uint32_t current_ms = task_get_ms();
        for (int i = 0; i < NUM_TASKS; i++)
        {
            if ((current_ms - tasks[i].last_run) >= tasks[i].interval)
            {
                tasks[i].last_run = current_ms;
                tasks[i].task_func();
            }
        }
    }
}

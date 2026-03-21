#pragma once
#include "serial_packet.h"

#define MAX_ENCODED_LEN 64

// command id
#define CMD_PIN_MODE 0x01
#define CMD_SETTER 0x02
#define CMD_GETTER 0x03
#define CMD_GPIO_LISTEN 0x05
#define CMD_I2C_CONFIG 0x10
#define CMD_I2C_WRITE 0x11
#define CMD_I2C_READ 0x12
// #define CMD_TIMER_CONFIG 0x50
// #define CMD_TIMER_START 0x51
// #define CMD_TIMER_STOP 0x52
#define CMD_PWM_CONFIG 0x56
#define CMD_PWM_WRITE 0x57
#define CMD_ULTRASONIC_READ 0x70
#define CMD_ULTRASONIC_CONFIG 0x71
#define CMD_FW_VERSION 0x91
#define CMD_HELLO 0x99

void command_hw_init(void);
void command_send(uint8_t cmd, const uint8_t *payload, size_t payload_len);
void command_receive(void (*command_handler)(Packet));

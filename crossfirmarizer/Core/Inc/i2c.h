#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void i2c_init(uint8_t bus, uint8_t scl_pin, uint8_t sda_pin);
void i2c_deinit(uint8_t bus);
void i2c_write(uint8_t bus, uint8_t address, uint8_t *data, uint8_t length);
void i2c_read(uint8_t bus, uint8_t address, uint8_t *data, uint8_t length);

#endif // I2C_H

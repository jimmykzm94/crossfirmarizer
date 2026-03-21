#include "i2c.h"
#include <avr/io.h>
#include <util/twi.h>

#define F_SCL 100000UL // SCL frequency of 100kHz
#define TWI_PRESCALER 1 // Prescaler of 1

// Helper functions for TWI operations
static void twi_start(void) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

static void twi_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

static void twi_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

static uint8_t twi_read_ack(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

static uint8_t twi_read_nack(void) {
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

void i2c_init(uint8_t bus, uint8_t scl_pin, uint8_t sda_pin) {
    // For ATmega2560, there is only one TWI (I2C) bus on PD0 (SCL) and PD1 (SDA).
    // The bus, scl_pin, and sda_pin parameters are ignored.
    
    // Set TWI clock frequency
    TWSR = 0; // Prescaler 1
    TWBR = ((F_CPU / F_SCL) - 16) / (2 * TWI_PRESCALER);
    
    // Enable TWI
    TWCR = (1 << TWEN);
}

void i2c_deinit(uint8_t bus) {
    // Disable TWI
    TWCR = 0;
}

void i2c_write(uint8_t bus, uint8_t address, uint8_t* data, uint8_t length) {
    twi_start();
    if ((TW_STATUS != TW_START) && (TW_STATUS != TW_REP_START)) {
        twi_stop();
        return;
    }

    twi_write((address << 1) | TW_WRITE);
    if (TW_STATUS != TW_MT_SLA_ACK) {
        twi_stop();
        return;
    }

    for (uint8_t i = 0; i < length; i++) {
        twi_write(data[i]);
        if (TW_STATUS != TW_MT_DATA_ACK) {
            twi_stop();
            return;
        }
    }

    twi_stop();
}

void i2c_read(uint8_t bus, uint8_t address, uint8_t* data, uint8_t length) {
    if (length == 0) {
        return;
    }
    
    twi_start();
    if ((TW_STATUS != TW_START) && (TW_STATUS != TW_REP_START)) {
        twi_stop();
        return;
    }

    twi_write((address << 1) | TW_READ);
    if (TW_STATUS != TW_MR_SLA_ACK) {
        twi_stop();
        return;
    }

    for (uint8_t i = 0; i < length - 1; i++) {
        data[i] = twi_read_ack();
        if (TW_STATUS != TW_MR_DATA_ACK) {
            twi_stop();
            return;
        }
    }
    
    data[length - 1] = twi_read_nack();
    if (TW_STATUS != TW_MR_DATA_NACK) {
        twi_stop();
        return;
    }

    twi_stop();
}

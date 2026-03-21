#include "i2c.h"
#include "pins.h"
#include "stm32f446xx.h"

void i2c_init(uint8_t bus, uint8_t scl_pin, uint8_t sda_pin)
{
    I2C_TypeDef *i2c_instance = (bus == 2) ? I2C2 : ((bus == 3) ? I2C3 : I2C1);
    if (i2c_instance == I2C1)
    {
        // Enable I2C1 clock
        RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

        // Enable clock for GPIO ports of SCL and SDA pins
        if (pins[scl_pin].port == GPIOA || pins[sda_pin].port == GPIOA)
        {
            RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
        }
        if (pins[scl_pin].port == GPIOB || pins[sda_pin].port == GPIOB)
        {
            RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
        }
        if (pins[scl_pin].port == GPIOC || pins[sda_pin].port == GPIOC)
        {
            RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
        }

        // Configure SCL and SDA pins for alternate function, open-drain
        pins[scl_pin].port->MODER |= (2 << (pins[scl_pin].pin * 2));
        pins[scl_pin].port->OTYPER |= (1 << pins[scl_pin].pin);
        pins[sda_pin].port->MODER |= (2 << (pins[sda_pin].pin * 2));
        pins[sda_pin].port->OTYPER |= (1 << pins[sda_pin].pin);

        // Set alternate function for I2C1
        if (pins[scl_pin].pin < 8)
        {
            pins[scl_pin].port->AFR[0] |= (4 << (pins[scl_pin].pin * 4));
        }
        else
        {
            pins[scl_pin].port->AFR[1] |= (4 << ((pins[scl_pin].pin - 8) * 4));
        }
        if (pins[sda_pin].pin < 8)
        {
            pins[sda_pin].port->AFR[0] |= (4 << (pins[sda_pin].pin * 4));
        }
        else
        {
            pins[sda_pin].port->AFR[1] |= (4 << ((pins[sda_pin].pin - 8) * 4));
        }
    }
    // Add other I2C instances here if needed

    // Configure I2C
    i2c_instance->CR1 &= ~I2C_CR1_PE; // Disable peripheral
    i2c_instance->CR1 |= I2C_CR1_SWRST; // Reset
    i2c_instance->CR1 &= ~I2C_CR1_SWRST;
    i2c_instance->CR2 = 45; // 45MHz
    i2c_instance->CCR = 225; // 100kHz
    i2c_instance->CR1 |= I2C_CR1_PE; // Enable peripheral
}

void i2c_deinit(uint8_t bus)
{
    I2C_TypeDef *i2c_instance = (bus == 2) ? I2C2 : ((bus == 3) ? I2C3 : I2C1);
    i2c_instance->CR1 &= ~I2C_CR1_PE; // Disable peripheral
    if (i2c_instance == I2C1)
    {
        RCC->APB1ENR &= ~RCC_APB1ENR_I2C1EN; // Disable I2C1 clock
    }
    // Add other I2C instances here if needed
}

void i2c_write(uint8_t bus, uint8_t address, uint8_t *data, uint8_t length)
{
    // Start condition
    I2C_TypeDef *i2c_instance = (bus == 2) ? I2C2 : ((bus == 3) ? I2C3 : I2C1);
    i2c_instance->CR1 |= I2C_CR1_START;
    while (!(i2c_instance->SR1 & I2C_SR1_SB));

    // Send address
    i2c_instance->DR = address << 1;
    while (!(i2c_instance->SR1 & I2C_SR1_ADDR))
        ;
    (void)i2c_instance->SR2; // Clear ADDR

    // Write data
    for (int i = 0; i < length; i++)
    {
        i2c_instance->DR = data[i];
        while (!(i2c_instance->SR1 & I2C_SR1_TXE))
            ;
    }

    // Stop condition
    i2c_instance->CR1 |= I2C_CR1_STOP;
}

void i2c_read(uint8_t bus, uint8_t address, uint8_t *data, uint8_t length)
{
    // Start condition
    I2C_TypeDef *i2c_instance = (bus == 2) ? I2C2 : ((bus == 3) ? I2C3 : I2C1);
    i2c_instance->CR1 |= I2C_CR1_START;
    while (!(i2c_instance->SR1 & I2C_SR1_SB));

    // Send address
    i2c_instance->DR = (address << 1) | 1;
    while (!(i2c_instance->SR1 & I2C_SR1_ADDR));

    if (length == 1)
    {
        i2c_instance->CR1 &= ~I2C_CR1_ACK; // NACK
        (void)i2c_instance->SR2;           // Clear ADDR
        i2c_instance->CR1 |= I2C_CR1_STOP; // Stop
    }
    else
    {
        (void)i2c_instance->SR2; // Clear ADDR
    }

    // Read data
    for (int i = 0; i < length; i++)
    {
        if (i == length - 1)
        {
            i2c_instance->CR1 &= ~I2C_CR1_ACK; // NACK
            i2c_instance->CR1 |= I2C_CR1_STOP; // Stop
        }
        while (!(i2c_instance->SR1 & I2C_SR1_RXNE));
        data[i] = i2c_instance->DR;
    }
}

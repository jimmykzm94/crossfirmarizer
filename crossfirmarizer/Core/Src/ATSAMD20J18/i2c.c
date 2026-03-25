#include "i2c.h"
#include "sam.h"
#include "pins.h"
#include <stdbool.h>
#include <stddef.h>

#include "command.h"
#include <string.h>
#include <stdio.h>


typedef struct
{
    sercom_registers_t *sercom;
} i2c_instance_t;

static i2c_instance_t i2c_instances[6];

// static sercom_registers_t *get_sercom_instance(uint8_t bus)
// {
//     switch (bus)
//     {
//     case 0:
//         return SERCOM0_REGS;
//     case 1:
//         return SERCOM1_REGS;
//     case 2:
//         return SERCOM2_REGS;
//     case 3:
//         return SERCOM3_REGS;
//     case 4:
//         return SERCOM4_REGS;
//     case 5:
//         return SERCOM5_REGS;
//     default:
//         return NULL;
//     }
// }

static inline bool i2c_sync(sercom_registers_t *sercom)
{
    uint32_t timeout = 100000;
    while ((sercom->I2CM.SERCOM_STATUS & SERCOM_I2CM_STATUS_SYNCBUSY_Msk) != 0)
    {
        if (--timeout == 0)
            return false;
    }
    return true;
}

void i2c_init(uint8_t bus, uint8_t scl_pin, uint8_t sda_pin)
{
    // FIXME For now, we implement EXT1 Sercom2 only, PA08, PA09
    if (bus != 2 || scl_pin != PIN_PA09 || sda_pin != PIN_PA08)
        return;

    sercom_registers_t *sercom = SERCOM2_REGS;
    i2c_instances[bus].sercom = sercom;

    PM_REGS->PM_APBCMASK |= PM_APBCMASK_SERCOM2_Msk;

    // 1. Core Clock (Requires SYNCBUSY check)
    GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN_Msk |
                              GCLK_CLKCTRL_GEN_GCLK0 |
                              GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM0_CORE_Val + bus);
    while (GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk)
        ;

    // 2. Slow Clock (CRITICAL: DO NOT add a SYNCBUSY check here, it will hang)
    GCLK_REGS->GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN_Msk |
                              GCLK_CLKCTRL_GEN_GCLK0 |
                              GCLK_CLKCTRL_ID_SERCOMX_SLOW;

    sercom->I2CM.SERCOM_CTRLA &= ~SERCOM_I2CM_CTRLA_ENABLE_Msk;
    i2c_sync(sercom);

    sercom->I2CM.SERCOM_CTRLA = SERCOM_I2CM_CTRLA_MODE_I2C_MASTER | SERCOM_I2CM_CTRLA_SDAHOLD_450NS;
    sercom->I2CM.SERCOM_CTRLB = SERCOM_I2CM_CTRLB_SMEN_Msk;
    i2c_sync(sercom);

    // baud rate = f_GCLK / (2 * f_SCL) - 1 = 8000000 / 200000 - 1 = 39
    sercom->I2CM.SERCOM_BAUD = 39; // ~100kHz @ 8MHz
    i2c_sync(sercom);

    // Overwrite PMUX properly
    PORT_REGS->GROUP[0].PORT_PMUX[8 >> 1] = PORT_PMUX_PMUXE(3) | PORT_PMUX_PMUXO(3);
    PORT_REGS->GROUP[0].PORT_PINCFG[8] = PORT_PINCFG_PMUXEN_Msk;
    PORT_REGS->GROUP[0].PORT_PINCFG[9] = PORT_PINCFG_PMUXEN_Msk;

    sercom->I2CM.SERCOM_CTRLA |= SERCOM_I2CM_CTRLA_ENABLE_Msk;
    i2c_sync(sercom);

    // Force bus state to IDLE
    sercom->I2CM.SERCOM_STATUS = SERCOM_I2CM_STATUS_BUSSTATE(1);
    i2c_sync(sercom);
}

void i2c_deinit(uint8_t bus)
{
    if (bus >= 6)
        return;
    sercom_registers_t *sercom = i2c_instances[bus].sercom;
    if (sercom)
    {
        sercom->I2CM.SERCOM_CTRLA &= ~SERCOM_I2CM_CTRLA_ENABLE_Msk;
        i2c_sync(sercom);
    }
}

void i2c_write(uint8_t bus, uint8_t address, uint8_t *data, uint8_t length)
{
    sercom_registers_t *sercom = i2c_instances[bus].sercom;
    if (!sercom || length == 0)
        return;

    uint32_t timeout;

    // Send Address
    sercom->I2CM.SERCOM_ADDR = (address << 1);
    timeout = 100000;
    while (!(sercom->I2CM.SERCOM_INTFLAG & SERCOM_I2CM_INTFLAG_MB_Msk))
    {
        if (--timeout == 0 || (sercom->I2CM.SERCOM_STATUS & (SERCOM_I2CM_STATUS_BUSERR_Msk | SERCOM_I2CM_STATUS_ARBLOST_Msk)))
        {
            sercom->I2CM.SERCOM_CTRLB |= SERCOM_I2CM_CTRLB_CMD(3);
            i2c_sync(sercom);
            return; // Abort cleanly
        }
    }

    if (sercom->I2CM.SERCOM_STATUS & SERCOM_I2CM_STATUS_RXNACK_Msk)
    {
        sercom->I2CM.SERCOM_CTRLB |= SERCOM_I2CM_CTRLB_CMD(3);
        i2c_sync(sercom);
        return;
    }

    // Send Data
    for (uint8_t i = 0; i < length; i++)
    {
        sercom->I2CM.SERCOM_DATA = data[i];
        timeout = 100000;

        while (!(sercom->I2CM.SERCOM_INTFLAG & SERCOM_I2CM_INTFLAG_MB_Msk))
        {
            if (--timeout == 0 || (sercom->I2CM.SERCOM_STATUS & (SERCOM_I2CM_STATUS_BUSERR_Msk | SERCOM_I2CM_STATUS_ARBLOST_Msk)))
            {
                sercom->I2CM.SERCOM_CTRLB |= SERCOM_I2CM_CTRLB_CMD(3);
                i2c_sync(sercom);
                return; // Abort cleanly
            }
        }

        if (sercom->I2CM.SERCOM_STATUS & SERCOM_I2CM_STATUS_RXNACK_Msk)
        {
            sercom->I2CM.SERCOM_CTRLB |= SERCOM_I2CM_CTRLB_CMD(3);
            i2c_sync(sercom);
            return;
        }
    }

    // Stop Condition
    sercom->I2CM.SERCOM_CTRLB |= SERCOM_I2CM_CTRLB_CMD(3);
    i2c_sync(sercom);
}

void i2c_read(uint8_t bus, uint8_t address, uint8_t *data, uint8_t length)
{
    if (bus >= 6 || length == 0)
        return;
    sercom_registers_t *sercom = i2c_instances[bus].sercom;
    if (!sercom)
        return;

    sercom->I2CM.SERCOM_INTFLAG = SERCOM_I2CM_INTFLAG_MB_Msk | SERCOM_I2CM_INTFLAG_SB_Msk;
    sercom->I2CM.SERCOM_CTRLB &= ~SERCOM_I2CM_CTRLB_ACKACT_Msk;
    i2c_sync(sercom);

    sercom->I2CM.SERCOM_ADDR = (address << 1) | 1;

    while (!(sercom->I2CM.SERCOM_INTFLAG & SERCOM_I2CM_INTFLAG_SB_Msk))
    {
        if (sercom->I2CM.SERCOM_STATUS & (SERCOM_I2CM_STATUS_BUSERR_Msk | SERCOM_I2CM_STATUS_ARBLOST_Msk))
        {
            sercom->I2CM.SERCOM_CTRLB |= SERCOM_I2CM_CTRLB_CMD(3);
            i2c_sync(sercom);
            return;
        }
    }

    if (sercom->I2CM.SERCOM_STATUS & SERCOM_I2CM_STATUS_RXNACK_Msk)
    {
        sercom->I2CM.SERCOM_CTRLB |= SERCOM_I2CM_CTRLB_CMD(3);
        i2c_sync(sercom);
        return;
    }

    for (uint8_t i = 0; i < length - 1; i++)
    {
        while (!(sercom->I2CM.SERCOM_INTFLAG & SERCOM_I2CM_INTFLAG_SB_Msk))
            ;
        data[i] = sercom->I2CM.SERCOM_DATA;
    }

    if (length > 0)
    {
        sercom->I2CM.SERCOM_CTRLB |= SERCOM_I2CM_CTRLB_ACKACT_Msk;
        i2c_sync(sercom);

        while (!(sercom->I2CM.SERCOM_INTFLAG & SERCOM_I2CM_INTFLAG_SB_Msk))
            ;

        sercom->I2CM.SERCOM_CTRLB |= SERCOM_I2CM_CTRLB_CMD(3);
        i2c_sync(sercom);

        data[length - 1] = sercom->I2CM.SERCOM_DATA;
    }
}

#include "calculations.h"
#include "registers_modbus.h"
#include "DEFINES.h"

static int16_t *modbus_reg_03;
static int16_t *modbus_reg_04;

void calculations_init(int16_t *modbus_reg_03_,
                  int16_t *modbus_reg_04_)
{
    modbus_reg_03 = modbus_reg_03_;
    modbus_reg_04 = modbus_reg_04_;
}

void CRC_CALC(uint8_t *msg, uint16_t counter, uint8_t *crc_result)
{
    uint16_t CRC = 65535;
    for(int16_t i = 0; i < counter; i++)
    { 
        CRC = CRC ^ *(msg + i); 
        for(uint8_t u = 0; u < 8; u++)
        {
            uint8_t bitL = CRC&1;
            CRC = CRC >> 1;
            if(bitL == 1) CRC = CRC ^ 40961;
        }
    }
    *crc_result = CRC >> 8;
    *(crc_result + 1) = CRC & 255;
}


int16_t recalc_val(uint8_t i1, uint8_t i2, uint8_t i3, uint8_t i4, uint8_t i5)
{
    int16_t recalc = (*(modbus_reg_04 + i1)) * 
                     (*(modbus_reg_03 + i2)) * i3 / 
                     (*(modbus_reg_03 + i4)) / (*(modbus_reg_03 + i5));
    return recalc;
}


int16_t SCALE_RECALC(int16_t i1, int16_t i2, int16_t i3)
{
    int16_t recalc = i1 * i2 / i3;
    return recalc;
}


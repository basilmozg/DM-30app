#ifndef CALCULATIONS_H__
#define CALCULATIONS_H__

#include <stdint.h>
#include <inttypes.h>

#define  readBit(value, num)                ((value >> num) & 1)
#define  bitSet(value, bit)                 ((value) |= (1UL << (bit)))
#define  bitClear(value, bit)               ((value) &= ~(1UL << (bit)))
#define  writebit(value, bit, bitvalue)     (bitvalue ? bitSet(value, bit) : bitClear(value, bit))


void CRC_CALC(uint8_t *msg, uint16_t counter, uint8_t *crc_result);

int16_t recalc_val(uint8_t i1, uint8_t i2, uint8_t i3, uint8_t i4, uint8_t i5);

int16_t SCALE_RECALC(int16_t i1, int16_t i2, int16_t i3);

void calculations_init(int16_t *modbus_reg_03_, int16_t *modbus_reg_04_);

void default_registers_set(int16_t *reg_03);

#endif

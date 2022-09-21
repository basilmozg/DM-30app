#ifndef LED_DI_HANDLER_H__
#define LED_DI_HANDLER_H__

#include <stdint.h>
#include <inttypes.h>

void DI_init();

void ESP_RESET();

uint8_t DI_CHANNEL_READ_0_to_10(uint8_t channel);

void D0_CHANNEL_WRITE_0_to_9(uint8_t channel, uint8_t state);

void DO_init();

#endif


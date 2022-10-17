#ifndef TEMPERATURE_WORK_H__
#define TEMPERATURE_WORK_H__

#include <stdint.h>
#include <inttypes.h>

void temperature_init(int16_t *temp_register_);

void temperature_get();

void temperature_init_msec(int16_t *temp_register_, uint32_t period_ms);

void temperature_init_sec(int16_t *temp_register_, uint32_t period_s);

#endif
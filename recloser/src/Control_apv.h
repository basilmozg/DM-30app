#ifndef APV_LOGIC_H__
#define APV_LOGIC_H__

#include <stdint.h>
#include <inttypes.h>


void apv_init(int16_t *_modbus_reg_03, int16_t *_modbus_reg_04, uint8_t *_main_window_updater);
void apv_start_for_ready();
uint8_t apv_ready();
uint8_t apv_start_for_cycle();
uint8_t apv_cycle();
void stop_timers_apv();
void emergency_disable_apv();
uint8_t apv_init_ready();
uint8_t apv_cycle_current_global();

void apv_zmn_await();
void apv_zmn_await_stop();
uint8_t apv_zmn_start_event();
void apv_zmn_reset();
uint8_t apv_zmn_init_ready();

#endif
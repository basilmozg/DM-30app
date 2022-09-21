/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "control_apv.h"
#include "registers_modbus.h"
#include "DEFINES.h"
#include "calculations.h"

#include <logging/log.h>
#include <zephyr.h>

LOG_MODULE_REGISTER(control_apv);

static int16_t *modbus_reg_04;
static int16_t *modbus_reg_03;

uint8_t apv_current_number;
uint8_t apv_cycle_done;
uint8_t apv_cycle_current;
uint8_t apv_enabled[4];
uint8_t apv_global_ready_done;
uint8_t ready_done;
uint8_t emergency_disable;
static uint8_t *main_window_updater;
uint8_t apv_zmn_start;


void reset_apv_seq()
{      LOG_INF_GLOB("reset_apv_seq");
    if(readBit(modbus_reg_03[mod_reg_03_system_mode], radio_on_apv1)) apv_enabled[1] = 1;
    else apv_enabled[1] = 0;
    if(readBit(modbus_reg_03[mod_reg_03_system_mode], radio_on_apv2)) apv_enabled[2] = 1;
    else apv_enabled[2] = 0;
    if(readBit(modbus_reg_03[mod_reg_03_system_mode], radio_on_apv3)) apv_enabled[3] = 1;
    else apv_enabled[3] = 0;
    modbus_reg_04[mod_reg_04_apv_cycle] = 0;
    *main_window_updater = 2;
}

static void part_init()
{
    LOG_INF_GLOB("part_init");
    stop_timers_apv();
    apv_current_number = 0;
    apv_cycle_done = 0;
    apv_global_ready_done = 0;
    apv_cycle_current = 0;
    ready_done = 0;
    emergency_disable = 0;
    apv_enabled[0] = 1;
    modbus_reg_04[mod_reg_04_apv_ready] = 0;
    apv_zmn_start = 0;
}

void emergency_disable_apv()
{
    if(!emergency_disable) 
    {  LOG_INF_GLOB("emergency_disable_apv()");
        stop_timers_apv();
        if(modbus_reg_04[mod_reg_04_apv_cycle] == 0) part_init();
        emergency_disable = 1;
    }
}

void apv_end_for_ready()
{
    switch(apv_current_number)
    {
        case 255:
            LOG_INF_GLOB("APV_time_ready end");
            apv_global_ready_done = 1;
            writebit(modbus_reg_04[mod_reg_04_apv_ready], radio_ready_apv, 1);
            *main_window_updater = 2;
            ready_done = 255;
        break;
        case 1:
        case 2:
        case 3:
            LOG_INF_GLOB("APV_time_ready end %d", apv_current_number);
            ready_done = 1;
            apv_cycle_current = 0;
            apv_enabled[0] = 0x81;
            reset_apv_seq();
        break;
    }
}

K_TIMER_DEFINE(k_timer_apv_start_for_ready, apv_end_for_ready, NULL);

uint8_t apv_ready()
{
    return ready_done;
}

uint8_t apv_cycle()
{
    return apv_cycle_done;
}

uint8_t apv_init_ready()
{
    return apv_global_ready_done;
}

uint8_t apv_cycle_current_global()
{
    return apv_cycle_current;
}

void apv_end_for_cycle()
{
    LOG_INF_GLOB("k_timer_apv_start_for_cycle %d", apv_current_number);
    apv_cycle_done = apv_current_number;
    apv_current_number = 0;
}

K_TIMER_DEFINE(k_timer_apv_start_for_cycle, apv_end_for_cycle, NULL);

uint8_t apv_exist_cycle_search(uint8_t cur_cyc)
{
    switch(cur_cyc)
    {
        case 0:
            if(readBit(modbus_reg_03[mod_reg_03_system_mode], radio_on_apv1)) return 1;
        case 1:
            if(readBit(modbus_reg_03[mod_reg_03_system_mode], radio_on_apv2)) return 2;
        case 2:
            if(readBit(modbus_reg_03[mod_reg_03_system_mode], radio_on_apv3)) return 3;
            else return 4;
        break;
        default:
            return 4;
        break;
    }
}

uint8_t apv_start_for_cycle()
{
    k_timer_stop(&k_timer_apv_start_for_ready);
    for(uint8_t i = 0; i < 3; i++)
    {
        if(apv_enabled[i] > 0x80) 
        {
            apv_enabled[i] = 0; 
            apv_cycle_current = apv_exist_cycle_search(i);
            if(apv_cycle_current == 4) 
            {
                LOG_INF_GLOB("no exist %d ", i);
                apv_global_ready_done = 0;
                return 1;
            }
            apv_current_number = apv_cycle_current;
            modbus_reg_04[mod_reg_04_apv_cycle] = apv_current_number;
            k_timer_start(&k_timer_apv_start_for_cycle, 
                          K_SECONDS(modbus_reg_04[mod_reg_04_APV_time_ready + apv_cycle_current]), 
                          K_NO_WAIT);
            LOG_INF_GLOB("APV_time_cycle start %d", apv_cycle_current);
            return 0;
        }
    }
    return 1;
}

void apv_zmn_reset()
{
    apv_zmn_start = 0;
}

uint8_t apv_zmn_start_event()
{
    return apv_zmn_start;
}

void apv_zmn_back()
{
    LOG_INF_GLOB("apv_zmn_back()");
    writebit(modbus_reg_04[mod_reg_04_apv_ready], radio_apv_zmn_active, 0);
    apv_zmn_start = 1;
}

K_TIMER_DEFINE(k_timer_apv_zmn_await, apv_zmn_back, NULL);

void apv_zmn_await_stop()
{
    if(k_timer_remaining_ticks(&k_timer_apv_zmn_await) != 0) 
    {
        k_timer_stop(&k_timer_apv_zmn_await);
        LOG_INF_GLOB("apv_zmn_await_stop()");
    }
}

void apv_zmn_await()
{
    if(k_timer_remaining_ticks(&k_timer_apv_zmn_await) == 0 && readBit(modbus_reg_04[mod_reg_04_apv_ready], radio_apv_zmn_active)) 
    {
        LOG_INF_GLOB("apv_zmn_await()");
        k_timer_start(&k_timer_apv_zmn_await, K_SECONDS(modbus_reg_04[mod_reg_04_APV_ZMN_time_back]), K_NO_WAIT);
    }
}

void apv_zmn_end_for_ready()
{
    LOG_INF_GLOB("APV_ZMN_ ready");
    writebit(modbus_reg_04[mod_reg_04_apv_ready], radio_ready_apv_zmn, 1);
    *main_window_updater = 2;
}

uint8_t apv_zmn_init_ready()
{
    return readBit(modbus_reg_04[mod_reg_04_apv_ready], radio_ready_apv_zmn);
}

K_TIMER_DEFINE(k_timer_apv_zmn_start_for_ready, apv_zmn_end_for_ready, NULL);

void stop_timers_apv()
{
    k_timer_stop(&k_timer_apv_start_for_ready);
    k_timer_stop(&k_timer_apv_start_for_cycle);
    k_timer_stop(&k_timer_apv_zmn_start_for_ready);
    k_timer_stop(&k_timer_apv_zmn_await);
}

void apv_init(int16_t *_modbus_reg_03, int16_t *_modbus_reg_04, uint8_t *_main_window_updater)
{
        LOG_INF_GLOB("apv_init");
        main_window_updater = _main_window_updater;
        modbus_reg_03 = _modbus_reg_03;
        modbus_reg_04 = _modbus_reg_04;
        part_init();
}

void apv_start_for_ready()
{
    if(((modbus_reg_03[mod_reg_03_system_mode] >> radio_on_apv1) & 7) != 0)
    {
        if(apv_current_number == 0)
        {
            emergency_disable = 0;
            apv_cycle_done = 0;
            if(apv_enabled[0] != 0 && apv_enabled[0] < 0x81) 
            {
                reset_apv_seq();
                apv_enabled[0] = apv_enabled[0] | 0x80;
                apv_current_number = 255;
                k_timer_start(&k_timer_apv_start_for_ready, K_SECONDS(modbus_reg_04[mod_reg_04_APV_time_ready]), K_NO_WAIT);
                LOG_INF_GLOB("APV_time_ready start");
            }
            else
                for(uint8_t i = 1; i < 4; i++)
                if(apv_enabled[i] != 0 && apv_enabled[i] < 0x81) 
                {
                    apv_enabled[i] = apv_enabled[i] | 0x80;
                    apv_current_number = i;
                    k_timer_start(&k_timer_apv_start_for_ready, K_SECONDS(modbus_reg_04[mod_reg_04_APV_time_ready]), K_NO_WAIT);
                    LOG_INF_GLOB("APV_time_ready start %d", i);
                    return;
                }
        }
    }
    if(readBit(modbus_reg_03[mod_reg_03_system_mode], radio_apv_zmn) &&
       k_timer_remaining_ticks(&k_timer_apv_zmn_start_for_ready) == 0)
    {
        LOG_INF_GLOB("APV_ZMN_time_ready start");
        k_timer_start(&k_timer_apv_zmn_start_for_ready, K_SECONDS(modbus_reg_04[mod_reg_04_APV_ZMN_time_ready]), K_NO_WAIT);
    }
}

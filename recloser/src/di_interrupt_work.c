/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "di_interrupt_work.h"
#include "DEFINES.h"

#include <zephyr.h>
#include <kernel.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(di_interrupt_work);

static int64_t start_time_on,  end_time_on;
static int64_t start_time_off, end_time_off;
static int16_t time_on_val, time_off_val;

void sw_timer_off_handler()
{
    time_off_val = end_time_off - start_time_off;
    LOG_INF_GLOB("time off %d", time_off_val);
}

void sw_timer_on_handler()
{
    time_on_val = end_time_on - start_time_on;
    LOG_INF_GLOB("time on %d", time_on_val);
}

K_TIMER_DEFINE(k_sw_timer_off, sw_timer_off_handler, NULL);

K_TIMER_DEFINE(k_sw_timer_on,  sw_timer_on_handler, NULL);

void bb_switch_on_event(uint8_t direct)
{
    if(direct) 
    {
        end_time_on = k_ticks_to_us_floor64(k_uptime_ticks());
        k_timer_start(&k_sw_timer_on, K_MSEC(10), K_NO_WAIT);
    }
    else
    {
        k_timer_stop(&k_sw_timer_on);
        start_time_off = k_ticks_to_us_floor64(k_uptime_ticks());
    }
}

void bb_switch_off_event(uint8_t direct)
{
    if(direct) 
    {
        end_time_off = k_ticks_to_us_floor64(k_uptime_ticks());
        k_timer_start(&k_sw_timer_off, K_MSEC(10), K_NO_WAIT);
    }
    else
    {
        k_timer_stop(&k_sw_timer_off);
        start_time_on = k_ticks_to_us_floor64(k_uptime_ticks());
    }
}
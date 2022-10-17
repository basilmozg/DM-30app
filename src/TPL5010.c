/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <drivers/gpio.h>
#include <logging/log.h>
#include "TPL5010.h"
#include "gpio_constructor.h"

LOG_MODULE_REGISTER(TPL5010);

/*
 * Get TIM_WAKE configuration from the devicetree sw0 alias. This is mandatory.
 */
#define TIM_WAKE_NODE	DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(TIM_WAKE_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec TIM_WAKE = GPIO_DT_SPEC_GET_OR(TIM_WAKE_NODE, gpios, {0});
static struct gpio_callback TIM_WAKE_cb_data;
static struct gpio_dt_spec TIM_DONE = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});

void TIM_DONE_function()
{
    gpio_pin_set_dt(&TIM_DONE, 0);
}

K_TIMER_DEFINE(TIM_timer, TIM_DONE_function, NULL);

void WDT_acknoledge()
{
    //LOG_INF("TPL5010 acknoledge event");
    gpio_pin_set_dt(&TIM_DONE, 1);
    k_timer_start(&TIM_timer, K_MSEC(100), K_NO_WAIT);
}

void TIM_WAKE_event(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    WDT_acknoledge();
}

void TPL5010_init()
{
    LOG_INF("TPL5010_init");
    if(pin_button_setup(&TIM_WAKE)) 
        LOG_ERR("TIM_WAKE install error");

    if(pin_button_interrupt_setup(&TIM_WAKE, &TIM_WAKE_cb_data, *TIM_WAKE_event, GPIO_INT_EDGE_RISING)) 
        LOG_ERR("TIM_WAKE interrupt install error");

    if(pin_led_setup(&TIM_DONE, GPIO_OUTPUT)) LOG_ERR("TIM_DONE install error");

    WDT_acknoledge();
}

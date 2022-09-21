/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <drivers/gpio.h>
#include <logging/log.h>
#include "gpio_constructor.h"

LOG_MODULE_REGISTER(gpio_constructor);

int pin_button_setup(const struct gpio_dt_spec *PIN_DI)
{
    if (!device_is_ready(PIN_DI->port)) 
    {
        LOG_INF("Error: %s is not ready", PIN_DI->port->name);
        return 11;
    }

    int ret = gpio_pin_configure_dt(PIN_DI, GPIO_INPUT);
    if (ret != 0) 
    {
        LOG_INF("Error %d: failed to configure %s pin %d",
		       ret, PIN_DI->port->name, PIN_DI->pin);
	return 1;
    }

   // LOG_INF("Set up button at %s pin %d", PIN_DI->port->name, PIN_DI->pin);
    return 0;
}

int pin_button_interrupt_setup(const struct gpio_dt_spec *PIN_DI, 
                               struct gpio_callback *PIN_DI_cb_data, 
                               void *PIN_DI_event,
                               gpio_flags_t GPIO_cal_direction)
{
    int ret = gpio_pin_interrupt_configure_dt(PIN_DI, GPIO_cal_direction);
    if (ret != 0) 
    {
        LOG_INF("Error %d: failed to configure interrupt on %s pin %d",
			ret, PIN_DI->port->name, PIN_DI->pin);
	return 1;
    }
    gpio_init_callback(PIN_DI_cb_data, PIN_DI_event, BIT(PIN_DI->pin));
    gpio_add_callback(PIN_DI->port, PIN_DI_cb_data);
    //LOG_INF("Set up interrupt button at %s pin %d", PIN_DI->port->name, PIN_DI->pin);
    return 0;
}

int pin_led_setup(struct gpio_dt_spec *PIN_LED, gpio_flags_t extra_flag)
{
    int ret = 0;
    if (PIN_LED->port && !device_is_ready(PIN_LED->port)) 
    {
        LOG_INF("Error %d: PIN_LED device %s is not ready; ignoring it",
		       ret, PIN_LED->port->name);
        PIN_LED->port = NULL;
        return 1;
    }
    if(PIN_LED->port) 
    {
        ret = gpio_pin_configure_dt(PIN_LED, extra_flag);
        if (ret != 0) 
        {
            /*LOG_INF("Error %d: failed to configure PIN_LED device %s pin %d",
			       ret, PIN_LED->port->name, PIN_LED->pin);*/
            PIN_LED->port = NULL;
            return 1;
	} 
        else 
        {
            //LOG_INF("Set up led at %s pin %d", PIN_LED->port->name, PIN_LED->pin);
	}
    }
    return 0;
}
/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "led_di_handler.h"
#include "gpio_constructor.h"
#include "DEFINES.h"

#include <nrfx_gpiote.h>
#include <drivers/gpio.h>
#include <logging/log.h>

#define PIN_DI_0_NODE	DT_ALIAS(sw2)
#if !DT_NODE_HAS_STATUS(PIN_DI_0_NODE, okay)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif
#define PIN_DI_1_NODE	DT_ALIAS(sw3)
#if !DT_NODE_HAS_STATUS(PIN_DI_1_NODE, okay)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif
#define PIN_DI_2_NODE	DT_ALIAS(sw4)
#if !DT_NODE_HAS_STATUS(PIN_DI_2_NODE, okay)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif
#define PIN_DI_3_NODE	DT_ALIAS(sw5)
#if !DT_NODE_HAS_STATUS(PIN_DI_3_NODE, okay)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif
#define PIN_DI_4_NODE	DT_ALIAS(sw6)
#if !DT_NODE_HAS_STATUS(PIN_DI_4_NODE, okay)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif
#define PIN_DI_5_NODE	DT_ALIAS(sw7)
#if !DT_NODE_HAS_STATUS(PIN_DI_5_NODE, okay)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif
#define PIN_DI_6_NODE	DT_ALIAS(sw8)
#if !DT_NODE_HAS_STATUS(PIN_DI_6_NODE, okay)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif

#define PIN_DI_7_NODE	DT_ALIAS(sw9)
#if !DT_NODE_HAS_STATUS(PIN_DI_7_NODE, okay)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif
#define PIN_DI_8_NODE	DT_ALIAS(sw10)
#if !DT_NODE_HAS_STATUS(PIN_DI_8_NODE, okay)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif
#define PIN_DI_9_NODE	DT_ALIAS(sw11)
#if !DT_NODE_HAS_STATUS(PIN_DI_9_NODE, okay)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif
#define PIN_DI_10_NODE	DT_ALIAS(sw12)
#if !DT_NODE_HAS_STATUS(PIN_DI_10_NODE, okay)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec PIN_DIG_IN[11] = 
{
    GPIO_DT_SPEC_GET_OR(PIN_DI_0_NODE,  gpios, {0}),
    GPIO_DT_SPEC_GET_OR(PIN_DI_1_NODE,  gpios, {0}),
    GPIO_DT_SPEC_GET_OR(PIN_DI_2_NODE,  gpios, {0}),
    GPIO_DT_SPEC_GET_OR(PIN_DI_3_NODE,  gpios, {0}),
    GPIO_DT_SPEC_GET_OR(PIN_DI_4_NODE,  gpios, {0}),
    GPIO_DT_SPEC_GET_OR(PIN_DI_5_NODE,  gpios, {0}),
    GPIO_DT_SPEC_GET_OR(PIN_DI_6_NODE,  gpios, {0}),
    GPIO_DT_SPEC_GET_OR(PIN_DI_7_NODE,  gpios, {0}),
    GPIO_DT_SPEC_GET_OR(PIN_DI_8_NODE,  gpios, {0}),
    GPIO_DT_SPEC_GET_OR(PIN_DI_9_NODE,  gpios, {0}),
    GPIO_DT_SPEC_GET_OR(PIN_DI_10_NODE, gpios, {0})
};

static struct gpio_callback PIN_DI_cb_data[11];

static struct gpio_dt_spec DO_ESP_RESET = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0});

static struct gpio_dt_spec DO_OUTPUT[10] = 
{
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2),  gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led3),  gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led4),  gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led5),  gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led6),  gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led7),  gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led8),  gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led9),  gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led10), gpios, {0}),
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led11), gpios, {0})
};


LOG_MODULE_REGISTER(led_di_handler);

#define  bitSet(value, bit)                 ((value) |= (1UL << (bit)))
#define  bitClear(value, bit)               ((value) &= ~(1UL << (bit)))
#define  writebit(value, bit, bitvalue)     (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

void (*main_off_event)();
void (*main_on_event)();

uint8_t DI_CHANNEL_READ_0_to_10(uint8_t channel)
{
    return gpio_pin_get_dt(&PIN_DIG_IN[channel]);
}

static void OFF_event(const struct device *port,
					struct gpio_callback *cb,
					gpio_port_pins_t pins)
{
    (*main_off_event)(gpio_pin_get_dt(&PIN_DIG_IN[2]));
}

static void ON_event(const struct device *port,
					struct gpio_callback *cb,
					gpio_port_pins_t pins)
{
    (*main_on_event)(gpio_pin_get_dt(&PIN_DIG_IN[3]));
}
void DI_init(void *PIN_DI_event_off, void *PIN_DI_event_on)
{
    LOG_INF("DI_pins_init");
    for(uint8_t i = 0; i < 11; i++)
    {
        pin_button_setup(&PIN_DIG_IN[i]); 
    }
    main_off_event = PIN_DI_event_off;
    main_on_event  = PIN_DI_event_on;/*
    if(pin_button_interrupt_setup(&PIN_DIG_IN[2], 
                                  &PIN_DI_cb_data[2], 
                                  OFF_event,
                                  GPIO_INT_EDGE_BOTH)) LOG_ERR("pin_button_interrupt_setup error");
    if(pin_button_interrupt_setup(&PIN_DIG_IN[3], 
                                  &PIN_DI_cb_data[3], 
                                  ON_event,
                                  GPIO_INT_EDGE_BOTH)) LOG_ERR("pin_button_interrupt_setup error");*/
}

void ESP_RESET_timeout()
{
    gpio_pin_set_dt(&DO_ESP_RESET, 0);
    LOG_INF_GLOB("ESP_RESET");
}

K_TIMER_DEFINE(k_ESP_RESET_timeout, ESP_RESET_timeout, NULL);

void ESP_RESET()
{
    gpio_pin_set_dt(&DO_ESP_RESET, 1);
    k_timer_start(&k_ESP_RESET_timeout, K_MSEC(5), K_NO_WAIT); 
}

void D0_CHANNEL_WRITE_0_to_9(uint8_t channel, uint8_t state)
{
    gpio_pin_set_dt(&DO_OUTPUT[channel], state);
}

void DO_init()
{
    for(uint8_t i = 0; i < 8; i++)
        if(pin_led_setup(&DO_OUTPUT[i], GPIO_OUTPUT | GPIO_OUTPUT_INIT_LOW)) LOG_ERR("PIN_DO %d install error", i);

    if(pin_led_setup(&DO_OUTPUT[8], GPIO_OUTPUT | GPIO_OUTPUT_INIT_HIGH)) LOG_ERR("DO_LED 1 install error");
    if(pin_led_setup(&DO_OUTPUT[9], GPIO_OUTPUT | GPIO_OUTPUT_INIT_HIGH)) LOG_ERR("DO_LED 1 install error");

    if(pin_led_setup(&DO_ESP_RESET, GPIO_OUTPUT | GPIO_OUTPUT_INIT_HIGH)) LOG_ERR("DO_ESP_RESET install error");
}
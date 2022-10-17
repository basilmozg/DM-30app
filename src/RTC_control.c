#include "registers_modbus.h"
#include "RTC_control.h"
#include <logging/log.h>
#include <drivers/gpio.h>
#include "gpio_constructor.h"
#include <zephyr.h>
#include "led_di_handler.h"
#include "DEFINES.h"


LOG_MODULE_REGISTER(RTC_control_log);

#define PIN_RTC_NODE	DT_ALIAS(sw1)
#if !DT_NODE_HAS_STATUS(PIN_RTC_NODE, okay)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec PIN_RTC = GPIO_DT_SPEC_GET_OR(PIN_RTC_NODE, gpios, {0});
static struct gpio_callback PIN_RTC_cb_data;

/*RTC cloack handler*/

static int64_t last_millis;
static uint16_t *modbus_reg_04;
static uint8_t RTC_ok;
static uint8_t rtc_read_try;
static void (*EVENT_SEND_ptr)(uint8_t , uint8_t, int16_t, uint8_t *);

void RTC_success()
{
    RTC_ok = 2;
    rtc_read_try = 0;
}

bool RTC_valid()
{
    if(RTC_ok == 2) return 1;
    return 0;
}

void Time_Get()
{
    if(RTC_ok == 1) {RTC_ok = 0; rtc_read_try++;}
}

static void EVENT_SEND(uint8_t event_type, uint8_t event_number, int16_t tag_value, uint8_t *data_transite)
{
    (*EVENT_SEND_ptr)(event_type, event_number, tag_value, data_transite);
}
/*
K_WORK_DEFINE(Time_Get_work, Time_Get);

void Time_Get_handler(struct k_timer *dummy)
{
    k_work_submit(&Time_Get_work);
}
*/
K_TIMER_DEFINE(Time_Get_timer, Time_Get, NULL);

void RTC_control()
{
    if(RTC_ok == 0) 
    {
        RTC_ok = 1;
        if(rtc_read_try == rtc_read_try_count) 
        {
            rtc_read_try = 0;
            ESP_RESET();
        }
        else EVENT_SEND(EVENT_SYSTEM_SCENARIO, mess_x05_event_GIVE_ME_TIME, 0, NULL); 
        k_timer_start(&Time_Get_timer, K_MSEC(1000), K_NO_WAIT); 
    } 
}
/*
K_WORK_DEFINE(RTC_control_work, RTC_control);

void RTC_control_timer_handler(struct k_timer *dummy)
{
    k_work_submit(&RTC_control_work);
}
*/
K_TIMER_DEFINE(RTC_control_timer, RTC_control, NULL);

uint8_t RealClockHandler(int16_t *modbus_reg_04)
{
    uint8_t rtc_update = 2;
    modbus_reg_04[mod_reg_04_RTC_millis] = 0;
    if(modbus_reg_04[mod_reg_04_RTC_second] < 59) modbus_reg_04[mod_reg_04_RTC_second]++;
    else
    {
        modbus_reg_04[mod_reg_04_RTC_second] = 0;
        if(modbus_reg_04[mod_reg_04_RTC_minute] < 59) modbus_reg_04[mod_reg_04_RTC_minute]++;
        else 
        {
            modbus_reg_04[mod_reg_04_RTC_minute] = 0;
            if(modbus_reg_04[mod_reg_04_RTC_hour] < 23) modbus_reg_04[mod_reg_04_RTC_hour]++;
                else 
                {
                    modbus_reg_04[mod_reg_04_RTC_hour] = 0;
                    modbus_reg_04[mod_reg_04_RTC_date]++;
                    if(modbus_reg_04[mod_reg_04_RTC_day] < 7) modbus_reg_04[mod_reg_04_RTC_day]++; 
                    else modbus_reg_04[mod_reg_04_RTC_day] = 1;

                    if(modbus_reg_04[mod_reg_04_RTC_date] == 32 && (modbus_reg_04[mod_reg_04_RTC_month] == 1 || modbus_reg_04[mod_reg_04_RTC_month] == 3 || 
                       modbus_reg_04[mod_reg_04_RTC_month] == 5 || modbus_reg_04[mod_reg_04_RTC_month] == 7 || modbus_reg_04[mod_reg_04_RTC_month] == 8 || 
                       modbus_reg_04[mod_reg_04_RTC_month] == 10 || modbus_reg_04[mod_reg_04_RTC_month] == 12)) 
                    {
                        modbus_reg_04[mod_reg_04_RTC_date] = 1;
                        modbus_reg_04[mod_reg_04_RTC_month] += 1;
                    } 
                    else
                    if(modbus_reg_04[mod_reg_04_RTC_date] == 31 && (modbus_reg_04[mod_reg_04_RTC_month] == 4 || 
                       modbus_reg_04[mod_reg_04_RTC_month] == 6 || modbus_reg_04[mod_reg_04_RTC_month] == 9 || modbus_reg_04[mod_reg_04_RTC_month] == 11)) 
                    {
                        modbus_reg_04[mod_reg_04_RTC_date] = 1;
                        modbus_reg_04[mod_reg_04_RTC_month] += 1;
                    } 
                    else
                    if(modbus_reg_04[mod_reg_04_RTC_date] == 29 && modbus_reg_04[mod_reg_04_RTC_month] == 2 && (modbus_reg_04[mod_reg_04_RTC_year] % 4) != 0) 
                    {
                        modbus_reg_04[mod_reg_04_RTC_date] = 1;
                        modbus_reg_04[mod_reg_04_RTC_month] += 1;
                    } 
                    else
                    if(modbus_reg_04[mod_reg_04_RTC_date] == 30 && modbus_reg_04[mod_reg_04_RTC_month] == 2 && (modbus_reg_04[mod_reg_04_RTC_year] % 4) == 0) 
                    {
                        modbus_reg_04[mod_reg_04_RTC_date] = 1;
                        modbus_reg_04[mod_reg_04_RTC_month] += 1;
                    }

                    if(modbus_reg_04[mod_reg_04_RTC_month] == 13) 
                    {
                        modbus_reg_04[mod_reg_04_RTC_month] = 1;
                        modbus_reg_04[mod_reg_04_RTC_year] += 1;
                    }
                }
            }
        if(modbus_reg_04[mod_reg_04_RTC_minute] == 30) rtc_update = 0;
    }
    return rtc_update;
}

void RTC_millis_get(int16_t *millis)
{
    *millis = (k_uptime_get() - last_millis)%1000;
}

void RTC_Function(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    last_millis = k_uptime_get();
    if(RTC_ok == 2) RTC_ok = RealClockHandler(modbus_reg_04);
    RTC_control();
}

void RealClockInit(int16_t *modbus_reg_04_, void *EVENT_SEND_)
{
    modbus_reg_04 = modbus_reg_04_;
    EVENT_SEND_ptr = EVENT_SEND_;

    LOG_INF_GLOB("RTC_pins_init");

    if(pin_button_setup(&PIN_RTC))
        LOG_ERR_GLOB("PIN_RTC install error");

    if(pin_button_interrupt_setup(&PIN_RTC, &PIN_RTC_cb_data, *RTC_Function, GPIO_INT_EDGE_RISING))
        LOG_ERR_GLOB("PIN_RTC interrupt install error");
    k_timer_start(&RTC_control_timer,  K_MSEC(1000), K_MSEC(1000));
}
#include "temperature_work.h"
#include <zephyr.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(temperature_work);

#define MASK_SIGN_EXTENSION (0xFFFFFC00UL)
#define MASK_SIGN           (0x00000200UL)

static int16_t *temp_register = NULL;
struct k_timer k_temp_timer;

void init_check()
{
    if(temp_register == NULL)
    {
        LOG_ERR("Temperature init error");
        return;
    }
}

static void temperature_measure()
{
    int32_t temp;
    NRF_TEMP->TASKS_START = 1; 
    /*
    while (NRF_TEMP->EVENTS_DATARDY == 0);
    */
    NRF_TEMP->EVENTS_DATARDY = 0;
    temp =  ((NRF_TEMP->TEMP & MASK_SIGN) != 0) ? 
            (int32_t)(NRF_TEMP->TEMP | MASK_SIGN_EXTENSION) : 
            (NRF_TEMP->TEMP);

    NRF_TEMP->TASKS_STOP = 1;
    *temp_register = temp * 10 / 4;
}

void temperature_get()
{
    init_check();
    temperature_measure();
}
/*
K_WORK_DEFINE(k_temp_work, temperature_get);

void k_temp_handler(struct k_timer *dummy)
{
    k_work_submit(&k_temp_work);
}
*/
K_TIMER_DEFINE(k_temp_timer, temperature_get/*k_temp_handler*/, NULL);

void temperature_init(int16_t *temp_register_)
{
    temp_register = temp_register_;
    LOG_INF("Temperature init ok");
}

void temperature_init_msec(int16_t *temp_register_, uint32_t period_ms)
{
    temperature_init(temp_register_);
    init_check();
    k_timer_start(&k_temp_timer,  K_MSEC(period_ms), K_MSEC(period_ms));
    temperature_measure();
}

void temperature_init_sec(int16_t *temp_register_, uint32_t period_s)
{
    temperature_init(temp_register_);
    init_check();
    k_timer_start(&k_temp_timer,  K_SECONDS(period_s), K_SECONDS(period_s));
    temperature_measure();
}

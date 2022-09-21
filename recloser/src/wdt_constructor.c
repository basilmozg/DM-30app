
#include <logging/log.h>
#include "wdt_constructor.h"
#include <zephyr.h>

LOG_MODULE_REGISTER(wdt);

#define reload_magic_value    0x6E524635
K_SEM_DEFINE(WDT_init_ok, 0, 1);

void FeedHungryDog()
{
    NRF_WDT->RR[0] = reload_magic_value;
}

void WDT_init_timer(uint32_t WDT_MAX_WINDOW_SEC)
{
    //if ((NRF_WDT->RUNSTATUS & 0x1) != 0) return; unused thing
    NRF_WDT->CONFIG = 1;
    NRF_WDT->CRV = 32768 * WDT_MAX_WINDOW_SEC;
    NRF_WDT->TASKS_START = 0x1;
    k_sem_give(&WDT_init_ok);
}

void WDT_init() // 10 sec default time
{
    WDT_init_timer(10);
}

void breakfast()
{
    k_sem_take(&WDT_init_ok, K_FOREVER);
    for(;;)
    {
        FeedHungryDog();
        k_sleep(K_MSEC(1000)); 
    }
}

#define STACKSIZE   512
#define PRIORITY    99
K_THREAD_DEFINE(breakfast_id, STACKSIZE, breakfast, NULL, NULL, NULL, PRIORITY, 0, 0);

#include "highspeed_timer.h"
#include <logging/log.h>

#include "nrfx_timer.h"
#include <zephyr.h>

LOG_MODULE_REGISTER(highspeed_timer);

#define DEFAULT_TIMER_INSTANCE     4
#define DEFAULT_TIMER_IRQ          NRFX_CONCAT_3(TIMER,	DEFAULT_TIMER_INSTANCE, _IRQn)
#define DEFAULT_TIMER_IRQ_HANDLER  NRFX_CONCAT_3(nrfx_timer_,	DEFAULT_TIMER_INSTANCE, _irq_handler)
#define DEFAULT_TIMER_PRIORITY     3

static void *timer_handler;

static const nrfx_timer_t millis_sample_timer = NRFX_TIMER_INSTANCE(4);

int highspeed_timer_init(void *timer_handler_)
{
    timer_handler = timer_handler_;
    nrfx_err_t err;
    nrfx_timer_config_t timer_cfg = {
		.frequency = NRF_TIMER_FREQ_16MHz,
		.mode = NRF_TIMER_MODE_TIMER,
		.bit_width = NRF_TIMER_BIT_WIDTH_32,
    };

    err = nrfx_timer_init(&millis_sample_timer, &timer_cfg, timer_handler);
    if (err != NRFX_SUCCESS) 
    {
        LOG_INF("nrfx_timer_init failed with: %d\n", err);
	return -EAGAIN;
    }

    IRQ_CONNECT(DEFAULT_TIMER_IRQ, DEFAULT_TIMER_PRIORITY,
		DEFAULT_TIMER_IRQ_HANDLER, NULL, 0);

    nrfx_timer_extended_compare(&millis_sample_timer,
                                NRF_TIMER_CC_CHANNEL4,
                                nrfx_timer_us_to_ticks(&millis_sample_timer, 1005),
                                NRF_TIMER_SHORT_COMPARE4_CLEAR_MASK,
                                true);

    nrfx_timer_enable(&millis_sample_timer);
    return 0;
}

void highspeed_timer_pause()
{
    nrfx_timer_pause(&millis_sample_timer);
}

void highspeed_timer_resume()
{
    nrfx_timer_resume(&millis_sample_timer);
}


#ifndef SAADC_WORK_H__
#define SAADC_WORK_H__

#include <zephyr.h>

#define ADC_CHANNELS_IN_USE                       8
#define SAADC_BUF_SIZE                            8
#define NRF_SAADC_CHANNEL_COUNT                   8
#define SAADC_BUF_COUNT                           2200
#define SAADC_BUF_COUNTx2                         (SAADC_BUF_COUNT * 2)
#define SAADC_BUF_COUNTx3                         (SAADC_BUF_COUNT * 3)
#define LEN                                       SAADC_BUF_COUNT/1*2
#define ADC_CHANNELS_TO_SEND                      ADC_CHANNELS_IN_USE
#define NRFX_SAADC_IRQ_PRIORITY                   0
#define NRFX_TIMER_IRQ_PRIORITY                   0

void adc_ppi_init(void *CURRENT_PHASE_EVENT_);

void ppi_restart();

void ppi_pause();

#endif
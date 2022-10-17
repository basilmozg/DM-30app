
#include "saadc_work.h"
#include <logging/log.h>
#include <zephyr.h>
#include "DEFINES.h"

#include <nrfx_saadc.h>
#include "nrfx_timer.h"
#include <helpers/nrfx_gppi.h>
#include <nrfx_ppi.h>

#if defined MANUAL_ADC
#include "seggerDebug.h"

int16_t CUR[8];
#endif

LOG_MODULE_REGISTER(saadc);
static void timer_handler(nrf_timer_event_t event_type, void * p_context){};

static const nrfx_timer_t m_sample_timer = NRFX_TIMER_INSTANCE(3);

static nrf_saadc_value_t samples[3][SAADC_BUF_COUNT][SAADC_BUF_SIZE];
static nrf_ppi_channel_t m_timer_saadc_ppi_channel;
static uint32_t buffer_index;
static uint32_t buf_select = 0;

static const nrf_saadc_input_t ANALOG_INPUT_MAP[NRF_SAADC_CHANNEL_COUNT] = 
{
    NRF_SAADC_INPUT_AIN2, NRF_SAADC_INPUT_AIN6, NRF_SAADC_INPUT_AIN4, NRF_SAADC_INPUT_AIN3,
    NRF_SAADC_INPUT_AIN7, NRF_SAADC_INPUT_AIN5, NRF_SAADC_INPUT_AIN0, NRF_SAADC_INPUT_AIN1
};

void (*CURRENT_PHASE_EVENT_ptr)(int16_t, uint8_t);

static void event_handler(nrfx_saadc_evt_t const * p_event)
{
    switch (p_event->type)
    {
        case NRFX_SAADC_EVT_DONE:
            #if defined (MANUAL_ADC)
                (*CURRENT_PHASE_EVENT_ptr)(CUR[0], 0);
                (*CURRENT_PHASE_EVENT_ptr)(CUR[1], 1);
                (*CURRENT_PHASE_EVENT_ptr)(CUR[2], 2);
                (*CURRENT_PHASE_EVENT_ptr)(CUR[3], 3);
                (*CURRENT_PHASE_EVENT_ptr)(CUR[4], 4);
                (*CURRENT_PHASE_EVENT_ptr)(CUR[5], 5); 
                (*CURRENT_PHASE_EVENT_ptr)(CUR[6], 6);
                (*CURRENT_PHASE_EVENT_ptr)(CUR[7], 7);
            #else
            (*CURRENT_PHASE_EVENT_ptr)(p_event->data.done.p_buffer[0], 0);
            (*CURRENT_PHASE_EVENT_ptr)(p_event->data.done.p_buffer[1], 1);
            (*CURRENT_PHASE_EVENT_ptr)(p_event->data.done.p_buffer[2], 2);
            (*CURRENT_PHASE_EVENT_ptr)(p_event->data.done.p_buffer[3], 3);
            (*CURRENT_PHASE_EVENT_ptr)(p_event->data.done.p_buffer[4], 4);
            (*CURRENT_PHASE_EVENT_ptr)(p_event->data.done.p_buffer[5], 5); 
            (*CURRENT_PHASE_EVENT_ptr)(p_event->data.done.p_buffer[6], 6);
            (*CURRENT_PHASE_EVENT_ptr)(p_event->data.done.p_buffer[7], 7);
            #endif
            
        break;

        case NRFX_SAADC_EVT_BUF_REQ:         

            if(buf_select < SAADC_BUF_COUNT) 
            {
                //current_frame = 0;
                nrfx_saadc_buffer_set(&samples[0][buffer_index][0], SAADC_BUF_SIZE);
                nrfx_saadc_buffer_set(&samples[0][buffer_index][0], SAADC_BUF_SIZE);

            }
            else
            if(buf_select < SAADC_BUF_COUNTx2)
            {
                //current_frame = 1;
                nrfx_saadc_buffer_set(&samples[1][buffer_index][0], SAADC_BUF_SIZE);
                nrfx_saadc_buffer_set(&samples[1][buffer_index][0], SAADC_BUF_SIZE);
            }
            else
            if(buf_select < SAADC_BUF_COUNTx3)
            {
                //current_frame = 2;
                nrfx_saadc_buffer_set(&samples[2][buffer_index][0], SAADC_BUF_SIZE);
                nrfx_saadc_buffer_set(&samples[2][buffer_index][0], SAADC_BUF_SIZE);
            }
            buf_select++;
            if(buf_select == SAADC_BUF_COUNTx3) buf_select = 0;
            //next_free_buf_index();
        break;

        default:
            LOG_INF_GLOB("default:");
        break;
        buffer_index++;
        if(buffer_index == SAADC_BUF_COUNT) buffer_index = 0;
    }
}

static void timer_init(void)
{
    nrfx_err_t err_code;

    nrfx_timer_config_t timer_config = NRFX_TIMER_DEFAULT_CONFIG;
    timer_config.frequency = NRF_TIMER_FREQ_16MHz;
    timer_config.interrupt_priority = NRFX_TIMER_IRQ_PRIORITY;
    err_code = nrfx_timer_init(&m_sample_timer, &timer_config, timer_handler);

    nrfx_timer_extended_compare(&m_sample_timer,
                                NRF_TIMER_CC_CHANNEL0,
                                nrfx_timer_us_to_ticks(&m_sample_timer, saadc_sampling_rate),
                                NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                false);
    nrfx_timer_resume(&m_sample_timer);
}


static void ppi_init(void)
{
    // Trigger task sample from timer
    nrfx_err_t err_code = nrfx_ppi_channel_alloc(&m_timer_saadc_ppi_channel);

    err_code = nrfx_ppi_channel_assign(m_timer_saadc_ppi_channel, 
                                       nrfx_timer_event_address_get(&m_sample_timer, NRF_TIMER_EVENT_COMPARE0),
                                       nrf_saadc_task_address_get(NRF_SAADC_TASK_SAMPLE));

    err_code = nrfx_ppi_channel_enable(m_timer_saadc_ppi_channel);
}

static void adc_configure(void)
{
    int err_code;

    nrfx_saadc_adv_config_t saadc_adv_config = NRFX_SAADC_DEFAULT_ADV_CONFIG;
    saadc_adv_config.internal_timer_cc = 0;
    saadc_adv_config.start_on_end = true;

    err_code = nrfx_saadc_init(NRFX_SAADC_IRQ_PRIORITY);

    static nrfx_saadc_channel_t channel_configs[ADC_CHANNELS_IN_USE];

    uint8_t channel_mask = 0;
    for(int i = 0; i < ADC_CHANNELS_IN_USE; i++) 
    {
        nrf_saadc_input_t pin = ANALOG_INPUT_MAP[i];
        // Apply default config to each channel
        nrfx_saadc_channel_t config = NRFX_SAADC_DEFAULT_CHANNEL_SE(pin, i);

        // Replace some parameters in default config
        config.channel_config.reference = NRF_SAADC_REFERENCE_INTERNAL;          
        config.channel_config.gain = SAADC_CH_CONFIG_GAIN_Gain1_6;
        config.channel_config.acq_time   = NRF_SAADC_ACQTIME_3US;
        config.channel_config.resistor_p = NRF_SAADC_RESISTOR_PULLUP;
        config.channel_config.resistor_p = NRF_SAADC_RESISTOR_PULLDOWN;

        // Copy to list of channel configs
        memcpy(&channel_configs[i], &config, sizeof(config));

        // Update channel mask
        channel_mask |= 1 << i;
    }

    err_code = nrfx_saadc_channels_config(channel_configs, ADC_CHANNELS_IN_USE);

    err_code = nrfx_saadc_advanced_mode_set(channel_mask,
                                            NRF_SAADC_RESOLUTION_14BIT,
                                            &saadc_adv_config,
                                            event_handler);
                                            
    // Configure two buffers to ensure double buffering of samples, to avoid data loss when the sampling frequency is high
    err_code = nrfx_saadc_buffer_set(&samples[0][0][0], SAADC_BUF_SIZE);

    err_code = nrfx_saadc_buffer_set(&samples[0][0][0], SAADC_BUF_SIZE);

    err_code = nrfx_saadc_mode_trigger();
}

void ppi_run()
{
    nrfx_timer_resume(&m_sample_timer);
}

K_TIMER_DEFINE(timer_ppi_restart, ppi_run, NULL);

void ppi_restart()
{
    k_timer_start(&timer_ppi_restart, K_MSEC(500), K_NO_WAIT); 
}

void ppi_pause()
{
    nrfx_timer_pause(&m_sample_timer);
}

#if defined (MANUAL_ADC)
void call_1(char *com, int *data, uint8_t num)
{
    CUR[*data] = *(data + 1);
}
#endif

void adc_ppi_init(void *CURRENT_PHASE_EVENT_)
{
    CURRENT_PHASE_EVENT_ptr = CURRENT_PHASE_EVENT_;
    adc_configure();
    ppi_init();
    timer_init();
    IRQ_CONNECT(DT_IRQN(DT_NODELABEL(adc)), 0,
		nrfx_isr, nrfx_saadc_irq_handler, 0);

    #if defined (MANUAL_ADC)
    Segger_init(&call_1, "q");
    Segger_start('\n', ' ');
    #endif
}
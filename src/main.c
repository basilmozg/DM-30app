
/*
ver.1.1.5
*/
#ifdef CONFIG_BOOTLOADER_MCUBOOT
    #define ver_char  CONFIG_MCUBOOT_IMAGE_VERSION
    #define NRF_FW_VERSION (((ver_char[0] - 48) << 8) + ((ver_char[2] - 48) << 4) + (ver_char[4] - 48))
#else
    #define NRF_FW_major                    1
    #define NRF_FW_minor                    2
    #define NRF_FW_patch                    8
    #define NRF_FW_VERSION                  ((NRF_FW_major << 8) + (NRF_FW_minor << 4) + NRF_FW_patch)
#endif

#include "IEC_104.h"
#include "registers_modbus.h"
#include "calculations.h"
#include "DEFINES.h"
#include "control_logic.h"

#include "wdt_constructor.h"
#include "TPL5010.h"
#include "led_di_handler.h"
#include "RTC_control.h"
#include "bluetooth_work.h"
#include "saadc_work.h"
#include "uart_work.h"
#include "temperature_work.h"
#include "FlashStorage.h"
#include "highspeed_timer.h"
#include "di_interrupt_work.h"
#include "seggerDebug.h"

#include <logging/log.h>
#include <zephyr.h>
#include <stdlib.h>

LOG_MODULE_REGISTER(RELE);

/*MASTER VARIABLES*/

/*IEC_104 vars*/
static int16_t modbus_reg_04[mod_reg_04_num];

static int16_t modbus_reg_03[mod_reg_03_num];

extern uint16_t GLOBAL_start_command;
extern uint8_t General_State;
extern uint8_t Global_trip;
extern uint8_t SWITCHER_ON_timeout_timer_flag;
extern uint8_t SWITCHER_OFF_timeout_timer_flag;
extern uint16_t current_system_mode;
extern uint8_t zvn_alarm;
extern uint8_t apv_zmn_trip;
extern int CURRENT[8];
/****************/

static uint16_t scenario = 0;
uint8_t gpio_change = 0;
uint8_t wifi_window_updater = 0;
uint8_t ethernet_window_updater = 0;
uint8_t mtz_window_updater = 0;
uint8_t calibration_window_updater = 0;
uint8_t mode_window_updater = 0;
uint8_t main_window_updater = 0;

uint8_t alarm = 255, eeprom_write;
uint8_t current_frame; 

int32_t RECALC_current_trip[6][3];
int32_t RECALC_current_back[6][3];
int32_t RECALC_MTZ_timer_trip[6][3];
int32_t RECALC_MTZ_timer_back[6];
int32_t MTZ_1T_depend_coef_k[3];
int32_t MTZ_1T_depend_coef_b[3];
int32_t last_current_trip[3];
uint16_t cutt_off_meas_value[8];

static uint8_t half_phase[8], full_phase[8], direct[8];
static int32_t min_val[8] = {32767,32767,32767,32767,32767,32767,32767,32767}, max_val[8] ={0,0,0,0,0,0,0,0};
static int32_t zero_val_final[8], zero_val[8], zero_val_counter[8];
static int32_t RMS[8];
static int32_t CURRENT_median[8];
static int32_t CURRENT_median_result[8];
static int32_t SUB_ZERO_RMS[8], ABOVE_ZERO_RMS[8];

/*GENERAL LOGIC*/
uint16_t switch_di_timer;
struct k_timer SWITCHER_ON_feedback_timer;
struct k_timer SWITCHER_OFF_feedback_timer;


static  int32_t MTZ_timer_trip[6][3] = {{max_32_bit_val, max_32_bit_val, max_32_bit_val},
                                        {max_32_bit_val, max_32_bit_val, max_32_bit_val},
                                        {max_32_bit_val, max_32_bit_val, max_32_bit_val},
                                        {max_32_bit_val, max_32_bit_val, max_32_bit_val},
                                        {max_32_bit_val, max_32_bit_val, max_32_bit_val},
                                        {max_32_bit_val, max_32_bit_val, max_32_bit_val}
                                       };

static  int32_t MTZ_timer_back[6][3] = {{max_32_bit_val, max_32_bit_val, max_32_bit_val},
                                        {max_32_bit_val, max_32_bit_val, max_32_bit_val},
                                        {max_32_bit_val, max_32_bit_val, max_32_bit_val},
                                        {max_32_bit_val, max_32_bit_val, max_32_bit_val},
                                        {max_32_bit_val, max_32_bit_val, max_32_bit_val},
                                        {max_32_bit_val, max_32_bit_val, max_32_bit_val}
                                       };
uint8_t MTZ_trip[6][3];
uint8_t MTZ_back[6][3];
uint8_t angle_cal_count;
uint8_t zmn_current_channel = 0;
uint8_t apv_current_channel = 0;
uint8_t ch_zmn_final[4];
void (*part_MTZ_invert_ptr[4])(int control_val, uint8_t pr, uint8_t ch, uint8_t num);
void (*part_ZVN_CONTROL_ptr[4])(int control_val);
void (*part_ZMN_await_ptr[4])(int control_val, uint8_t num);

void part_MTZ_invert(int control_val, uint8_t pr, uint8_t ch, uint8_t num);
void part_ZVN_CONTROL(int control_val);
void part_ZMN_await(int control_val, uint8_t num);

uint8_t channel_selection[6][3] =
{
    {0,1,2},
    {0,1,2},
    {0,1,2},
    {0,1,2},
    {4,5,6},
    {3,0,0}
};

/*uart*/
#define uart_speed                      1000000//460800
#define UART_BUF_SIZE                   200
#define UART_BUF_SIZE_1                 UART_BUF_SIZE - 1
static uint8_t data_array[BUF_SIZE_GLOBAL + 10];

/**/
uint8_t tx_buffer[4][UART_BUF_SIZE];
static uint8_t tx_timer_full[4] = {0};
static uint8_t tx_buffer_size[4] = {0};
static bool KTS[4] = {false};
struct k_timer tx_timeout_timer[4];
/**/
/************ 0 */
void tx_timeout_handle_0()
{
    uart_answer(&tx_buffer[0][0], tx_buffer_size[0]); 
    tx_timer_full[0] = 0;
    tx_buffer_size[0] = 0;
    KTS[0] = 0;
}
/*
K_WORK_DEFINE(tx_timeout_work_0, tx_timeout_handle_0);

void tx_timeout_handle_work_0()
{
    k_work_submit(&tx_timeout_work_0);
}
*/
/************ 1 */
void tx_timeout_handle_1()
{
    uart_answer(&tx_buffer[1][0], tx_buffer_size[1]); 
    tx_timer_full[1] = 0;
    tx_buffer_size[1] = 0;
    KTS[1] = 0;
}
/*
K_WORK_DEFINE(tx_timeout_work_1, tx_timeout_handle_1);

void tx_timeout_handle_work_1()
{
    k_work_submit(&tx_timeout_work_1);
}
*/
/************ 0 */
void tx_timeout_handle_2()
{
    uart_answer(&tx_buffer[2][0], tx_buffer_size[2]); 
    tx_timer_full[2] = 0;
    tx_buffer_size[2] = 0;
    KTS[2] = 0;
}
/*
K_WORK_DEFINE(tx_timeout_work_2, tx_timeout_handle_2);

void tx_timeout_handle_work_2()
{
    k_work_submit(&tx_timeout_work_2);
}
*/
/************ 0 */
void tx_timeout_handle_3()
{
    uart_answer(&tx_buffer[3][0], tx_buffer_size[3]); 
    tx_timer_full[3] = 0;
    tx_buffer_size[3] = 0;
    KTS[3] = 0;
}
/*
K_WORK_DEFINE(tx_timeout_work_3, tx_timeout_handle_3);

void tx_timeout_handle_work_3()
{
    k_work_submit(&tx_timeout_work_3);
}
*/
/*****************************************************/
uint16_t max_k_timer()
{
    uint16_t ttt = k_timer_remaining_get(&tx_timeout_timer[0]);

    if(ttt < k_timer_remaining_get(&tx_timeout_timer[1]))
    {
        ttt = k_timer_remaining_get(&tx_timeout_timer[1]);
    } 

    if(ttt < k_timer_remaining_get(&tx_timeout_timer[2]))
    {
        ttt = k_timer_remaining_get(&tx_timeout_timer[2]);
    } 

    if(ttt < k_timer_remaining_get(&tx_timeout_timer[3]))
    {
        ttt = k_timer_remaining_get(&tx_timeout_timer[3]);
    } 
    return ttt;
}

static bool buffer_selection(uint8_t ind, uint16_t pack_length, uint8_t *event_mess)
{ 
    if(tx_timer_full[ind] == 0) 
    {
        if((tx_buffer_size[ind] + pack_length) < UART_BUF_SIZE_1)
        {
            memcpy(&tx_buffer[ind][tx_buffer_size[ind]], event_mess, pack_length);
            tx_buffer_size[ind] += pack_length;
            if(!KTS[ind])
            {
                KTS[ind] = 1;
                k_timer_start(&tx_timeout_timer[ind], K_MSEC(ESP_PACK_INTERVEL_ms + max_k_timer()), K_NO_WAIT);
            }
            return false;
        }
        else tx_timer_full[ind] = 1;
    }
    return true;
}


static void EVENT_SEND(uint8_t event_type, uint8_t event_number, int16_t tag_value, uint8_t *data_transite)
{
    uint16_t pack_length = 0;

    uint8_t event_mess[260] = {0xAB, 0xCD, 0xEF};

    switch(event_type)
    {
        case EVENT_SYSTEM_SCENARIO:
            if(event_number == mess_x05_event_TAKE_MY_TIME)
            {
                event_mess[11] = modbus_reg_04[mod_reg_04_RTC_second];
                event_mess[12] = modbus_reg_04[mod_reg_04_RTC_minute];
                event_mess[13] = modbus_reg_04[mod_reg_04_RTC_hour];
                event_mess[14] = modbus_reg_04[mod_reg_04_RTC_day];
                event_mess[15] = modbus_reg_04[mod_reg_04_RTC_date];
                event_mess[16] = modbus_reg_04[mod_reg_04_RTC_month];
                event_mess[17] = modbus_reg_04[mod_reg_04_RTC_year] - 2000;
                pack_length = 19;
            }
            else
            {
                event_mess[11] = tag_value >> 8;
                event_mess[12] = tag_value & 255;
                pack_length = 14;
            }
        break;

        case WRITE_TAG_SCENARIO:
            memcpy(&event_mess[11], data_transite + 2, tag_value);
            pack_length = 12 + tag_value;
        break;

        case EVENT_DISCRET_OUT_SCENARIO:
        case EVENT_DISCRET_IN_SCENARIO:
        case READ_REQUEST_SCENARIO:
            event_mess[11] = tag_value >> 8;
            event_mess[12] = tag_value & 255;
            pack_length = 14;
        break;

        case EVENT_X04_SCENARIO:
            event_mess[11] = tag_value >> 8;
            event_mess[12] = tag_value & 255;
            event_mess[13] = *data_transite;
            event_mess[14] = *(data_transite + 1);
            pack_length = 16;
        break;

        case IEC_104_WINDOW_SCENARIO:
            memcpy(&event_mess[11], data_transite, event_number); 
            pack_length = 12 + event_number;
        break;
    }

    event_mess[3] = pack_length - 1; // pack_length
    event_mess[4] = event_type;
    event_mess[5] = event_number;
    if(RTC_valid()) event_mess[6] = modbus_reg_04[mod_reg_04_RTC_hour];
    else event_mess[6] = 255;
    event_mess[7] = modbus_reg_04[mod_reg_04_RTC_minute];
    event_mess[8] = modbus_reg_04[mod_reg_04_RTC_second];
    RTC_millis_get(&modbus_reg_04[mod_reg_04_RTC_millis]);
    event_mess[9] = modbus_reg_04[mod_reg_04_RTC_millis] >> 8;
    event_mess[10] = modbus_reg_04[mod_reg_04_RTC_millis];
    event_mess[pack_length - 1] = 0;

    for(uint16_t i = 0; i < pack_length - 1; i++) event_mess[pack_length - 1] += event_mess[i];

    if(!buffer_selection(0, pack_length, event_mess)) return;
    if(!buffer_selection(1, pack_length, event_mess)) return;
    if(!buffer_selection(2, pack_length, event_mess)) return;
    if(!buffer_selection(3, pack_length, event_mess)) return;
}

void rx_timeout(uint16_t *uart_index_global)
{
    int index = 0;
    while(index < (*(uart_index_global) - 8))
    {
        uint8_t CRC = 0;
        if(data_array[index] == 0xAB && data_array[index + 1] == 0xCD && data_array[index + 2] == 0xEF)
        {
            for(int i = index; i < index + data_array[index + 3]; i++) CRC += data_array[i]; 

            if(CRC == data_array[index + data_array[index + 3]])
            {
                switch(data_array[index + 4])
                {
                    case EVENT_SYSTEM_SCENARIO:
                        switch(data_array[index + 5])
                        {
                            case mess_x05_event_GIVE_ME_TIME:
                                RTC_success();
                                modbus_reg_04[mod_reg_04_RTC_year]   =  data_array[index + 6] + 2000;
                                modbus_reg_04[mod_reg_04_RTC_month]  =  data_array[index + 7];
                                modbus_reg_04[mod_reg_04_RTC_date]   =  data_array[index + 8];
                                modbus_reg_04[mod_reg_04_RTC_day]    =  data_array[index + 9];
                                modbus_reg_04[mod_reg_04_RTC_hour]   =  data_array[index + 10];
                                modbus_reg_04[mod_reg_04_RTC_minute] =  data_array[index + 11];
                                modbus_reg_04[mod_reg_04_RTC_second] =  data_array[index + 12];
                                modbus_reg_04[mod_reg_04_ESP_FW]     = (data_array[index + 13] << 8) + data_array[index + 14];
                            break;
                        }
                    break;
                            
                    case READ_REQUEST_SCENARIO:
                        switch(data_array[index + 5])
                        {
                            case SYSTEM_MODE_WINDOW_SCENARIO:
                            {
                                //LOG_INF_GLOB("Scenario answer - %d %d", SYSTEM_MODE_WINDOW_SCENARIO, mess_x03_esp_version);
                                switch(data_array[index + 6])
                                {
                                    case mess_x03_esp_version:
                                        modbus_reg_04[mod_reg_04_ESP_FW] = (data_array[index + 7] << 8) + data_array[index + 8];
                                    break;
                                }
                            }
                            break;

                            case WIFI_SETTINGS_WINDOW_SCENARIO:
                            {
                                LOG_INF_GLOB("Scenario answer - %d %d", WIFI_SETTINGS_WINDOW_SCENARIO, mess_x03_all_wifi_param_read);
                                uint8_t data_scenario[data_array[index + 3] - 2];
                                data_scenario[0] = 3;
                                data_scenario[1] = WIFI_SETTINGS_WINDOW_SCENARIO;
                                data_scenario[2] = data_array[index + 6];
                                memcpy(&data_scenario[3], &data_array[index + 7], data_array[index + 3] - 7);
                                uint16_t length = sizeof(data_scenario);
                                CRC_CALC(data_scenario, length - 2, &data_scenario[length - 2]);
                                int err_code = bt_send_work(data_scenario, length);
                                if(err_code) LOG_ERR_GLOB("Failed sending NUS message. Error %d ", err_code);
                            }
                            break;

                            case ETHERNET_SETTINGS_WINDOW_SCENARIO:
                            {
                                LOG_INF_GLOB("Scenario answer - %d %d", ETHERNET_SETTINGS_WINDOW_SCENARIO, data_array[6]);
                                uint8_t data_scenario[data_array[index + 3] - 2];
                                data_scenario[0] = 3;
                                data_scenario[1] = ETHERNET_SETTINGS_WINDOW_SCENARIO;
                                data_scenario[2] = data_array[index + 6];
                                memcpy(&data_scenario[3], &data_array[index + 7], data_array[index + 3] - 7);
                                uint16_t length = sizeof(data_scenario);
                                CRC_CALC(data_scenario, length - 2, &data_scenario[length - 2]);
                                int err_code = bt_send_work(data_scenario, length);
                                if(err_code) LOG_ERR_GLOB("Failed sending NUS message. Error %d ", err_code);
                            }
                            break;
                        }
                    break;

                    case IEC_104_WINDOW_SCENARIO:
                    {
                        uint8_t data_transite[data_array[index + 5]];
                        memcpy(data_transite, &data_array[index + 6], data_array[index + 5]);
                        IEC_104_parsing(data_transite);
                    }
                    break;
                }
                index += data_array[index + 3];
            }
            else index++;
        }
        else index++;
    }
}

/**/

void uart_callback_event(uint8_t event, uint16_t *uart_index_global)
{
    rx_timeout(uart_index_global);
}

int16_t ANGLE(int16_t angle_1, int16_t angle_2)
{
    int16_t angle = 360.0*((angle_1 - angle_2) * 10.0/angle_1);
    return (angle > 1800) ? angle - 3600 : angle;
}

void SAVE_DIAGRAM()
{
    if(alarm == 255)
    switch (current_frame)
    {
        case 0:
            alarm = 1;
        break;
        case 1:
            alarm = 2;
        break; 
        case 2:
            alarm = 0;
        break; 
    }
}

void DO_TOGGLE(uint8_t do_channel, uint8_t state)
{
    if(state == 1) 
    {
        if(!readBit(modbus_reg_04[mod_reg_04_DO_states], (do_channel - 1))) 
        {
            D0_CHANNEL_WRITE_0_to_9(do_channel - 1, 1);
            bitSet(modbus_reg_04[mod_reg_04_DO_states], (do_channel - 1));
        }
        else return;
    }
    else 
    { 
        if(readBit(modbus_reg_04[mod_reg_04_DO_states], (do_channel - 1))) 
        {
            D0_CHANNEL_WRITE_0_to_9(do_channel - 1, 0);
            bitClear(modbus_reg_04[mod_reg_04_DO_states], (do_channel - 1));
        }
        else return;
    }
    EVENT_SEND(EVENT_DISCRET_OUT_SCENARIO, (do_channel - 1), state, NULL);
    gpio_change = 1;
}

void RESET_MTZ_timers_and_trips()
{
    for(uint8_t z = 0; z < 6; z++)
        for(uint8_t x = 0; x < 3; x++)
        {
            MTZ_timer_trip[z][x] = max_32_bit_val;
            MTZ_timer_back[z][x] = max_32_bit_val;
            MTZ_trip[z][x] = 0;
            MTZ_back[z][x] = 0;
        }
}

void INIT_ptr()
{
    uint8_t ind = 0;
    if(readBit(modbus_reg_03[mod_reg_03_voltage_trans_strategy], 0)) {channel_selection[4][ind] = 4; ch_zmn_final[0] = ind; ind++;}
    if(readBit(modbus_reg_03[mod_reg_03_voltage_trans_strategy], 1)) {channel_selection[4][ind] = 5; ch_zmn_final[1] = ind; ind++;}
    if(readBit(modbus_reg_03[mod_reg_03_voltage_trans_strategy], 2)) {channel_selection[4][ind] = 6; ch_zmn_final[2] = ind; ind++;}
    if(readBit(modbus_reg_03[mod_reg_03_voltage_trans_strategy], 3)) {channel_selection[4][ind] = 7; ch_zmn_final[3] = ind; ind++;}

    for(uint8_t ch = 4; ch < 8; ch++)
    {
        if(readBit(modbus_reg_03[mod_reg_03_voltage_trans_strategy], (ch - 4)))
        {
            if(readBit(current_system_mode, radio_on_zmn)) 
                part_MTZ_invert_ptr[ch - 4] = &part_MTZ_invert;
            else part_MTZ_invert_ptr[ch - 4] = NULL;

            if(readBit(modbus_reg_03[mod_reg_03_system_mode], radio_on_zmn))
                part_ZMN_await_ptr[ch - 4] = &part_ZMN_await;
            else part_ZMN_await_ptr[ch - 4] = NULL;
        }
        else
        {
            part_MTZ_invert_ptr[ch - 4] = NULL;
            part_ZMN_await_ptr[ch - 4] = NULL;
        }
        if(readBit(modbus_reg_03[mod_reg_03_system_mode], radio_zvn)) 
        {
            if(readBit(modbus_reg_03[mod_reg_03_voltage_trans_strategy], ch))
            {
                part_ZVN_CONTROL_ptr[ch - 4] = &part_ZVN_CONTROL;
            }
            else part_ZVN_CONTROL_ptr[ch - 4] = NULL;
        }
        else 
        {
            part_ZVN_CONTROL_ptr[ch - 4] = NULL;
            zvn_alarm = 0;
        }
    }
}

void MTZ_stop_procedure()
{
    current_system_mode = modbus_reg_03[mod_reg_03_system_mode] & 0xFFC0;
    INIT_ptr();
    RESET_MTZ_timers_and_trips();
    highspeed_timer_pause();
}

void MTZ_resume_procedure()
{
    current_system_mode = modbus_reg_03[mod_reg_03_system_mode];
    highspeed_timer_resume();
}

void INIT_VALUES()
{
    modbus_reg_04[mod_reg_04_calibration_alarm] = modbus_reg_03[mod_reg_03_calibration_done];
    modbus_reg_04[mod_reg_04_NRF_FW] = NRF_FW_VERSION;

    if(readBit(modbus_reg_03[mod_reg_03_system_mode], radio_group) == 1)
        memcpy(&modbus_reg_04[mod_reg_04_MTZ_1_current_trip], &modbus_reg_03[mod_reg_03_A_MTZ_1_current_trip], 62);
    else
        memcpy(&modbus_reg_04[mod_reg_04_MTZ_1_current_trip], &modbus_reg_03[mod_reg_03_B_MTZ_1_current_trip], 62);

    RECALC_MTZ_timer_trip[0][0] = modbus_reg_04[mod_reg_04_MTZ_1_time_trip] * 1000;
    RECALC_MTZ_timer_trip[0][1] = modbus_reg_04[mod_reg_04_MTZ_1_time_trip] * 1000;
    RECALC_MTZ_timer_trip[0][2] = modbus_reg_04[mod_reg_04_MTZ_1_time_trip] * 1000;
    RECALC_MTZ_timer_trip[1][0] = modbus_reg_04[mod_reg_04_MTZ_2_time_trip] * 1000;
    RECALC_MTZ_timer_trip[1][1] = modbus_reg_04[mod_reg_04_MTZ_2_time_trip] * 1000;
    RECALC_MTZ_timer_trip[1][1] = modbus_reg_04[mod_reg_04_MTZ_2_time_trip] * 1000;
    RECALC_MTZ_timer_trip[2][0] = modbus_reg_04[mod_reg_04_MTZ_3_time_trip];
    RECALC_MTZ_timer_trip[2][1] = modbus_reg_04[mod_reg_04_MTZ_3_time_trip];
    RECALC_MTZ_timer_trip[2][2] = modbus_reg_04[mod_reg_04_MTZ_3_time_trip];
    RECALC_MTZ_timer_trip[3][0] = modbus_reg_04[mod_reg_04_TO_time_trip];
    RECALC_MTZ_timer_trip[3][1] = modbus_reg_04[mod_reg_04_TO_time_trip];
    RECALC_MTZ_timer_trip[3][2] = modbus_reg_04[mod_reg_04_TO_time_trip];
    RECALC_MTZ_timer_trip[4][0] = modbus_reg_04[mod_reg_04_ZMN_time_trip];
    RECALC_MTZ_timer_trip[4][1] = modbus_reg_04[mod_reg_04_ZMN_time_trip];
    RECALC_MTZ_timer_trip[4][2] = modbus_reg_04[mod_reg_04_ZMN_time_trip];
    RECALC_MTZ_timer_trip[5][0] = modbus_reg_04[mod_reg_04_ZZ_time_trip] * 1000;
    RECALC_MTZ_timer_trip[5][1] = modbus_reg_04[mod_reg_04_ZZ_time_trip] * 1000;
    RECALC_MTZ_timer_trip[5][2] = modbus_reg_04[mod_reg_04_ZZ_time_trip] * 1000;

    RECALC_MTZ_timer_back[0] = modbus_reg_04[mod_reg_04_MTZ_1_time_back] * 1000;
    RECALC_MTZ_timer_back[1] = modbus_reg_04[mod_reg_04_MTZ_2_time_back] * 1000;
    RECALC_MTZ_timer_back[2] = modbus_reg_04[mod_reg_04_MTZ_3_time_back];
    RECALC_MTZ_timer_back[3] = modbus_reg_04[mod_reg_04_TO_time_back];
    RECALC_MTZ_timer_back[4] = modbus_reg_04[mod_reg_04_ZMN_time_back];
    RECALC_MTZ_timer_back[5] = modbus_reg_04[mod_reg_04_ZZ_time_back] * 1000;


    RECALC_current_trip[0][0] = recalc_val(mod_reg_04_MTZ_1_current_trip, mod_reg_03_cal_adc_100_chan_0, 100, 
                                           mod_reg_03_cal_scale_coef_0,   mod_reg_03_cal_scale_100_chan_0);
    RECALC_current_trip[0][1] = recalc_val(mod_reg_04_MTZ_1_current_trip, mod_reg_03_cal_adc_100_chan_1, 100, 
                                           mod_reg_03_cal_scale_coef_1,   mod_reg_03_cal_scale_100_chan_1);
    RECALC_current_trip[0][2] = recalc_val(mod_reg_04_MTZ_1_current_trip, mod_reg_03_cal_adc_100_chan_2, 100, 
                                           mod_reg_03_cal_scale_coef_2,   mod_reg_03_cal_scale_100_chan_2);

    RECALC_current_trip[1][0] = recalc_val(mod_reg_04_MTZ_2_current_trip, mod_reg_03_cal_adc_100_chan_0, 100, 
                                           mod_reg_03_cal_scale_coef_0,   mod_reg_03_cal_scale_100_chan_0);
    RECALC_current_trip[1][1] = recalc_val(mod_reg_04_MTZ_2_current_trip, mod_reg_03_cal_adc_100_chan_1, 100, 
                                           mod_reg_03_cal_scale_coef_1,   mod_reg_03_cal_scale_100_chan_1);
    RECALC_current_trip[1][2] = recalc_val(mod_reg_04_MTZ_2_current_trip, mod_reg_03_cal_adc_100_chan_2, 100, 
                                           mod_reg_03_cal_scale_coef_2,   mod_reg_03_cal_scale_100_chan_2);

    RECALC_current_trip[2][0] = recalc_val(mod_reg_04_MTZ_3_current_trip, mod_reg_03_cal_adc_100_chan_0, 100, 
                                           mod_reg_03_cal_scale_coef_0,   mod_reg_03_cal_scale_100_chan_0);
    RECALC_current_trip[2][1] = recalc_val(mod_reg_04_MTZ_3_current_trip, mod_reg_03_cal_adc_100_chan_1, 100, 
                                           mod_reg_03_cal_scale_coef_1,   mod_reg_03_cal_scale_100_chan_1);
    RECALC_current_trip[2][2] = recalc_val(mod_reg_04_MTZ_3_current_trip, mod_reg_03_cal_adc_100_chan_2, 100, 
                                           mod_reg_03_cal_scale_coef_2,   mod_reg_03_cal_scale_100_chan_2);

    RECALC_current_trip[3][0] = recalc_val(mod_reg_04_TO_current_trip, mod_reg_03_cal_adc_100_chan_0, 100, 
                                           mod_reg_03_cal_scale_coef_0,   mod_reg_03_cal_scale_100_chan_0);
    RECALC_current_trip[3][1] = recalc_val(mod_reg_04_TO_current_trip, mod_reg_03_cal_adc_100_chan_1, 100, 
                                           mod_reg_03_cal_scale_coef_1,   mod_reg_03_cal_scale_100_chan_1);
    RECALC_current_trip[3][2] = recalc_val(mod_reg_04_TO_current_trip, mod_reg_03_cal_adc_100_chan_2, 100, 
                                           mod_reg_03_cal_scale_coef_2,   mod_reg_03_cal_scale_100_chan_2);

    RECALC_current_trip[4][0] = recalc_val(mod_reg_04_ZMN_voltage_trip, mod_reg_03_cal_adc_100_chan_4, 10, 
                                           mod_reg_03_cal_scale_coef_4,   mod_reg_03_cal_scale_100_chan_4);
    RECALC_current_trip[4][1] = recalc_val(mod_reg_04_ZMN_voltage_trip, mod_reg_03_cal_adc_100_chan_5, 10, 
                                           mod_reg_03_cal_scale_coef_5,   mod_reg_03_cal_scale_100_chan_5);
    RECALC_current_trip[4][2] = recalc_val(mod_reg_04_ZMN_voltage_trip, mod_reg_03_cal_adc_100_chan_6, 10, 
                                           mod_reg_03_cal_scale_coef_6,   mod_reg_03_cal_scale_100_chan_6);

    RECALC_current_trip[5][0] = recalc_val(mod_reg_04_ZZ_current_trip, mod_reg_03_cal_adc_100_chan_3, 10, 
                                           mod_reg_03_cal_scale_coef_3,   mod_reg_03_cal_scale_100_chan_3);

/*************************/

    RECALC_current_back[0][0] = recalc_val(mod_reg_04_MTZ_1_current_back, mod_reg_03_cal_adc_100_chan_0, 10, 
                                           mod_reg_03_cal_scale_coef_0,   mod_reg_03_cal_scale_100_chan_0);
    RECALC_current_back[0][1] = recalc_val(mod_reg_04_MTZ_1_current_back, mod_reg_03_cal_adc_100_chan_1, 10, 
                                           mod_reg_03_cal_scale_coef_1,   mod_reg_03_cal_scale_100_chan_1);
    RECALC_current_back[0][2] = recalc_val(mod_reg_04_MTZ_1_current_back, mod_reg_03_cal_adc_100_chan_2, 10, 
                                           mod_reg_03_cal_scale_coef_2,   mod_reg_03_cal_scale_100_chan_2);

    RECALC_current_back[1][0] = recalc_val(mod_reg_04_MTZ_2_current_back, mod_reg_03_cal_adc_100_chan_0, 10, 
                                           mod_reg_03_cal_scale_coef_0,   mod_reg_03_cal_scale_100_chan_0);
    RECALC_current_back[1][1] = recalc_val(mod_reg_04_MTZ_2_current_back, mod_reg_03_cal_adc_100_chan_1, 10, 
                                           mod_reg_03_cal_scale_coef_1,   mod_reg_03_cal_scale_100_chan_1);
    RECALC_current_back[1][2] = recalc_val(mod_reg_04_MTZ_2_current_back, mod_reg_03_cal_adc_100_chan_2, 10, 
                                           mod_reg_03_cal_scale_coef_2,   mod_reg_03_cal_scale_100_chan_2);

    RECALC_current_back[2][0] = recalc_val(mod_reg_04_MTZ_3_current_back, mod_reg_03_cal_adc_100_chan_0, 10, 
                                           mod_reg_03_cal_scale_coef_0,   mod_reg_03_cal_scale_100_chan_0);
    RECALC_current_back[2][1] = recalc_val(mod_reg_04_MTZ_3_current_back, mod_reg_03_cal_adc_100_chan_1, 10, 
                                           mod_reg_03_cal_scale_coef_1,   mod_reg_03_cal_scale_100_chan_1);
    RECALC_current_back[2][2] = recalc_val(mod_reg_04_MTZ_3_current_back, mod_reg_03_cal_adc_100_chan_2, 10, 
                                           mod_reg_03_cal_scale_coef_2,   mod_reg_03_cal_scale_100_chan_2);

    RECALC_current_back[3][0] = recalc_val(mod_reg_04_TO_current_back, mod_reg_03_cal_adc_100_chan_0, 10, 
                                           mod_reg_03_cal_scale_coef_0,   mod_reg_03_cal_scale_100_chan_0);
    RECALC_current_back[3][1] = recalc_val(mod_reg_04_TO_current_back, mod_reg_03_cal_adc_100_chan_1, 10, 
                                           mod_reg_03_cal_scale_coef_1,   mod_reg_03_cal_scale_100_chan_1);
    RECALC_current_back[3][2] = recalc_val(mod_reg_04_TO_current_back, mod_reg_03_cal_adc_100_chan_2, 10, 
                                           mod_reg_03_cal_scale_coef_2,   mod_reg_03_cal_scale_100_chan_2);

    RECALC_current_back[4][0] = recalc_val(mod_reg_04_ZMN_voltage_back, mod_reg_03_cal_adc_100_chan_4, 10, 
                                           mod_reg_03_cal_scale_coef_4,   mod_reg_03_cal_scale_100_chan_4);
    RECALC_current_back[4][1] = recalc_val(mod_reg_04_ZMN_voltage_back, mod_reg_03_cal_adc_100_chan_5, 10, 
                                           mod_reg_03_cal_scale_coef_5,   mod_reg_03_cal_scale_100_chan_5);
    RECALC_current_back[4][2] = recalc_val(mod_reg_04_ZMN_voltage_back, mod_reg_03_cal_adc_100_chan_6, 10, 
                                           mod_reg_03_cal_scale_coef_6,   mod_reg_03_cal_scale_100_chan_6);

    RECALC_current_back[5][0] = recalc_val(mod_reg_04_ZZ_current_back, mod_reg_03_cal_adc_100_chan_3, 10, 
                                           mod_reg_03_cal_scale_coef_3,   mod_reg_03_cal_scale_100_chan_3);

    cutt_off_meas_value[mod_reg_04_current_A]   = SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_0], zero_cutt_off_val, modbus_reg_03[mod_reg_03_cal_adc_100_chan_0]) *
                                                          modbus_reg_03[mod_reg_03_cal_scale_coef_0] / 100;
    cutt_off_meas_value[mod_reg_04_current_B]   = SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_1], zero_cutt_off_val, modbus_reg_03[mod_reg_03_cal_adc_100_chan_1]) * 
                                                          modbus_reg_03[mod_reg_03_cal_scale_coef_1] / 100;
    cutt_off_meas_value[mod_reg_04_current_C]   = SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_2], zero_cutt_off_val, modbus_reg_03[mod_reg_03_cal_adc_100_chan_2]) * 
                                                          modbus_reg_03[mod_reg_03_cal_scale_coef_2] / 100;
    cutt_off_meas_value[mod_reg_04_current_3I0] = SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_3], zero_cutt_off_val, modbus_reg_03[mod_reg_03_cal_adc_100_chan_3]) * 
                                                          modbus_reg_03[mod_reg_03_cal_scale_coef_3] / 10;
    cutt_off_meas_value[mod_reg_04_voltage_A]   = SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_4], zero_cutt_off_val, modbus_reg_03[mod_reg_03_cal_adc_100_chan_4]) * 
                                                          modbus_reg_03[mod_reg_03_cal_scale_coef_4] / 10;
    cutt_off_meas_value[mod_reg_04_voltage_B]   = SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_5], zero_cutt_off_val, modbus_reg_03[mod_reg_03_cal_adc_100_chan_5]) * 
                                                          modbus_reg_03[mod_reg_03_cal_scale_coef_5] / 10;
    cutt_off_meas_value[mod_reg_04_voltage_C]   = SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_6], zero_cutt_off_val, modbus_reg_03[mod_reg_03_cal_adc_100_chan_6]) * 
                                                          modbus_reg_03[mod_reg_03_cal_scale_coef_6] / 10;
    cutt_off_meas_value[mod_reg_04_voltage_x]   = SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_7], zero_cutt_off_val, modbus_reg_03[mod_reg_03_cal_adc_100_chan_7]) * 
                                                          modbus_reg_03[mod_reg_03_cal_scale_coef_7] / 10;

    for(uint8_t i = 0; i < 3; i++)
    {
        MTZ_1T_depend_coef_k[i] = (RECALC_MTZ_timer_trip[2][i]  - RECALC_MTZ_timer_trip[1][i]) * 10000 /
                                  (RECALC_current_trip[2][i] - RECALC_current_trip[1][i]);
        MTZ_1T_depend_coef_b[i] = (RECALC_current_trip[2][i] * RECALC_MTZ_timer_trip[1][i] - RECALC_current_trip[1][i] * RECALC_MTZ_timer_trip[2][i]) / 
                                  (RECALC_current_trip[2][i] - RECALC_current_trip[1][i]);
        last_current_trip[i] = 0;
    }
}

void SWITCHER_ON_timeout()
{
    LOG_INF_GLOB("SWITCHER_ON_timeout"); 
    DO_TOGGLE(DO_COIL_START, 0);
    SWITCHER_ON_timeout_timer_flag = 2;
}

K_TIMER_DEFINE(SWITCHER_ON_timeout_timer, SWITCHER_ON_timeout, NULL);

void SWITCHER_OFF_timeout()
{
    LOG_INF_GLOB("SWITCHER_OFF_timeout"); 
    DO_TOGGLE(DO_COIL_STOP, 0);
    SWITCHER_OFF_timeout_timer_flag = 2;
}

K_TIMER_DEFINE(SWITCHER_OFF_timeout_timer, SWITCHER_OFF_timeout, NULL);

void SWITCHER_ON_feedback()
{
    switch_di_timer = switch_di_timer + switch_di_tic;
    if(readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_GENERAL_ON - 1)) || (switch_di_timer >= modbus_reg_03[mod_reg_03_switch_toggle_time]))
    {
        k_timer_stop(&SWITCHER_ON_feedback_timer);
        SWITCHER_ON_timeout_timer_flag = 2;
        k_timer_stop(&SWITCHER_ON_timeout_timer);
        DO_TOGGLE(DO_COIL_START, 0); 
        switch_di_timer = 0;
    }
}

K_TIMER_DEFINE(SWITCHER_ON_feedback_timer, SWITCHER_ON_feedback, NULL);

void SWITCHER_OFF_feedback()
{
    switch_di_timer = switch_di_timer + switch_di_tic;
    if(readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_GENERAL_OFF - 1)) || (switch_di_timer >= modbus_reg_03[mod_reg_03_switch_toggle_time]))
    {
        k_timer_stop(&SWITCHER_OFF_feedback_timer);
        SWITCHER_OFF_timeout_timer_flag = 2;
        k_timer_stop(&SWITCHER_OFF_timeout_timer);
        DO_TOGGLE(DO_COIL_STOP, 0); 
        switch_di_timer = 0;
    }
}

K_TIMER_DEFINE(SWITCHER_OFF_feedback_timer, SWITCHER_OFF_feedback, NULL);

void SWITCHER_action(uint8_t sw_act)
{
    switch(modbus_reg_03[mod_reg_03_switch_strategy])
    {
        case 0:
            DO_TOGGLE(DO_COIL_START, sw_act);
        break;

        case 2:
            k_timer_stop(&SWITCHER_ON_feedback_timer);
            k_timer_stop(&SWITCHER_OFF_feedback_timer);
            switch_di_timer = 0;
            if(sw_act) k_timer_start(&SWITCHER_ON_feedback_timer, K_MSEC(switch_di_tic), K_MSEC(switch_di_tic));
            else k_timer_start(&SWITCHER_OFF_feedback_timer, K_MSEC(switch_di_tic), K_MSEC(switch_di_tic));

        case 1:
            if(sw_act)
            {
                DO_TOGGLE(DO_COIL_START, 1); 
                if(SWITCHER_OFF_timeout_timer_flag == 1)
                {
                    LOG_INF_GLOB("timer off collapse");
                    SWITCHER_OFF_timeout_timer_flag = 0;
                    k_timer_stop(&SWITCHER_OFF_timeout_timer);
                    DO_TOGGLE(DO_COIL_STOP, 0);
                }
                SWITCHER_ON_timeout_timer_flag = 1;
                k_timer_start(&SWITCHER_ON_timeout_timer, K_MSEC(modbus_reg_03[mod_reg_03_switch_toggle_time]), K_NO_WAIT);
            }
            else
            {
                MTZ_stop_procedure();
                DO_TOGGLE(DO_COIL_STOP, 1);
                if(SWITCHER_ON_timeout_timer_flag == 1)
                {
                    LOG_INF_GLOB("timer on collapse");
                    SWITCHER_ON_timeout_timer_flag = 0;
                    k_timer_stop(&SWITCHER_ON_timeout_timer);
                    DO_TOGGLE(DO_COIL_START, 0);
                }
                SWITCHER_OFF_timeout_timer_flag = 1;
                k_timer_start(&SWITCHER_OFF_timeout_timer, K_MSEC(modbus_reg_03[mod_reg_03_switch_toggle_time]), K_NO_WAIT);
            }
        break;
    }
}

void STOP_ALARM()
{
    SWITCHER_action(0);
    bitClear(modbus_reg_03[mod_reg_03_system_mode], run_mode_enable);
    bitSet(  modbus_reg_03[mod_reg_03_system_mode], esd_mode_enable);
    MTZ_stop_procedure();
    main_window_updater = 2;
    LOG_INF_GLOB("STOP_ALARM");
    //SAVE_DIAGRAM();
}

const int16_t divid[8] = {100,100,100,10,10,10,10,10};

void part_MTZ_depended(int control_val, uint8_t pr, uint8_t ch, uint8_t num)
{
    if(control_val >= RECALC_current_trip[pr][num])
    {
        if(control_val > last_current_trip[num])
        {
            last_current_trip[num] = control_val;
            RECALC_MTZ_timer_trip[1][num] = MTZ_1T_depend_coef_k[num] * control_val / 10000 + MTZ_1T_depend_coef_b[num];
            if(RECALC_MTZ_timer_trip[1][num] < modbus_reg_04[mod_reg_04_MTZ_3_time_trip])
                RECALC_MTZ_timer_trip[1][num] = modbus_reg_04[mod_reg_04_MTZ_3_time_trip];
        }
        if(MTZ_trip[pr][num] == 0)
        {
            if(MTZ_timer_trip[pr][num] == max_32_bit_val) 
            {
                MTZ_timer_trip[pr][num] = 0; 
                uint8_t protection[] = {pr, protection_alarm};
                EVENT_SEND(EVENT_X04_SCENARIO, 
                           mod_reg_04_current_A + ch, 
                           SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_0 + ch], 
                                        control_val,   
                                        modbus_reg_03[mod_reg_03_cal_adc_100_chan_0 + ch]) * 
                                        modbus_reg_03[mod_reg_03_cal_scale_coef_0 + ch] / divid[ch],
                           protection);
            }
            MTZ_timer_back[pr][num] = max_32_bit_val;
            MTZ_trip[pr][num] = 1;
            MTZ_back[pr][num] = 0;
            return;
        }
    }

    if(control_val < RECALC_current_back[pr][num] && MTZ_trip[pr][num] > 0)
    {
        if(MTZ_trip[pr][num] == 1)
        {/*
            uint8_t protection[] = {pr, protection_back_value};
            EVENT_SEND(EVENT_X04_SCENARIO, 
                       mod_reg_04_current_A + ch, 
                       SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_0 + ch], 
                                    control_val,   
                                    modbus_reg_03[mod_reg_03_cal_adc_100_chan_0 + ch]) * 
                                    modbus_reg_03[mod_reg_03_cal_scale_coef_0 + ch] / divid[ch], 
                       protection);*/
            if(MTZ_timer_back[pr][num] == max_32_bit_val) MTZ_timer_back[pr][num] = 0; 
        }
        else if(MTZ_trip[pr][num] == 2) MTZ_timer_trip[pr][num] = max_32_bit_val;
        MTZ_trip[pr][num] = 0;
    }
}

void part_MTZ_direct(int control_val, uint8_t pr, uint8_t ch, uint8_t num)
{
    if(control_val >= RECALC_current_trip[pr][num] && MTZ_trip[pr][num] == 0)
    {
        if(MTZ_timer_trip[pr][num] == max_32_bit_val) 
        {
            MTZ_timer_trip[pr][num] = 0; 
            uint8_t protection[] = {pr, protection_alarm};
            EVENT_SEND(EVENT_X04_SCENARIO, 
                       mod_reg_04_current_A + ch, 
                       SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_0 + ch], 
                                    control_val,   
                                    modbus_reg_03[mod_reg_03_cal_adc_100_chan_0 + ch]) * 
                                    modbus_reg_03[mod_reg_03_cal_scale_coef_0 + ch] / divid[ch],
                       protection);
        }
        MTZ_timer_back[pr][num] = max_32_bit_val;
        MTZ_trip[pr][num] = 1;
        MTZ_back[pr][num] = 0;
    }
    else 
    if(control_val < RECALC_current_back[pr][num] && MTZ_trip[pr][num] > 0)
    {
        if(MTZ_trip[pr][num] == 1)
        {/*
            uint8_t protection[] = {pr, protection_back_value};
            EVENT_SEND(EVENT_X04_SCENARIO, 
                       mod_reg_04_current_A + ch, 
                       SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_0 + ch], 
                                    control_val,   
                                    modbus_reg_03[mod_reg_03_cal_adc_100_chan_0 + ch]) * 
                                    modbus_reg_03[mod_reg_03_cal_scale_coef_0 + ch] / divid[ch], 
                       protection);*/
            if(MTZ_timer_back[pr][num] == max_32_bit_val) MTZ_timer_back[pr][num] = 0; 
        }
        else if(MTZ_trip[pr][num] == 2) MTZ_timer_trip[pr][num] = max_32_bit_val;
        MTZ_trip[pr][num] = 0;
    }
}

void part_MTZ_invert(int control_val, uint8_t pr, uint8_t ch, uint8_t num)
{
    if(control_val <= RECALC_current_trip[pr][num] && MTZ_trip[pr][num] == 0)
    {
        if(MTZ_timer_trip[pr][num] == max_32_bit_val) 
        {
            MTZ_timer_trip[pr][num] = 0; 
            uint8_t protection[] = {pr, protection_alarm};
            EVENT_SEND(EVENT_X04_SCENARIO, 
                       mod_reg_04_current_A + ch, 
                       SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_0 + ch], 
                                    control_val,   
                                    modbus_reg_03[mod_reg_03_cal_adc_100_chan_0 + ch]) * 
                                    modbus_reg_03[mod_reg_03_cal_scale_coef_0 + ch] / divid[ch],
                       protection);
        }
        MTZ_timer_back[pr][num] = max_32_bit_val;
        MTZ_trip[pr][num] = 1;
        MTZ_back[pr][num] = 0;
    }
    else 
    if(control_val > RECALC_current_back[pr][num] && MTZ_trip[pr][num] > 0)
    {
        if(MTZ_trip[pr][num] == 1)
        {/*
            uint8_t protection[] = {pr, protection_back_value};
            EVENT_SEND(EVENT_X04_SCENARIO,  
                       mod_reg_04_current_A + ch, 
                       SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_0 + ch], 
                                    control_val,   
                                    modbus_reg_03[mod_reg_03_cal_adc_100_chan_0 + ch]) * 
                                    modbus_reg_03[mod_reg_03_cal_scale_coef_0 + ch] / divid[ch],
                                    protection);*/
            if(MTZ_timer_back[pr][num] == max_32_bit_val) MTZ_timer_back[pr][num] = 0; 
        }
        else if(MTZ_trip[pr][num] == 2) MTZ_timer_trip[pr][num] = max_32_bit_val;
        MTZ_trip[pr][num] = 0;
    }
}

void part_ZMN_await(int control_val, uint8_t num)
{
    if(control_val <= RECALC_current_trip[radio_on_zmn][num] || !apv_current_channel) 
    {
        apv_current_channel = 0;
    }
    if(zmn_current_channel == 3)
    {
        zmn_current_channel = 0;
        if(apv_current_channel) apv_zmn_trip = 0;
        else apv_zmn_trip = 1;
        apv_current_channel = 1;
    }
    else zmn_current_channel++;
}

void part_ZVN_CONTROL(int control_val)
{
    if(control_val >= zero_cutt_off_val) zvn_alarm = 1;
    else zvn_alarm = 0;
}

void MTZ(uint32_t control_val, uint8_t ch)
{
    switch(ch)
    {
        /*MTZ_1_2_3 + TO*/
        case 0:
        case 1:
        case 2:
            if(readBit(current_system_mode, radio_on_mtz1)) part_MTZ_direct(control_val, radio_on_mtz1, ch, ch);
            if(readBit(current_system_mode, radio_on_mtz2)) part_MTZ_depended(control_val, radio_on_mtz2, ch, ch);
            if(readBit(current_system_mode, radio_on_mtz3)) part_MTZ_direct(control_val, radio_on_mtz3, ch, ch);
            if(readBit(current_system_mode, radio_on_to))   part_MTZ_direct(control_val, radio_on_to,   ch, ch);
        break;

        /*ZZZ*/
        case 3:
            if(readBit(current_system_mode, radio_on_zz)) part_MTZ_direct(control_val, radio_on_zz, ch, 0);
        break;

        /*ZMN ZVN*/
        case 4: 
            if(part_MTZ_invert_ptr[0]   != NULL) (*part_MTZ_invert_ptr[0])(control_val, radio_on_zmn, ch, ch_zmn_final[0]);
            if(part_ZMN_await_ptr[0]    != NULL) (*part_ZMN_await_ptr[0])(control_val, ch_zmn_final[0]);
            if(part_ZVN_CONTROL_ptr[0]  != NULL) (*part_ZVN_CONTROL_ptr[0])(control_val);
        break;
        case 5:
            if(part_MTZ_invert_ptr[1]   != NULL) (*part_MTZ_invert_ptr[1])(control_val, radio_on_zmn, ch, ch_zmn_final[1]);
            if(part_ZMN_await_ptr[1]    != NULL) (*part_ZMN_await_ptr[1])(control_val, ch_zmn_final[1]);
            if(part_ZVN_CONTROL_ptr[1]  != NULL) (*part_ZVN_CONTROL_ptr[1])(control_val);
        break;
        case 6:
            if(part_MTZ_invert_ptr[2]   != NULL) (*part_MTZ_invert_ptr[2])(control_val, radio_on_zmn, ch, ch_zmn_final[2]);
            if(part_ZMN_await_ptr[2]    != NULL) (*part_ZMN_await_ptr[2])(control_val, ch_zmn_final[2]);
            if(part_ZVN_CONTROL_ptr[2]  != NULL) (*part_ZVN_CONTROL_ptr[2])(control_val);
        break;
        case 7:
            if(part_MTZ_invert_ptr[3]   != NULL) (*part_MTZ_invert_ptr[3])(control_val, radio_on_zmn, ch, ch_zmn_final[3]);
            if(part_ZMN_await_ptr[3]    != NULL) (*part_ZMN_await_ptr[3])(control_val, ch_zmn_final[3]);
            if(part_ZVN_CONTROL_ptr[3]  != NULL) (*part_ZVN_CONTROL_ptr[3])(control_val);
        break;
    }  
}

/*MEASURE RUTINE*/

void CURRENT_PHASE_EVENT(int16_t ADC_val, uint8_t channel)
{
    if(ADC_val > max_val[channel]) max_val[channel] = ADC_val;
    if(ADC_val < min_val[channel]) min_val[channel] = ADC_val;

    half_phase[channel]++;
    full_phase[channel]++;

    RMS[channel] += ADC_val;

    if((half_phase[channel] > period_counts_1 && ADC_val > modbus_reg_03[mod_reg_03_cal_adc_0_chan_0 + channel] && direct[channel] == 0) || (half_phase[channel] > period_counts_2 && direct[channel] == 0))
    {
        direct[channel] = 1; 
        ABOVE_ZERO_RMS[channel] = abs(RMS[channel] / half_phase[channel] - modbus_reg_03[mod_reg_03_cal_adc_0_chan_0 + channel]);
        CURRENT[channel] = (ABOVE_ZERO_RMS[channel] + SUB_ZERO_RMS[channel]) / 2; 

        CURRENT_median[channel] += CURRENT[channel];
        zero_val[channel] += (max_val[channel] + min_val[channel]) / 2;

        zero_val_counter[channel]++;

        if(zero_val_counter[channel] == max_zero_val_counter)
        {
            zero_val_final[channel] = zero_val[channel] / zero_val_counter[channel];
            zero_val[channel] = 0;
            CURRENT_median_result[channel] = CURRENT_median[channel] / zero_val_counter[channel];
            CURRENT_median[channel] = 0;
            zero_val_counter[channel] = 0;
        }
        MTZ(ABOVE_ZERO_RMS[channel], channel);
        if(channel < 3) 
        {
            switch (channel)
            {
                case 0: 
                    modbus_reg_04[mod_reg_04_angle_currentA_voltageA] = ANGLE(full_phase[channel],full_phase[channel + 4]);
                break;

                case 1: 
                    modbus_reg_04[mod_reg_04_angle_currentB_voltageB] = ANGLE(full_phase[channel],full_phase[channel + 4]);
                break;

                case 2: 
                    modbus_reg_04[mod_reg_04_angle_currentC_voltageC] = ANGLE(full_phase[channel],full_phase[channel + 4]);
                break;
                    
            }
        }
        max_val[channel] = 0; 
        min_val[channel] = 32767; 
        half_phase[channel] = 0; 
        RMS[channel] = 0; 
        full_phase[channel] = 0;
    }
    if((half_phase[channel] > period_counts_1 && ADC_val <= modbus_reg_03[mod_reg_03_cal_adc_0_chan_0 + channel] && direct[channel] == 1) || (half_phase[channel] > period_counts_2 && direct[channel] == 1)) 
    {
        direct[channel] = 0;
        SUB_ZERO_RMS[channel] = abs(modbus_reg_03[mod_reg_03_cal_adc_0_chan_0 + channel] - RMS[channel] / half_phase[channel]);

        MTZ(SUB_ZERO_RMS[channel], channel);
        RMS[channel] = 0;
        half_phase[channel] = 0; 
    }
}

static void MTZ_handler()
{           
    for(uint8_t z = 0; z < 6; z++)
    {
        for(uint8_t x = 0; x < 3; x++)
        {
            if(MTZ_timer_back[z][x] == RECALC_MTZ_timer_back[z]) 
            {
                if(MTZ_trip[z][x] == 0) 
                {
                    MTZ_back[z][x] = 1;
                    if(z == 1) 
                    {
                        last_current_trip[x] = 0;
                        RECALC_MTZ_timer_trip[z][x] = modbus_reg_04[mod_reg_04_MTZ_2_time_trip] * 1000;
                    }
                    uint8_t protection[] = {z, protection_back_timer};
                    EVENT_SEND(EVENT_X04_SCENARIO, 
                               mod_reg_04_current_A + channel_selection[z][x], 
                               SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_0 + channel_selection[z][x]], 
                                            CURRENT[channel_selection[z][x]],   
                                            modbus_reg_03[mod_reg_03_cal_adc_100_chan_0 + channel_selection[z][x]]) * 
                                            modbus_reg_03[mod_reg_03_cal_scale_coef_0 + channel_selection[z][x]] / divid[channel_selection[z][x]],
                               protection);
                    MTZ_timer_trip[z][x] = max_32_bit_val;
                }
                MTZ_timer_back[z][x] = max_32_bit_val;
            }
            if(MTZ_timer_trip[z][x] >= RECALC_MTZ_timer_trip[z][x] && MTZ_timer_trip[z][x] != max_32_bit_val) 
            {
                if((MTZ_trip[z][x] == 1 || MTZ_back[z][x] == 0) && !readBit(modbus_reg_04[mod_reg_04_alarm_equipment], (z * 3 + x)))
                {
                    LOG_ERR_GLOB("TRIP");

                    if(z == 1) 
                    {
                        last_current_trip[x] = 0;
                        RECALC_MTZ_timer_trip[z][x] = modbus_reg_04[mod_reg_04_MTZ_2_time_trip] * 1000;
                    }

                    if(readBit(modbus_reg_04[mod_reg_04_trip_action], z)) 
                    {
                        Global_trip = z;
                        STOP_ALARM();
                    }
                    else 
                    {
                        writebit(modbus_reg_04[mod_reg_04_alarm_equipment], z * 3 + x, 1);
                        main_window_updater = 2;
                        if(MTZ_back[z][x] == 1) 
                        {
                            MTZ_timer_back[z][x] = max_32_bit_val;
                            MTZ_trip[z][x] = 0;
                        }
                    }

                    uint8_t protection[] = {z, protection_trip};
                    EVENT_SEND(EVENT_X04_SCENARIO, 
                               mod_reg_04_current_A + channel_selection[z][x], 
                               SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_0 + channel_selection[z][x]], 
                                            CURRENT[channel_selection[z][x]],   
                                            modbus_reg_03[mod_reg_03_cal_adc_100_chan_0 + channel_selection[z][x]]) * 
                                            modbus_reg_03[mod_reg_03_cal_scale_coef_0 + channel_selection[z][x]] / divid[channel_selection[z][x]],
                               protection);
                }
            }
            if(MTZ_timer_trip[z][x] < RECALC_MTZ_timer_trip[z][x] && MTZ_trip[z][x]) 
            {
                MTZ_timer_trip[z][x]++; 
            }
            if(MTZ_timer_back[z][x] < RECALC_MTZ_timer_back[z]) 
            {
                MTZ_timer_back[z][x]++;
            }
            if(z == 5) break;
        }
    }
}

void TAG_read_1()
{
    modbus_reg_04[mod_reg_04_current_A]   = SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_0], CURRENT_median_result[mod_reg_04_current_A],   modbus_reg_03[mod_reg_03_cal_adc_100_chan_0]);
    modbus_reg_04[mod_reg_04_current_B]   = SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_1], CURRENT_median_result[mod_reg_04_current_B],   modbus_reg_03[mod_reg_03_cal_adc_100_chan_1]);
    modbus_reg_04[mod_reg_04_current_C]   = SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_2], CURRENT_median_result[mod_reg_04_current_C],   modbus_reg_03[mod_reg_03_cal_adc_100_chan_2]);
    modbus_reg_04[mod_reg_04_current_3I0] = SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_3], CURRENT_median_result[mod_reg_04_current_3I0], modbus_reg_03[mod_reg_03_cal_adc_100_chan_3]);
    modbus_reg_04[mod_reg_04_voltage_A]   = SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_4], CURRENT_median_result[mod_reg_04_voltage_A],   modbus_reg_03[mod_reg_03_cal_adc_100_chan_4]);
    modbus_reg_04[mod_reg_04_voltage_B]   = SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_5], CURRENT_median_result[mod_reg_04_voltage_B],   modbus_reg_03[mod_reg_03_cal_adc_100_chan_5]);
    modbus_reg_04[mod_reg_04_voltage_C]   = SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_6], CURRENT_median_result[mod_reg_04_voltage_C],   modbus_reg_03[mod_reg_03_cal_adc_100_chan_6]);
    modbus_reg_04[mod_reg_04_voltage_x]   = SCALE_RECALC(modbus_reg_03[mod_reg_03_cal_scale_100_chan_7], CURRENT_median_result[mod_reg_04_voltage_x],   modbus_reg_03[mod_reg_03_cal_adc_100_chan_7]);
}

K_TIMER_DEFINE(TAG_read_timer_1, TAG_read_1, NULL);

void TAG_read_2()
{
    modbus_reg_04[mod_reg_04_current_A_primary]   = modbus_reg_04[mod_reg_04_current_A]   * modbus_reg_03[mod_reg_03_cal_scale_coef_0] / 100;
    modbus_reg_04[mod_reg_04_current_B_primary]   = modbus_reg_04[mod_reg_04_current_B]   * modbus_reg_03[mod_reg_03_cal_scale_coef_1] / 100;
    modbus_reg_04[mod_reg_04_current_C_primary]   = modbus_reg_04[mod_reg_04_current_C]   * modbus_reg_03[mod_reg_03_cal_scale_coef_2] / 100;
    modbus_reg_04[mod_reg_04_current_3I0_primary] = modbus_reg_04[mod_reg_04_current_3I0] * modbus_reg_03[mod_reg_03_cal_scale_coef_3] / 10;
    modbus_reg_04[mod_reg_04_voltage_A_primary]   = modbus_reg_04[mod_reg_04_voltage_A]   * modbus_reg_03[mod_reg_03_cal_scale_coef_4] / 10;
    modbus_reg_04[mod_reg_04_voltage_B_primary]   = modbus_reg_04[mod_reg_04_voltage_B]   * modbus_reg_03[mod_reg_03_cal_scale_coef_5] / 10;
    modbus_reg_04[mod_reg_04_voltage_C_primary]   = modbus_reg_04[mod_reg_04_voltage_C]   * modbus_reg_03[mod_reg_03_cal_scale_coef_6] / 10;
    modbus_reg_04[mod_reg_04_voltage_x_primary]   = modbus_reg_04[mod_reg_04_voltage_x]   * modbus_reg_03[mod_reg_03_cal_scale_coef_7] / 10;

    if(modbus_reg_04[mod_reg_04_current_A_primary]   < cutt_off_meas_value[mod_reg_04_current_A])   modbus_reg_04[mod_reg_04_current_A_primary] = 0;
    if(modbus_reg_04[mod_reg_04_current_B_primary]   < cutt_off_meas_value[mod_reg_04_current_B])   modbus_reg_04[mod_reg_04_current_B_primary] = 0;
    if(modbus_reg_04[mod_reg_04_current_C_primary]   < cutt_off_meas_value[mod_reg_04_current_C])   modbus_reg_04[mod_reg_04_current_C_primary] = 0;
    if(modbus_reg_04[mod_reg_04_current_3I0_primary] < cutt_off_meas_value[mod_reg_04_current_3I0]) modbus_reg_04[mod_reg_04_current_3I0_primary] = 0;
    if(modbus_reg_04[mod_reg_04_voltage_A_primary]   < cutt_off_meas_value[mod_reg_04_voltage_A])   modbus_reg_04[mod_reg_04_voltage_A_primary] = 0;
    if(modbus_reg_04[mod_reg_04_voltage_B_primary]   < cutt_off_meas_value[mod_reg_04_voltage_B])   modbus_reg_04[mod_reg_04_voltage_B_primary] = 0;
    if(modbus_reg_04[mod_reg_04_voltage_C_primary]   < cutt_off_meas_value[mod_reg_04_voltage_C])   modbus_reg_04[mod_reg_04_voltage_C_primary] = 0;
    if(modbus_reg_04[mod_reg_04_voltage_x_primary]   < cutt_off_meas_value[mod_reg_04_voltage_x])   modbus_reg_04[mod_reg_04_voltage_x_primary] = 0;
}

K_TIMER_DEFINE(TAG_read_timer_2, TAG_read_2, NULL);

void HW_RESER_EVENT()
{
    NVIC_SystemReset();
}

K_TIMER_DEFINE(HW_RESER_EVENT_timer, HW_RESER_EVENT, NULL);

static void MODBUS_PROCESS(uint8_t *data_in, uint16_t length)
{
        //NRF_LOG_INF_GLOBO("MODBUS_PROCESS");
        scenario = data_in[1];
        switch (scenario)
        {
                case WIFI_SETTINGS_WINDOW_SCENARIO:
                    switch(data_in[2])
                    {
                        case 1:
                            wifi_window_updater = 1;
                        break;   
                        case 2:
                            wifi_window_updater = 2;
                        break;   
                        case 3:
                            ESP_RESET();
                        break; 
                    }  
                    //LOG_INF_GLOB("WIFI_SETTINGS_WINDOW_SCENARIO");
                break;

                case ETHERNET_SETTINGS_WINDOW_SCENARIO:
                    switch(data_in[2])
                    {
                        case 1:
                            ethernet_window_updater = 1;
                        break;   
                        case 2:
                            ethernet_window_updater = 2;
                        break;   
                    }  
                    //LOG_INF_GLOB("ETHERNET_SETTINGS_WINDOW_SCENARIO");
                break;

                case MTZ_SETTINGS_WINDOW_SCENARIO:
                    switch(data_in[2])
                    {
                        case 1:
                            mtz_window_updater = 1;
                        break;                      
                        case 2:
                            mtz_window_updater = 1;
                            modbus_reg_03[data_in[3]] = BytesToInt(data_in, 4);
                            INIT_VALUES();
                            eeprom_write = 1;
                        break;
                        case 3:
                            modbus_reg_03[data_in[3]] = BytesToInt(data_in, 4);
                            mtz_window_updater = 1;
                            INIT_VALUES();
                            eeprom_write = 1;
                        break;
                    }
                    //LOG_INF_GLOB("MTZ_SETTINGS_WINDOW_SCENARIO");
                break;

                case SYSTEM_MODE_WINDOW_SCENARIO:
                    switch(data_in[2])
                    {
                        case 1:
                            mode_window_updater = 1;
                        break;
                        case 2:
                            modbus_reg_03[mod_reg_03_system_mode] = BytesToInt(data_in, 3);
                            INIT_VALUES();
                            INIT_STATUS();
                            General_State = LOGIC_init_position;
                            mode_window_updater = 1;
                            eeprom_write = 1;
                        break;
                        case 3:
                            EVENT_SEND(EVENT_SYSTEM_SCENARIO, mess_x05_event_UPDATE, 0, NULL);
                        break;
                        case 4:
                            modbus_reg_04[mod_reg_04_RTC_year]   = BytesToInt(data_in, 3);     
                            modbus_reg_04[mod_reg_04_RTC_month]  = data_in[5];            
                            modbus_reg_04[mod_reg_04_RTC_date]   = data_in[6];            
                            modbus_reg_04[mod_reg_04_RTC_day]    = data_in[7];          
                            modbus_reg_04[mod_reg_04_RTC_hour]   = data_in[8];     
                            modbus_reg_04[mod_reg_04_RTC_minute] = data_in[9];     
                            modbus_reg_04[mod_reg_04_RTC_second] = data_in[10];
                            EVENT_SEND(EVENT_SYSTEM_SCENARIO, mess_x05_event_TAKE_MY_TIME, 0, NULL);
                            mode_window_updater = 1;
                        break;
                        case 5:
                            if(data_in[3] == 5 && data_in[4] == 5) 
                            {
                                modbus_reg_03[mod_reg_03_crc] = 12345; 
                                eeprom_write = 1;
                                k_timer_start(&HW_RESER_EVENT_timer, K_MSEC(2000), K_NO_WAIT);
                            }
                        break;
                        case 6:
                            if(data_in[3] == 6 && data_in[4] == 6) 
                            {
                                EVENT_SEND(EVENT_SYSTEM_SCENARIO, mess_x05_event_ESP_HW_RESET, 0, NULL);
                            }
                        break;
                        case 7:
                            modbus_reg_03[mod_reg_03_voltage_trans_strategy] = BytesToInt(data_in, 3);
                            mode_window_updater = 1;
                            eeprom_write = 1;
                            INIT_VALUES();
                            INIT_STATUS();
                        break;
                    }
                    //LOG_INF_GLOB("SYSTEM_MODE_WINDOW_SCENARIO");
                break;

                case SYSTEM_TEST_WINDOW_SCENARIO:
                    switch (data_in[2])
                    {
                        case 1:
                            if(readBit(current_system_mode, radio_on_test)) 
                            { 
                                DO_TOGGLE(data_in[3], data_in[4]);
                            }
                        break;

                        case 2:
                            modbus_reg_03[mod_reg_03_switch_strategy] = data_in[3];
                            eeprom_write = 1;
                        break;

                        case 3:
                            modbus_reg_03[mod_reg_03_switch_toggle_time] = BytesToInt(data_in, 3);
                            eeprom_write = 1;
                        break;
                    }
                    gpio_change = 1;
                break;

                case CALIBRATION_WINDOW_SCENARIO:
                    switch(data_in[2])
                    {
                        case 1:
                            calibration_window_updater = 1;
                        break;
                        case 2:
                            calibration_window_updater = 2;
                        break;
                        case 3:
                            modbus_reg_03[mod_reg_03_calibr_command] = BytesToInt(data_in, 3);
                        break;
                        case 4:
                            modbus_reg_03[mod_reg_03_cal_scale_100_chan_0] = BytesToInt(data_in, 3);
                            modbus_reg_03[mod_reg_03_cal_scale_100_chan_1] = BytesToInt(data_in, 5);
                            modbus_reg_03[mod_reg_03_cal_scale_100_chan_2] = BytesToInt(data_in, 7);
                            modbus_reg_03[mod_reg_03_calibration_done] &= 0xF8FF;
                            modbus_reg_04[mod_reg_04_calibration_alarm] = modbus_reg_03[mod_reg_03_calibration_done];
                            eeprom_write = 1;
                            calibration_window_updater = 1;
                        break;
                        case 5:
                            modbus_reg_03[mod_reg_03_cal_scale_100_chan_3] = BytesToInt(data_in, 3);
                            modbus_reg_03[mod_reg_03_calibration_done] &= 0xF7FF;
                            modbus_reg_04[mod_reg_04_calibration_alarm] = modbus_reg_03[mod_reg_03_calibration_done];
                            eeprom_write = 1;
                            calibration_window_updater = 1;
                        break;
                        case 6:
                            modbus_reg_03[mod_reg_03_cal_scale_100_chan_4] = BytesToInt(data_in, 3);
                            modbus_reg_03[mod_reg_03_cal_scale_100_chan_5] = BytesToInt(data_in, 5);
                            modbus_reg_03[mod_reg_03_cal_scale_100_chan_6] = BytesToInt(data_in, 7);
                            modbus_reg_03[mod_reg_03_calibration_done] &= 0x8FFF;
                            modbus_reg_04[mod_reg_04_calibration_alarm] = modbus_reg_03[mod_reg_03_calibration_done];
                            eeprom_write = 1;
                            calibration_window_updater = 1;
                        break;
                        case 7:
                            modbus_reg_03[mod_reg_03_cal_scale_100_chan_7] = BytesToInt(data_in, 3);
                            modbus_reg_03[mod_reg_03_calibration_done] &= 0x7FFF;
                            modbus_reg_04[mod_reg_04_calibration_alarm] = modbus_reg_03[mod_reg_03_calibration_done];
                            eeprom_write = 1;
                            calibration_window_updater = 1;
                        break;
                        case 8:
                            modbus_reg_03[mod_reg_03_cal_scale_coef_0] = BytesToInt(data_in, 3);
                            modbus_reg_03[mod_reg_03_cal_scale_coef_1] = BytesToInt(data_in, 5);
                            modbus_reg_03[mod_reg_03_cal_scale_coef_2] = BytesToInt(data_in, 7);
                            eeprom_write = 1;
                            calibration_window_updater = 1;
                        break;
                        case 9:
                            modbus_reg_03[mod_reg_03_cal_scale_coef_3] = BytesToInt(data_in, 3);
                            eeprom_write = 1;
                            calibration_window_updater = 1;
                        break;
                        case 10:
                            modbus_reg_03[mod_reg_03_cal_scale_coef_4] = BytesToInt(data_in, 3);
                            modbus_reg_03[mod_reg_03_cal_scale_coef_5] = BytesToInt(data_in, 5);
                            modbus_reg_03[mod_reg_03_cal_scale_coef_6] = BytesToInt(data_in, 7);
                            eeprom_write = 1;
                            calibration_window_updater = 1;
                        break;
                        case 11:
                            modbus_reg_03[mod_reg_03_cal_scale_coef_7] = BytesToInt(data_in, 3);
                            eeprom_write = 1;
                            calibration_window_updater = 1;
                        break;
                        case 16:
                            uart_answer(data_in, length);
                        break;
                    }
                    //LOG_INF_GLOB("CALIBRATION_WINDOW_SCENARIO %d", data_in[2]);
                break;

                case MAIN_WINDOW_SCENARIO:
                    switch(data_in[2])
                    {
                        case 1:
                            main_window_updater = 1;
                        break;
                        case 2:
                        {
                            uint16_t system_mode = BytesToInt(data_in, 3);
                            
                            GLOBAL_start_command = readBit(system_mode, run_mode_enable);
                            main_window_updater = 2;
                        }
                        break;
                        case 3:
                            main_window_updater = 2;
                        break;
                        case 4:
                            modbus_reg_03[mod_reg_03_system_mode] = BytesToInt(data_in, 3);
                            INIT_STATUS();
                            modbus_reg_04[mod_reg_04_alarm_equipment] = 0;
                            EVENT_SEND(EVENT_SYSTEM_SCENARIO, mess_x05_event_ACKNOLEDGE_TRIP, 0, NULL);
                            main_window_updater = 2;
                        break;
                         
                    }
                break;

                case WRITE_TAG_SCENARIO:
                    EVENT_SEND(WRITE_TAG_SCENARIO, 0, length - 4, data_in);
                break;

                default:
                    LOG_INF_GLOB("no scenario");
                break;
        }
}

void Scenario_handler()
{
    switch(scenario)
    {
/*WIFI_SETTINGS_WINDOW_SCENARIO*/
        case WIFI_SETTINGS_WINDOW_SCENARIO:
            if(wifi_window_updater == mess_x03_all_wifi_param_read) 
            {
                EVENT_SEND(READ_REQUEST_SCENARIO, WIFI_SETTINGS_WINDOW_SCENARIO, mess_x03_all_wifi_param_read, NULL);
                wifi_window_updater = 0;
            }
            else
            if(wifi_window_updater == mess_x03_dyn_wifi_param_read) 
            {
                EVENT_SEND(READ_REQUEST_SCENARIO, WIFI_SETTINGS_WINDOW_SCENARIO, mess_x03_dyn_wifi_param_read, NULL);
                wifi_window_updater = 0;
            }
            break;
/*ETHERNET_SETTINGS_WINDOW_SCENARIO*/
        case ETHERNET_SETTINGS_WINDOW_SCENARIO:
            if(ethernet_window_updater == mess_x03_all_eth_param_read) 
            {
                EVENT_SEND(READ_REQUEST_SCENARIO, ETHERNET_SETTINGS_WINDOW_SCENARIO, mess_x03_all_eth_param_read, NULL);
                ethernet_window_updater = 0;
            }
            else
            if(ethernet_window_updater == mess_x03_dyn_eth_param_read) 
            {
                EVENT_SEND(3, ETHERNET_SETTINGS_WINDOW_SCENARIO, mess_x03_dyn_eth_param_read, NULL);
                ethernet_window_updater = 0;
            }
            break;
/*MTZ_SETTINGS_WINDOW_SCENARIO*/
        case MTZ_SETTINGS_WINDOW_SCENARIO:
            if(mtz_window_updater == 1) 
            {
                uint8_t data_scenario[129];
                data_scenario[0] = 5;
                data_scenario[1] = MTZ_SETTINGS_WINDOW_SCENARIO;
                data_scenario[2] = 1;
                memcpy(&data_scenario[3], &modbus_reg_03[mod_reg_03_A_MTZ_1_current_trip], 124);  
                uint16_t length = sizeof(data_scenario);
                CRC_CALC(data_scenario, length - 2, &data_scenario[length - 2]);
                int err_code = bt_send_work(data_scenario, length);
		if(err_code)
		{
                    LOG_ERR_GLOB("Failed sending NUS message. Error %d ", err_code);
		}
                mtz_window_updater = 0;
            }
            break;
/*SYSTEM_MODE_WINDOW_SCENARIO*/
        case SYSTEM_MODE_WINDOW_SCENARIO:
            if(mode_window_updater == 1) 
            {
                RTC_millis_get(&modbus_reg_04[mod_reg_04_RTC_millis]);
                uint8_t data_scenario[] = { 5, 
                                               SYSTEM_MODE_WINDOW_SCENARIO, 1,
                                               IntToBytes(modbus_reg_03[mod_reg_03_system_mode]), 
                                               IntToBytes(modbus_reg_03[mod_reg_03_voltage_trans_strategy]), 
                                               IntToBytes(modbus_reg_04[mod_reg_04_RTC_year]), 
                                               IntToBytes(modbus_reg_04[mod_reg_04_RTC_month]),  
                                               IntToBytes(modbus_reg_04[mod_reg_04_RTC_date]),  
                                               IntToBytes(modbus_reg_04[mod_reg_04_RTC_hour]),  
                                               IntToBytes(modbus_reg_04[mod_reg_04_RTC_minute]),  
                                               IntToBytes(modbus_reg_04[mod_reg_04_RTC_second]),  
                                               IntToBytes(modbus_reg_04[mod_reg_04_RTC_millis]), 
                                               IntToBytes(modbus_reg_04[mod_reg_04_NRF_FW]), 
                                               IntToBytes(modbus_reg_04[mod_reg_04_ESP_FW]),  
                                               0,0};

                uint16_t length = sizeof(data_scenario);
                CRC_CALC(data_scenario, length - 2, &data_scenario[length - 2]);
                int err_code = bt_send_work(data_scenario, length);
		if(err_code)
		{
                    LOG_ERR_GLOB("Failed sending NUS message. Error %d ", err_code);
		}
                mode_window_updater = 0;
                EVENT_SEND(READ_REQUEST_SCENARIO, SYSTEM_MODE_WINDOW_SCENARIO, mess_x03_esp_version, NULL);
            }
            break;
/*SYSTEM_TEST_WINDOW_SCENARIO*/
        case SYSTEM_TEST_WINDOW_SCENARIO:
            if(gpio_change) 
            {
                uint8_t data_scenario[] = { 5, 
                                               SYSTEM_TEST_WINDOW_SCENARIO, 0,
                                               IntToBytes(modbus_reg_04[mod_reg_04_DI_states]),
                                               IntToBytes(modbus_reg_04[mod_reg_04_DO_states]),
                                               IntToBytes(modbus_reg_03[mod_reg_03_switch_strategy]),
                                               IntToBytes(modbus_reg_03[mod_reg_03_switch_toggle_time]),
                                               0,0};
                uint16_t length = sizeof(data_scenario);
                CRC_CALC(data_scenario, length - 2, &data_scenario[length - 2]);
                int err_code = bt_send_work(data_scenario, length);
		if(err_code)
		{
                    LOG_ERR_GLOB("Failed sending NUS message. Error %d ", err_code);
		}
                gpio_change = 0;
            }
            break;
/*CALIBRATION_WINDOW_SCENARIO*/        
        case CALIBRATION_WINDOW_SCENARIO:
            if(calibration_window_updater == 1)
            {
                uint8_t data_scenario[] = { 5, 
                                                CALIBRATION_WINDOW_SCENARIO, 1,
                                                IntToBytes(modbus_reg_04[mod_reg_04_current_A]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_current_B]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_current_C]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_current_3I0]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_voltage_A]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_voltage_B]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_voltage_C]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_voltage_x]),
                                                
                                                IntToBytes(modbus_reg_04[mod_reg_04_angle_currentA_voltageA]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_angle_currentB_voltageB]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_angle_currentC_voltageC]),

                                                IntToBytes(modbus_reg_04[mod_reg_04_calibration_alarm]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_temperature]),

                                                IntToBytes(modbus_reg_03[mod_reg_03_cal_scale_100_chan_0]),
                                                IntToBytes(modbus_reg_03[mod_reg_03_cal_scale_100_chan_1]),
                                                IntToBytes(modbus_reg_03[mod_reg_03_cal_scale_100_chan_2]),
                                                IntToBytes(modbus_reg_03[mod_reg_03_cal_scale_100_chan_3]),
                                                IntToBytes(modbus_reg_03[mod_reg_03_cal_scale_100_chan_4]),
                                                IntToBytes(modbus_reg_03[mod_reg_03_cal_scale_100_chan_5]),
                                                IntToBytes(modbus_reg_03[mod_reg_03_cal_scale_100_chan_6]),
                                                IntToBytes(modbus_reg_03[mod_reg_03_cal_scale_100_chan_7]),

                                                IntToBytes(modbus_reg_03[mod_reg_03_cal_scale_coef_0]),
                                                IntToBytes(modbus_reg_03[mod_reg_03_cal_scale_coef_1]),
                                                IntToBytes(modbus_reg_03[mod_reg_03_cal_scale_coef_2]),
                                                IntToBytes(modbus_reg_03[mod_reg_03_cal_scale_coef_3]),
                                                IntToBytes(modbus_reg_03[mod_reg_03_cal_scale_coef_4]),
                                                IntToBytes(modbus_reg_03[mod_reg_03_cal_scale_coef_5]),
                                                IntToBytes(modbus_reg_03[mod_reg_03_cal_scale_coef_6]),
                                                IntToBytes(modbus_reg_03[mod_reg_03_cal_scale_coef_7]),
                                                0,0};
                                                
                uint16_t length = sizeof(data_scenario);
                CRC_CALC(data_scenario, length - 2, &data_scenario[length - 2]);
                int err_code = bt_send_work(data_scenario, length);
		if(err_code)
		{
                    LOG_ERR_GLOB("Failed sending NUS message. Error %d ", err_code);
		}
                calibration_window_updater = 0;
            }
            else
            if(calibration_window_updater == 2)
            {
                 uint8_t data_scenario[] = { 5, 
                                                CALIBRATION_WINDOW_SCENARIO, 2,
                                                IntToBytes(modbus_reg_04[mod_reg_04_current_A]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_current_B]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_current_C]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_current_3I0]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_voltage_A]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_voltage_B]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_voltage_C]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_voltage_x]),
                                                
                                                IntToBytes(modbus_reg_04[mod_reg_04_angle_currentA_voltageA]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_angle_currentB_voltageB]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_angle_currentC_voltageC]),

                                                IntToBytes(modbus_reg_04[mod_reg_04_calibration_alarm]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_temperature]),
                                                0,0};
                                                
                uint16_t length = sizeof(data_scenario);
                CRC_CALC(data_scenario, length - 2, &data_scenario[length - 2]);
                int err_code = bt_send_work(data_scenario, length);
		if(err_code)
		{
                    LOG_ERR_GLOB("Failed sending NUS message. Error %d ", err_code);
		}
                calibration_window_updater = 0;
            }
            break;
/*MAIN_WINDOW_SCENARIO*/        
        case MAIN_WINDOW_SCENARIO:
            if(main_window_updater == 1)
            {
                uint8_t data_scenario[] = { 5, 
                                                MAIN_WINDOW_SCENARIO, 1,
                                                IntToBytes(modbus_reg_04[mod_reg_04_current_A_primary]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_current_B_primary]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_current_C_primary]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_current_3I0_primary]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_voltage_A_primary]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_voltage_B_primary]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_voltage_C_primary]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_voltage_x_primary]),
                                                0,0};
                                                
                uint16_t length = sizeof(data_scenario);
                CRC_CALC(data_scenario, length - 2, &data_scenario[length - 2]);
                int err_code = bt_send_work(data_scenario, length);
		if(err_code)
		{
                    LOG_ERR_GLOB("Failed sending NUS message. Error %d ", err_code);
		}
                main_window_updater = 0;
            }
            else
            if(main_window_updater == 2)
            {
                uint8_t data_scenario[] = { 5, 
                                                MAIN_WINDOW_SCENARIO, 2,
                                                IntToBytes(modbus_reg_03[mod_reg_03_system_mode]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_error_equipment]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_alarm_equipment]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_apv_cycle]),
                                                IntToBytes(modbus_reg_04[mod_reg_04_apv_ready]),
                                                0,0};
                                           
                uint16_t length = sizeof(data_scenario);
                CRC_CALC(data_scenario, length - 2, &data_scenario[length - 2]);
                int err_code = bt_send_work(data_scenario, length);
		if(err_code)
		{
                    LOG_ERR_GLOB("Failed sending NUS message. Error %d ", err_code);
		}
                main_window_updater = 0;
            }
        break;
    }
}

static void DI_Function()
{
    
    for(uint8_t i = 0; i < 11; i++)
    {
        uint8_t di_state = DI_CHANNEL_READ_0_to_10(i);
        if(readBit(modbus_reg_04[mod_reg_04_DI_states], i) != di_state)
        {
            writebit(modbus_reg_04[mod_reg_04_DI_states], i, di_state);
            EVENT_SEND(EVENT_DISCRET_IN_SCENARIO, i, di_state, NULL);
            SPORADIC_answer_02(mod_reg_04_DI_states, i);
            gpio_change = 1;
        }
    }
}
  
void adc_calibration_handler()
{
    if(modbus_reg_03[mod_reg_03_calibr_command] != 0)
    {
        for(uint8_t i = 0; i < 8; i++)
        {
            if(readBit(modbus_reg_03[mod_reg_03_calibr_command], (i + 8)) == 1) 
            {
                modbus_reg_03[mod_reg_03_cal_adc_100_chan_0 + i] = CURRENT_median_result[i];
                bitSet(modbus_reg_03[mod_reg_03_calibration_done], i + 8);
                modbus_reg_04[mod_reg_04_calibration_alarm] = modbus_reg_03[mod_reg_03_calibration_done];

                modbus_reg_03[mod_reg_03_cal_adc_0_chan_0 + i] = zero_val_final[i];
                /*
                bitSet(modbus_reg_03[mod_reg_03_calibration_done], i);
                modbus_reg_04[mod_reg_04_calibration_alarm] = modbus_reg_03[mod_reg_03_calibration_done];*/
            }
        }            
        modbus_reg_03[mod_reg_03_calibr_command] = 0;
        eeprom_write = 1;
        INIT_VALUES();
    }
}

void rutine_handler()
{
    DI_Function();
    adc_calibration_handler();
    storage_save(&eeprom_write);
    //oscilogramma_send();
    General_Logic();
    Scenario_handler();
}

K_TIMER_DEFINE(k_rutine_timer, rutine_handler, NULL);

/*
static void call_1(char *com, int *data, uint8_t num)
{
    
}
*/
void main(void)
{
    LOG_WRN_GLOB("RESET\n\n");
    LOG_INF_GLOB("Previous Reset Reason: %d \n", NRF_POWER->RESETREAS);
    NRF_POWER->RESETREAS = 0; 

    FlashStorageRead(modbus_reg_03);
     
    calculations_init(modbus_reg_03, modbus_reg_04);
    General_Logic_Init(&DO_TOGGLE, &SWITCHER_action, modbus_reg_03, modbus_reg_04, &main_window_updater, &MTZ_resume_procedure, &INIT_ptr);

    modbus_reg_03[mod_reg_03_system_mode] = modbus_reg_03[mod_reg_03_system_mode] & 0x3FFF;
    current_system_mode = modbus_reg_03[mod_reg_03_system_mode] & 0x7FF;

    INIT_VALUES();
    INIT_STATUS(); 

    LOG_INF("Last register number %d", mod_reg_03_crc);
    LOG_INF_GLOB("mod_reg_03_voltage_trans_strategy %d", modbus_reg_03[mod_reg_03_voltage_trans_strategy]);

    uart_init(data_array, &uart_callback_event, uart_speed);
    
    k_timer_init(&tx_timeout_timer[0], tx_timeout_handle_0, NULL); 	
    k_timer_init(&tx_timeout_timer[1], tx_timeout_handle_1, NULL); 
    k_timer_init(&tx_timeout_timer[2], tx_timeout_handle_2, NULL); 
    k_timer_init(&tx_timeout_timer[3], tx_timeout_handle_3, NULL); 

    EVENT_SEND(EVENT_SYSTEM_SCENARIO, mess_x05_event_RESET_NRF52, 0, NULL);

    /*BLE init*/
    BLE_init(&MODBUS_PROCESS);

    /*WATCHDOG*/
    WDT_init_timer(2);

    /*TPL WDT init*/
    //TPL5010_init();
 
    /* IEC_104 init */
    IEC_init(modbus_reg_04, modbus_reg_03, &EVENT_SEND);

    /* GPIO settings */
    DO_init();      
    DI_init(&bb_switch_off_event, &bb_switch_on_event);   

    /* RTC init */
    RealClockInit(modbus_reg_04, &EVENT_SEND);

    k_timer_start(&k_rutine_timer,   K_MSEC(50),   K_MSEC(50));
    k_timer_start(&TAG_read_timer_1, K_MSEC(2000), K_MSEC(2000));
    k_timer_start(&TAG_read_timer_2, K_MSEC(1000), K_MSEC(2000));
   
    highspeed_timer_init(&MTZ_handler);
    MTZ_stop_procedure();

    adc_ppi_init(&CURRENT_PHASE_EVENT);

    temperature_init_sec(&modbus_reg_04[mod_reg_04_temperature], 60);
    
/*
    Segger_init(&call_1, "w");
    Segger_start('\n', ' ');*/
}

//k_sem_give(&ble_init_ok_0); 
/*
K_SEM_DEFINE(ble_init_ok_0, 0, 1);
K_SEM_DEFINE(ble_init_ok_1, 0, 1);
*/

/*
void ble_write_thread_0()
{
        while(1)
        {
            k_sem_take(&ble_init_ok_0, K_FOREVER);k_sleep(K_MSEC(1000)); 
            k_sem_give(&ble_init_ok_1);
            LOG_INF_GLOB("ble_write_thread 0");
        }
}

void ble_write_thread_1()
{
        while(1)
        {
            k_sem_take(&ble_init_ok_1, K_FOREVER);k_sleep(K_MSEC(1000)); 
            k_sem_give(&ble_init_ok_0);
            LOG_INF_GLOB("ble_write_thread 1");
        }
}

#define PRIORITY_0  4
#define PRIORITY_1  0
#define STACKSIZE 512

K_THREAD_DEFINE(ble_write_thread_id_0, STACKSIZE, ble_write_thread_0, NULL, NULL, NULL, PRIORITY_0, 0, 0);

K_THREAD_DEFINE(ble_write_thread_id_1, STACKSIZE, ble_write_thread_1, NULL, NULL, NULL, PRIORITY_1, 0, 0);

void sum_series(uint8_t *data, uint8_t id, uint8_t scena, uint8_t list, size_t num, ...)
{
    va_list argptr;
    va_start(argptr, num);
    data[0] = id; 
    data[1] = scena; 
    data[2] = list; 
    for(uint8_t i = 0; i < num; i++) 
    {
        int va_data = va_arg(argptr, int);
        int16_t *reg_type = NULL;
        switch(va_data)
        {
            case 3:
                reg_type = modbus_reg_03;
            break;
            case 4:
                reg_type = modbus_reg_04;
            break;
        }
        va_data = va_arg(argptr, int);
        data[i * 2 + 3] = reg_type[va_data] >> 8;
        data[i * 2 + 4] = reg_type[va_data] &255;
    }
    va_end(argptr);
}


void brackfast()
{LOG_INF("brackfast");
    k_sem_take(&WDT_init_ok, K_MSEC(2000));
    for(;;)
    {LOG_INF("gav");
       // FeedHungryDog();
        k_sleep(K_MSEC(1000)); 
    }
}


#define STACKSIZE   512
#define PRIORITY    6
K_THREAD_DEFINE(brackfast_id, STACKSIZE, brackfast, NULL, NULL, NULL, PRIORITY, 0, 0);

*/







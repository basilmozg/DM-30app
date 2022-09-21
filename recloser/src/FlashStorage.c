#include "FlashStorage.h"
#include "registers_modbus.h"
#include "DEFINES.h"
#include "saadc_work.h"

#include <drivers/flash.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(FlashStorage);

#define BYTE_NUM_WRITE     mod_reg_03_num * 2
#define BYTE4_NUM_WRITE    mod_reg_03_num / 2

static const struct device *flash_device;
static int16_t *modbus_reg_03;

void FlashStorageWrite(int16_t *reg_03)
{
    int ret;

    ret = flash_erase(flash_device, start_address_flash, count_bytes_flash);
    if (ret) LOG_INF_GLOB("flash_erase failed (err:%d).\n", ret);
    else LOG_INF_GLOB("%d flash_erase ok (err:%d).\n", start_address_flash, ret);
	
    uint16_t len = BYTE_NUM_WRITE;

    static uint8_t bytes_to_flash[BYTE_NUM_WRITE] = {0};
    if(*(reg_03 + mod_reg_03_crc) != 12345) *(reg_03 + mod_reg_03_crc) = 32100;

    for(uint16_t i = 0; i < mod_reg_03_num; i++)
    {
        bytes_to_flash[i * 2]     = *(reg_03 + i) & 255;
        bytes_to_flash[i * 2 + 1] = *(reg_03 + i) >>  8;
    }
        
    ret = flash_write(flash_device, start_address_flash, bytes_to_flash, len);
    if (ret) LOG_INF_GLOB("flash_write failed (err:%d).\n", ret);
}


void default_registers_set(int16_t *reg_03)
{
        if(*(reg_03 + mod_reg_03_crc) != 32100)
        {
            *(reg_03 + mod_reg_03_cal_adc_100_chan_0) = mod_reg_03_default_cal_100_adc_cur_phase;
            *(reg_03 + mod_reg_03_cal_adc_100_chan_1) = mod_reg_03_default_cal_100_adc_cur_phase;
            *(reg_03 + mod_reg_03_cal_adc_100_chan_2) = mod_reg_03_default_cal_100_adc_cur_phase;
            *(reg_03 + mod_reg_03_cal_adc_100_chan_3) = mod_reg_03_default_cal_100_adc_cur_3I0;
            *(reg_03 + mod_reg_03_cal_adc_100_chan_4) = mod_reg_03_default_cal_100_adc_volt;
            *(reg_03 + mod_reg_03_cal_adc_100_chan_5) = mod_reg_03_default_cal_100_adc_volt;
            *(reg_03 + mod_reg_03_cal_adc_100_chan_6) = mod_reg_03_default_cal_100_adc_volt;
            *(reg_03 + mod_reg_03_cal_adc_100_chan_7) = mod_reg_03_default_cal_100_adc_volt;

            *(reg_03 + mod_reg_03_cal_adc_0_chan_0) = mod_reg_03_default_cal_0_adc_cur_phase;
            *(reg_03 + mod_reg_03_cal_adc_0_chan_1) = mod_reg_03_default_cal_0_adc_cur_phase;
            *(reg_03 + mod_reg_03_cal_adc_0_chan_2) = mod_reg_03_default_cal_0_adc_cur_phase;
            *(reg_03 + mod_reg_03_cal_adc_0_chan_3) = mod_reg_03_default_cal_0_adc_cur_3I0;
            *(reg_03 + mod_reg_03_cal_adc_0_chan_4) = mod_reg_03_default_cal_0_adc_volt;
            *(reg_03 + mod_reg_03_cal_adc_0_chan_5) = mod_reg_03_default_cal_0_adc_volt;
            *(reg_03 + mod_reg_03_cal_adc_0_chan_6) = mod_reg_03_default_cal_0_adc_volt;
            *(reg_03 + mod_reg_03_cal_adc_0_chan_7) = mod_reg_03_default_cal_0_adc_volt;

            *(reg_03 + mod_reg_03_calibr_command)       = 0;
            *(reg_03 + mod_reg_03_calibration_done)     = 0;

            *(reg_03 + mod_reg_03_cal_scale_100_chan_0) = mod_reg_03_default_cal_cur_phase_scale;
            *(reg_03 + mod_reg_03_cal_scale_100_chan_1) = mod_reg_03_default_cal_cur_phase_scale;
            *(reg_03 + mod_reg_03_cal_scale_100_chan_2) = mod_reg_03_default_cal_cur_phase_scale;
            *(reg_03 + mod_reg_03_cal_scale_100_chan_3) = mod_reg_03_default_cal_cur_3I0_scale;
            *(reg_03 + mod_reg_03_cal_scale_100_chan_4) = mod_reg_03_default_cal_voltage_scale;
            *(reg_03 + mod_reg_03_cal_scale_100_chan_5) = mod_reg_03_default_cal_voltage_scale;
            *(reg_03 + mod_reg_03_cal_scale_100_chan_6) = mod_reg_03_default_cal_voltage_scale;
            *(reg_03 + mod_reg_03_cal_scale_100_chan_7) = mod_reg_03_default_cal_voltage_x_scale;

            *(reg_03 + mod_reg_03_cal_scale_coef_0) = mod_reg_03_default_cal_cur_scale_coef;
            *(reg_03 + mod_reg_03_cal_scale_coef_1) = mod_reg_03_default_cal_cur_scale_coef;
            *(reg_03 + mod_reg_03_cal_scale_coef_2) = mod_reg_03_default_cal_cur_scale_coef;
            *(reg_03 + mod_reg_03_cal_scale_coef_3) = mod_reg_03_default_cal_cur_3I0_scale_coef;
            *(reg_03 + mod_reg_03_cal_scale_coef_4) = mod_reg_03_default_cal_voltage_scale_coef;
            *(reg_03 + mod_reg_03_cal_scale_coef_5) = mod_reg_03_default_cal_voltage_scale_coef;
            *(reg_03 + mod_reg_03_cal_scale_coef_6) = mod_reg_03_default_cal_voltage_scale_coef;
            *(reg_03 + mod_reg_03_cal_scale_coef_7) = mod_reg_03_default_cal_voltage_x_scale_coef;

            /*Ãðóïïà À*/
            *(reg_03 + mod_reg_03_A_MTZ_1_current_trip) = mod_reg_03_MTZ_1_current_trip_default;
            *(reg_03 + mod_reg_03_A_MTZ_2_current_trip) = mod_reg_03_MTZ_2_current_trip_default;
            *(reg_03 + mod_reg_03_A_MTZ_3_current_trip) = mod_reg_03_MTZ_3_current_trip_default;
            *(reg_03 + mod_reg_03_A_TO_current_trip)    = mod_reg_03_TO_current_trip_default;
            *(reg_03 + mod_reg_03_A_ZMN_voltage_trip)   = mod_reg_03_ZMN_voltage_trip_default;
            *(reg_03 + mod_reg_03_A_ZZ_current_trip)    = mod_reg_03_ZZ_current_trip_default;

            *(reg_03 + mod_reg_03_A_MTZ_1_current_back) = mod_reg_03_MTZ_1_current_back_default;
            *(reg_03 + mod_reg_03_A_MTZ_2_current_back) = mod_reg_03_MTZ_2_current_back_default;
            *(reg_03 + mod_reg_03_A_MTZ_3_current_back) = mod_reg_03_MTZ_3_current_back_default;
            *(reg_03 + mod_reg_03_A_TO_current_back)    = mod_reg_03_TO_current_back_default;
            *(reg_03 + mod_reg_03_A_ZMN_voltage_back)   = mod_reg_03_ZMN_voltage_back_default;
            *(reg_03 + mod_reg_03_A_ZZ_current_back)    = mod_reg_03_ZZ_current_back_default;

            *(reg_03 + mod_reg_03_A_MTZ_1_time_trip)    = mod_reg_03_MTZ_1_time_trip_default;
            *(reg_03 + mod_reg_03_A_MTZ_2_time_trip)    = mod_reg_03_MTZ_2_time_trip_default;
            *(reg_03 + mod_reg_03_A_MTZ_3_time_trip)    = mod_reg_03_MTZ_3_time_trip_default;
            *(reg_03 + mod_reg_03_A_TO_time_trip)       = mod_reg_03_TO_time_trip_default;
            *(reg_03 + mod_reg_03_A_ZMN_time_trip)      = mod_reg_03_ZMN_time_trip_default;
            *(reg_03 + mod_reg_03_A_ZZ_time_trip)       = mod_reg_03_ZZ_time_trip_default;

            *(reg_03 + mod_reg_03_A_MTZ_1_time_back)    = mod_reg_03_MTZ_1_time_back_default;
            *(reg_03 + mod_reg_03_A_MTZ_2_time_back)    = mod_reg_03_MTZ_2_time_back_default;
            *(reg_03 + mod_reg_03_A_MTZ_3_time_back)    = mod_reg_03_MTZ_3_time_back_default;
            *(reg_03 + mod_reg_03_A_TO_time_back)       = mod_reg_03_TO_time_back_default;
            *(reg_03 + mod_reg_03_A_ZMN_time_back)      = mod_reg_03_ZMN_time_back_default;
            *(reg_03 + mod_reg_03_A_ZZ_time_back)       = mod_reg_03_ZZ_time_back_default;
            *(reg_03 + mod_reg_03_A_trip_action)        = mod_reg_03_trip_action_default;

            *(reg_03 + mod_reg_03_A_APV_time_ready)     = mod_reg_03_APV_time_ready_default;
            *(reg_03 + mod_reg_03_A_APV1_time_cycle)    = mod_reg_03_APV1_time_cycle_default;
            *(reg_03 + mod_reg_03_A_APV2_time_cycle)    = mod_reg_03_APV2_time_cycle_default;
            *(reg_03 + mod_reg_03_A_APV3_time_cycle)    = mod_reg_03_APV3_time_cycle_default;
            *(reg_03 + mod_reg_03_A_APV_ZMN_time_ready) = mod_reg_03_APV_ZMN_time_ready_default;
            *(reg_03 + mod_reg_03_A_APV_ZMN_time_back)  = mod_reg_03_APV_ZMN_time_back_default;

            /*Ãðóïïà Á*/
            *(reg_03 + mod_reg_03_B_MTZ_1_current_trip) = mod_reg_03_MTZ_1_current_trip_default;
            *(reg_03 + mod_reg_03_B_MTZ_2_current_trip) = mod_reg_03_MTZ_2_current_trip_default;
            *(reg_03 + mod_reg_03_B_MTZ_3_current_trip) = mod_reg_03_MTZ_3_current_trip_default;
            *(reg_03 + mod_reg_03_B_TO_current_trip)    = mod_reg_03_TO_current_trip_default;
            *(reg_03 + mod_reg_03_B_ZMN_voltage_trip)   = mod_reg_03_ZMN_voltage_trip_default;
            *(reg_03 + mod_reg_03_B_ZZ_current_trip)    = mod_reg_03_ZZ_current_trip_default;

            *(reg_03 + mod_reg_03_B_MTZ_1_current_back) = mod_reg_03_MTZ_1_current_back_default;
            *(reg_03 + mod_reg_03_B_MTZ_2_current_back) = mod_reg_03_MTZ_2_current_back_default;
            *(reg_03 + mod_reg_03_B_MTZ_3_current_back) = mod_reg_03_MTZ_3_current_back_default;
            *(reg_03 + mod_reg_03_B_TO_current_back)    = mod_reg_03_TO_current_back_default;
            *(reg_03 + mod_reg_03_B_ZMN_voltage_back)   = mod_reg_03_ZMN_voltage_back_default;
            *(reg_03 + mod_reg_03_B_ZZ_current_back)    = mod_reg_03_ZZ_current_back_default;

            *(reg_03 + mod_reg_03_B_MTZ_1_time_trip)    = mod_reg_03_MTZ_1_time_trip_default;
            *(reg_03 + mod_reg_03_B_MTZ_2_time_trip)    = mod_reg_03_MTZ_2_time_trip_default;
            *(reg_03 + mod_reg_03_B_MTZ_3_time_trip)    = mod_reg_03_MTZ_3_time_trip_default;
            *(reg_03 + mod_reg_03_B_TO_time_trip)       = mod_reg_03_TO_time_trip_default;
            *(reg_03 + mod_reg_03_B_ZMN_time_trip)      = mod_reg_03_ZMN_time_trip_default;
            *(reg_03 + mod_reg_03_B_ZZ_time_trip)       = mod_reg_03_ZZ_time_trip_default;

            *(reg_03 + mod_reg_03_B_MTZ_1_time_back)    = mod_reg_03_MTZ_1_time_back_default;
            *(reg_03 + mod_reg_03_B_MTZ_2_time_back)    = mod_reg_03_MTZ_2_time_back_default;
            *(reg_03 + mod_reg_03_B_MTZ_3_time_back)    = mod_reg_03_MTZ_3_time_back_default;
            *(reg_03 + mod_reg_03_B_TO_time_back)       = mod_reg_03_TO_time_back_default;
            *(reg_03 + mod_reg_03_B_ZMN_time_back)      = mod_reg_03_ZMN_time_back_default;
            *(reg_03 + mod_reg_03_B_ZZ_time_back)       = mod_reg_03_ZZ_time_back_default;

            *(reg_03 + mod_reg_03_B_trip_action)        = mod_reg_03_trip_action_default;

            *(reg_03 + mod_reg_03_B_APV_time_ready)     = mod_reg_03_APV_time_ready_default;
            *(reg_03 + mod_reg_03_B_APV1_time_cycle)    = mod_reg_03_APV1_time_cycle_default;
            *(reg_03 + mod_reg_03_B_APV2_time_cycle)    = mod_reg_03_APV2_time_cycle_default;
            *(reg_03 + mod_reg_03_B_APV3_time_cycle)    = mod_reg_03_APV3_time_cycle_default;

            *(reg_03 + mod_reg_03_B_APV_ZMN_time_ready) = mod_reg_03_APV_ZMN_time_ready_default;
            *(reg_03 + mod_reg_03_B_APV_ZMN_time_back)  = mod_reg_03_APV_ZMN_time_back_default;

            *(reg_03 + mod_reg_03_rezerv)               = 0;
            *(reg_03 + mod_reg_03_switch_strategy)      = mod_reg_03_switch_strategy_default;
            *(reg_03 + mod_reg_03_switch_toggle_time)   = mod_reg_03_switch_toggle_time_default;

            *(reg_03 + mod_reg_03_voltage_trans_strategy)   = mod_reg_03_voltage_trans_strategy_default;

            *(reg_03 + mod_reg_03_system_mode)          = 0;

            *(reg_03 + mod_reg_03_crc) = 32100;
        }
}

void FlashStorageRead(int16_t *reg_03)
{
    modbus_reg_03 = reg_03;
    flash_device = device_get_binding(DT_CHOSEN_ZEPHYR_FLASH_CONTROLLER_LABEL);
    flash_read(flash_device, start_address_flash, reg_03, BYTE_NUM_WRITE);
    default_registers_set(reg_03);
}

static void storage_done()
{
    ppi_pause();
    FlashStorageWrite(modbus_reg_03);
    ppi_restart();
}

K_WORK_DEFINE(storage_handle_work, storage_done);

void storage_handle(struct k_timer *dummy)
{
    k_work_submit(&storage_handle_work);
}

K_TIMER_DEFINE(storage_handle_timer, storage_handle, NULL);

void storage_save(uint8_t *eeprom_write)
{
    if(*eeprom_write)
    {
        *eeprom_write = 0;
        k_timer_start(&storage_handle_timer,  K_MSEC(1000), K_NO_WAIT);
    }
}


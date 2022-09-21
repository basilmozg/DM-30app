#ifndef REGISTERS_MODBUS_H__
#define REGISTERS_MODBUS_H__

/*регистры модбас x04*/

    #define mod_reg_04_current_A                    0
    #define mod_reg_04_current_B                    mod_reg_04_current_A + 1
    #define mod_reg_04_current_C                    mod_reg_04_current_B + 1
    #define mod_reg_04_current_3I0                  mod_reg_04_current_C + 1
    #define mod_reg_04_voltage_A                    mod_reg_04_current_3I0 + 1
    #define mod_reg_04_voltage_B                    mod_reg_04_voltage_A + 1
    #define mod_reg_04_voltage_C                    mod_reg_04_voltage_B + 1
    #define mod_reg_04_voltage_x                    mod_reg_04_voltage_C + 1

    #define mod_reg_04_alarm_equipment              mod_reg_04_voltage_x + 1
    #define mod_reg_04_error_equipment              mod_reg_04_alarm_equipment + 1

    #define mod_reg_04_angle_currentA_voltageA      mod_reg_04_error_equipment + 1
    #define mod_reg_04_angle_currentB_voltageB      mod_reg_04_angle_currentA_voltageA + 1
    #define mod_reg_04_angle_currentC_voltageC      mod_reg_04_angle_currentB_voltageB + 1

    /*рабочая группа*/
    #define mod_reg_04_MTZ_1_current_trip           mod_reg_04_angle_currentC_voltageC + 1
    #define mod_reg_04_MTZ_2_current_trip           mod_reg_04_MTZ_1_current_trip + 1
    #define mod_reg_04_MTZ_3_current_trip           mod_reg_04_MTZ_2_current_trip + 1
    #define mod_reg_04_TO_current_trip              mod_reg_04_MTZ_3_current_trip + 1
    #define mod_reg_04_ZMN_voltage_trip             mod_reg_04_TO_current_trip + 1
    #define mod_reg_04_ZZ_current_trip              mod_reg_04_ZMN_voltage_trip + 1

    #define mod_reg_04_MTZ_1_current_back           mod_reg_04_ZZ_current_trip + 1
    #define mod_reg_04_MTZ_2_current_back           mod_reg_04_MTZ_1_current_back + 1
    #define mod_reg_04_MTZ_3_current_back           mod_reg_04_MTZ_2_current_back + 1
    #define mod_reg_04_TO_current_back              mod_reg_04_MTZ_3_current_back + 1
    #define mod_reg_04_ZMN_voltage_back             mod_reg_04_TO_current_back + 1
    #define mod_reg_04_ZZ_current_back              mod_reg_04_ZMN_voltage_back + 1

    #define mod_reg_04_MTZ_1_time_trip              mod_reg_04_ZZ_current_back + 1
    #define mod_reg_04_MTZ_2_time_trip              mod_reg_04_MTZ_1_time_trip + 1
    #define mod_reg_04_MTZ_3_time_trip              mod_reg_04_MTZ_2_time_trip + 1
    #define mod_reg_04_TO_time_trip                 mod_reg_04_MTZ_3_time_trip + 1
    #define mod_reg_04_ZMN_time_trip                mod_reg_04_TO_time_trip + 1
    #define mod_reg_04_ZZ_time_trip                 mod_reg_04_ZMN_time_trip + 1

    #define mod_reg_04_MTZ_1_time_back              mod_reg_04_ZZ_time_trip + 1
    #define mod_reg_04_MTZ_2_time_back              mod_reg_04_MTZ_1_time_back + 1
    #define mod_reg_04_MTZ_3_time_back              mod_reg_04_MTZ_2_time_back + 1
    #define mod_reg_04_TO_time_back                 mod_reg_04_MTZ_3_time_back + 1
    #define mod_reg_04_ZMN_time_back                mod_reg_04_TO_time_back + 1
    #define mod_reg_04_ZZ_time_back                 mod_reg_04_ZMN_time_back + 1

    #define mod_reg_04_APV_time_ready               mod_reg_04_ZZ_time_back + 1
    #define mod_reg_04_APV1_time_cycle              mod_reg_04_APV_time_ready + 1
    #define mod_reg_04_APV2_time_cycle              mod_reg_04_APV1_time_cycle + 1
    #define mod_reg_04_APV3_time_cycle              mod_reg_04_APV2_time_cycle + 1

    #define mod_reg_04_APV_ZMN_time_ready           mod_reg_04_APV3_time_cycle + 1
    #define mod_reg_04_APV_ZMN_time_back            mod_reg_04_APV_ZMN_time_ready + 1

    #define mod_reg_04_trip_action                  mod_reg_04_APV_ZMN_time_back + 1

    /**/

    #define mod_reg_04_calibration_alarm            mod_reg_04_trip_action + 1

    #define mod_reg_04_DI_states                    mod_reg_04_calibration_alarm + 1
    #define mod_reg_04_DO_states                    mod_reg_04_DI_states + 1

    #define mod_reg_04_RTC_year                     mod_reg_04_DO_states + 1
    #define mod_reg_04_RTC_month                    mod_reg_04_RTC_year + 1
    #define mod_reg_04_RTC_date                     mod_reg_04_RTC_month + 1
    #define mod_reg_04_RTC_day                      mod_reg_04_RTC_date + 1
    #define mod_reg_04_RTC_hour                     mod_reg_04_RTC_day + 1
    #define mod_reg_04_RTC_minute                   mod_reg_04_RTC_hour + 1
    #define mod_reg_04_RTC_second                   mod_reg_04_RTC_minute + 1
    #define mod_reg_04_RTC_millis                   mod_reg_04_RTC_second + 1

    #define mod_reg_04_NRF_FW                       mod_reg_04_RTC_millis + 1
    #define mod_reg_04_ESP_FW                       mod_reg_04_NRF_FW + 1

    /*ethernet*/
    #define mod_reg_04_ether_dinamic_ip_1_2         mod_reg_04_ESP_FW + 1
    #define mod_reg_04_ether_dinamic_ip_3_4         mod_reg_04_ether_dinamic_ip_1_2 + 1
    #define mod_reg_04_ether_dinamic_subnet_1_2     mod_reg_04_ether_dinamic_ip_3_4 + 1
    #define mod_reg_04_ether_dinamic_subnet_3_4     mod_reg_04_ether_dinamic_subnet_1_2 + 1
    #define mod_reg_04_ether_dinamic_gateway_1_2    mod_reg_04_ether_dinamic_subnet_3_4 + 1
    #define mod_reg_04_ether_dinamic_gateway_3_4    mod_reg_04_ether_dinamic_gateway_1_2 + 1
    #define mod_reg_04_ether_dinamic_dns_1_2        mod_reg_04_ether_dinamic_gateway_3_4 + 1
    #define mod_reg_04_ether_dinamic_dns_3_4        mod_reg_04_ether_dinamic_dns_1_2 + 1

    /*wifi*/
    #define mod_reg_04_wifi_dinamic_ip_1_2          mod_reg_04_ether_dinamic_dns_3_4 + 1
    #define mod_reg_04_wifi_dinamic_ip_3_4          mod_reg_04_wifi_dinamic_ip_1_2 + 1
    #define mod_reg_04_wifi_dinamic_subnet_1_2      mod_reg_04_wifi_dinamic_ip_3_4 + 1
    #define mod_reg_04_wifi_dinamic_subnet_3_4      mod_reg_04_wifi_dinamic_subnet_1_2 + 1
    #define mod_reg_04_wifi_dinamic_gateway_1_2     mod_reg_04_wifi_dinamic_subnet_3_4 + 1
    #define mod_reg_04_wifi_dinamic_gateway_3_4     mod_reg_04_wifi_dinamic_gateway_1_2 + 1
    #define mod_reg_04_wifi_dinamic_dns_1_2         mod_reg_04_wifi_dinamic_gateway_3_4 + 1
    #define mod_reg_04_wifi_dinamic_dns_3_4         mod_reg_04_wifi_dinamic_dns_1_2 + 1

    #define mod_reg_04_current_A_primary            mod_reg_04_wifi_dinamic_dns_3_4 + 1
    #define mod_reg_04_current_B_primary            mod_reg_04_current_A_primary + 1
    #define mod_reg_04_current_C_primary            mod_reg_04_current_B_primary + 1
    #define mod_reg_04_current_3I0_primary          mod_reg_04_current_C_primary + 1
    #define mod_reg_04_voltage_A_primary            mod_reg_04_current_3I0_primary + 1
    #define mod_reg_04_voltage_B_primary            mod_reg_04_voltage_A_primary + 1
    #define mod_reg_04_voltage_C_primary            mod_reg_04_voltage_B_primary + 1
    #define mod_reg_04_voltage_x_primary            mod_reg_04_voltage_C_primary + 1

    #define mod_reg_04_temperature                  mod_reg_04_voltage_x_primary + 1
    #define mod_reg_04_apv_cycle                    mod_reg_04_temperature + 1
    #define mod_reg_04_apv_ready                    mod_reg_04_apv_cycle + 1

            #define radio_ready_apv                 0
            #define radio_ready_apv_zmn             1
            #define radio_apv_zmn_active            2

    #define mod_reg_04_num                          mod_reg_04_apv_ready + 1  //количество регистров

    /*регистры модбас x03*/

    #define mod_reg_03_cal_adc_100_chan_0           0
    #define mod_reg_03_cal_adc_100_chan_1           mod_reg_03_cal_adc_100_chan_0 + 1
    #define mod_reg_03_cal_adc_100_chan_2           mod_reg_03_cal_adc_100_chan_1 + 1
    #define mod_reg_03_cal_adc_100_chan_3           mod_reg_03_cal_adc_100_chan_2 + 1
    #define mod_reg_03_cal_adc_100_chan_4           mod_reg_03_cal_adc_100_chan_3 + 1
    #define mod_reg_03_cal_adc_100_chan_5           mod_reg_03_cal_adc_100_chan_4 + 1
    #define mod_reg_03_cal_adc_100_chan_6           mod_reg_03_cal_adc_100_chan_5 + 1
    #define mod_reg_03_cal_adc_100_chan_7           mod_reg_03_cal_adc_100_chan_6 + 1

    #define mod_reg_03_cal_adc_0_chan_0             mod_reg_03_cal_adc_100_chan_7 + 1
    #define mod_reg_03_cal_adc_0_chan_1             mod_reg_03_cal_adc_0_chan_0 + 1
    #define mod_reg_03_cal_adc_0_chan_2             mod_reg_03_cal_adc_0_chan_1 + 1
    #define mod_reg_03_cal_adc_0_chan_3             mod_reg_03_cal_adc_0_chan_2 + 1
    #define mod_reg_03_cal_adc_0_chan_4             mod_reg_03_cal_adc_0_chan_3 + 1
    #define mod_reg_03_cal_adc_0_chan_5             mod_reg_03_cal_adc_0_chan_4 + 1
    #define mod_reg_03_cal_adc_0_chan_6             mod_reg_03_cal_adc_0_chan_5 + 1
    #define mod_reg_03_cal_adc_0_chan_7             mod_reg_03_cal_adc_0_chan_6 + 1
    
    #define mod_reg_03_calibr_command               mod_reg_03_cal_adc_0_chan_7 + 1
    #define mod_reg_03_calibration_done             mod_reg_03_calibr_command + 1

    #define mod_reg_03_cal_scale_100_chan_0         mod_reg_03_calibration_done + 1
    #define mod_reg_03_cal_scale_100_chan_1         mod_reg_03_cal_scale_100_chan_0 + 1
    #define mod_reg_03_cal_scale_100_chan_2         mod_reg_03_cal_scale_100_chan_1 + 1
    #define mod_reg_03_cal_scale_100_chan_3         mod_reg_03_cal_scale_100_chan_2 + 1
    #define mod_reg_03_cal_scale_100_chan_4         mod_reg_03_cal_scale_100_chan_3 + 1
    #define mod_reg_03_cal_scale_100_chan_5         mod_reg_03_cal_scale_100_chan_4 + 1
    #define mod_reg_03_cal_scale_100_chan_6         mod_reg_03_cal_scale_100_chan_5 + 1
    #define mod_reg_03_cal_scale_100_chan_7         mod_reg_03_cal_scale_100_chan_6 + 1
  
    #define mod_reg_03_cal_scale_coef_0             mod_reg_03_cal_scale_100_chan_7 + 1
    #define mod_reg_03_cal_scale_coef_1             mod_reg_03_cal_scale_coef_0 + 1
    #define mod_reg_03_cal_scale_coef_2             mod_reg_03_cal_scale_coef_1 + 1
    #define mod_reg_03_cal_scale_coef_3             mod_reg_03_cal_scale_coef_2 + 1
    #define mod_reg_03_cal_scale_coef_4             mod_reg_03_cal_scale_coef_3 + 1
    #define mod_reg_03_cal_scale_coef_5             mod_reg_03_cal_scale_coef_4 + 1
    #define mod_reg_03_cal_scale_coef_6             mod_reg_03_cal_scale_coef_5 + 1
    #define mod_reg_03_cal_scale_coef_7             mod_reg_03_cal_scale_coef_6 + 1

    /*Группа А*/
    #define mod_reg_03_A_MTZ_1_current_trip         mod_reg_03_cal_scale_coef_7 + 1
    #define mod_reg_03_A_MTZ_2_current_trip         mod_reg_03_A_MTZ_1_current_trip + 1
    #define mod_reg_03_A_MTZ_3_current_trip         mod_reg_03_A_MTZ_2_current_trip + 1
    #define mod_reg_03_A_TO_current_trip            mod_reg_03_A_MTZ_3_current_trip + 1
    #define mod_reg_03_A_ZMN_voltage_trip           mod_reg_03_A_TO_current_trip + 1
    #define mod_reg_03_A_ZZ_current_trip            mod_reg_03_A_ZMN_voltage_trip + 1

    #define mod_reg_03_A_MTZ_1_current_back         mod_reg_03_A_ZZ_current_trip + 1
    #define mod_reg_03_A_MTZ_2_current_back         mod_reg_03_A_MTZ_1_current_back + 1
    #define mod_reg_03_A_MTZ_3_current_back         mod_reg_03_A_MTZ_2_current_back + 1
    #define mod_reg_03_A_TO_current_back            mod_reg_03_A_MTZ_3_current_back + 1
    #define mod_reg_03_A_ZMN_voltage_back           mod_reg_03_A_TO_current_back + 1
    #define mod_reg_03_A_ZZ_current_back            mod_reg_03_A_ZMN_voltage_back + 1

    #define mod_reg_03_A_MTZ_1_time_trip            mod_reg_03_A_ZZ_current_back + 1
    #define mod_reg_03_A_MTZ_2_time_trip            mod_reg_03_A_MTZ_1_time_trip + 1
    #define mod_reg_03_A_MTZ_3_time_trip            mod_reg_03_A_MTZ_2_time_trip + 1
    #define mod_reg_03_A_TO_time_trip               mod_reg_03_A_MTZ_3_time_trip + 1
    #define mod_reg_03_A_ZMN_time_trip              mod_reg_03_A_TO_time_trip + 1
    #define mod_reg_03_A_ZZ_time_trip               mod_reg_03_A_ZMN_time_trip + 1

    #define mod_reg_03_A_MTZ_1_time_back            mod_reg_03_A_ZZ_time_trip + 1
    #define mod_reg_03_A_MTZ_2_time_back            mod_reg_03_A_MTZ_1_time_back + 1
    #define mod_reg_03_A_MTZ_3_time_back            mod_reg_03_A_MTZ_2_time_back + 1
    #define mod_reg_03_A_TO_time_back               mod_reg_03_A_MTZ_3_time_back + 1
    #define mod_reg_03_A_ZMN_time_back              mod_reg_03_A_TO_time_back + 1
    #define mod_reg_03_A_ZZ_time_back               mod_reg_03_A_ZMN_time_back + 1

    #define mod_reg_03_A_APV_time_ready             mod_reg_03_A_ZZ_time_back + 1
    #define mod_reg_03_A_APV1_time_cycle            mod_reg_03_A_APV_time_ready + 1
    #define mod_reg_03_A_APV2_time_cycle            mod_reg_03_A_APV1_time_cycle + 1
    #define mod_reg_03_A_APV3_time_cycle            mod_reg_03_A_APV2_time_cycle + 1

    #define mod_reg_03_A_APV_ZMN_time_ready         mod_reg_03_A_APV3_time_cycle + 1
    #define mod_reg_03_A_APV_ZMN_time_back          mod_reg_03_A_APV_ZMN_time_ready + 1
    
    #define mod_reg_03_A_trip_action                mod_reg_03_A_APV_ZMN_time_back + 1

    /*Группа Б*/
    #define mod_reg_03_B_MTZ_1_current_trip         mod_reg_03_A_trip_action + 1
    #define mod_reg_03_B_MTZ_2_current_trip         mod_reg_03_B_MTZ_1_current_trip + 1
    #define mod_reg_03_B_MTZ_3_current_trip         mod_reg_03_B_MTZ_2_current_trip + 1
    #define mod_reg_03_B_TO_current_trip            mod_reg_03_B_MTZ_3_current_trip + 1
    #define mod_reg_03_B_ZMN_voltage_trip           mod_reg_03_B_TO_current_trip + 1
    #define mod_reg_03_B_ZZ_current_trip            mod_reg_03_B_ZMN_voltage_trip + 1

    #define mod_reg_03_B_MTZ_1_current_back         mod_reg_03_B_ZZ_current_trip + 1
    #define mod_reg_03_B_MTZ_2_current_back         mod_reg_03_B_MTZ_1_current_back + 1
    #define mod_reg_03_B_MTZ_3_current_back         mod_reg_03_B_MTZ_2_current_back + 1
    #define mod_reg_03_B_TO_current_back            mod_reg_03_B_MTZ_3_current_back + 1
    #define mod_reg_03_B_ZMN_voltage_back           mod_reg_03_B_TO_current_back + 1
    #define mod_reg_03_B_ZZ_current_back            mod_reg_03_B_ZMN_voltage_back + 1

    #define mod_reg_03_B_MTZ_1_time_trip            mod_reg_03_B_ZZ_current_back + 1
    #define mod_reg_03_B_MTZ_2_time_trip            mod_reg_03_B_MTZ_1_time_trip + 1
    #define mod_reg_03_B_MTZ_3_time_trip            mod_reg_03_B_MTZ_2_time_trip + 1
    #define mod_reg_03_B_TO_time_trip               mod_reg_03_B_MTZ_3_time_trip + 1
    #define mod_reg_03_B_ZMN_time_trip              mod_reg_03_B_TO_time_trip + 1
    #define mod_reg_03_B_ZZ_time_trip               mod_reg_03_B_ZMN_time_trip + 1

    #define mod_reg_03_B_MTZ_1_time_back            mod_reg_03_B_ZZ_time_trip + 1
    #define mod_reg_03_B_MTZ_2_time_back            mod_reg_03_B_MTZ_1_time_back + 1
    #define mod_reg_03_B_MTZ_3_time_back            mod_reg_03_B_MTZ_2_time_back + 1
    #define mod_reg_03_B_TO_time_back               mod_reg_03_B_MTZ_3_time_back + 1
    #define mod_reg_03_B_ZMN_time_back              mod_reg_03_B_TO_time_back + 1
    #define mod_reg_03_B_ZZ_time_back               mod_reg_03_B_ZMN_time_back + 1

    #define mod_reg_03_B_APV_time_ready             mod_reg_03_B_ZZ_time_back + 1
    #define mod_reg_03_B_APV1_time_cycle            mod_reg_03_B_APV_time_ready + 1
    #define mod_reg_03_B_APV2_time_cycle            mod_reg_03_B_APV1_time_cycle + 1
    #define mod_reg_03_B_APV3_time_cycle            mod_reg_03_B_APV2_time_cycle + 1
    
    #define mod_reg_03_B_APV_ZMN_time_ready         mod_reg_03_B_APV3_time_cycle + 1
    #define mod_reg_03_B_APV_ZMN_time_back          mod_reg_03_B_APV_ZMN_time_ready + 1

    #define mod_reg_03_B_trip_action                mod_reg_03_B_APV_ZMN_time_back + 1

    /*main mode control*/
    #define mod_reg_03_system_mode                  mod_reg_03_B_trip_action + 1 
            /*mod_reg_03_system_mode*/
            #define radio_on_mtz1                           0
            #define radio_on_mtz2                           1
            #define radio_on_mtz3                           2
            #define radio_on_to                             3
            #define radio_on_zmn                            4
            #define radio_on_zz                             5
            #define radio_zvn                               6
            #define radio_on_apv1                           7
            #define radio_on_apv2                           8
            #define radio_on_apv3                           9
            #define radio_apv_zmn                           10
            #define radio_group                             11
            #define radio_on_oscilogramma                   12
            #define radio_on_test                           13
            #define run_mode_enable                         14
            #define esd_mode_enable                         15
            /**/

    /*ethernet*/
    #define mod_reg_03_ether_static_ip_1_2          mod_reg_03_system_mode + 1
    #define mod_reg_03_ether_static_ip_3_4          mod_reg_03_ether_static_ip_1_2 + 1
    #define mod_reg_03_ether_static_subnet_1_2      mod_reg_03_ether_static_ip_3_4 + 1
    #define mod_reg_03_ether_static_subnet_3_4      mod_reg_03_ether_static_subnet_1_2 + 1
    #define mod_reg_03_ether_static_gateway_1_2     mod_reg_03_ether_static_subnet_3_4 + 1
    #define mod_reg_03_ether_static_gateway_3_4     mod_reg_03_ether_static_gateway_1_2 + 1
    #define mod_reg_03_ether_static_dns_1_2         mod_reg_03_ether_static_gateway_3_4 + 1
    #define mod_reg_03_ether_static_dns_3_4         mod_reg_03_ether_static_dns_1_2 + 1
    #define mod_reg_03_ether_dinamic_host           mod_reg_03_ether_static_dns_3_4 + 1 // .. +8
    #define mod_reg_03_ether_web_service_port       mod_reg_03_ether_dinamic_host + 8
    #define mod_reg_03_ether_mek_service_port       mod_reg_03_ether_web_service_port + 1
    #define mod_reg_03_ethernet_mode                mod_reg_03_ether_mek_service_port + 1

    /*wifi*/
    #define mod_reg_03_wifi_static_ip_1_2           mod_reg_03_ethernet_mode + 1
    #define mod_reg_03_wifi_static_ip_3_4           mod_reg_03_wifi_static_ip_1_2 + 1
    #define mod_reg_03_wifi_static_subnet_1_2       mod_reg_03_wifi_static_ip_3_4 + 1
    #define mod_reg_03_wifi_static_subnet_3_4       mod_reg_03_wifi_static_subnet_1_2 + 1
    #define mod_reg_03_wifi_static_gateway_1_2      mod_reg_03_wifi_static_subnet_3_4 + 1
    #define mod_reg_03_wifi_static_gateway_3_4      mod_reg_03_wifi_static_gateway_1_2 + 1
    #define mod_reg_03_wifi_static_dns_1_2          mod_reg_03_wifi_static_gateway_3_4 + 1
    #define mod_reg_03_wifi_static_dns_3_4          mod_reg_03_wifi_static_dns_1_2 + 1
    
          #define AP_PSWD_LENGTH                    16
          
    #define mod_reg_03_wifi_dinamic_host            mod_reg_03_wifi_static_dns_3_4 + 1 // .. 83
    #define mod_reg_03_wifi_ESP_AP_name             mod_reg_03_wifi_dinamic_host + 8 // .. 91
    #define mod_reg_03_wifi_ESP_AP_pswd             mod_reg_03_wifi_ESP_AP_name + 8 // .. 99
    #define mod_reg_03_wifi_STATION_AP_name         mod_reg_03_wifi_ESP_AP_pswd + 8 // .. 107
    #define mod_reg_03_wifi_STATION_AP_pswd         mod_reg_03_wifi_STATION_AP_name + 8 // .. 115
/*Update server*/
    #define mod_reg_03_update_server                mod_reg_03_wifi_STATION_AP_pswd + 8
    #define mod_reg_03_wifi_web_service_port        mod_reg_03_update_server + 16
    #define mod_reg_03_wifi_mek_service_port        mod_reg_03_wifi_web_service_port + 1
    #define mod_reg_03_wifi_mode                    mod_reg_03_wifi_mek_service_port + 1

/*Switcher strategy*/
    #define mod_reg_03_rezerv                       mod_reg_03_wifi_mode + 1
    #define mod_reg_03_switch_strategy              mod_reg_03_rezerv + 1
    #define mod_reg_03_switch_toggle_time           mod_reg_03_switch_strategy + 1

    #define mod_reg_03_voltage_trans_strategy       mod_reg_03_switch_toggle_time + 1

    #define mod_reg_03_crc                          mod_reg_03_voltage_trans_strategy + 1

    #define mod_reg_03_num                          ((mod_reg_03_crc / 4) * 4 + 4)    //количество регистров

/*Scenario list*/

#define mess_x05_event_RESET_NRF52            0  
#define mess_x05_event_GIVE_ME_TIME           1 
#define mess_x05_event_TAKE_MY_TIME           2
#define mess_x05_event_RESET_ESP8266          3    
#define mess_x05_event_W5500_OFF              4    
#define mess_x05_event_W5500_dynamic          5    
#define mess_x05_event_W5500_static           6    
#define mess_x05_event_WIFI_OFF               7  
#define mess_x05_event_WIFI_STA_dynamic       8 
#define mess_x05_event_WIFI_STA_static        9  
#define mess_x05_event_WIFI_AP                10   
#define mess_x05_event_UPDATE                 11
#define mess_x05_event_ACKNOLEDGE_TRIP        12
#define mess_x05_event_ESP_HW_RESET           13
#define mess_x05_event_ETHERNET_DISCONNECT    14

/*SCENARIO DEFINES*/

#define WIFI_SETTINGS_WINDOW_SCENARIO       1
  /*Read tags events*/
  #define mess_x03_all_wifi_param_read      1 
  #define mess_x03_dyn_wifi_param_read      2
  /**/
#define ETHERNET_SETTINGS_WINDOW_SCENARIO   2
  /*Read tags events*/
  #define mess_x03_all_eth_param_read       1 
  #define mess_x03_dyn_eth_param_read       2
  /**/
#define MTZ_SETTINGS_WINDOW_SCENARIO        3
#define SYSTEM_MODE_WINDOW_SCENARIO         4
  /*Read tags events*/
  #define mess_x03_esp_version              1
  /**/
#define SYSTEM_TEST_WINDOW_SCENARIO         5
#define CALIBRATION_WINDOW_SCENARIO         6
#define MAIN_WINDOW_SCENARIO                7

#define EVENT_DISCRET_OUT_SCENARIO          1
#define EVENT_DISCRET_IN_SCENARIO           2
#define READ_REQUEST_SCENARIO               3
#define EVENT_X04_SCENARIO                  4
#define EVENT_SYSTEM_SCENARIO               5
#define WRITE_TAG_SCENARIO                  16
#define IEC_104_WINDOW_SCENARIO             104


    /******************************************************************/


#endif
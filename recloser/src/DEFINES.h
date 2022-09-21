#ifndef DEFINES_H__
#define DEFINES_H__
/*NRF FW version*/

/*RTC PIN*/
#define RTC_PIN                         16

/*ESP8266 RESET PIN*/
#define ESP_PIN                         17

/*TPL5010 PINS*/
#define TIM_WAKE                        46
#define TIM_DONE                        47

#define switch_di_tic                   50

/**/

#define byte_num                                  LEN * ADC_CHANNELS_TO_SEND + 2 * ADC_CHANNELS_TO_SEND + 2
#define max_32_bit_val                            0x7fffffff
//#define us_ticker                                 1003
#define timer2_ms                                 1000 //TIMER_2 ms
#define ms_to_tic(time, period)                   (time * 1000 / period)
#define ESP_PACK_INTERVEL_ms                      150
//#define MANUAL_ADC

//#define tag_irq_high_time       ms_to_tic(200,  timer2_ms)
//#define tag_irq_low_time        ms_to_tic(2000, timer2_ms)
//#define timers_control_time     ms_to_tic(1000, timer2_ms)

//#define di_asq_period           ms_to_tic(100,  timer2_ms) 
#define max_zero_val_counter    100        // количество точек дл€ вычислени€ нул€ измерени€
#define zero_cutt_off_val       400         // отсечка нул€


    #define mod_reg_03_default_cal_100_adc_cur_phase       5700//2561
    #define mod_reg_03_default_cal_100_adc_cur_3I0         5700//9962
    #define mod_reg_03_default_cal_100_adc_volt            5700//1675


    #define mod_reg_03_default_cal_0_adc_cur_phase         8192//7970
    #define mod_reg_03_default_cal_0_adc_cur_3I0           8192//9296
    #define mod_reg_03_default_cal_0_adc_volt              8192//7945

    #define mod_reg_03_default_cal_cur_phase_scale         2000              // 20  јћѕ≈–
    #define mod_reg_03_default_cal_cur_3I0_scale           500               // 5  јћѕ≈–
    #define mod_reg_03_default_cal_voltage_scale           3000              // 300 ¬ќЋ№“
    #define mod_reg_03_default_cal_voltage_x_scale         3000              // 300 ¬ќЋ№“

    #define mod_reg_03_default_cal_cur_scale_coef             50              
    #define mod_reg_03_default_cal_cur_3I0_scale_coef         20               
    #define mod_reg_03_default_cal_voltage_scale_coef         30             
    #define mod_reg_03_default_cal_voltage_x_scale_coef       30            

    #define mod_reg_03_MTZ_1_current_trip_default          140              
    #define mod_reg_03_MTZ_2_current_trip_default          140              
    #define mod_reg_03_MTZ_3_current_trip_default          360            
    #define mod_reg_03_TO_current_trip_default             400             
    #define mod_reg_03_ZMN_voltage_trip_default            4500             
    #define mod_reg_03_ZZ_current_trip_default             50             

    #define mod_reg_03_MTZ_1_current_back_default          1350             
    #define mod_reg_03_MTZ_2_current_back_default          1350              
    #define mod_reg_03_MTZ_3_current_back_default          3550             
    #define mod_reg_03_TO_current_back_default             3950               
    #define mod_reg_03_ZMN_voltage_back_default            4600              
    #define mod_reg_03_ZZ_current_back_default             45     
    
    #define mod_reg_03_APV_time_ready_default              5
    #define mod_reg_03_APV1_time_cycle_default             2    
    #define mod_reg_03_APV2_time_cycle_default             10    
    #define mod_reg_03_APV3_time_cycle_default             20   
    #define mod_reg_03_APV_ZMN_time_ready_default          10       
    #define mod_reg_03_APV_ZMN_time_back_default           15       

    #define mod_reg_03_MTZ_1_time_trip_default             10 // 10 с
    #define mod_reg_03_MTZ_2_time_trip_default             10 // 10 с
    #define mod_reg_03_MTZ_3_time_trip_default             ms_to_tic(1000,  timer2_ms) // 300 мс
    #define mod_reg_03_TO_time_trip_default                ms_to_tic(1000,  timer2_ms) // 500 мс
    #define mod_reg_03_ZMN_time_trip_default               ms_to_tic(1000,  timer2_ms) // 300 мс
    #define mod_reg_03_ZZ_time_trip_default                10 // 10 с

    #define mod_reg_03_MTZ_1_time_back_default             2 // 2 с
    #define mod_reg_03_MTZ_2_time_back_default             2 // 2 с
    #define mod_reg_03_MTZ_3_time_back_default             ms_to_tic(1000,  timer2_ms) // 500 мс
    #define mod_reg_03_TO_time_back_default                ms_to_tic(1000,  timer2_ms) // 500 мс
    #define mod_reg_03_ZMN_time_back_default               ms_to_tic(1000,  timer2_ms) // 500 мс
    #define mod_reg_03_ZZ_time_back_default                2 // 2 с

    #define mod_reg_03_trip_action_default                 0

    #define mod_reg_03_reserv                              0 //((1 << 12) + (4 << 8) + (2 << 4) + 3)
    #define mod_reg_03_switch_toggle_time_default          300
    #define mod_reg_03_switch_strategy_default             2

    #define mod_reg_03_voltage_trans_strategy_default      0x87

    /*protection events*/

    #define protection_MTZ_1                                   0
    #define protection_MTZ_2                                   1 
    #define protection_MTZ_3                                   2
    #define protection_TO                                      3
    #define protection_ZMN                                     4
    #define protection_ZZ                                      5

    #define protection_alarm                                   0
    #define protection_back_value                              1
    #define protection_back_timer                              2
    #define protection_trip                                    3


/*DI/DO defines*/

#define     DO_COIL_START           1
#define     DO_COIL_STOP            2
#define     DO_LOAD220_1            3
#define     DO_LOAD220_2            4
#define     DO_MOTOR                5
#define     DO_LAMP_OFF             6
#define     DO_LAMP_ON_ALARM        7
#define     DO_BATTERY              8
#define     DO_LED_1                9
#define     DO_LED_2                10

#define     DI_SAC                  1
#define     DI_SQ1                  2
#define     DI_GENERAL_OFF          3
#define     DI_GENERAL_ON           4
#define     DI_SQY                  5
#define     DI_SBT                  6
#define     DI_SBO                  7
#define     DI_SBR                  8
#define     DI_BATERY               9
#define     DI_START_COIL_HEALTH    10
#define     DI_STOP_COIL_HEALTH     11

#endif

#define LOG_INF_GLOB(...) NULL//LOG_INF(__VA_ARGS__)
#define LOG_WRN_GLOB(...) NULL//LOG_WRN(__VA_ARGS__)
#define LOG_ERR_GLOB(...) NULL//LOG_ERR(__VA_ARGS__)

#define IntToBytes(x)     x >> 8, x & 255
#define BytesToInt(x, y)  (x[y] << 8) + x[y + 1]
#ifndef CONTROL_LOGIC_H__
#define CONTROL_LOGIC_H__

#include <stdint.h>
#include <inttypes.h>

void General_Logic();
void General_Logic_Init(void *DO_TOGGLE_, void *SWITCHER_action_, int16_t *modbus_reg_03_, 
                        int16_t *modbus_reg_04_, uint8_t *main_window_updater_, void *MTZ_resume_procedure, void *INIT_ptr);
void INIT_STATUS();

/*General States*/

enum
{
    LOGIC_init_position,
    LOGIC_zero_position,
    LOGIC_run_servo_start,
    LOGIC_servo_wait_for_done,
    LOGIC_servo_failed,
    LOGIC_post_init,
    LOGIC_wait_for_remote_command,
    LOGIC_run_process,
    LOGIC_start_procces,
    LOGIC_servo_stick_control,
    LOGIC_start_procces_failed,
    LOGIC_stop_procces_failed,
    LOGIC_stop_procces,
    LOGIC_control_run,
    LOGIC_wait_for_acknoledge,
    LOGIC_wait_for_apv
};

/*General Logic errors*/

    #define error_missmatch_BB_swithers         0
    #define error_YAO_coil_fault                1
    #define error_YAT_coil_fault                2
    #define error_servo_fault                   3
    #define error_run_procces_timeout           4
    #define error_stop_procces_timeout          5
    #define error_MTZ_1                         6
    #define error_MTZ_2                         7
    #define error_MTZ_3                         8
    #define error_MTZ_TO                        9
    #define error_MTZ_ZMN                       10
    #define error_MTZ_ZZ                        11
    #define error_wait_for_servo                12
    #define error_BB_damaged                    13
    #define error_ZVN                           14



/* Time delay defines */
    
    #define DELAY_missmatch_BB_swithers         0
    #define DELAY_YAO_coil_fault                1
    #define DELAY_YAT_coil_fault                2
    #define DELAY_DI_SQY                        3
    #define DELAY_EXTERNAL_OFF                  4

    #define SERVO_TIMEOUT_ms                    15000
    #define SERVO_BLOCK_TIMEOUT_ms              500
    #define SERVO_STICK_TIMEOUT                 300
    #define START_BLOCK_TIMEOUT                 1000
    #define EXTERNAL_OFF_timeout_50ms           5
    #define DELAY_SWITCHERS_TIMEOUT_50ms        20
    #define BB_SWITCHER_DAMAGE_TIMEOUT_ms       300
    #define ZVN_TIMEOUT                         100

#endif
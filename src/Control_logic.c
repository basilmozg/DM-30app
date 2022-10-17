/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "control_logic.h"
#include "control_apv.h"
#include "registers_modbus.h"
#include "DEFINES.h"
#include "calculations.h"

#include <logging/log.h>
#include <zephyr.h>

LOG_MODULE_REGISTER(control_logic);

static int16_t *modbus_reg_04;
static int16_t *modbus_reg_03;
static int16_t last_error_equipment;
static void (*DO_TOGGLE_ptr)(uint8_t, uint8_t);
static void (*SWITCHER_action_ptr)(uint8_t);
static void (*MTZ_resume_procedure)();
static void (*INIT_ptr)();
static uint8_t *main_window_updater;
static uint8_t switcher_delay[6];
static uint16_t last_DI_states;
static uint8_t run_block;
static uint8_t servo_run_timer;
static uint8_t emergency_force_stop = LOGIC_wait_for_remote_command;
static uint8_t catch_servo_switch_stick;

uint16_t GLOBAL_start_command;
uint8_t General_State;
uint8_t Global_trip = 255;
uint8_t SWITCHER_ON_timeout_timer_flag;
uint8_t SWITCHER_OFF_timeout_timer_flag;
uint16_t current_system_mode;
int CURRENT[8];

uint8_t zvn_alarm;
uint8_t apv_zmn_trip;
/*APV*/


/**/

static void DO_TOGGLE(uint8_t do_channel, uint8_t state)
{
    (*DO_TOGGLE_ptr)(do_channel, state);
}

void INIT_STATUS()
{
    if(readBit(modbus_reg_03[mod_reg_03_system_mode], radio_on_test) != readBit(current_system_mode, radio_on_test))
    {
        if(readBit(modbus_reg_03[mod_reg_03_system_mode], radio_on_test))
        {
            bitClear(modbus_reg_03[mod_reg_03_system_mode], run_mode_enable);
            current_system_mode = modbus_reg_03[mod_reg_03_system_mode] & 0xBFC0;
        }
        else current_system_mode = modbus_reg_03[mod_reg_03_system_mode];
        /*DO init to ZERO*/
        for(uint8_t i = 1; i < 11; i++)
            DO_TOGGLE(i, 0);
        /**/
    }
    if(!readBit(modbus_reg_03[mod_reg_03_system_mode], esd_mode_enable))
    {
        if(!readBit(modbus_reg_03[mod_reg_03_system_mode], radio_on_test))
            if((modbus_reg_03[mod_reg_03_system_mode] & 0x7FF) != (current_system_mode & 0x7FF)) 
                current_system_mode = modbus_reg_03[mod_reg_03_system_mode];
        if(!readBit(modbus_reg_03[mod_reg_03_system_mode], run_mode_enable))
            current_system_mode = modbus_reg_03[mod_reg_03_system_mode] & 0xFFC0;
    }
    (*INIT_ptr)();
}

void Change_General_State(uint8_t state)
{ 
    switch(state)
    {
        case LOGIC_init_position:
            LOG_ERR_GLOB("LOGIC_init_position");
        break;
        case LOGIC_zero_position:
            LOG_ERR_GLOB("LOGIC_zero_position");
        break;
        case LOGIC_run_servo_start:
            LOG_ERR_GLOB("LOGIC_run_servo_start");
        break;
        case LOGIC_servo_wait_for_done:
            LOG_ERR_GLOB("LOGIC_servo_wait_for_done");
        break;
        case LOGIC_servo_failed:
            LOG_ERR_GLOB("LOGIC_servo_failed");
        break;
        case LOGIC_post_init:
            LOG_ERR_GLOB("LOGIC_post_init");
        break;
        case LOGIC_wait_for_remote_command:
            LOG_ERR_GLOB("LOGIC_wait_for_remote_command");
        break;
        case LOGIC_start_procces:
            LOG_ERR_GLOB("LOGIC_start_procces");
        break;
        case LOGIC_servo_stick_control:
            LOG_ERR_GLOB("LOGIC_servo_stick_control");
        break;
        case LOGIC_start_procces_failed:
            LOG_ERR_GLOB("LOGIC_start_procces_failed");
        break;
        case LOGIC_stop_procces_failed:
            LOG_ERR_GLOB("LOGIC_stop_procces_failed");
        break;
        case LOGIC_stop_procces:
            LOG_ERR_GLOB("LOGIC_stop_procces");
        break;
        case LOGIC_control_run:
            LOG_ERR_GLOB("LOGIC_control_run");
        break;
        case LOGIC_wait_for_acknoledge:
            LOG_ERR_GLOB("LOGIC_wait_for_acknoledge");
        break;
        case LOGIC_wait_for_apv:
            LOG_ERR_GLOB("LOGIC_wait_for_apv");
        break;
    }
    General_State = state;
}

void ERROR_init()
{
    modbus_reg_04[mod_reg_04_error_equipment] = 0;
    last_error_equipment = 0;
    switcher_delay[DELAY_missmatch_BB_swithers] = DELAY_SWITCHERS_TIMEOUT_50ms;
    switcher_delay[DELAY_YAO_coil_fault] = DELAY_SWITCHERS_TIMEOUT_50ms;
    switcher_delay[DELAY_YAT_coil_fault] = DELAY_SWITCHERS_TIMEOUT_50ms;
    switcher_delay[3] = DELAY_SWITCHERS_TIMEOUT_50ms;
    *main_window_updater = 2;
}

void ERROR_set(uint8_t error)
{
    LOG_ERR_GLOB("error %d", error);
    writebit(modbus_reg_04[mod_reg_04_error_equipment], error, 1);
    *main_window_updater = 2;
}

void ERROR_clear(uint8_t error)
{
    writebit(modbus_reg_04[mod_reg_04_error_equipment], error, 0);
    *main_window_updater = 2;
}

void servo_block_handler()
{
    if(General_State != LOGIC_servo_failed &&
       General_State != LOGIC_wait_for_acknoledge) 
    DO_TOGGLE(DO_MOTOR, 1);
}

K_TIMER_DEFINE(k_servo_block_timer, servo_block_handler, NULL);

void run_block_handler()
{
    run_block = 0;
}

K_TIMER_DEFINE(k_run_block_timer, run_block_handler, NULL);

void servo_run_handler()
{
    servo_run_timer = 2;
}

K_TIMER_DEFINE(k_servo_run_timer, servo_run_handler, NULL);

void check_for_servo_switch_handle()
{
    /*Long error for wait */
    if(catch_servo_switch_stick)
        Change_General_State(LOGIC_servo_failed);
    else Change_General_State(LOGIC_control_run);
}

K_TIMER_DEFINE(k_timer_servo_switch, check_for_servo_switch_handle, NULL);

void bb_switcher_damage_timeout()
{
    if(CURRENT[mod_reg_04_current_A] > zero_cutt_off_val || 
        CURRENT[mod_reg_04_current_B] > zero_cutt_off_val || 
        CURRENT[mod_reg_04_current_C] > zero_cutt_off_val)
        {        
            ERROR_set(error_BB_damaged);
            writebit(modbus_reg_03[mod_reg_03_system_mode], esd_mode_enable, 1);
            *main_window_updater = 2;
            if(General_State != LOGIC_wait_for_acknoledge)
            {
                DO_TOGGLE(DO_LAMP_ON_ALARM, 0);
                DO_TOGGLE(DO_LAMP_OFF, 0);
                Change_General_State(LOGIC_wait_for_acknoledge);
            }
        }
}

K_TIMER_DEFINE(k_bb_switcher_damage_timeout, bb_switcher_damage_timeout, NULL);

static void SWITCHER_action(uint8_t sw_act)
{
    if(readBit(modbus_reg_04[mod_reg_04_DO_states], (DO_MOTOR - 1)))
    {
        DO_TOGGLE(DO_MOTOR, 0);
        k_timer_start(&k_servo_block_timer, K_MSEC(SERVO_BLOCK_TIMEOUT_ms), K_NO_WAIT); 
    }
    (*SWITCHER_action_ptr)(sw_act); 
}

void ZVN_timeout()
{
    if(zvn_alarm == 1 && readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_GENERAL_OFF - 1)))
    {
        DO_TOGGLE(DO_MOTOR, 0);
        DO_TOGGLE(DO_LAMP_ON_ALARM, 0);
        DO_TOGGLE(DO_LAMP_OFF, 0);
        Change_General_State(LOGIC_wait_for_acknoledge);
        ERROR_set(error_ZVN);
        writebit(modbus_reg_03[mod_reg_03_system_mode], esd_mode_enable, 1);
        *main_window_updater = 2;
    }
}

K_TIMER_DEFINE(k_ZVN_timeout, ZVN_timeout, NULL);

void Servo_recharge()
{
    ERROR_set(error_wait_for_servo);
    if(servo_run_timer == 0) 
    {
        k_timer_start(&k_servo_run_timer, K_MSEC(SERVO_TIMEOUT_ms), K_NO_WAIT); 
        servo_run_timer = 1;
    }
}

void General_Logic()
{
    if(readBit(modbus_reg_03[mod_reg_03_system_mode], radio_on_test)) return;

    if(readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_BATERY - 1))) DO_TOGGLE(DO_BATTERY, 1);
    else DO_TOGGLE(DO_BATTERY, 0);

    /* ошибка концевых выключателя */ 

        if(!readBit(modbus_reg_04[mod_reg_04_DO_states], (DO_COIL_START - 1)) &&
           !readBit(modbus_reg_04[mod_reg_04_DO_states], (DO_COIL_STOP - 1)))
        {
        /* error_missmatch_BB_swithers */
                if(readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_GENERAL_ON - 1)) ==
                   readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_GENERAL_OFF - 1)))
                {
                    switcher_delay[DELAY_missmatch_BB_swithers]++;
                    if(switcher_delay[DELAY_missmatch_BB_swithers] >= DELAY_SWITCHERS_TIMEOUT_50ms && 
                       !readBit(modbus_reg_04[mod_reg_04_error_equipment], error_missmatch_BB_swithers)) 
                        ERROR_set(error_missmatch_BB_swithers);
                } else switcher_delay[DELAY_missmatch_BB_swithers] = 0;

        /* error_YAO_coil_fault */
                if(!readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_STOP_COIL_HEALTH - 1)))
                {
                    switcher_delay[DELAY_YAO_coil_fault]++;
                    if(switcher_delay[DELAY_YAO_coil_fault] >= DELAY_SWITCHERS_TIMEOUT_50ms && 
                       !readBit(modbus_reg_04[mod_reg_04_error_equipment], error_YAO_coil_fault)) 
                        ERROR_set(error_YAO_coil_fault);
                } else switcher_delay[DELAY_YAO_coil_fault] = 0;

        /* error_YAT_coil_fault */
                if(!readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_START_COIL_HEALTH - 1)))
                {
                    switcher_delay[DELAY_YAT_coil_fault]++;
                    if(switcher_delay[DELAY_YAT_coil_fault] >= DELAY_SWITCHERS_TIMEOUT_50ms && 
                       !readBit(modbus_reg_04[mod_reg_04_error_equipment], error_YAT_coil_fault)) 
                        ERROR_set(error_YAT_coil_fault);
                } else switcher_delay[DELAY_YAT_coil_fault] = 0;

        /* total error */
                if((modbus_reg_04[mod_reg_04_error_equipment] & 0x7) > 0 && 
                    last_error_equipment != modbus_reg_04[mod_reg_04_error_equipment])
                {
                    last_error_equipment = modbus_reg_04[mod_reg_04_error_equipment];
                    Change_General_State(LOGIC_wait_for_acknoledge);
                    DO_TOGGLE(DO_LAMP_OFF, 0);
                    DO_TOGGLE(DO_LAMP_ON_ALARM, 0);
                    writebit(modbus_reg_03[mod_reg_03_system_mode], esd_mode_enable, 1);
                }
        }
/*********************************/

/*External on/off control*/

    if(General_State != LOGIC_wait_for_acknoledge)
    {
        if((readBit(modbus_reg_03[mod_reg_03_system_mode], run_mode_enable)     && 
           !readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_GENERAL_ON - 1))   &&
            readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_GENERAL_OFF - 1))) ||
           (!readBit(modbus_reg_03[mod_reg_03_system_mode], run_mode_enable)    && 
            readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_GENERAL_ON - 1))   &&
            !readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_GENERAL_OFF - 1))))
        {
                
            switcher_delay[DELAY_EXTERNAL_OFF]++;
            if(switcher_delay[DELAY_EXTERNAL_OFF] >= EXTERNAL_OFF_timeout_50ms) 
            {
                Change_General_State(LOGIC_zero_position);
                switcher_delay[DELAY_EXTERNAL_OFF] = 0;
            }
        }
        else switcher_delay[DELAY_EXTERNAL_OFF] = 0;
    }
/*********************************/

/*SERVO CONTROL*/

    if(General_State != LOGIC_init_position &&
       General_State != LOGIC_zero_position &&
       General_State != LOGIC_run_servo_start &&
       General_State != LOGIC_servo_failed &&
       General_State != LOGIC_servo_wait_for_done &&
       General_State != LOGIC_servo_failed &&
       General_State != LOGIC_wait_for_acknoledge)
    {
        if(!readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_SQY - 1)))
        {
            switch(servo_run_timer) 
            {
                case 0:
                    switcher_delay[DELAY_DI_SQY]++;
                    if(switcher_delay[DELAY_DI_SQY] >= DELAY_SWITCHERS_TIMEOUT_50ms) 
                    {
                        if(General_State == LOGIC_wait_for_remote_command) Change_General_State(LOGIC_run_servo_start);
                        else Servo_recharge();
                    }
                break;
                case 1:
                    if(General_State == LOGIC_wait_for_remote_command) 
                    {
                        Change_General_State(LOGIC_servo_wait_for_done);
                    }
                break;
                case 2:
                    servo_run_timer = 0;
                    Change_General_State(LOGIC_servo_failed);
                break;
            }
        }
        else 
        {
            switcher_delay[DELAY_DI_SQY] = 0;
            if(servo_run_timer != 0)
            {
                k_timer_stop(&k_servo_run_timer); 
                servo_run_timer = 0;
                ERROR_clear(error_wait_for_servo);
            }
        }
    }

    /*ZMN control*/
    if(readBit(modbus_reg_04[mod_reg_04_apv_ready], radio_apv_zmn_active) && 
       readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_GENERAL_OFF - 1)))
    {
        if(!apv_zmn_trip) apv_zmn_await();
        else apv_zmn_await_stop();
    }
            
    /*ZVN control*/

    if(zvn_alarm == 1 && !readBit(modbus_reg_04[mod_reg_04_error_equipment], error_ZVN) 
                      && readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_GENERAL_OFF - 1))) 
    {
        if(k_timer_remaining_ticks(&k_ZVN_timeout) == 0)
        {
            k_timer_start(&k_ZVN_timeout, K_MSEC(ZVN_TIMEOUT), K_NO_WAIT); 
        }
    }

    /*ESD CONTROL*/

    if(Global_trip < 255 && apv_ready() == 0)
    {
        Change_General_State(LOGIC_control_run);
    }

/*********************************/

    /*BUTTONS CONTROL*/

    /*DI_SBR button*/

    if(readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_SBR - 1)))
    {
        if(!readBit(last_DI_states, (DI_SBR - 1)))
        {
            writebit(last_DI_states, (DI_SBR - 1), 1);
            if(General_State == LOGIC_wait_for_acknoledge)
            {
                writebit(modbus_reg_03[mod_reg_03_system_mode], esd_mode_enable, 0);
                modbus_reg_04[mod_reg_04_alarm_equipment] = 0;
            }
            else
            {
                modbus_reg_04[mod_reg_04_alarm_equipment] = 0;
                INIT_STATUS();
                *main_window_updater = 2;
            }
        }
    }
    else writebit(last_DI_states, (DI_SBR - 1), 0);

    /*SBT button*/

    if(General_State == LOGIC_wait_for_remote_command)
    {
        if(readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_SBT - 1)))
        {
            if(!readBit(last_DI_states, (DI_SBT - 1)))
            {
                writebit(last_DI_states, (DI_SBT - 1), 1);
                if(!readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_SBO - 1)))
                {
                    apv_init(modbus_reg_03, modbus_reg_04, main_window_updater);
                    GLOBAL_start_command = 1;  
                }
            }
        }
        else writebit(last_DI_states, (DI_SBT - 1), 0);
    }

     /*SBO button*/

    if(readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_SBO - 1)) || GLOBAL_start_command == 0) 
    {
        if(!readBit(last_DI_states, (DI_SBO - 1)))
        {
            writebit(last_DI_states, (DI_SBO - 1), 1);
            apv_init(modbus_reg_03, modbus_reg_04, main_window_updater);
            if(readBit(modbus_reg_03[mod_reg_03_system_mode], esd_mode_enable)) 
            {
                SWITCHER_action(0);
                emergency_force_stop = LOGIC_wait_for_acknoledge;
                Change_General_State(LOGIC_stop_procces);
            }
            else
            {
                if(General_State != LOGIC_control_run) Change_General_State(LOGIC_control_run);
                GLOBAL_start_command = 0;
            }
        } 
    }
    else writebit(last_DI_states, (DI_SBO - 1), 0);

    /*BB damage control*/

    if(!readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_GENERAL_ON - 1)))
        if(!readBit(modbus_reg_04[mod_reg_04_error_equipment], error_BB_damaged))
            if(CURRENT[mod_reg_04_current_A] > zero_cutt_off_val || 
                CURRENT[mod_reg_04_current_B] > zero_cutt_off_val || 
                CURRENT[mod_reg_04_current_C] > zero_cutt_off_val)
                {
                    if(k_timer_remaining_get(&k_bb_switcher_damage_timeout) == 0)
                        k_timer_start(&k_bb_switcher_damage_timeout, K_MSEC(modbus_reg_03[mod_reg_03_switch_toggle_time]), K_NO_WAIT); 
                }
    /**/

   
/*********************************/

    switch(General_State)
    {
        case LOGIC_init_position:
            if(!readBit(modbus_reg_03[mod_reg_03_system_mode], esd_mode_enable))
            {
                Change_General_State(LOGIC_zero_position);
                ERROR_init();
            }
            else Change_General_State(LOGIC_wait_for_acknoledge);
        break;

        case LOGIC_zero_position:
            apv_init(modbus_reg_03, modbus_reg_04, main_window_updater);
            if(readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_GENERAL_OFF - 1)))
            {
                writebit(modbus_reg_03[mod_reg_03_system_mode], run_mode_enable, 0);
                DO_TOGGLE(DO_LAMP_OFF, 1);
            }

            if(readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_GENERAL_ON - 1))) 
            {
                GLOBAL_start_command = 1;
                DO_TOGGLE(DO_LAMP_OFF, 0);
                DO_TOGGLE(DO_LAMP_ON_ALARM, 1);
                (*MTZ_resume_procedure)();
                writebit(modbus_reg_03[mod_reg_03_system_mode], run_mode_enable, 1);
            }
            Change_General_State(LOGIC_run_servo_start);

        break;

        case LOGIC_run_servo_start:
            if(!readBit(modbus_reg_04[mod_reg_04_DO_states], (DO_MOTOR - 1)))
                DO_TOGGLE(DO_MOTOR, 1);
            if(!readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_SQY - 1)))
            {
                Change_General_State(LOGIC_servo_wait_for_done);
                Servo_recharge();
            }
            else Change_General_State(LOGIC_post_init);
        break;

        case LOGIC_servo_wait_for_done:
            if(readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_SQY - 1)))
            {
                k_timer_stop(&k_servo_run_timer); 
                servo_run_timer = 0;
                Change_General_State(LOGIC_post_init);
                ERROR_clear(error_wait_for_servo);
            }
            if(servo_run_timer == 2)
            {
                servo_run_timer = 0;
                Change_General_State(LOGIC_servo_failed);
            }
        break;

        case LOGIC_servo_failed:
            DO_TOGGLE(DO_MOTOR, 0);
            DO_TOGGLE(DO_LAMP_ON_ALARM, 0);
            DO_TOGGLE(DO_LAMP_OFF, 0);
            Change_General_State(LOGIC_wait_for_acknoledge);
            ERROR_clear(error_wait_for_servo);
            catch_servo_switch_stick = 0;
            ERROR_set(error_servo_fault);
            writebit(modbus_reg_03[mod_reg_03_system_mode], esd_mode_enable, 1);
            *main_window_updater = 2;
        break;

        case LOGIC_post_init:
            GLOBAL_start_command = 2;
            servo_run_timer = 0;
            if(readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_GENERAL_ON - 1)))
                Change_General_State(LOGIC_start_procces);
            else
                Change_General_State(LOGIC_stop_procces);
        break;

        case LOGIC_wait_for_remote_command:
        {
            if(GLOBAL_start_command == 1)
            {
                Change_General_State(LOGIC_run_process);
            }    
            else GLOBAL_start_command = 2;
        }
        break;

        case LOGIC_run_process:
            if(!run_block)
            {/*
                if(apv_zmn_trip && readBit(modbus_reg_03[mod_reg_03_system_mode], radio_on_zmn))
                {
                    Global_trip = 4;
                    ERROR_set(Global_trip + error_MTZ_1);
                    DO_TOGGLE(DO_LAMP_ON_ALARM, 0);
                    DO_TOGGLE(DO_LAMP_OFF, 0);
                    Change_General_State(LOGIC_wait_for_acknoledge);
                    *main_window_updater = 2;
                    writebit(modbus_reg_03[mod_reg_03_system_mode], esd_mode_enable, 1);
                    return;
                }*/
                (*MTZ_resume_procedure)();
                SWITCHER_action(GLOBAL_start_command);
                Change_General_State(LOGIC_start_procces);
                catch_servo_switch_stick = 2;
                GLOBAL_start_command = 2;
            }
        break;

        case LOGIC_start_procces:
            apv_start_for_ready();
            if(readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_GENERAL_ON - 1)))
            {
                DO_TOGGLE(DO_LAMP_OFF, 0);
                DO_TOGGLE(DO_LAMP_ON_ALARM, 1);
                writebit(modbus_reg_03[mod_reg_03_system_mode], run_mode_enable, 1);
                *main_window_updater = 2;
                INIT_STATUS();
                if(catch_servo_switch_stick == 2)
                {
                    k_timer_start(&k_timer_servo_switch, K_MSEC(SERVO_STICK_TIMEOUT), K_NO_WAIT);
                    Change_General_State(LOGIC_servo_stick_control);
                    catch_servo_switch_stick = 1;
                }
                else Change_General_State(LOGIC_control_run);
            }
            else
            if(SWITCHER_ON_timeout_timer_flag == 2) Change_General_State(LOGIC_start_procces_failed);
        break;

        case LOGIC_servo_stick_control:
            if(!readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_SQY - 1)))
                catch_servo_switch_stick = 0;
        break;

        case LOGIC_stop_procces:
            if(readBit(modbus_reg_04[mod_reg_04_DI_states], (DI_GENERAL_OFF - 1)))
            {
                
                DO_TOGGLE(DO_LAMP_ON_ALARM, 0);
                Change_General_State(emergency_force_stop);
                if(emergency_force_stop == LOGIC_wait_for_acknoledge)
                {
                    DO_TOGGLE(DO_LAMP_OFF, 0);
                }
                else DO_TOGGLE(DO_LAMP_OFF, 1);
                emergency_force_stop = LOGIC_wait_for_remote_command;
                writebit(modbus_reg_03[mod_reg_03_system_mode], run_mode_enable, 0);
                *main_window_updater = 2;
                INIT_STATUS();
            }
            else
            if(SWITCHER_OFF_timeout_timer_flag == 2) Change_General_State(LOGIC_stop_procces_failed);
        break;

        case LOGIC_start_procces_failed:
            ERROR_set(error_run_procces_timeout);
            Change_General_State(LOGIC_wait_for_acknoledge);
            writebit(modbus_reg_03[mod_reg_03_system_mode], esd_mode_enable, 1);
        break;

        case LOGIC_stop_procces_failed:
            ERROR_set(error_stop_procces_timeout);
            Change_General_State(LOGIC_wait_for_acknoledge);
            writebit(modbus_reg_03[mod_reg_03_system_mode], esd_mode_enable, 1);
        break;

        case LOGIC_control_run:
            if(readBit(modbus_reg_03[mod_reg_03_system_mode], esd_mode_enable))
            {
                DO_TOGGLE(DO_LAMP_ON_ALARM, 0);
                DO_TOGGLE(DO_LAMP_OFF, 0);
                LOG_INF_GLOB("pre catch apv");
                if(Global_trip < 255) 
                {
                    if(Global_trip < 4 && 
                        apv_cycle_current_global() < 3 && 
                        apv_init_ready() && 
                        apv_start_for_cycle() != 1 &&
                        readBit(modbus_reg_04[mod_reg_04_trip_action], (Global_trip + 6)))
                    {
                        LOG_INF_GLOB("catch apv");
                        Change_General_State(LOGIC_wait_for_apv);
                        Global_trip = 255;
                        return;
                    }
                    else
                    if(Global_trip == 4 && apv_zmn_init_ready())
                    {
                        LOG_INF_GLOB("catch apv");
                        Change_General_State(LOGIC_wait_for_apv);
                        writebit(modbus_reg_04[mod_reg_04_apv_ready], radio_apv_zmn_active, 1);
                        writebit(modbus_reg_04[mod_reg_04_apv_ready], radio_ready_apv_zmn, 0);
                        Global_trip = 255;
                        return;
                    }
                    ERROR_set(Global_trip + error_MTZ_1);
                }
                Change_General_State(LOGIC_wait_for_acknoledge);
                *main_window_updater = 2;
            }
            Global_trip = 255;

            if(GLOBAL_start_command == 0)
            {
                switcher_delay[DELAY_DI_SQY] = 0;
                SWITCHER_action(GLOBAL_start_command);
                Change_General_State(LOGIC_stop_procces);
                GLOBAL_start_command = 2;
                run_block = 1;
                k_timer_start(&k_run_block_timer, K_MSEC(START_BLOCK_TIMEOUT), K_NO_WAIT); 
            }
            else 
            {
                GLOBAL_start_command = 2;
            }

        break;

        case LOGIC_wait_for_acknoledge:
            if(!readBit(modbus_reg_03[mod_reg_03_system_mode], esd_mode_enable))
            {
                DO_TOGGLE(DO_LAMP_ON_ALARM, 0);
                DO_TOGGLE(DO_LAMP_OFF, 1);
                Change_General_State(LOGIC_init_position);
                ERROR_init();
            }
            else emergency_disable_apv();
            GLOBAL_start_command = 2;
        break;
        case LOGIC_wait_for_apv:
            if(apv_cycle() > 0 || apv_zmn_start_event())
            {
                Global_trip = 255;
                apv_zmn_reset();
                writebit(modbus_reg_03[mod_reg_03_system_mode], esd_mode_enable, 0);
                Change_General_State(LOGIC_run_process);
                GLOBAL_start_command = 1;
            }
        break;
    }
}

void General_Logic_Init(void *DO_TOGGLE_, void *SWITCHER_action_, int16_t *modbus_reg_03_, 
                        int16_t *modbus_reg_04_, uint8_t *main_window_updater_, void *MTZ_resume_procedure_, void *INIT_ptr_)
{
    DO_TOGGLE_ptr = DO_TOGGLE_;
    SWITCHER_action_ptr = SWITCHER_action_;
    MTZ_resume_procedure = MTZ_resume_procedure_;
    INIT_ptr = INIT_ptr_;
    modbus_reg_03 = modbus_reg_03_;
    modbus_reg_04 = modbus_reg_04_;
    main_window_updater = main_window_updater_;
    GLOBAL_start_command = 2;
    LOG_INF_GLOB("Logic init");
}


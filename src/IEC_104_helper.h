#include <stdint.h>
#include <inttypes.h>
#include "registers_modbus.h"

#define TAG_GROUP_1_NUMBER                    19
uint16_t TAG_GROUP_1[2][TAG_GROUP_1_NUMBER] = 
{
    {
        mod_reg_04_current_A,
        mod_reg_04_current_B,
        mod_reg_04_current_C,
        mod_reg_04_current_3I0,
        mod_reg_04_voltage_A,
        mod_reg_04_voltage_B,
        mod_reg_04_voltage_C,
        mod_reg_04_voltage_x,
        mod_reg_04_angle_currentA_voltageA,
        mod_reg_04_angle_currentB_voltageB,
        mod_reg_04_angle_currentC_voltageC,
        mod_reg_04_current_A_primary,
        mod_reg_04_current_B_primary,
        mod_reg_04_current_C_primary,
        mod_reg_04_current_3I0_primary,
        mod_reg_04_voltage_A_primary,
        mod_reg_04_voltage_B_primary,
        mod_reg_04_voltage_C_primary,
        mod_reg_04_voltage_x_primary
    },
    {
        100,
        100,
        100,
        100,
        10,
        10,
        10,
        10,
        1,
        1,
        1,
        1,
        1,
        1,
        10,
        1000,
        1000,
        1000,
        1000
    }
};

#define TAG_GROUP_2_NUMBER                    18
uint16_t TAG_GROUP_2[2][TAG_GROUP_2_NUMBER] = 
{
    {
        mod_reg_04_MTZ_1_current_trip,
        mod_reg_04_MTZ_2_current_trip,
        mod_reg_04_MTZ_3_current_trip,
        mod_reg_04_TO_current_trip,
        mod_reg_04_ZMN_voltage_trip,
        mod_reg_04_ZZ_current_trip,

        mod_reg_04_MTZ_1_current_back,
        mod_reg_04_MTZ_2_current_back,
        mod_reg_04_MTZ_3_current_back,
        mod_reg_04_TO_current_back,
        mod_reg_04_ZMN_voltage_back,
        mod_reg_04_ZZ_current_back,

        mod_reg_04_MTZ_1_time_trip,
        mod_reg_04_MTZ_2_time_trip,
        mod_reg_04_MTZ_3_time_trip,
        mod_reg_04_TO_time_trip,
        mod_reg_04_ZMN_time_trip,
        mod_reg_04_ZZ_time_trip,
    },
    {
        1,
        1,
        1,
        1,
        1,
        10,
        1,
        1,
        1,
        1,
        1,
        10,
        1,
        1,
        1,
        1,
        1,
        10
    }
};

#define TAG_GROUP_3_NUMBER                    12
uint16_t TAG_GROUP_3[2][TAG_GROUP_3_NUMBER] = 
{
    {
        mod_reg_04_MTZ_1_time_back,
        mod_reg_04_MTZ_2_time_back,
        mod_reg_04_MTZ_3_time_back,
        mod_reg_04_TO_time_back,
        mod_reg_04_ZMN_time_back,
        mod_reg_04_ZZ_time_back,

        mod_reg_04_APV_time_ready,
        mod_reg_04_APV1_time_cycle,
        mod_reg_04_APV2_time_cycle,
        mod_reg_04_APV3_time_cycle,

        mod_reg_04_APV_ZMN_time_ready,
        mod_reg_04_APV_ZMN_time_back
    },
    {
        1,
        1,
        1,
        1,
        1,
        10,
        1,
        1,
        1,
        1,
        1,
        1
    }
};

#define TAG_GROUP_4_NUMBER                    12
uint16_t TAG_GROUP_4[2][TAG_GROUP_4_NUMBER] = 
{
    {
        mod_reg_03_A_MTZ_1_current_trip,
        mod_reg_03_A_MTZ_2_current_trip,
        mod_reg_03_A_MTZ_3_current_trip,
        mod_reg_03_A_TO_current_trip,
        mod_reg_03_A_ZMN_voltage_trip,
        mod_reg_03_A_ZZ_current_trip,

        mod_reg_03_A_MTZ_1_current_back,
        mod_reg_03_A_MTZ_2_current_back,
        mod_reg_03_A_MTZ_3_current_back,
        mod_reg_03_A_TO_current_back,
        mod_reg_03_A_ZMN_voltage_back,
        mod_reg_03_A_ZZ_current_back
    },
    {
        1,
        1,
        1,
        1,
        1,
        10,
        1,
        1,
        1,
        1,
        1,
        10
    }
};

#define TAG_GROUP_5_NUMBER                    18
uint16_t TAG_GROUP_5[2][TAG_GROUP_5_NUMBER] = 
{
    {
        mod_reg_03_A_MTZ_1_time_trip,
        mod_reg_03_A_MTZ_2_time_trip,
        mod_reg_03_A_MTZ_3_time_trip,
        mod_reg_03_A_TO_time_trip,
        mod_reg_03_A_ZMN_time_trip,
        mod_reg_03_A_ZZ_time_trip,

        mod_reg_03_A_MTZ_1_time_back,
        mod_reg_03_A_MTZ_2_time_back,
        mod_reg_03_A_MTZ_3_time_back,
        mod_reg_03_A_TO_time_back,
        mod_reg_03_A_ZMN_time_back,
        mod_reg_03_A_ZZ_time_back,

        mod_reg_03_A_APV_time_ready,
        mod_reg_03_A_APV1_time_cycle,
        mod_reg_03_A_APV2_time_cycle,
        mod_reg_03_A_APV3_time_cycle,

        mod_reg_03_A_APV_ZMN_time_ready,
        mod_reg_03_A_APV_ZMN_time_back
    },
    {
        1,
        1,
        1,
        1,
        1,
        10,
        1,
        1,
        1,
        1,
        1,
        10,
        1,
        1,
        1,
        1,
        1,
        1
    }
};

#define TAG_GROUP_6_NUMBER                    12
uint16_t TAG_GROUP_6[2][TAG_GROUP_6_NUMBER] = 
{
    {
        mod_reg_03_B_MTZ_1_current_trip,
        mod_reg_03_B_MTZ_2_current_trip,
        mod_reg_03_B_MTZ_3_current_trip,
        mod_reg_03_B_TO_current_trip,
        mod_reg_03_B_ZMN_voltage_trip,
        mod_reg_03_B_ZZ_current_trip,

        mod_reg_03_B_MTZ_1_current_back,
        mod_reg_03_B_MTZ_2_current_back,
        mod_reg_03_B_MTZ_3_current_back,
        mod_reg_03_B_TO_current_back,
        mod_reg_03_B_ZMN_voltage_back,
        mod_reg_03_B_ZZ_current_back
    },
    {
        1,
        1,
        1,
        1,
        1,
        10,
        1,
        1,
        1,
        1,
        1,
        10
    }
};

#define TAG_GROUP_7_NUMBER                    18
uint16_t TAG_GROUP_7[2][TAG_GROUP_7_NUMBER] = 
{
    {
        mod_reg_03_B_MTZ_1_time_trip,
        mod_reg_03_B_MTZ_2_time_trip,
        mod_reg_03_B_MTZ_3_time_trip,
        mod_reg_03_B_TO_time_trip,
        mod_reg_03_B_ZMN_time_trip,
        mod_reg_03_B_ZZ_time_trip,

        mod_reg_03_B_MTZ_1_time_back,
        mod_reg_03_B_MTZ_2_time_back,
        mod_reg_03_B_MTZ_3_time_back,
        mod_reg_03_B_TO_time_back,
        mod_reg_03_B_ZMN_time_back,
        mod_reg_03_B_ZZ_time_back,

        mod_reg_03_B_APV_time_ready,
        mod_reg_03_B_APV1_time_cycle,
        mod_reg_03_B_APV2_time_cycle,
        mod_reg_03_B_APV3_time_cycle,

        mod_reg_03_B_APV_ZMN_time_ready,
        mod_reg_03_B_APV_ZMN_time_back
    },
    {
        1,
        1,
        1,
        1,
        1,
        10,
        1,
        1,
        1,
        1,
        1,
        10,
        1,
        1,
        1,
        1,
        1,
        1
    }
};

#define TAG_GROUP_8_NUMBER                    2
uint16_t TAG_GROUP_8[2][TAG_GROUP_8_NUMBER] = 
{
    {
        mod_reg_04_alarm_equipment,
        mod_reg_04_error_equipment
    },
    {
        16,
        15
    }
};

#define TAG_GROUP_9_NUMBER                    4
uint16_t TAG_GROUP_9[2][TAG_GROUP_9_NUMBER] = 
{
    {
        mod_reg_04_trip_action,
        mod_reg_04_DI_states,
        mod_reg_04_DO_states,
        mod_reg_04_apv_ready
    },
    {
        10,
        11,
        10,
        3
    }
};

#define TAG_GROUP_10_NUMBER                    3
uint16_t TAG_GROUP_10[2][TAG_GROUP_10_NUMBER] = 
{
    {
        mod_reg_03_A_trip_action,
        mod_reg_03_B_trip_action,
        mod_reg_03_system_mode
    },
    {
        10,
        10,
        16
    }
};

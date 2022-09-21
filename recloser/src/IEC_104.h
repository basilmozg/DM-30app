/* Message type IDs */
#include <stdint.h>
#include <inttypes.h>

/*IEC 104 defaults*/
    #define mod_reg_03_IEC_COT_config_default              9  // 9 or 8
    #define mod_reg_03_IEC_COA_config_default              2  // 2 or 1
    #define IEC_w_param                                    5 
    #define IEC_w_period                                   (5 /*sec*/ * 1000 / 100)
    #define IEC_k_param                                    20  

#define M_SP_NA_1       1
#define M_SP_TA_1       2
#define M_DP_NA_1       3
#define M_DP_TA_1       4
#define M_ST_NA_1       5
#define M_ST_TA_1       6
#define M_BO_NA_1       7
#define M_BO_TA_1       8
#define M_ME_NA_1       9
#define M_ME_TA_1       10
#define M_ME_NB_1       11
#define M_ME_TB_1       12
#define M_ME_NC_1       13
#define M_ME_TC_1       14
#define M_IT_NA_1       15
#define M_IT_TA_1       16
#define M_EP_TA_1       17
#define M_EP_TB_1       18
#define M_EP_TC_1       19
#define M_PS_NA_1       20
#define M_ME_ND_1       21
#define M_SP_TB_1       30
#define M_DP_TB_1       31
#define M_ST_TB_1       32
#define M_BO_TB_1       33
#define M_ME_TD_1       34
#define M_ME_TE_1       35
#define M_ME_TF_1       36
#define M_IT_TB_1       37
#define M_EP_TD_1       38
#define M_EP_TE_1       39
#define M_EP_TF_1       40
#define S_IT_TC_1       41
#define C_SC_NA_1       45
#define C_DC_NA_1       46
#define C_RC_NA_1       47
#define C_SE_NA_1       48
#define C_SE_NB_1       49
#define C_SE_NC_1       50
#define C_BO_NA_1       51
#define C_SC_TA_1       58
#define C_DC_TA_1       59
#define C_RC_TA_1       60
#define C_SE_TA_1       61
#define C_SE_TB_1       62
#define C_SE_TC_1       63
#define C_BO_TA_1       64
#define M_EI_NA_1       70
#define S_CH_NA_1       81
#define S_RP_NA_1       82
#define S_AR_NA_1       83
#define S_KR_NA_1       84
#define S_KS_NA_1       85
#define S_KC_NA_1       86
#define S_ER_NA_1       87
#define S_US_NA_1       90
#define S_UQ_NA_1       91
#define S_UR_NA_1       92
#define S_UK_NA_1       93
#define S_UA_NA_1       94
#define S_UC_NA_1       95
#define C_IC_NA_1       100
#define C_CI_NA_1       101
#define C_RD_NA_1       102
#define C_CS_NA_1       103
#define C_TS_NA_1       104
#define C_RP_NA_1       105
#define C_CD_NA_1       106
#define C_TS_TA_1       107
#define P_ME_NA_1       110
#define P_ME_NB_1       111
#define P_ME_NC_1       112
#define P_AC_NA_1       113
#define F_FR_NA_1       120
#define F_SR_NA_1       121
#define F_SC_NA_1       122
#define F_LS_NA_1       123
#define F_AF_NA_1       124
#define F_SG_NA_1       125
#define F_DR_TA_1       126
#define F_SC_NB_1       127

/* QualityDescriptorP */

#define IEC60870_QUALITY_GOOD                 0
#define IEC60870_QUALITY_OVERFLOW             0x01 
#define IEC60870_QUALITY_RESERVED             0x04 
#define IEC60870_QUALITY_ELAPSED_TIME_INVALID 0x08 
#define IEC60870_QUALITY_BLOCKED              0x10 
#define IEC60870_QUALITY_SUBSTITUTED          0x20 
#define IEC60870_QUALITY_NON_TOPICAL          0x40 
#define IEC60870_QUALITY_INVALID              0x80 

//Cause Of Transmission
#define eIEC870_COT_UNUSED                    0
#define eIEC870_COT_CYCLIC                    1
#define eIEC870_COT_BACKGROUND                2
#define eIEC870_COT_SPONTAN                   3
#define eIEC870_COT_INIT                      4
#define eIEC870_COT_REQ                       5
#define eIEC870_COT_ACT                       6
#define eIEC870_COT_ACT_CON                   7
#define eIEC870_COT_DEACT                     8
#define eIEC870_COT_DEACT_CON                 9
#define eIEC870_COT_ACT_TERM                  10
#define eIEC870_COT_RETREM                    11
#define eIEC870_COT_RETLOC                    12
#define eIEC870_COT_FILE                      13
#define eIEC870_COT_INROGEN                   20
#define eIEC870_COT_INRO1                     21
#define eIEC870_COT_INRO2                     22
#define eIEC870_COT_INRO3                     23
#define eIEC870_COT_INRO4                     24
#define eIEC870_COT_INRO5                     25
#define eIEC870_COT_INRO6                     26
#define eIEC870_COT_INRO7                     27
#define eIEC870_COT_INRO8                     28
#define eIEC870_COT_INRO9                     29
#define eIEC870_COT_INRO10                    30
#define eIEC870_COT_INRO11                    31
#define eIEC870_COT_INRO12                    32
#define eIEC870_COT_INRO13                    33
#define eIEC870_COT_INRO14                    34
#define eIEC870_COT_INRO15                    35
#define eIEC870_COT_INRO16                    36
#define eIEC870_COT_REQCOGEN                  37
#define eIEC870_COT_REQCO1                    38
#define eIEC870_COT_REQCO2                    39
#define eIEC870_COT_REQCO3                    40
#define eIEC870_COT_REQCO4                    41
#define eIEC870_COT_UNKNOWN_TYPE              44
#define eIEC870_COT_UNKNOWN_CAUSE             45
#define eIEC870_COT_UNKNOWN_ASDU_ADDRESS      46
#define eIEC870_COT_UNKNOWN_OBJECT_ADDRESS    47

#define TAG_GROUP_1_NUMBER                    19


void IEC_init(int16_t *modbus_reg_04_, int16_t *modbus_reg_03_, void *EVENT_SEND_);

void IEC_104_parsing(uint8_t *data_in);

/***********************************/


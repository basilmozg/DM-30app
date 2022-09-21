/* Message type IDs */
#include "IEC_104.h"
#include "RTC_control.h"
#include "registers_modbus.h"
#include "calculations.h"
#include <logging/log.h>
#include <zephyr.h>

LOG_MODULE_REGISTER(IEC_104_log);

static int16_t *modbus_reg_04;

static int16_t *modbus_reg_03;
static uint8_t ACK_w;
static uint8_t ACK_k; 
static uint8_t ACK_w_timer;
static uint8_t IEC_ORG;

static void (*EVENT_SEND_ptr)(uint8_t , uint8_t, int16_t, uint8_t *);


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

/*IEC104 vars*/
int V_S, V_R;
uint32_t IEC_position = 0;
/**/

void IEC_init(int16_t *modbus_reg_04_, int16_t *modbus_reg_03_, void *EVENT_SEND_)
{
    modbus_reg_04 = modbus_reg_04_;
    modbus_reg_03 = modbus_reg_03_;
    EVENT_SEND_ptr = EVENT_SEND_;
}

static void EVENT_SEND(uint8_t event_type, uint8_t event_number, int16_t tag_value, uint8_t *data_transite)
{
    (*EVENT_SEND_ptr)(event_type, event_number, tag_value, data_transite);
}

void IEC_real_to_CP56(uint8_t *cp56_part)
{
    int16_t loc_msec;
    RTC_millis_get(&loc_msec);
    loc_msec += *(modbus_reg_04 + mod_reg_04_RTC_second) * 1000;
    *(cp56_part)     = loc_msec &255;
    *(cp56_part + 1) = loc_msec >> 8;
    *(cp56_part + 2) = *(modbus_reg_04 + mod_reg_04_RTC_minute) | 0x80; //real time???
    *(cp56_part + 3) = *(modbus_reg_04 + mod_reg_04_RTC_hour);  
    *(cp56_part + 4) = *(modbus_reg_04 + mod_reg_04_RTC_date) | (*(modbus_reg_04 + mod_reg_04_RTC_day) << 5);
    *(cp56_part + 5) = *(modbus_reg_04 + mod_reg_04_RTC_month);
    *(cp56_part + 6) = *(modbus_reg_04 + mod_reg_04_RTC_year) - 2000;
}

void APCI_maker_104(uint8_t *data_apci, uint8_t data_len, uint16_t V_S, uint16_t V_R)
{
    * data_apci      = 104;
    *(data_apci + 1) = data_len;
    *(data_apci + 2) = (V_S << 1);
    *(data_apci + 3) = (V_S >> 7);
    *(data_apci + 4) = (V_R << 1);
    *(data_apci + 5) = (V_R >> 7);
}

void DUI_maker(uint8_t *data_dui, uint8_t IT, uint8_t NoO, uint8_t COT, uint8_t ORG, uint16_t COA)
{
    * data_dui      = IT;
    *(data_dui + 1) = NoO;
    *(data_dui + 2) = COT;
    *(data_dui + 3) = ORG;
    *(data_dui + 4) = COA &255;
    *(data_dui + 5) = COA >> 8;
}

/*telegram*/

void M_ME_NC_1_maker(uint8_t *data, uint32_t val_adr, uint8_t *value, uint8_t QDP, uint8_t *len)
{
    *(data + *len)     =  val_adr        & 255;
    *(data + *len + 1) = (val_adr >> 8)  & 255;
    *(data + *len + 2) = (val_adr >> 16) & 255;
    *(data + *len + 3) = *value;
    *(data + *len + 4) = *(value + 1);
    *(data + *len + 5) = *(value + 2);
    *(data + *len + 6) = *(value + 3);
    *(data + *len + 7) = QDP;
    *len = *len + 8;
}

void M_ME_TF_1_maker(uint8_t *data, uint32_t val_adr, uint8_t *value, uint8_t QDP, uint8_t *len)
{
    M_ME_NC_1_maker(data, val_adr, value, QDP, len);
    IEC_real_to_CP56(data + *len);
    *len = *len + 7;
}

void M_EI_NA_1_maker(uint8_t *data, uint32_t val_adr, uint8_t *value, uint8_t QDP, uint8_t *len)
{
    *(data + *len)     = 0;
    *(data + *len + 1) = 0;
    *(data + *len + 2) = 0;
    *(data + *len + 3) = 0;
    *len = *len + 4;
}

void C_IC_NA_1_maker(uint8_t *data, uint32_t val_adr, uint8_t *value, uint8_t QDP, uint8_t *len)
{
    *(data + *len)     = 0;
    *(data + *len + 1) = 0;
    *(data + *len + 2) = 0;
    *(data + *len + 3) = val_adr;
    *len = *len + 4;
}
/***********************************/

/*IEC_104*/

void IEC_CP56_to_real(uint8_t *cp56_part)
{
    modbus_reg_04[mod_reg_04_RTC_year]   = (*(cp56_part + 6) + 2000);
    modbus_reg_04[mod_reg_04_RTC_month]  = (*(cp56_part + 5) & 0x0F);
    modbus_reg_04[mod_reg_04_RTC_date]   = (*(cp56_part + 4) & 0x1F);
    modbus_reg_04[mod_reg_04_RTC_day]    = (*(cp56_part + 4) >> 5) & 0x07; 
    modbus_reg_04[mod_reg_04_RTC_hour]   = (*(cp56_part + 3)) & 0x1F; //??
    modbus_reg_04[mod_reg_04_RTC_minute] = (*(cp56_part + 2)) & 0x3F; //??
    modbus_reg_04[mod_reg_04_RTC_second] = (*cp56_part + (*(cp56_part + 1) << 8))/1000;
    modbus_reg_04[mod_reg_04_RTC_millis] = (*cp56_part + (*(cp56_part + 1) << 8))%1000;
    EVENT_SEND(EVENT_SYSTEM_SCENARIO, mess_x05_event_TAKE_MY_TIME, 0, NULL);
}

void GROUP_1_answer()
{
    //104 len 0 0 0 0 13 1 20 0 1 0 1 0 0 1h 2h 3h 4h 0 

    uint8_t  data_out[255];
    uint8_t  data_len = 12;

    uint8_t  ASDU_type = M_ME_NC_1;
    uint8_t  NoO = TAG_GROUP_1_NUMBER;
    uint8_t  CoT = eIEC870_COT_INROGEN;
    uint8_t  ORG = IEC_ORG;
    uint16_t COA = 1;

    uint8_t  QDP = 0;

    DUI_maker(&data_out[6], ASDU_type, NoO, CoT, ORG, COA);

    for(uint8_t i = 0; i < TAG_GROUP_1_NUMBER; i++)
    {
        float f_data = (float) modbus_reg_04[TAG_GROUP_1[0][i]] / TAG_GROUP_1[1][i];
        M_ME_NC_1_maker(&data_out[0], 
                        TAG_GROUP_1[0][i], 
                        (uint8_t *) &f_data,
                        QDP, 
                        &data_len);
    }
    APCI_maker_104(&data_out[0], data_len - 2, V_S, V_R);

    EVENT_SEND(IEC_104_WINDOW_SCENARIO, data_len, 0, data_out);
    V_S++;
    ACK_k++;
    IEC_position = 200;
}

void S_format_ACK()
{
    uint8_t data_out[6];
    uint8_t data_len = 6;

    data_out[0] = 104;
    data_out[1] = 4;
    data_out[2] = 1;
    data_out[3] = 0;
    data_out[4] = (V_R << 1);
    data_out[5] = (V_R >> 7);
    EVENT_SEND(IEC_104_WINDOW_SCENARIO, data_len, 0, data_out);
    ACK_w = 0;
    ACK_w_timer = 0;
}

void IEC_104_handler()
{
    if(ACK_w >= IEC_w_param) S_format_ACK();
    if(ACK_w_timer == IEC_w_period) S_format_ACK();
    if(ACK_w > 0) ACK_w_timer++;

    switch (IEC_position)
    {
        case 1:
        {
            uint8_t  data_out[16];
            uint8_t  data_len = 12;
            uint8_t  ASDU_type = M_EI_NA_1;
            uint8_t  NoO = 1;
            uint8_t  CoT = eIEC870_COT_INIT;
            uint8_t  ORG = IEC_ORG;
            uint16_t COA = 1;
            DUI_maker(&data_out[6], ASDU_type, NoO, CoT, ORG, COA);
            M_EI_NA_1_maker(&data_out[0], 0, 0, 0, &data_len);
            APCI_maker_104(&data_out[0], data_len - 2, V_S, V_R);
            EVENT_SEND(IEC_104_WINDOW_SCENARIO, data_len, 0, data_out);
            V_S++;
            ACK_k++;
            IEC_position = 2;
        }
        break;
        case 100:
        {
            GROUP_1_answer();
        }
        break;
        case 200:
        {
            uint8_t  data_out[16];
            uint8_t  data_len = 12;
            uint8_t  ASDU_type = C_IC_NA_1;
            uint8_t  NoO = 1;
            uint8_t  CoT = eIEC870_COT_ACT_TERM;
            uint8_t  ORG = IEC_ORG;
            uint16_t COA = 1;
            DUI_maker(&data_out[6], ASDU_type, NoO, CoT, ORG, COA);
            C_IC_NA_1_maker(&data_out[0], eIEC870_COT_INROGEN, 0, 0, &data_len);
            APCI_maker_104(&data_out[0], data_len - 2, V_S, V_R);
            EVENT_SEND(IEC_104_WINDOW_SCENARIO, data_len, 0, data_out);
            V_S++;
            ACK_k++;
            IEC_position = 2;
        }
        break;

        default:
        break;
    }
}

K_WORK_DEFINE(IEC_104_handler_timer_work, IEC_104_handler);

void IEC_104_timer_handler(struct k_timer *dummy)
{
    k_work_submit(&IEC_104_handler_timer_work);
}

K_TIMER_DEFINE(IEC_104_handler_timer, IEC_104_timer_handler, NULL);

void wrong_data_in(uint8_t *data_in, uint8_t COT_err, uint8_t *TI)
{
    *TI = 0;
    *(data_in + 8) = COT_err | 64;
    *(data_in + 2) = (V_S << 1);
    *(data_in + 3) = (V_S >> 7);
    *(data_in + 4) = (V_R << 1);
    *(data_in + 5) = (V_R >> 7);
    uint8_t data_len = *(data_in + 1) + 2;
    EVENT_SEND(IEC_104_WINDOW_SCENARIO, data_len, 0, data_in);                
    V_S++; ACK_k++;
}

void I_format(uint8_t *data_in)
{
/* Received/sent packet counter */
    uint16_t V_S_in = ((*(data_in + 4) >> 1) + (*(data_in + 5) << 7));
    uint16_t V_R_in = ((*(data_in + 2) >> 1) + (*(data_in + 3) << 7));

/*1*/
/* Type identification */
    uint8_t TI  = *(data_in + 6);
/*2*/
/* Structure Qualifier */
    uint8_t SQ  = readBit(*(data_in + 7), 7);
/* Number of objects/elements */
    uint8_t NOO = (*(data_in + 7)) & 0x7F;
/*3*/
/* Cause of transmission */
    uint8_t COT = (*(data_in + 8))&63;
    switch(COT)
    {
        case eIEC870_COT_ACT:
            COT = eIEC870_COT_ACT_CON;
        break;
        case eIEC870_COT_DEACT:
            COT = eIEC870_COT_DEACT_CON;
        break;
        default:
            COT = eIEC870_COT_UNKNOWN_CAUSE | 64;
        break;
    }
    uint8_t P_N = readBit((*(data_in + 8)), 6);
/*4*/
/* Originator Address*/
    uint8_t ORG = 0;
    if(mod_reg_03_IEC_COT_config_default == 9) ORG = (*(data_in + 9));
/*5*/ 
/* Common Address of ASDU 1 byte*/
    uint16_t COA = (*(data_in + mod_reg_03_IEC_COT_config_default + 1));
/*6*/
/* Common Address of ASDU 2 byte*/
    if(mod_reg_03_IEC_COA_config_default == 2) COA += (*(data_in + mod_reg_03_IEC_COT_config_default + 2) << 8);

    LOG_INF("TI = %d, SQ  = %d, NOO = %d, COT = %d, ORG = %d, COA = %d", TI, SQ, NOO, COT, ORG, COA);

    if(V_R == V_R_in && V_S == V_S_in)
    {
    /*wrong requrest*/
        if(COA != 1)                         wrong_data_in(data_in, eIEC870_COT_UNKNOWN_ASDU_ADDRESS, &TI);
        else
        if(COT == eIEC870_COT_UNKNOWN_CAUSE) wrong_data_in(data_in, eIEC870_COT_UNKNOWN_CAUSE,        &TI);
    /**/

        switch (TI)
        {
            case C_CS_NA_1: // 103
            {
                uint8_t data_len = *(data_in + 1) + 2;
                uint8_t data_out[data_len];

                IEC_CP56_to_real(data_in + 15);

                memcpy(data_out, data_in, data_len);

                data_out[2] = (V_S << 1);
                data_out[3] = (V_S >> 7);
                data_out[4] = (V_R << 1);
                data_out[5] = (V_R >> 7);
                data_out[8] = COT;
                EVENT_SEND(IEC_104_WINDOW_SCENARIO, data_len, 0, data_out);
                V_S++;
                ACK_k++;
            } 
            break;

            case C_IC_NA_1: // 100
            {
                uint8_t data_len = *(data_in + 1) + 2;
                uint8_t data_out[data_len];

                memcpy(data_out, data_in, data_len);

                data_out[2] = (V_S << 1);
                data_out[3] = (V_S >> 7);
                data_out[4] = (V_R << 1);
                data_out[5] = (V_R >> 7);
                data_out[8] = COT;
                EVENT_SEND(IEC_104_WINDOW_SCENARIO, data_len, 0, data_out);
                V_S++;
                IEC_ORG = ORG;
                ACK_k++;
                if(COT == eIEC870_COT_DEACT_CON) break;
                switch (*(data_in + 15))
                {
                    case 20:
                        IEC_position = 100;
                        // main INTERROGATION 
                    break;
                    case 21:
                        // group 1 INTERROGATION 
                    break;
                    case 22:
                        // group 2 INTERROGATION 
                    break;
                }
            } 
            break;
            default:
                wrong_data_in(data_in, eIEC870_COT_UNKNOWN_TYPE, &TI);
            break;
        }
        V_R++;
        ACK_w++;
    }
    else 
    {
        LOG_INF("Seq err. Disconnect");
        EVENT_SEND(EVENT_SYSTEM_SCENARIO, mess_x05_event_ETHERNET_DISCONNECT, 0, NULL);
    }
}

void S_format(uint8_t *data_in)
{
    uint16_t V_S_in = ((*(data_in + 4) >> 1) + (*(data_in + 5) << 7));
    LOG_INF("S_format %d ___ %d", V_S, V_S_in);
    if(V_S == V_S_in) {LOG_INF("ACNOLEDGE ok"); ACK_k = 0;}
    else LOG_INF("ACNOLEDGE err");
}

void U_format(uint8_t *data_in)
{
    LOG_INF("U_format");
    
    uint8_t com_bit_2 = readBit(*(data_in + 2), 2);
    uint8_t com_bit_4 = readBit(*(data_in + 2), 4);
    uint8_t com_bit_6 = readBit(*(data_in + 2), 6);

    uint8_t data_out[6] = {104, 4, 0, 0, 0, 0};
    
    if(com_bit_2) 
    {
        data_out[2] = 0x0B;
        V_S = 0; V_R = 0; IEC_position = 1;
        k_timer_start(&IEC_104_handler_timer,  K_MSEC(100), K_MSEC(100));
    }
    if(com_bit_4) 
    {
        data_out[2] = 0x23;
        V_S = 0; V_R = 0; IEC_position = 0;
        k_timer_stop(&IEC_104_handler_timer);
    }
    if(com_bit_6) data_out[2] = 0x83;

    EVENT_SEND(IEC_104_WINDOW_SCENARIO, 6, 0, data_out);
}

void IEC_104_parsing(uint8_t *data_in)
{
    if(*data_in != 104) return;
    
    if(readBit(*(data_in + 2), 0) == 0) I_format(data_in);
    else
    {
      if(readBit(*(data_in + 2), 1) == 0) S_format(data_in);
      else U_format(data_in);
    }
}

/*end IEC_104*/
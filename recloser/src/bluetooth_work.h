#ifndef BLUETOOTH_WORK_H__
#define BLUETOOTH_WORK_H__

#include <stdint.h>
#include <inttypes.h>

void BLE_init();
int bt_send_work(uint8_t *data_scenario, uint16_t length);

/*BLE device definitions*/
#define adv_param_interval_min          500
#define adv_param_interval_max          600

#endif
#include "bluetooth_work.h"
#include "calculations.h"
#include "nus.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci_vs.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <logging/log.h>
#include <settings/settings.h>
#include <mgmt/mcumgr/smp_bt.h>
#include <sys/byteorder.h>

#ifdef CONFIG_MCUMGR_CMD_OS_MGMT
#include "os_mgmt/os_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_IMG_MGMT
#include "img_mgmt/img_mgmt.h"
#endif

LOG_MODULE_REGISTER(bluetooth_work);


#define BT_LE_ADV_CONN_TEST BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE, \
                                               adv_param_interval_min, \
                                               adv_param_interval_max, NULL)

#define DEVICE_NAME             CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN	        (sizeof(DEVICE_NAME) - 1)

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};

static struct bt_conn *default_conn;
static void (*ble_data_ptr)(const uint8_t *const, uint16_t);

int bt_send_work(uint8_t *data, uint16_t length)
{
    //LOG_INF("bt send %d", length);
    if(default_conn != NULL)
        return bt_nus_send(default_conn, data, length);
    else return 0;
}

static uint8_t CRC_CALC_CHECK(uint8_t *msg, uint16_t counter)
{
    uint8_t CRC[2];
    CRC_CALC(msg, counter - 2, CRC);
    if(CRC[0] == *(msg + (counter - 2)) && CRC[1] == *(msg + (counter - 1))) return 1;
    return 0;
}

static void bt_receive_cb(struct bt_conn *conn, 
                          const uint8_t *const m_nus_data_array,
			  uint16_t len)
{
        if(CRC_CALC_CHECK((uint8_t *)m_nus_data_array, len))
            (*ble_data_ptr)((uint8_t *)m_nus_data_array, len);
        else LOG_INF("bad crc");
};

static struct bt_nus_cb nus_cb = {
	.received = bt_receive_cb,
};

/*MTU exchange*/

static void mtu_exchange_cb(struct bt_conn *conn, uint8_t err,
			    struct bt_gatt_exchange_params *params)
{
	LOG_INF_GLOB("MTU exchange %u %s (%u)\n", bt_conn_index(conn),
                      err == 0U ? "successful" : "failed", bt_gatt_get_mtu(conn));
}

static struct bt_gatt_exchange_params mtu_exchange_params[CONFIG_BT_MAX_CONN];

static int mtu_exchange(struct bt_conn *conn)
{
	uint8_t conn_index;
	int err;

	conn_index = bt_conn_index(conn);

	LOG_INF_GLOB("MTU (%u): %u\n", conn_index, bt_gatt_get_mtu(conn));

	mtu_exchange_params[conn_index].func = mtu_exchange_cb;

	err = bt_gatt_exchange_mtu(conn, &mtu_exchange_params[conn_index]);
	if (err) {
		LOG_INF_GLOB("MTU exchange failed (err %d)", err);
	} else {
		LOG_INF_GLOB("Exchange pending...");
	}

	return err;
}

static void connected(struct bt_conn *conn, uint8_t conn_err)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

        if (conn_err) {
		LOG_INF_GLOB("Failed %s (%d)", log_strdup(addr), conn_err);
		bt_conn_unref(conn);
		return;
	}
        default_conn = conn;
     // bt_conn_ref(conn);
        mtu_exchange(conn);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];
	int err;

        struct bt_conn_info info;
	err = bt_conn_get_info(conn, &info);
        default_conn = NULL;
        bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
        LOG_INF_GLOB("Disconnected: %s (reason %u) role %d", log_strdup(addr), reason, info.role);
        bt_le_adv_stop();
        err = bt_le_adv_start(BT_LE_ADV_CONN_TEST, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR_GLOB("Advertising failed to start (err %d)", err);
		return;
	}
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

void BLE_init(void *ble_data_ptr_)
{
    ble_data_ptr = ble_data_ptr_;
    int err = bt_enable(NULL);
    if (err) LOG_ERR_GLOB("Bluetooth init failed (err %d)", err);
    LOG_INF_GLOB("Bluetooth initialized");
    if (IS_ENABLED(CONFIG_SETTINGS)) settings_load();
    bt_conn_cb_register(&conn_callbacks);
    err = bt_nus_init(&nus_cb);
    if (err) LOG_ERR_GLOB("Failed to initialize BT NUS shell (err: %d)", err);
    err = bt_le_adv_start(BT_LE_ADV_CONN_TEST, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) LOG_ERR_GLOB("Advertising failed to start (err %d)", err);

    #ifdef CONFIG_MCUMGR_CMD_OS_MGMT
        os_mgmt_register_group();
        img_mgmt_register_group();
        smp_bt_register();
    #endif
}
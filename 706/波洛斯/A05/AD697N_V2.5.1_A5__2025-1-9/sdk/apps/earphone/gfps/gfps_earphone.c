#include "app_config.h"

#include "earphone.h"
#include "app_main.h"
#include "update_tws.h"
#include "3th_profile_api.h"

#include "btstack/avctp_user.h"
#include "btstack/btstack_task.h"
#include "bt_tws.h"
#include "spp_user.h"
#include "btstack/GFP_include/GFPS_msg.h"

#define GFPS_FUNC_ID_GFP_INFO_SYNC       TWS_FUNC_ID('G', 'F', 'P', 'S')

#if GFP_EN

void gfps_ring_set(u8 en, u8 ch);

#define LOG_TAG             "[GFPS_APP]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#define RING_CH_L   BIT(0)
#define RING_CH_R   BIT(1)
#define RING_CH_LR  (RING_CH_L | RING_CH_R)

static uint8_t google_model_id_used[3] = {0xF2, 0x63, 0x8D};
static char google_public_key_used[64] = {
    0x5f, 0x17, 0xf0, 0x5a, 0x22, 0x03, 0xe2, 0xdc, 0x93, 0x1d, 0x5c, 0x00, 0xe8, 0xa4, 0x06, 0x7c,
    0x3c, 0x09, 0xa0, 0xad, 0x91, 0x82, 0x3e, 0x45, 0xda, 0xe7, 0xe5, 0x2f, 0x5e, 0xe9, 0x80, 0x1b,
    0xa6, 0xe2, 0x21, 0x9f, 0x65, 0x85, 0xa8, 0xab, 0x65, 0xf1, 0x47, 0x7e, 0x46, 0x7a, 0xdb, 0xb2,
    0x4b, 0x46, 0x72, 0xb8, 0x35, 0x89, 0x0b, 0x99, 0x57, 0xb7, 0x0e, 0xa9, 0x45, 0xe1, 0x7a, 0xbb
};
static char google_private_key_used[32] = {
    0x86, 0x89, 0x57, 0x30, 0xFB, 0x95, 0x8A, 0x9E, 0x42, 0x57, 0x56, 0x97,
    0x39, 0x54, 0x77, 0xE7, 0x62, 0x44, 0x7A, 0xA1, 0x2B, 0xDF, 0x35, 0xFE,
    0xE9, 0xC5, 0x82, 0x28, 0x1B, 0xD7, 0xF5, 0xBE
};

int gfps_earphone_state_init()
{
    return 0;
}

int gfps_earphone_state_set_page_scan_enable()
{
    return 0;
}

int gfps_earphone_state_get_connect_mac_addr()
{
    return 0;
}

int gfps_earphone_state_cancel_page_scan()
{
    return 0;
}

int gfps_earphone_state_tws_init(int paired)
{
    return 0;
}

int gfps_earphone_state_tws_connected(int first_pair, u8 *comm_addr)
{
    if (first_pair) {
        extern void ble_module_enable(u8 en);
        extern void bt_update_mac_addr(u8 * addr);
        extern void lib_make_ble_address(u8 * ble_address, u8 * edr_address);
        u8 tmp_ble_addr[6] = {0};
        lib_make_ble_address(tmp_ble_addr, comm_addr);

        //将ble广播地址改成公共地址
        le_controller_set_mac(tmp_ble_addr);
        bt_update_mac_addr(comm_addr);


        /*新的连接，公共地址改变了，要重新将新的地址广播出去*/
        if (tws_api_get_role() == TWS_ROLE_MASTER) {
            printf("\nNew Connect Master!!!\n\n");
            ble_app_disconnect();
            bt_ble_adv_enable(0);
            bt_ble_adv_enable(1);
            gfps_sibling_sync_info();
        } else {
            printf("\nConnect Slave!!!\n\n");
            /*从机ble关掉*/
            ble_app_disconnect();
            bt_ble_adv_enable(0);
        }
    }

    return 0;
}

int gfps_earphone_state_enter_soft_poweroff()
{
    extern void bt_ble_exit(void);
    bt_ble_exit();
    return 0;
}

int gfps_adv_bt_status_event_handler(struct bt_event *bt)
{
    return 0;
}


int gfps_adv_hci_event_handler(struct bt_event *bt)
{

    return 0;
}

void gfps_bt_tws_event_handler(struct bt_event *bt)
{
    int role = bt->args[0];
    int phone_link_connection = bt->args[1];
    int reason = bt->args[2];

    switch (bt->event) {
    case TWS_EVENT_CONNECTED:
        //bt_ble_adv_enable(1);
        if (tws_api_get_role() == TWS_ROLE_SLAVE) {
            //master enable
            printf("\nConnect Slave!!!\n\n");
            /*从机ble关掉*/
            ble_app_disconnect();
            bt_ble_adv_enable(0);
        } else {
            gfps_sibling_sync_info();
        }

        break;
    case TWS_EVENT_PHONE_LINK_DETACH:
        /*
         * 跟手机的链路LMP层已完全断开, 只有tws在连接状态才会收到此事件
         */
        break;
    case TWS_EVENT_CONNECTION_DETACH:
        /*
         * TWS连接断开
         */
        if (app_var.goto_poweroff_flag) {
            break;
        }
        if (get_app_connect_type() == 0) {
            printf("\ntws detach to open ble~~~\n\n");
            bt_ble_adv_enable(1);
        }
        set_ble_connect_type(TYPE_NULL);
        break;
    case TWS_EVENT_SYNC_FUN_CMD:
        break;
    case TWS_EVENT_ROLE_SWITCH:
        break;
    }

#if OTA_TWS_SAME_TIME_ENABLE
    tws_ota_app_event_deal(bt->event);
#endif
}

static int gfp_bt_connction_status_event_handler(struct bt_event *bt)
{
    switch (bt->event) {
    case BT_STATUS_SECOND_CONNECTED:
    case BT_STATUS_FIRST_CONNECTED:
        gfps_enter_nopair_mode();
        break;
    default:
        break;
    }
    return 0;
}

int gfps_sys_event_handler_specific(struct sys_event *event)
{
    switch (event->type) {
    case SYS_BT_EVENT:
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            gfp_bt_connction_status_event_handler(&event->u.bt);
        } else if ((u32)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {

        }
#if TCFG_USER_TWS_ENABLE
        else if (((u32)event->arg == SYS_BT_EVENT_FROM_TWS)) {
            gfps_bt_tws_event_handler(&event->u.bt);
        }
#endif
#if OTA_TWS_SAME_TIME_ENABLE
        else if (((u32)event->arg == SYS_BT_OTA_EVENT_TYPE_STATUS)) {
            bt_ota_event_handler(&event->u.bt);
        }
#endif
        break;
    case SYS_DEVICE_EVENT:
        break;
    }

    return 0;
}

int gfps_app_message_deal(int opcode, u8 *data, u32 len)
{
    switch (opcode) {
    case APP_PROTOCOL_LIB_TWS_DATA_SYNC:
        tws_api_send_data_to_sibling(data, len, GFPS_FUNC_ID_GFP_INFO_SYNC);
        break;
    case APP_PROTOCOL_GFPS_RING_STOP_ALL:
        printf("GFPS_RING_STOP_ALL");
        gfps_ring_set(0, RING_CH_LR);
        break;
    case APP_PROTOCOL_GFPS_RING_RIGHT:
        printf("GFPS_RING_RIGHT");
        gfps_ring_set(1, RING_CH_R);
        gfps_ring_set(0, RING_CH_L);
        break;
    case APP_PROTOCOL_GFPS_RING_LEFT:
        printf("GFPS_RING_LEFT");
        gfps_ring_set(1, RING_CH_L);
        gfps_ring_set(0, RING_CH_R);
        break;
    case APP_PROTOCOL_GFPS_RING_ALL:
        printf("GFPS_RING_ALL");
        gfps_ring_set(1, RING_CH_LR);
        break;
    }
}

/**
 * @brief GFP亮铃函数
 *
 * @param en 1为响铃，0为不响铃
 * @param ch 设置的通道号
 */
void gfps_ring_set(u8 en, u8 ch)
{
    if (((ch & RING_CH_L) && tws_api_get_local_channel() == 'L')
        || ((ch & RING_CH_R) && tws_api_get_local_channel() == 'R')) {

    }
}

/**
 * @brief GFP电量获取函数
 *
 * @param battery_type 电量类型
 * @return uint8_t 电量值为0~100,最高位0表示没有充电,为1表示充电中
 */
static uint8_t gfps_app_get_battery_value(u8 battery_type)
{
    uint8_t value = 0;
    switch (battery_type) {
    case APP_PROTOCOL_BAT_T_TWS_LEFT:
        value = 50;
        break;
    case APP_PROTOCOL_BAT_T_TWS_RIGHT:
        value = 60;
        break;
    case APP_PROTOCOL_BAT_T_BOX:
        value = 70;
        break;
    default:
        value = 0xff;
        printf("gfps_app_get_bat_value error");
        break;
    }
    return value;
}

bool gfps_is_tws_master_role()
{
#if TCFG_USER_TWS_ENABLE
    return (tws_api_get_role() == TWS_ROLE_MASTER);
#endif
    return 1;
}

void gfps_app_init()
{
    gfps_is_tws_master_callback_register(gfps_is_tws_master_role);
    gfps_get_battery_callback_register(gfps_app_get_battery_value);
    gfps_message_callback_register(gfps_app_message_deal);
    gfps_set_model_id(google_model_id_used);
    gfps_set_anti_spoofing_public_key(google_public_key_used);
    gfps_set_anti_spoofing_private_key(google_private_key_used);
    gfps_init();
}

static void gfps_tws_get_data_from_sibling(void *_data, u16 len, bool rx)
{
    u8 *data = _data;
    if (rx) {
        gfps_tws_data_deal(data, len);
    }
}

REGISTER_TWS_FUNC_STUB(gfps_event_sync) = {
    .func_id = GFPS_FUNC_ID_GFP_INFO_SYNC,
    .func    = gfps_tws_get_data_from_sibling,
};

#endif

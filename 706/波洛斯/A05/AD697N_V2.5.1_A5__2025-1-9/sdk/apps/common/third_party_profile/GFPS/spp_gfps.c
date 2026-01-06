/*********************************************************************************************
    *   Filename        : le_gfps.c

    *   Description     :

    *   Author          :

    *   Email           : zh-jieli.com

    *   Last modifiled  : 2017-01-17 11:14

    *   Copyright:(c)JIELI  2011-2016  @ , All Rights Reserved.
*********************************************************************************************/

// *****************************************************************************
/* EXAMPLE_START(le_counter): LE Peripheral - Heartbeat Counter over GATT
 *
 * @text All newer operating systems provide GATT Client functionality.
 * The LE Counter examples demonstrates how to specify a minimal GATT Database
 * with a custom GATT Service and a custom Characteristic that sends periodic
 * notifications.
 */
// *****************************************************************************
#include "btstack/avctp_user.h"
#include "system/includes.h"
#include "spp_user.h"
#include "string.h"
#include "ble_user.h"

#include "btstack/GFP_include/GFPS_msg.h"

#include "spp_gfps.h"
#include "app_config.h"

#if GFP_EN

#if 1
#define log_info(x, ...)  printf("[gfps_spp]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  printf_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

extern u8 gfp_debug_enable;

static struct spp_operation_t *spp_api = NULL;
static u8 spp_state;

static uint8_t gfps_rfcomm_model_id_info[7] = {0x03, 0x01, 0x00, 0x03};
static uint8_t gfps_rfcomm_ble_addr_info[10] = {0x03, 0x02, 0x00, 0x06};
static uint8_t gfps_rfcomm_battery_data[7] = {0x03, 0x03, 0x00, 0x03};

static uint8_t gfps_rfcomm_connect_state = 0;
extern uint8_t gfps_model_id[3];

extern int multi_spp_send_data(u8 local_cid, u8 rfcomm_cid,  u8 *buf, u16 len);
static int gfps_spp_send_data(void *priv, u8 *buf, u16 len)
{
    if (gfps_rfcomm_connect_state == 0) {
        log_info("gfps_rfcomm is no connect, don't send\n");
        return 1;
    }
    log_info("gfps_spp_send_data\n");
    multi_spp_send_data(0x0A, 0, buf, len);
    return 0;
}

static uint8_t get_device_active(void)
{
#if TCFG_USER_TWS_ENABLE
    uint8_t value;
    value = gfps_get_device_actice_status();
    printf("get_device_active:0x%x", value);
    return value;
#else
    return 0x01;
#endif
}

void gfps_battery_update(void)
{
    if (gfps_get_ble_work_state() == BLE_ST_ADV) {
        gfps_bt_ble_adv_enable(0);
        gfps_bt_ble_adv_enable(1);
    }
    generate_rfcomm_battery_data(&gfps_rfcomm_battery_data[4]);
    gfps_spp_send_data(0x0A, gfps_rfcomm_battery_data, 7);
}

static void gfps_spp_init_info(void)
{
    uint8_t info_buf[24];
    uint8_t my_le_mac[6];
    uint8_t tmp_mac[6];

    memcpy(&gfps_rfcomm_model_id_info[4], gfps_model_id, 3);

    le_controller_get_mac(tmp_mac);
    for (int i = 0; i < 6; i++) {
        my_le_mac[i] = tmp_mac[5 - i];
    }
    memcpy(&gfps_rfcomm_ble_addr_info[4], my_le_mac, 6);

    generate_rfcomm_battery_data(&gfps_rfcomm_battery_data[4]);

    uint8_t active_rsp[5] = {0x03, 0x06, 0x00, 0x01};
    active_rsp[4] = get_device_active();

    memcpy(&info_buf[0], gfps_rfcomm_model_id_info, 7);
    memcpy(&info_buf[7], gfps_rfcomm_ble_addr_info, 10);
    memcpy(&info_buf[17], gfps_rfcomm_battery_data, 7);

    gfps_spp_send_data(0x0A, info_buf, 24);
}

static void gfps_rfcomm_data_prase(u8 *data, u16 len)
{
    uint8_t msg_group_name = data[0];
    uint8_t msg_code_name = data[1];
    uint16_t additional_data_len = data[3] + (data[2] << 8);
    log_info("msg_group_type:0x%x, msg_name_type:0x%x, additional_data_len: 0x%x\n", msg_group_name, msg_code_name, additional_data_len);
    log_info_hexdump(data, len);
    if (msg_group_name == GFPS_RFCOMM_MSG_GROUP_BLUETOOTH_EVENT) {
        log_info("gfp blue group msg");
        switch (msg_code_name) {
        case GFPS_RFCOMM_MSG_CODE_ENABLE_SLIENCE_MODE:
            break;
        case GFPS_RFCOMM_MSG_CODE_DISABLE_SLIENCE_MODE:
            break;
        default:
            log_info("msg_code error, code: 0x%x", msg_code_name);
            break;
        }
    }
    if (msg_group_name == GFPS_RFCOMM_MSG_GROUP_COMPANION_APP_EVENT) {
        log_info("gfp companion app group msg");
        switch (msg_code_name) {
        case GFPS_RFCOMM_MSG_CODE_LOG_BUFFER_FULL:
            break;
        default:
            log_info("msg_code error, code: 0x%x", msg_code_name);
            break;
        }
    }
    if (msg_group_name == GFPS_RFCOMM_MSG_GROUP_DEVICE_INFO_EVENT) {
        log_info("gfp device info group msg");
        switch (msg_code_name) {
        case GFPS_RFCOMM_MSG_CODE_MODEL_ID:
            break;
        case GFPS_RFCOMM_MSG_CODE_BLE_ADDR_UPDATED:
            break;
        case GFPS_RFCOMM_MSG_CODE_BATTERY_UPDATED:
            break;
        case GFPS_RFCOMM_MSG_CODE_REMAIN_BATTERY_TIME:
            break;
        case GFPS_RFCOMM_MSG_CODE_ACTIVE_COMPONENTS_REQ:
            uint8_t active_rsp[5] = {0x03, 0x06, 0x00, 0x01};
            active_rsp[4] = get_device_active();
            gfps_spp_send_data(0x0A, active_rsp, 5);
            break;
        case GFPS_RFCOMM_MSG_CODE_CAPABILITIES:
            break;
        case GFPS_RFCOMM_MSG_CODE_PLATFORM_TYPE:
            break;
        default:
            log_info("msg_code error, code: 0x%x", msg_code_name);
            break;
        }
    }
    if (msg_group_name == GFPS_RFCOMM_MSG_GROUP_DEVICE_ACTION_EVENT) {
        log_info("gfp device action group msg");
        switch (msg_code_name) {
        case GFPS_RFCOMM_MSG_CODE_RING_DEVICE:
            uint8_t ring_type = data[4];
            log_info("ring_type = 0x%x", ring_type);
            uint8_t ring_ack_msg[6] = {0xff, 0x01, 0x00, 0x02, 0x04, 0x01};
            switch (ring_type) {
            case GFPS_RING_STOP_ALL:
                gfps_app_message_deal(APP_PROTOCOL_GFPS_RING_STOP_ALL, NULL, 0);
                gfps_spp_send_data(0x0A, ring_ack_msg, sizeof(ring_ack_msg));
                gfps_sibling_data_send(GFPS_SIBLING_SYNC_TYPE_RING_HEADST, &ring_type, 1);
                break;
            case GFPS_RING_RIGHT:
                gfps_app_message_deal(APP_PROTOCOL_GFPS_RING_RIGHT, NULL, 0);
                gfps_spp_send_data(0x0A, ring_ack_msg, sizeof(ring_ack_msg));
                gfps_sibling_data_send(GFPS_SIBLING_SYNC_TYPE_RING_HEADST, &ring_type, 1);
                break;
            case GFPS_RING_LEFT:
                gfps_app_message_deal(APP_PROTOCOL_GFPS_RING_LEFT, NULL, 0);
                gfps_spp_send_data(0x0A, ring_ack_msg, sizeof(ring_ack_msg));
                gfps_sibling_data_send(GFPS_SIBLING_SYNC_TYPE_RING_HEADST, &ring_type, 1);
                break;
            case GFPS_RING_ALL:
                gfps_app_message_deal(APP_PROTOCOL_GFPS_RING_ALL, NULL, 0);
                gfps_spp_send_data(0x0A, ring_ack_msg, sizeof(ring_ack_msg));
                gfps_sibling_data_send(GFPS_SIBLING_SYNC_TYPE_RING_HEADST, &ring_type, 1);
                break;
            default:
                log_info("device action error, name:0x%x", msg_code_name);
                break;
            }
            break;
        default:
            log_info("msg_code error, code: 0x%x", msg_code_name);
            break;
        }
    }
    if (msg_group_name == GFPS_RFCOMM_MSG_GROUP_ACKNOWLEDGEMENT_EVENT) {
        log_info("gfp acknowledgement group msg");
        switch (msg_code_name) {
        case GFPS_RFCOMM_MSG_CODE_ACKNOWLEDGEMENT_ACK:
            break;
        case GFPS_RFCOMM_MSG_CODE_ACKNOWLEDGEMENT_NAK:
            break;
        default:
            log_info("msg_code error, code: 0x%x", msg_code_name);
            break;
        }
    }
}

int mutil_handle_data_deal(u8 local_id, u8 packet_type, u16 channel, u8 *packet, u16 size)
{
    printf("local_id:%d\n", local_id);
    switch (packet_type) {
    case 1:
        log_info("gfps rfcomm connect###########\n");
        gfps_rfcomm_connect_state = 1;
        gfps_spp_init_info();
        gfps_set_battery_ui_enable(1);
        break;
    case 2:
        log_info("gfps rfcomm disconnect#########\n");
        gfps_rfcomm_connect_state = 0;
        gfps_set_battery_ui_enable(0);
        break;
    case 7:
        log_info("gfps rfcomm packet");
        /* log_info_hexdump(packet, size); */
        gfps_rfcomm_data_prase(packet, size);
        break;
    }
    return 0;
}

#endif

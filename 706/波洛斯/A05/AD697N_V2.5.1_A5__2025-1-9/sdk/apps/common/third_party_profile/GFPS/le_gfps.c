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
#include "system/app_core.h"
#include "system/includes.h"

#include "btstack/btstack_task.h"
#include "btstack/bluetooth.h"
#include "user_cfg.h"
#include "vm.h"
#include "btcontroller_modules.h"
//#include "bt_common.h"
//#include "3th_profile_api.h"

#include "btstack/third_party/common/ble_user.h"

#include "btstack/avctp_user.h"
//#include "rcsp_bluetooth.h"
//#include "JL_rcsp_api.h"
//#include "custom_cfg.h"
#include "le_gfps.h"
#include "le_common.h"
#include "btstack/GFP_include/GFPS_msg.h"

#if (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_GFP)

#if 1
#define log_info(x, ...)  printf("[gfps_ble]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  printf_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

bool gfps_auto_test_mode = 0;

//ATT发送的包长,    note: 20 <=need >= MTU
#define ATT_LOCAL_MTU_SIZE    (83)                   //
//ATT缓存的buffer大小,  note: need >= 20,可修改
#define ATT_SEND_CBUF_SIZE        (512)                   //

//共配置的RAM
#define ATT_RAM_BUFSIZE           (ATT_CTRL_BLOCK_SIZE + ATT_LOCAL_MTU_SIZE + ATT_SEND_CBUF_SIZE)                   //note:
static u8 att_ram_buffer[ATT_RAM_BUFSIZE] __attribute__((aligned(4)));

// 广播周期 (unit:0.625ms)
//#define ADV_INTERVAL_MIN          (130)
static u8 adv_interval_min = 80;

#define HOLD_LATENCY_CNT_MIN  (3)  //(0~0xffff)
#define HOLD_LATENCY_CNT_MAX  (15) //(0~0xffff)
#define HOLD_LATENCY_CNT_ALL  (0xffff)
#define GFPS_BT_STATUS_CONNECTING 45 //avctp_user.c的 BT_STATUS_CONNECTING


static volatile hci_con_handle_t con_handle;

//连接参数更新请求设置
//是否使能参数请求更新,0--disable, 1--enable
static const uint8_t connection_update_enable = 1; ///0--disable, 1--enable
//当前请求的参数表index
static uint8_t connection_update_cnt = 0; //

//参数表
static const struct conn_update_param_t connection_param_table[] = {
    {16, 24, 10, 600},//11
    {12, 28, 10, 600},//3.7
    {8,  20, 10, 600},
    /* {12, 28, 4, 600},//3.7 */
    /* {12, 24, 30, 600},//3.05 */
};

//共可用的参数组数
#define CONN_PARAM_TABLE_CNT      (sizeof(connection_param_table)/sizeof(struct conn_update_param_t))


//用户可配对的，这是样机跟客户开发的app配对的秘钥
/* const u8 link_key_data[16] = {0x06, 0x77, 0x5f, 0x87, 0x91, 0x8d, 0xd4, 0x23, 0x00, 0x5d, 0xf1, 0xd8, 0xcf, 0x0c, 0x14, 0x2b}; */
#define EIR_TAG_STRING   0xd6, 0x05, 0x08, 0x00, 'J', 'L', 'A', 'I', 'S', 'D','K'
static const char user_tag_string[] = {EIR_TAG_STRING};
uint8_t gfps_model_id[3];

static u8 adv_data_len;
static u8 gfps_adv_data[ADV_RSP_PACKET_MAX];//max is 31
static u8 scan_rsp_data_len;
static u8 scan_rsp_data[ADV_RSP_PACKET_MAX];//max is 31

static char gap_device_name[BT_NAME_LEN_MAX] = "jl_ble_test";
static u8 gap_device_name_len = 0; //名字长度，不包含结束符
static u8 ble_work_state = 0;      //ble 状态变化
static u8 adv_ctrl_en;             //广播控制
static u8 gfps_tx_power = 0xdc;

static u8 test_read_write_buf[4];

static void (*app_recieve_callback)(void *priv, void *buf, u16 len) = NULL;
static void (*app_ble_state_callback)(void *priv, ble_state_e state) = NULL;
static void (*ble_resume_send_wakeup)(void) = NULL;
static u32 channel_priv;

static int app_send_user_data_check(u16 len);
static int app_send_user_data(u16 handle, u8 *data, u16 len, u8 handle_type);
void gfps_bt_ble_adv_enable(u8 enable);


//------------------------------------------------------
//广播参数设置
static void advertisements_setup_init();
static int set_adv_enable(void *priv, u32 en);
static int get_buffer_vaild_len(void *priv);
extern const char *bt_get_local_name();
extern void clr_wdt(void);
extern void sys_auto_shut_down_disable(void);
extern void sys_auto_shut_down_enable(void);
extern u8 get_total_connect_dev(void);
extern bool get_gfps_pair_state(void);

extern uint8_t gfps_account_data[GFPS_ACCOUNT_DATA_LEN_MAX];

static void send_request_connect_parameter(u8 table_index)
{
    struct conn_update_param_t *param = (void *)&connection_param_table[table_index];//static ram

    log_info("update_request:-%d-%d-%d-%d-\n", param->interval_min, param->interval_max, param->latency, param->timeout);
    if (con_handle) {
        ble_op_conn_param_request(con_handle, param);
    }
}

static void check_connetion_updata_deal(void)
{
    if (connection_update_enable) {
        if (connection_update_cnt < CONN_PARAM_TABLE_CNT) {
            send_request_connect_parameter(connection_update_cnt);
        }
    }
}

static void connection_update_complete_success(u8 *packet)
{
    int con_handle, conn_interval, conn_latency, conn_timeout;

    con_handle = hci_subevent_le_connection_update_complete_get_connection_handle(packet);
    conn_interval = hci_subevent_le_connection_update_complete_get_conn_interval(packet);
    conn_latency = hci_subevent_le_connection_update_complete_get_conn_latency(packet);
    conn_timeout = hci_subevent_le_connection_update_complete_get_supervision_timeout(packet);

    log_info("conn_interval = %d\n", conn_interval);
    log_info("conn_latency = %d\n", conn_latency);
    log_info("conn_timeout = %d\n", conn_timeout);
}


static void set_ble_work_state(ble_state_e state)
{
    if (state != ble_work_state) {
        log_info("ble_work_st:%x->%x\n", ble_work_state, state);
        ble_work_state = state;
        if (app_ble_state_callback) {
            app_ble_state_callback((void *)channel_priv, state);
        }
    }
}

ble_state_e gfps_get_ble_work_state(void)
{
    return ble_work_state;
}

static void cbk_sm_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    sm_just_event_t *event = (void *)packet;
    u32 tmp32;
    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {
        case SM_EVENT_JUST_WORKS_REQUEST:
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            log_info("Just Works Confirmed.\n");
            break;
        case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
            log_info_hexdump(packet, size);
            memcpy(&tmp32, event->data, 4);
            log_info("Passkey display: %06u.\n", tmp32);
            break;
        }
        break;
    }
}


static void can_send_now_wakeup(void)
{
    /* putchar('E'); */
    if (ble_resume_send_wakeup) {
        ble_resume_send_wakeup();
    }
}
const char *const phy_result[] = {
    "None",
    "1M",
    "2M",
    "Coded",
};

static void set_connection_data_length(u16 tx_octets, u16 tx_time)
{
    if (con_handle) {
        ble_op_set_data_length(con_handle, tx_octets, tx_time);
    }
}

static void set_connection_data_phy(u8 tx_phy, u8 rx_phy)
{
    if (0 == con_handle) {
        return;
    }

    u8 all_phys = 0;
    u16 phy_options = CONN_SET_PHY_OPTIONS_S8;

    ble_op_set_ext_phy(con_handle, all_phys, tx_phy, rx_phy, phy_options);
}

static void server_profile_start(u16 con_handle)
{
    ble_op_att_send_init(con_handle, att_ram_buffer, ATT_RAM_BUFSIZE, ATT_LOCAL_MTU_SIZE);
    set_ble_work_state(BLE_ST_CONNECT);

    /* set_connection_data_phy(CONN_SET_CODED_PHY, CONN_SET_CODED_PHY); */
}

_WEAK_
u8 ble_update_get_ready_jump_flag(void)
{
    return 0;
}
/*
 * @section Packet Handler
 *
 * @text The packet handler is used to:
 *        - stop the counter after a disconnect
 *        - send a notification when the requested ATT_EVENT_CAN_SEND_NOW is received
 */

/* LISTING_START(packetHandler): Packet Handler */
static void cbk_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    int mtu;
    u32 tmp;
    u8 status;
    const char *attribute_name;

    //printf("cbk packet_type:0x%x, packet[0]:0x%x, packet[2]:0x%x", packet_type, packet[0], packet[2]);
    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {

        /* case DAEMON_EVENT_HCI_PACKET_SENT: */
        /* break; */
        case ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE:
            log_info("ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE\n");
        case ATT_EVENT_CAN_SEND_NOW:
            can_send_now_wakeup();
            break;

        case HCI_EVENT_LE_META:
            switch (hci_event_le_meta_get_subevent_code(packet)) {
            case HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE:
                status = hci_subevent_le_enhanced_connection_complete_get_status(packet);
                if (status) {
                    log_info("LE_SLAVE CONNECTION FAIL!!! %0x\n", status);
                    set_ble_work_state(BLE_ST_DISCONN);
                    break;
                }
                con_handle = hci_subevent_le_enhanced_connection_complete_get_connection_handle(packet);
                log_info("HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE : %0x\n", con_handle);
                log_info("conn_interval = %d\n", hci_subevent_le_enhanced_connection_complete_get_conn_interval(packet));
                log_info("conn_latency = %d\n", hci_subevent_le_enhanced_connection_complete_get_conn_latency(packet));
                log_info("conn_timeout = %d\n", hci_subevent_le_enhanced_connection_complete_get_supervision_timeout(packet));
                server_profile_start(con_handle);
                break;

            case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                con_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                log_info("HCI_SUBEVENT_LE_CONNECTION_COMPLETE: %0x\n", con_handle);
                connection_update_complete_success(packet + 8);
                server_profile_start(con_handle);

                log_info("ble remote rssi= %d\n", ble_vendor_get_peer_rssi(con_handle));
                break;

            case HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE:
                log_info("HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE\n");
                connection_update_complete_success(packet);
                break;

            case HCI_SUBEVENT_LE_DATA_LENGTH_CHANGE:
                log_info("APP HCI_SUBEVENT_LE_DATA_LENGTH_CHANGE\n");
                /* set_connection_data_phy(CONN_SET_CODED_PHY, CONN_SET_CODED_PHY); */
                break;

            case HCI_SUBEVENT_LE_PHY_UPDATE_COMPLETE:
                log_info("APP HCI_SUBEVENT_LE_PHY_UPDATE %s\n", hci_event_le_meta_get_phy_update_complete_status(packet) ? "Fail" : "Succ");
                log_info("Tx PHY: %s\n", phy_result[hci_event_le_meta_get_phy_update_complete_tx_phy(packet)]);
                log_info("Rx PHY: %s\n", phy_result[hci_event_le_meta_get_phy_update_complete_rx_phy(packet)]);
                break;
            default:
                /* log_info("default HCI_EVENT_LE_META type: 0x%x", hci_event_le_meta_get_subevent_code(packet)); */
                break;
            }
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            log_info("HCI_EVENT_DISCONNECTION_COMPLETE: %0x\n", packet[5]);

            con_handle = 0;
            ble_op_att_send_init(con_handle, 0, 0, 0);
            set_ble_work_state(BLE_ST_DISCONN);

            if (!ble_update_get_ready_jump_flag()) {
                gfps_bt_ble_adv_enable(1);
            }
            connection_update_cnt = 0;
            if (gfps_auto_test_mode == 1) {
                extern void gfps_set_pair_mode(void);
                sys_timeout_add(NULL, gfps_set_pair_mode, 10000);
            }
            __set_simple_pair_param(3, 0, 0);
            break;

        case ATT_EVENT_MTU_EXCHANGE_COMPLETE:
            mtu = att_event_mtu_exchange_complete_get_MTU(packet) - 3;
            log_info("ATT MTU = %u\n", mtu);
            ble_op_att_set_send_mtu(mtu);
            /* set_connection_data_length(251, 2120); */
            break;

        case HCI_EVENT_VENDOR_REMOTE_TEST:
            log_info("--- HCI_EVENT_VENDOR_REMOTE_TEST\n");
            break;

        case L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_RESPONSE:
            tmp = little_endian_read_16(packet, 4);
            log_info("-update_rsp: %02x\n", tmp);
            if (tmp) {
                connection_update_cnt++;
                log_info("remoter reject!!!\n");
                check_connetion_updata_deal();
            } else {
                connection_update_cnt = CONN_PARAM_TABLE_CNT;
            }
            break;

        case HCI_EVENT_ENCRYPTION_CHANGE:
            log_info("HCI_EVENT_ENCRYPTION_CHANGE= %d\n", packet[2]);
            break;

        default:
            /* log_info("default HCI_EVENT_PACKET type: 0x%x", hci_event_packet_get_type(packet)); */
            break;


        }
        break;
    }
}

static uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{

    uint16_t  att_value_len = 0;
    uint16_t handle = att_handle;

    log_info("<-------------read_callback, handle= 0x%04x,buffer= %08x\n", handle, (u32)buffer);

    switch (handle) {
    case ATT_CHARACTERISTIC_2A00_01_VALUE_HANDLE:
        att_value_len = gap_device_name_len;

        if ((offset >= att_value_len) || (offset + buffer_size) > att_value_len) {
            break;
        }

        if (buffer) {
            memcpy(buffer, &gap_device_name[offset], buffer_size);
            att_value_len = buffer_size;
            log_info("\n------read gap_name: %s \n", gap_device_name);
        }
        break;

    //model id
    case ATT_CHARACTERISTIC_FE2C1233_8366_4814_8EB0_01DE32100BEA_01_VALUE_HANDLE:

        if (buffer) {
            memcpy(buffer, &gfps_model_id, buffer_size);
            att_value_len = buffer_size;
            log_info("\n------read gfps_model_id: %s \n", gfps_model_id);
        }
        break;

    //Key-based Pairing
    case ATT_CHARACTERISTIC_FE2C1234_8366_4814_8EB0_01DE32100BEA_01_CLIENT_CONFIGURATION_HANDLE:
    //Additional Data
    case ATT_CHARACTERISTIC_FE2C1237_8366_4814_8EB0_01DE32100BEA_01_CLIENT_CONFIGURATION_HANDLE:
    //Passkey
    case ATT_CHARACTERISTIC_FE2C1235_8366_4814_8EB0_01DE32100BEA_01_CLIENT_CONFIGURATION_HANDLE:
        if (buffer) {
            buffer[0] = att_get_ccc_config(handle);
            buffer[1] = 0;
        }
        att_value_len = 2;
        break;

    // Firmware Revision
    case ATT_CHARACTERISTIC_2A26_01_VALUE_HANDLE:
        log_info("read Firmware Revision");
        break;

    default:
        break;
    }

    log_info("att_value_len= %d\n", att_value_len);
    return att_value_len;

}

static int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    int result = 0;
    u16 tmp16;

    u16 handle = att_handle;

    log_info("<-------------write_callback, handle= 0x%04x,size = %d\n", handle, buffer_size);
    log_info_hexdump(buffer, buffer_size);

    switch (handle) {
    case ATT_CHARACTERISTIC_2A00_01_VALUE_HANDLE:
        break;

    //Key-based Pairing 0x0008
    case ATT_CHARACTERISTIC_FE2C1234_8366_4814_8EB0_01DE32100BEA_01_VALUE_HANDLE:
        gfps_encrypted_request(buffer, buffer_size);
        break;

    //Key-based Pairing 0x0009
    case ATT_CHARACTERISTIC_FE2C1234_8366_4814_8EB0_01DE32100BEA_01_CLIENT_CONFIGURATION_HANDLE:
    //Additional Data 0x0011
    case ATT_CHARACTERISTIC_FE2C1237_8366_4814_8EB0_01DE32100BEA_01_CLIENT_CONFIGURATION_HANDLE:
    //Passkey 0x000c
    case ATT_CHARACTERISTIC_FE2C1235_8366_4814_8EB0_01DE32100BEA_01_CLIENT_CONFIGURATION_HANDLE:
        set_ble_work_state(BLE_ST_NOTIFY_IDICATE);
        check_connetion_updata_deal();
        log_info("\n------write ccc:%04x,%02x\n", handle, buffer[0]);
        att_set_ccc_config(handle, buffer[0]);
        break;

    //Additional Data 0x0010
    case ATT_CHARACTERISTIC_FE2C1237_8366_4814_8EB0_01DE32100BEA_01_VALUE_HANDLE:
        gfps_additional_data_get(buffer, buffer_size);
        break;
    //Passkey 0x000b
    case ATT_CHARACTERISTIC_FE2C1235_8366_4814_8EB0_01DE32100BEA_01_VALUE_HANDLE:
        gfps_passkey_req(buffer, buffer_size);
        break;

    //Account Key 0x000e
    case ATT_CHARACTERISTIC_FE2C1236_8366_4814_8EB0_01DE32100BEA_01_VALUE_HANDLE:
        gfps_accountkey_rec(buffer, buffer_size);
    default:
        break;
    }

    return 0;
}

static int app_send_user_data(u16 handle, u8 *data, u16 len, u8 handle_type)
{
    u32 ret = APP_BLE_NO_ERROR;

    if (!con_handle) {
        return APP_BLE_OPERATION_ERROR;
    }

    if (!att_get_ccc_config(handle + 1)) {
        log_info("fail,no write ccc!!!,%04x\n", handle + 1);
        return APP_BLE_NO_WRITE_CCC;
    }

    ret = ble_op_att_send_data(handle, data, len, handle_type);
    if (ret == BLE_BUFFER_FULL) {
        ret = APP_BLE_BUFF_FULL;
    }

    if (ret) {
        log_info("app_send_fail:%d !!!!!!\n", ret);
    }
    return ret;
}

void gfps_set_model_id(uint8_t *model_id)
{
    memcpy(gfps_model_id, model_id, 3);
    log_info("set_model_id:");
    log_info_hexdump(gfps_model_id, 3);
}

//------------------------------------------------------
static int make_set_adv_data(void)
{
    u8 offset = 0;
    u8 *buf = gfps_adv_data;

    u8 model_id[5];
    model_id[0] = 0x2C;
    model_id[1] = 0xFE;
    memcpy(&model_id[2], gfps_model_id, 3);

    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_FLAGS, 0x0A, 1);

    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_16BIT_SERVICE_UUIDS, 0x05D6, 2);

    offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_SERVICE_DATA, (void *)model_id, 5);

    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_TX_POWER_LEVEL, gfps_tx_power, 1);

    if (offset > ADV_RSP_PACKET_MAX) {
        puts("***adv_data overflow!!!!!!\n");
        return -1;
    }
    log_info("adv_data(%d):", offset);
    log_info_hexdump(buf, offset);
    adv_data_len = offset;
    ble_op_set_adv_data(offset, buf);
    return 0;
}

static int make_set_adv_data_not_discoverable(void)
{
    u8 offset = 0;
    u8 *buf = gfps_adv_data;

    gfps_account_adv_data_set();
    //  gfps_account_data_len = filter_len + 4{3flag + 1salt} + UUID
    uint8_t gfps_account_data_len = (gfps_account_data[1] >> 4) + 4 + 2;
    uint8_t adv_data[GFPS_ACCOUNT_DATA_LEN_MAX + 2 + 4];
    adv_data[0] = 0x2C;
    adv_data[1] = 0xFE;

    if (gfps_get_account_key_num() == 0) {
        gfps_account_data_len = 2 + 2;
        u8 empty_buf[2] = {0x00, 0x00};// flag + Account Key Data
        memcpy(&adv_data[2], empty_buf, gfps_account_data_len - 2);
    } else {
        memcpy(&adv_data[2], gfps_account_data, gfps_account_data_len - 2);
    }

    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_FLAGS, 0x0A, 1);

    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_16BIT_SERVICE_UUIDS, 0x05D6, 2);

#if BATTERY_SHOW_ENABLE
    uint8_t bat_data[4];
    gfps_battery_data_update(bat_data);
    memcpy(&adv_data[gfps_account_data_len], bat_data, 4);
    offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_SERVICE_DATA, (void *)adv_data, gfps_account_data_len + 4); // filter_len + 4(3flag + 1salt) + UUID
#else
    offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_SERVICE_DATA, (void *)adv_data, gfps_account_data_len); // filter_len + 4(3flag + 1salt) + UUID
#endif

    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_TX_POWER_LEVEL, gfps_tx_power, 1);

    if (offset > ADV_RSP_PACKET_MAX) {
        puts("***adv_data overflow!!!!!!\n");
        return -1;
    }
    log_info("adv_data(%d):", offset);
    log_info_hexdump(buf, offset);
    adv_data_len = offset;
    ble_op_set_adv_data(offset, buf);
    return 0;
}

static int make_set_rsp_data(void)
{
    u8 offset = 0;
    u8 *buf = scan_rsp_data;

    u8 name_len = gap_device_name_len;
    u8 vaild_len = ADV_RSP_PACKET_MAX - (offset + 2);
    if (name_len > vaild_len) {
        name_len = vaild_len;
    }
    offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME, (void *)gap_device_name, name_len);

    if (offset > ADV_RSP_PACKET_MAX) {
        puts("***rsp_data overflow!!!!!!\n");
        return -1;
    }

    log_info("rsp_data(%d):", offset);
    log_info_hexdump(buf, offset);
    scan_rsp_data_len = offset;
    ble_op_set_rsp_data(offset, buf);
    return 0;
}

void gfps_adv_interval_set(u8 value)
{
    adv_interval_min = value;
}

//广播参数设置
static void advertisements_setup_init()
{
    uint8_t adv_type = ADV_IND;
    uint8_t adv_channel = ADV_CHANNEL_ALL;
    int ret = 0;

    ble_op_set_adv_param(adv_interval_min, adv_type, adv_channel);

    if (get_gfps_pair_state()) {
        ret |= make_set_adv_data_not_discoverable();
    } else {
        ret |= make_set_adv_data();
    }
    ret |= make_set_rsp_data();

    if (ret) {
        puts("advertisements_setup_init fail !!!!!!\n");
        return;
    }

}

#define PASSKEY_ENTER_ENABLE      1 //输入passkey使能，可修改passkey
//重设passkey回调函数，在这里可以重新设置passkey
//passkey为6个数字组成，十万位、万位。。。。个位 各表示一个数字 高位不够为0
static void reset_passkey_cb(u32 *key)
{
#if 1
    u32 newkey = rand32();//获取随机数

    newkey &= 0xfffff;
    if (newkey > 999999) {
        newkey = newkey - 999999; //不能大于999999
    }
    *key = newkey; //小于或等于六位数
    log_info("set new_key= %06u\n", *key);
#else
    *key = 123456; //for debug
#endif
}

void ble_sm_setup_init(io_capability_t io_type, u8 auth_req, uint8_t min_key_size, u8 security_en)
{
    //setup SM: Display only
    sm_init();
    sm_set_io_capabilities(io_type);
    sm_set_authentication_requirements(auth_req);
    sm_set_encryption_key_size_range(min_key_size, 16);
    sm_set_request_security(security_en);
    sm_event_callback_set(&cbk_sm_packet_handler);

    if (io_type == IO_CAPABILITY_DISPLAY_ONLY) {
        reset_PK_cb_register(reset_passkey_cb);
    }
}


#define TRANS_TCFG_BLE_SECURITY_EN          0/*是否发请求加密命令*/
void ble_profile_init(void)
{
    log_info("ble profile init\n");
    le_device_db_init();

#if PASSKEY_ENTER_ENABLE
    ble_sm_setup_init(IO_CAPABILITY_DISPLAY_YES_NO, SM_AUTHREQ_MITM_PROTECTION | SM_AUTHREQ_BONDING, 7, TRANS_TCFG_BLE_SECURITY_EN);
#else
    ble_sm_setup_init(IO_CAPABILITY_NO_INPUT_NO_OUTPUT, SM_AUTHREQ_MITM_PROTECTION | SM_AUTHREQ_BONDING, 7, TRANS_TCFG_BLE_SECURITY_EN);
#endif

    /* setup ATT server */
    att_server_init(profile_data, att_read_callback, att_write_callback);
    att_server_register_packet_handler(cbk_packet_handler);
    /* gatt_client_register_packet_handler(packet_cbk); */

    // register for HCI events
    hci_event_callback_set(&cbk_packet_handler);
    /* ble_l2cap_register_packet_handler(packet_cbk); */
    /* sm_event_packet_handler_register(packet_cbk); */
    le_l2cap_register_packet_handler(&cbk_packet_handler);

    ble_vendor_set_default_att_mtu(ATT_LOCAL_MTU_SIZE);
}

static int set_adv_enable(void *priv, u32 en)
{
    ble_state_e next_state, cur_state;

    if (!adv_ctrl_en && en) {
        log_info("adv ret 1, en:%d, adv_ctrl_en:%d\n", en, adv_ctrl_en);
        return APP_BLE_OPERATION_ERROR;
    }

    if (con_handle) {
        log_info("adv ret 2, con_handle:%d\n", con_handle);
        return APP_BLE_OPERATION_ERROR;
    }

    if (en) {
        next_state = BLE_ST_ADV;
    } else {
        next_state = BLE_ST_IDLE;
    }

    cur_state =  gfps_get_ble_work_state();
    switch (cur_state) {
    case BLE_ST_ADV:
    case BLE_ST_IDLE:
    case BLE_ST_INIT_OK:
    case BLE_ST_NULL:
    case BLE_ST_DISCONN:
        break;
    default:
        log_info("adv ret 3");
        return APP_BLE_OPERATION_ERROR;
        break;
    }

    if (cur_state == next_state) {
        log_info("adv ret 4, cur_state:%d", cur_state);
        return APP_BLE_NO_ERROR;
    }
    log_info("adv_en:%d\n", en);
    set_ble_work_state(next_state);

    if (en) {
        if (!gfps_check_tws_is_master()) {
            log_info("slave don't open adv\n");
            return APP_BLE_NO_ERROR;
        }
    }

    if (en) {
        advertisements_setup_init();
    }

    ble_op_adv_enable(en);

    return APP_BLE_NO_ERROR;
}

static int ble_disconnect(void *priv)
{
    if (con_handle) {
        if (BLE_ST_SEND_DISCONN != gfps_get_ble_work_state()) {
            log_info(">>>ble send disconnect\n");
            set_ble_work_state(BLE_ST_SEND_DISCONN);
            ble_op_disconnect(con_handle);
        } else {
            log_info(">>>ble wait disconnect...\n");
        }
        return APP_BLE_NO_ERROR;
    } else {
        return APP_BLE_OPERATION_ERROR;
    }
}


static int get_buffer_vaild_len(void *priv)
{
    u32 vaild_len = 0;
    ble_op_att_get_remain(&vaild_len);
    return vaild_len;
}

static int gfps_send_user_data_do(u16 handle, u8 *data, u16 len)
{
#if 0//PRINT_DMA_DATA_EN
    if (len < 128) {
        printf("-le_tx(%d):");
        put_buf(data, len);
    } else {
        putchar('L');
    }
#endif
    return app_send_user_data(handle, data, len, ATT_OP_AUTO_READ_CCC);
}

void gfps_key_base_pair_data_send(u8 *data, u16 len)
{
    gfps_send_user_data_do(ATT_CHARACTERISTIC_FE2C1234_8366_4814_8EB0_01DE32100BEA_01_VALUE_HANDLE, data, len);
}

void gfps_passkey_data_send(u8 *data, u16 len)
{
    gfps_send_user_data_do(ATT_CHARACTERISTIC_FE2C1235_8366_4814_8EB0_01DE32100BEA_01_VALUE_HANDLE, data, len);
}

void gfps_additional_data_send(u8 *data, u16 len)
{
    gfps_send_user_data_do(ATT_CHARACTERISTIC_FE2C1237_8366_4814_8EB0_01DE32100BEA_01_VALUE_HANDLE, data, len);
}

static int app_send_user_data_check(u16 len)
{
    u32 buf_space = get_buffer_vaild_len(0);
    if (len <= buf_space) {
        return 1;
    }
    return 0;
}

static int regiest_wakeup_send(void *priv, void *cbk)
{
    ble_resume_send_wakeup = cbk;
    return APP_BLE_NO_ERROR;
}

static int regiest_recieve_cbk(void *priv, void *cbk)
{
    channel_priv = (u32)priv;
    app_recieve_callback = cbk;
    return APP_BLE_NO_ERROR;
}

static int regiest_state_cbk(void *priv, void *cbk)
{
    channel_priv = (u32)priv;
    app_ble_state_callback = cbk;
    return APP_BLE_NO_ERROR;
}

void gfps_tx_power_set(u8 value)
{
    gfps_tx_power = value;
}


void gfps_bt_ble_adv_enable(u8 enable)
{
    set_adv_enable(0, enable);
}

u16 bt_ble_is_connected(void)
{
    return con_handle;
}

static void ble_module_enable(u8 en)
{
    log_info("mode_en:%d\n", en);
    if (en) {
        adv_ctrl_en = 1;
        gfps_bt_ble_adv_enable(1);
    } else {
        if (con_handle) {
            adv_ctrl_en = 0;
            ble_disconnect(NULL);
        } else {
            gfps_bt_ble_adv_enable(0);
            adv_ctrl_en = 0;
        }
    }
}


void bt_ble_init(void)
{
    log_info("***** fgps ble_init******\n");
    log_info("ble_file: %s", __FILE__);

    char *name_p;

    u8 ext_name_len = 5;

    name_p = bt_get_local_name();
    gap_device_name_len = strlen(name_p);
    if (gap_device_name_len > BT_NAME_LEN_MAX - ext_name_len - 1) {
        gap_device_name_len = BT_NAME_LEN_MAX - ext_name_len - 1;//1是结束符
    }

    memcpy(gap_device_name, name_p, gap_device_name_len);

    //增加后缀，区分名字
    memcpy(&gap_device_name[gap_device_name_len], "(BLE)", ext_name_len);
    gap_device_name_len += ext_name_len;
    gap_device_name[gap_device_name_len] = 0;//结束符
    log_info("ble name(%d): %s \n", gap_device_name_len, gap_device_name);

    gfps_app_init();
    set_ble_work_state(BLE_ST_INIT_OK);
    ble_module_enable(1);
}

void bt_ble_exit(void)
{
    log_info("***** gfps ble_exit******\n");

    ble_module_enable(0);
}

int ble_app_disconnect(void)
{
    ble_disconnect(NULL);
    return 0;
}

int gfps_all_init(void)
{
    gfps_bt_ble_init();
    return 0;
}

int gfps_all_exit(void)
{
    bt_ble_exit();
    return 0;
}

void bt_ble_adv_enable(u8 enable)
{
    set_adv_enable(0, enable);
}

int gfps_disconnect(void)
{
    ble_app_disconnect();
}

static const struct ble_server_operation_t mi_ble_operation = {
    .adv_enable = set_adv_enable,
    .disconnect = ble_disconnect,
    .get_buffer_vaild = get_buffer_vaild_len,
    .regist_wakeup_send = regiest_wakeup_send,
    .regist_recieve_cbk = regiest_recieve_cbk,
    .regist_state_cbk = regiest_state_cbk,
};

void gfp_ble_get_server_operation_table(struct ble_server_operation_t **interface_pt)
{
    *interface_pt = (void *)&mi_ble_operation;
}

#endif

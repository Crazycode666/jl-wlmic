#ifndef __GFPS_MSG_L_H__
#define __GFPS_MSG_L_H__

#include "typedef.h"
#include "GFPS_config.h"

//APP_PROTOCOL获取电量的类型
#define APP_PROTOCOL_BAT_T_CHARGE_FLAG   0
#define APP_PROTOCOL_BAT_T_MAIN          1
#define APP_PROTOCOL_BAT_T_BOX           2
#define APP_PROTOCOL_BAT_T_TWS_LEFT      3
#define APP_PROTOCOL_BAT_T_TWS_RIGHT     4
#define APP_PROTOCOL_BAT_T_TWS_SIBLING   5
#define APP_PROTOCOL_BAT_T_LOW_POWER     6
#define APP_PROTOCOL_BAT_T_MAX           8

// GFPS私有消息
enum {
    APP_PROTOCOL_GFPS_RING_STOP_ALL                     =   0x000A,
    APP_PROTOCOL_GFPS_RING_RIGHT,
    APP_PROTOCOL_GFPS_RING_LEFT,
    APP_PROTOCOL_GFPS_RING_ALL,
    APP_PROTOCOL_LIB_TWS_DATA_SYNC,
};

#define GFPS_RFCOMM_MSG_GROUP_BLUETOOTH_EVENT           0X01
#define GFPS_RFCOMM_MSG_CODE_ENABLE_SLIENCE_MODE        0X01
#define GFPS_RFCOMM_MSG_CODE_DISABLE_SLIENCE_MODE       0X02


#define GFPS_RFCOMM_MSG_GROUP_COMPANION_APP_EVENT       0X02
#define GFPS_RFCOMM_MSG_CODE_LOG_BUFFER_FULL            0X01

#define GFPS_RFCOMM_MSG_GROUP_DEVICE_INFO_EVENT         0X03
#define GFPS_RFCOMM_MSG_CODE_MODEL_ID                   0X01
#define GFPS_RFCOMM_MSG_CODE_BLE_ADDR_UPDATED           0X02
#define GFPS_RFCOMM_MSG_CODE_BATTERY_UPDATED            0X03
#define GFPS_RFCOMM_MSG_CODE_REMAIN_BATTERY_TIME        0X04
#define GFPS_RFCOMM_MSG_CODE_ACTIVE_COMPONENTS_REQ      0X05
#define GFPS_RFCOMM_MSG_CODE_ACTIVE_COMPONENTS_RSP      0X06
#define GFPS_RFCOMM_MSG_CODE_CAPABILITIES               0X07
#define GFPS_RFCOMM_MSG_CODE_PLATFORM_TYPE              0X08

#define GFPS_RFCOMM_MSG_GROUP_DEVICE_ACTION_EVENT       0X04
#define GFPS_RFCOMM_MSG_CODE_RING_DEVICE                0X01
#define GFPS_RING_STOP_ALL                              0X00
#define GFPS_RING_RIGHT                                 0X01
#define GFPS_RING_LEFT                                  0X02
#define GFPS_RING_ALL                                   0X03

#define GFPS_RFCOMM_MSG_GROUP_ACKNOWLEDGEMENT_EVENT     0xFF
#define GFPS_RFCOMM_MSG_CODE_ACKNOWLEDGEMENT_ACK        0X01
#define GFPS_RFCOMM_MSG_CODE_ACKNOWLEDGEMENT_NAK        0X02

#define GFPS_SIBLING_SYNC_TYPE_ACCOUNT_KEY      0x01
#define GFPS_SIBLING_SYNC_TYPE_PERSONAL_NAME    0x02
#define GFPS_SIBLING_SYNC_TYPE_RING_HEADST      0x03





/* Maximum Account Key Filter Length */
#define GFPS_ACCOUNT_KEY_FILTER_LEN_MAX     (ACCOUNT_KEY_SIZE* 1) + \
                                            (ACCOUNT_KEY_SIZE* 2 / 10) + \
                                            3

/* Definition used for BLE Advertisement Data. */
#define GFPS_ACCOUNT_KEY_DATA_RANDOM_SALT_FIELD_LEN             sizeof(uint8_t) + sizeof(uint8_t)
#define GFPS_ACCOUNT_KEY_DATA_ACCOUNT_KEY_FILTER_FIELD_LEN_MAX  GFPS_ACCOUNT_KEY_FILTER_LEN_MAX + sizeof(uint8_t)
#define GFPS_ACCOUNT_KEY_DATA_LEN_MAX                           GFPS_ACCOUNT_KEY_DATA_ACCOUNT_KEY_FILTER_FIELD_LEN_MAX + \
                                                                GFPS_ACCOUNT_KEY_DATA_RANDOM_SALT_FIELD_LEN
#define GFPS_ACCOUNT_DATA_LEN_MAX                               GFPS_ACCOUNT_KEY_DATA_LEN_MAX + sizeof(uint8_t)

#endif

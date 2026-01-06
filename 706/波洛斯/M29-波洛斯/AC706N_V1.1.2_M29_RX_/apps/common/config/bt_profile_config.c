#include "system/includes.h"
#include "app_config.h"
#include "btcontroller_config.h"
#include "btstack/bt_profile_config.h"
#include "bt_common.h"

#define LOG_TAG     "[BT-CFG]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#include "debug.h"

#if (LEA_BIG_CTRLER_TX_EN || LEA_BIG_CTRLER_RX_EN || LEA_CIG_CENTRAL_EN || LEA_CIG_PERIPHERAL_EN || LE_AUDIO_TEST_EN)
const u8 hci_inquiry_support = 0;
const u8 btstack_emitter_support  = 0;  /*定义用于优化代码编译*/
const u8 a2dp_mutual_support = 0;
const u8 adt_profile_support = 0;
const u8 pbg_support_enable = 0;
const u8 more_avctp_cmd_support = 0;
const int config_stack_modules = BT_BTSTACK_LE;

#else

const u8 hci_inquiry_support = 0;
const u8 btstack_emitter_support  = 0;  /*定义用于优化代码编译*/
const u8 a2dp_mutual_support = 0;
const u8 adt_profile_support = 0;
const u8 pbg_support_enable = 0;
const u8 more_avctp_cmd_support = 0;
const int config_stack_modules = 0;

#endif


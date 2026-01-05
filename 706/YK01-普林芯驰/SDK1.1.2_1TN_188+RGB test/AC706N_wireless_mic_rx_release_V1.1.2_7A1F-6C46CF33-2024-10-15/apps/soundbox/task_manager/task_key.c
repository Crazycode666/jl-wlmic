#include "key_event_deal.h"
#include "key_driver.h"
#include "app_config.h"
#include "board_config.h"
#include "app_task.h"

//ad key
extern const u16 linein_key_ad_table[KEY_AD_NUM_MAX][KEY_EVENT_MAX];
extern const u16 pc_key_ad_table[KEY_AD_NUM_MAX][KEY_EVENT_MAX];
extern const u16 idle_key_ad_table[KEY_AD_NUM_MAX][KEY_EVENT_MAX];
extern const u16 live_mic_key_ad_table[KEY_AD_NUM_MAX][KEY_EVENT_MAX];
extern const u16 live_iis_key_ad_table[KEY_AD_NUM_MAX][KEY_EVENT_MAX];
/***********************************************************
 *				adkey table 映射管理
 ***********************************************************/
typedef const u16(*type_key_ad_table)[KEY_EVENT_MAX];
static const type_key_ad_table ad_table[APP_TASK_MAX_INDEX] = {
#if TCFG_APP_LINEIN_EN
    [APP_LINEIN_TASK] 	= linein_key_ad_table,
#endif
#if TCFG_APP_PC_EN
    [APP_PC_TASK] 		= pc_key_ad_table,
#endif
#if TCFG_APP_LIVE_MIC_EN
    [APP_LIVE_MIC_TASK]   = live_mic_key_ad_table,
#endif
#if TCFG_APP_LIVE_IIS_EN
    [APP_LIVE_IIS_TASK]   = live_iis_key_ad_table,
#endif
    [APP_IDLE_TASK]     = idle_key_ad_table,
};

u16 adkey_event_to_msg(u8 cur_task, struct key_event *key)
{
    if (ad_table[cur_task] == NULL) {
        return KEY_NULL;
    }

    type_key_ad_table cur_task_ad_table = ad_table[cur_task];
    return	cur_task_ad_table[key->value][key->event];
}

//io key
extern const u16 linein_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX];
extern const u16 pc_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX];
extern const u16 idle_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX];
extern const u16 live_mic_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX];
extern const u16 live_iis_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX];
/***********************************************************
 *				iokey table 映射管理
 ***********************************************************/
typedef const u16(*type_key_io_table)[KEY_EVENT_MAX];
static const type_key_io_table io_table[APP_TASK_MAX_INDEX] = {
#if TCFG_APP_LINEIN_EN
    [APP_LINEIN_TASK] 	= linein_key_io_table,
#endif
#if TCFG_APP_PC_EN
    [APP_PC_TASK] 		= pc_key_io_table,
#endif
#if TCFG_APP_LIVE_MIC_EN
    [APP_LIVE_MIC_TASK]	= live_mic_key_io_table,
#endif
#if TCFG_APP_LIVE_IIS_EN
    [APP_LIVE_IIS_TASK]	= live_iis_key_io_table,
#endif
    [APP_IDLE_TASK] 	= idle_key_io_table,
};

u16 iokey_event_to_msg(u8 cur_task, struct key_event *key)
{
    if (io_table[cur_task] == NULL) {
        return KEY_NULL;
    }

    type_key_io_table cur_task_io_table = io_table[cur_task];
    return	cur_task_io_table[key->value][key->event];
}

//touch key
extern const u16 linein_key_touch_table[KEY_TOUCH_NUM_MAX][KEY_EVENT_MAX];
extern const u16 pc_key_touch_table[KEY_TOUCH_NUM_MAX][KEY_EVENT_MAX];
extern const u16 idle_key_touch_table[KEY_TOUCH_NUM_MAX][KEY_EVENT_MAX];
extern const u16 live_mic_key_touch_table[KEY_AD_NUM_MAX][KEY_EVENT_MAX];
extern const u16 live_iis_key_touch_table[KEY_AD_NUM_MAX][KEY_EVENT_MAX];
/***********************************************************
 *				touch_key table 映射管理
 ***********************************************************/
typedef const u16(*type_key_touch_table)[KEY_EVENT_MAX];
static const type_key_touch_table touch_table[APP_TASK_MAX_INDEX] = {
#if TCFG_APP_LINEIN_EN
    [APP_LINEIN_TASK] 	= linein_key_touch_table,
#endif
#if TCFG_APP_LIVE_MIC_EN
    [APP_LIVE_MIC_TASK]	= live_mic_key_touch_table,
#endif
#if TCFG_APP_LIVE_IIS_EN
    [APP_LIVE_IIS_TASK]	= live_iis_key_touch_table,
#endif
    [APP_IDLE_TASK] 	= idle_key_touch_table,
};

u16 touch_key_event_to_msg(u8 cur_task, struct key_event *key)
{
    if (touch_table[cur_task] == NULL) {
        return KEY_NULL;
    }

    type_key_touch_table cur_task_touch_table = touch_table[cur_task];
    return	cur_task_touch_table[key->value][key->event];
}

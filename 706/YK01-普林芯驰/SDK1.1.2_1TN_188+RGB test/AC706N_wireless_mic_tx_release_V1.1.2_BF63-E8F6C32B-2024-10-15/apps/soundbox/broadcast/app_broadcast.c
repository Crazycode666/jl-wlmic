/*********************************************************************************************
    *   Filename        : app_broadcast.c

    *   Description     :

    *   Author          : Weixin Liang

    *   Email           : liangweixin@zh-jieli.com

    *   Last modifiled  : 2022-12-15 14:18

    *   Copyright:(c)JIELI  2011-2022  @ , All Rights Reserved.
*********************************************************************************************/
#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".app_broadcast_bss")
#pragma data_seg(".app_broadcast_data")
#pragma code_seg(".app_broadcast_text")
#pragma const_seg(".app_broadcast_const")
#endif

#include "system/includes.h"
#include "broadcast_api.h"
#include "app_broadcast.h"
#include "app_config.h"
#include "app_cfg.h"
#include "bt/bt.h"
#include "app_task.h"
#include "btstack/avctp_user.h"
#include "tone_player.h"
#include "app_main.h"
#include "audio_mode.h"
#if TCFG_LINEIN_ENABLE
#include "linein/linein.h"
#endif
#include "music_player.h"

#if TCFG_BROADCAST_ENABLE
#include "ui_manage.h"
/**************************************************************************************************
  Macros
**************************************************************************************************/
#define LOG_TAG_CONST       APP
#define LOG_TAG             "[BC]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#define TRANSMITTER_AUTO_TEST_EN    0

/**************************************************************************************************
  Data Types
**************************************************************************************************/
struct app_big_hdl_info {
    u8 used;
    u16 volatile big_status;
    big_hdl_t hdl;
};

/**************************************************************************************************
  Static Prototypes
**************************************************************************************************/
static void broadcast_pair_tx_event_callback(const PAIR_EVENT event, void *priv);
static void broadcast_pair_rx_event_callback(const PAIR_EVENT event, void *priv);

/**************************************************************************************************
  Global Variables
**************************************************************************************************/
static OS_MUTEX mutex;
static u8 broadcast_last_role = 0; /*!< 挂起前广播角色 */
static u8 broadcast_app_mode_exit = 0;  /*!< 音源模式退出标志 */
static struct app_big_hdl_info app_big_hdl_info[BIG_MAX_NUMS];
static u8 config_broadcast_as_master = 0;   /*!< 配置广播强制做主机 */
static u8 broadcast_in_pair_mode = 0;   /*!< 设备处于配对码交换模式 */
static const pair_callback_t pair_tx_cb = {
    .pair_event_cb = broadcast_pair_tx_event_callback,
};

const static u8 audio_mode_map[][2] = {
    {APP_BT_TASK, LIVE_A2DP_CAPTURE_MODE},
    {APP_MUSIC_TASK, LIVE_FILE_CAPTURE_MODE},
    {APP_LINEIN_TASK, LIVE_AUX_CAPTURE_MODE},
    {APP_PC_TASK, LIVE_USB_CAPTURE_MODE},
    {APP_LIVE_MIC_TASK, LIVE_MIC_CAPTURE_MODE},
    {APP_LIVE_IIS_TASK, LIVE_IIS_CAPTURE_MODE},
};

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/* --------------------------------------------------------------------------*/
/**
 * @brief 申请互斥量，用于保护临界区代码，与app_broadcast_mutex_post成对使用
 *
 * @param mutex:已创建的互斥量指针变量
 */
/* ----------------------------------------------------------------------------*/
static inline void app_broadcast_mutex_pend(OS_MUTEX *mutex, u32 line)
{
    int os_ret;
    os_ret = os_mutex_pend(mutex, 0);
    if (os_ret != OS_NO_ERR) {
        log_error("%s err, os_ret:0x%x", __FUNCTION__, os_ret);
        ASSERT(os_ret != OS_ERR_PEND_ISR, "line:%d err, os_ret:0x%x", line, os_ret);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 释放互斥量，用于保护临界区代码，与app_broadcast_mutex_pend成对使用
 *
 * @param mutex:已创建的互斥量指针变量
 */
/* ----------------------------------------------------------------------------*/
static inline void app_broadcast_mutex_post(OS_MUTEX *mutex, u32 line)
{
    int os_ret;
    os_ret = os_mutex_post(mutex);
    if (os_ret != OS_NO_ERR) {
        log_error("%s err, os_ret:0x%x", __FUNCTION__, os_ret);
        ASSERT(os_ret != OS_ERR_PEND_ISR, "line:%d err, os_ret:0x%x", line, os_ret);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief BIG开关提示音结束回调接口
 *
 * @param priv:传递的参数
 * @param flag:结束方式 -- 0正常关闭，1被打断关闭
 */
/* ----------------------------------------------------------------------------*/
static void  broadcast_tone_play_end_callback(void *priv, int flag)
{
    u32 index = (u32)priv;
    int temp_broadcast_hdl;

    switch (index) {
    case IDEX_TONE_BROADCAST_OPEN:
        break;
    case IDEX_TONE_BROADCAST_CLOSE:
        break;
    default:
        break;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief BIG状态事件处理函数
 *
 * @param wireless_trans:状态事件附带的返回参数，参数类型是big_hdl_t
 */
/* ----------------------------------------------------------------------------*/
int mic_connect_flag = 0;
extern void user_pwm_open(u16 pwm_ch);
extern void user_pwm_close(u16 pwm_ch);
extern void led_flash_add();
extern void led_flash_del();

extern void ui_user_mode_set(u8 status, u8 display, u8 para_sel);
void app_broadcast_conn_status_event_handler(struct wireless_trans_event *wireless_trans)
{
    u8 i, mode;
    u8 find = 0;
    u8 big_hdl;
    u16 crc16;
    int ret;
    big_hdl_t *hdl;
    switch (wireless_trans->event) {
    case BIG_EVENT_TRANSMITTER_CONNECT:
        g_printf("BIG_EVENT_TRANSMITTER_CONNECT");

        mic_connect_flag = 1;
        ui_user_mode_set(STATUS_BT_CONN, PWM_LED0_ON, 0);

        //由于是异步操作需要加互斥量保护，避免broadcast_close的代码与其同时运行,添加的流程请放在互斥量保护区里面
        app_broadcast_mutex_pend(&mutex, __LINE__);

        hdl = (big_hdl_t *)wireless_trans->value;
        crc16 = CRC16(hdl, sizeof(big_hdl_t));

        for (i = 0; i < BIG_MAX_NUMS; i++) {
            if (app_big_hdl_info[i].used && (app_big_hdl_info[i].hdl.big_hdl == hdl->big_hdl)) {
                find = 1;
                memcpy(&app_big_hdl_info[i].hdl, hdl, sizeof(big_hdl_t));
                break;
            }
        }

        if ((!find) || (crc16 != wireless_trans->crc16)) {
            //释放互斥量
            app_broadcast_mutex_post(&mutex, __LINE__);
            break;
        }

        mode = app_get_curr_task();
        for (i = 0; i < (sizeof(audio_mode_map) / sizeof(audio_mode_map[0])); i++) {
            if (mode == audio_mode_map[i][0]) {
                mode = audio_mode_map[i][1];
                break;
            }
        }
        ret = broadcast_transmitter_connect_deal((void *)wireless_trans->value, wireless_trans->crc16, mode);
        if (ret < 0) {
            r_printf("broadcast_transmitter_connect_deal fail");
        }

        //释放互斥量
        app_broadcast_mutex_post(&mutex, __LINE__);

        break;

    case BIG_EVENT_TRANSMITTER_DISCONNECT:


        mic_connect_flag = 0;
        ui_user_mode_set(STATUS_BT_DISCONN, PWM_LED0_LED1_FAST_FLASH, 0);
#if (!PRODUCT_TEST_MODE_ENABLE)
        g_printf("BIG_EVENT_TRANSMITTER_DISCONNECT");
#if BROADCAST_ENTER_PAIR_BEFORE_RUN
        app_broadcast_close(APP_BROADCAST_STATUS_STOP);
        app_broadcast_enter_pair(BROADCAST_ENTER_PAIR_BEFORE_RUN - 1);
#endif
#endif
        mic_connect_flag = 0;
        //led_flash_add();
        break;

    case BIG_EVENT_TRANSMITTER_HB_SYNC:
        //tx与rx同步成功事件
        //由于是异步操作需要加互斥量保护，避免broadcast_close的代码与其同时运行,添加的流程请放在互斥量保护区里面
        app_broadcast_mutex_pend(&mutex, __LINE__);



        //释放互斥量
        app_broadcast_mutex_post(&mutex, __LINE__);
        break;

    default:
        break;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 获取当前是否在退出模式的状态
 *
 * @return 1；是，0：否
 */
/* ----------------------------------------------------------------------------*/
u8 get_broadcast_app_mode_exit_flag(void)
{
    return broadcast_app_mode_exit;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 检测广播当前是否处于挂起状态
 *
 * @return true:处于挂起状态，false:处于非挂起状态
 */
/* ----------------------------------------------------------------------------*/
static bool is_need_resume_broadcast()
{
    u8 i;
    u8 find = 0;
    for (i = 0; i < BIG_MAX_NUMS; i++) {
        if (app_big_hdl_info[i].big_status == APP_BROADCAST_STATUS_SUSPEND) {
            find = 1;
            break;
        }
    }
    if (find) {
        return true;
    } else {
        return false;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 广播从挂起状态恢复
 */
/* ----------------------------------------------------------------------------*/
static void app_broadcast_resume()
{
    int temp_broadcast_hdl;
    big_parameter_t *params;

    if (!app_bt_hdl.init_ok) {
        return;
    }

    if (!is_need_resume_broadcast()) {
        return;
    }

    app_broadcast_open();
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 广播进入挂起状态
 */
/* ----------------------------------------------------------------------------*/
static void app_broadcast_suspend()
{
    u8 i;
    u8 find = 0;
    for (i = 0; i < BIG_MAX_NUMS; i++) {
        if (app_big_hdl_info[i].used) {
            find = 1;
            break;
        }
    }
    if (find) {
        broadcast_last_role = get_broadcast_role();
        app_broadcast_close(APP_BROADCAST_STATUS_SUSPEND);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 开启广播
 */
/* ----------------------------------------------------------------------------*/
void app_broadcast_open()
{
    u8 i;
    u8 big_available_num = 0;
    int temp_broadcast_hdl;
    big_parameter_t *params;

    if (!app_bt_hdl.init_ok) {
        return;
    }

    if (broadcast_in_pair_mode) {
        return;
    }

    for (i = 0; i < BIG_MAX_NUMS; i++) {
        if (!app_big_hdl_info[i].used) {
            big_available_num++;
        }
    }

    if (!big_available_num) {
        return;
    }

    if ((app_get_curr_task() == APP_BT_TASK) &&
        (get_call_status() != BT_CALL_HANGUP)) {
        return;
    }

    if ((app_get_curr_task() == APP_FM_TASK) ||
        (app_get_curr_task() == APP_PC_TASK)) {
        return;
    }

    log_info("broadcast_open");

    //vm写ram使能
    vm_ram_direct_2_flash_switch(FALSE);

    //初始化广播发送端参数
    params = set_big_params(app_get_curr_task(), BROADCAST_ROLE_TRANSMITTER, 0);

    //打开big，打开成功后会在函数app_broadcast_conn_status_event_handler做后续处理
    temp_broadcast_hdl = broadcast_transmitter(params);
#if TRANSMITTER_AUTO_TEST_EN
    extern void wireless_trans_auto_test3_init(void);
    extern void wireless_trans_auto_test4_init(void);
    //不定时切换模式
    wireless_trans_auto_test3_init();
    //不定时暂停播放
    wireless_trans_auto_test4_init();
#endif

    if (temp_broadcast_hdl >= 0) {
        for (i = 0; i < BIG_MAX_NUMS; i++) {
            if (!app_big_hdl_info[i].used) {
                app_big_hdl_info[i].hdl.big_hdl = temp_broadcast_hdl;
                app_big_hdl_info[i].big_status = APP_BROADCAST_STATUS_START;
                app_big_hdl_info[i].used = 1;
                break;
            }
        }
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 关闭广播
 */
/* ----------------------------------------------------------------------------*/
void app_broadcast_close(u8 status)
{
    u8 i;

    log_info("broadcast_close");

    //由于是异步操作需要加互斥量保护，避免和开启开广播的流程同时运行,添加的流程请放在互斥量保护区里面
    app_broadcast_mutex_pend(&mutex, __LINE__);

    //关闭VM写RAM
    vm_ram_direct_2_flash_switch(TRUE);

    for (i = 0; i < BIG_MAX_NUMS; i++) {
        if (app_big_hdl_info[i].used && app_big_hdl_info[i].hdl.big_hdl) {
            broadcast_close(app_big_hdl_info[i].hdl.big_hdl);
            memset(&app_big_hdl_info[i], 0, sizeof(struct app_big_hdl_info));
            app_big_hdl_info[i].big_status = status;
        }
    }

    //释放互斥量
    app_broadcast_mutex_post(&mutex, __LINE__);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 广播开关切换
 *
 * @return -1:切换失败，1:关闭成功，2：开启成功
 */
/* ----------------------------------------------------------------------------*/
int app_broadcast_switch(void)
{
    u8 i;
    u8 find = 0;
    int temp_broadcast_hdl;
    big_parameter_t *params;

    if (!app_bt_hdl.init_ok) {
        return -1;
    }

    if ((app_get_curr_task() == APP_BT_TASK) &&
        (get_call_status() != BT_CALL_HANGUP)) {
        return -1;
    }

    if ((app_get_curr_task() == APP_FM_TASK) ||
        (app_get_curr_task() == APP_PC_TASK)) {
        return -1;
    }

    for (i = 0; i < BIG_MAX_NUMS; i++) {
        if (app_big_hdl_info[i].used) {
            find = 1;
            break;
        }
    }

    if (find) {
        tone_play_with_callback_by_name(tone_table[IDEX_TONE_BROADCAST_CLOSE], 1, broadcast_tone_play_end_callback, (void *)IDEX_TONE_BROADCAST_CLOSE);
        app_broadcast_close(APP_BROADCAST_STATUS_STOP);
        return 1;
    } else {
        tone_play_with_callback_by_name(tone_table[IDEX_TONE_BROADCAST_OPEN], 1, broadcast_tone_play_end_callback, (void *)IDEX_TONE_BROADCAST_OPEN);
        app_broadcast_open();
        return 2;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 广播开启情况下，不同场景的处理流程
 *
 * @param switch_mode:当前系统状态
 *
 * @return -1:无需处理，0:处理事件但不拦截后续流程，1:处理事件并拦截后续流程
 */
/* ----------------------------------------------------------------------------*/
int app_broadcast_deal(int switch_mode)
{
    u8 i;
    int ret = -1;
    static int cur_mode = -1;
    static u8 phone_start_cnt = 0;

    if (!app_bt_hdl.init_ok) {
        return ret;
    }

    if ((cur_mode == switch_mode) &&
        (switch_mode != BROADCAST_PHONE_START) &&
        (switch_mode != BROADCAST_PHONE_STOP)) {
        log_error("app_broadcast_deal,cur_mode not be modified:%d", switch_mode);
        return ret;
    }

    cur_mode = switch_mode;

    switch (switch_mode) {
    case BROADCAST_APP_MODE_ENTER:
        log_info("BROADCAST_APP_MODE_ENTER");
        //进入当前模式
        broadcast_app_mode_exit = 0;
        config_broadcast_as_master = 1;
        ret = 0;
        if (app_get_curr_task() == APP_BT_TASK) {
            if (broadcast_last_role == BROADCAST_ROLE_TRANSMITTER) {
                /* break; */
            }
        }
        if (is_need_resume_broadcast()) {
            app_broadcast_resume();
            ret = 1;
        }
        break;
    case BROADCAST_APP_MODE_EXIT:
        log_info("BROADCAST_APP_MODE_EXIT");
        //退出当前模式
        broadcast_app_mode_exit = 1;
        config_broadcast_as_master = 0;
#if WIRELESS_1tN_EN
        app_broadcast_close(APP_BROADCAST_STATUS_STOP);
#else
        app_broadcast_suspend();
#endif
        ret = 0;
        break;
    case BROADCAST_MUSIC_START:
    case BROADCAST_A2DP_START:
        log_info("BROADCAST_MUSIC_START");
        //启动a2dp播放
        ret = 0;
        /* broadcast_app_mode_exit = 0; */
        config_broadcast_as_master = 1;
#if WIRELESS_1tN_EN
        break;
#endif
        if (broadcast_app_mode_exit) {
            //防止蓝牙非后台情况下退出蓝牙模式时，会先出现BROADCAST_APP_MODE_EXIT，再出现BROADCAST_A2DP_START，导致广播状态发生改变
            break;
        }
        for (i = 0; i < BIG_MAX_NUMS; i++) {
            broadcast_audio_capture_reset(app_big_hdl_info[i].hdl.big_hdl);
        }
        break;
        for (i = 0; i < BIG_MAX_NUMS; i++) {
            if (app_big_hdl_info[i].used && (app_big_hdl_info[i].big_status == APP_BROADCAST_STATUS_START)) {
                //(1)当前广播处于挂起状态时，恢复广播并作为发送设备。
                if (get_broadcast_role() == BROADCAST_ROLE_TRANSMITTER) {
                    ret = 1;
                }
                break;
            }
        }
#if BT_SUPPORT_MUSIC_VOL_SYNC
        bt_set_music_device_volume(get_music_sync_volume());
#endif
        if (is_need_resume_broadcast()) {
            app_broadcast_resume();
            ret = 1;
        }
        break;
    case BROADCAST_MUSIC_STOP:
    case BROADCAST_A2DP_STOP:
        log_info("BROADCAST_MUSIC_STOP");
        //停止a2dp播放
        ret = 0;
        config_broadcast_as_master = 0;
#if WIRELESS_1tN_EN
        break;
#endif
        if (broadcast_app_mode_exit) {
            //防止蓝牙非后台情况下退出蓝牙模式时，会先出现BROADCAST_APP_MODE_EXIT，再出现BROADCAST_A2DP_STOP，导致广播状态发生改变
            break;
        }
        //当前处于广播挂起状态时，停止手机播放，恢复广播并接收其他设备的音频数据
        app_broadcast_suspend();
        if (is_need_resume_broadcast()) {
            app_broadcast_resume();
        }
        break;
    default:
        log_error("%s invalid operation\n", __FUNCTION__);
        break;
    }

    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 更新广播同步状态的数据
 *
 * @param value:更新值
 *
 * @return -1:fail，0:success
 */
/* ----------------------------------------------------------------------------*/
int update_broadcast_custom_data(u16 value)
{
    u8 i;
    int ret = -1;

    for (i = 0; i < BIG_MAX_NUMS; i++) {
        /* if (app_big_hdl_info[i].used) { */
        ret = broadcast_set_sync_data(app_big_hdl_info[i].hdl.big_hdl, &value, BROADCAST_CUSTOM_DATA_LEN);
        /* } */
    }

    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 广播资源初始化
 */
/* ----------------------------------------------------------------------------*/
void app_broadcast_init(void)
{
    if (!app_bt_hdl.init_ok) {
        return;
    }

    int os_ret = os_mutex_create(&mutex);
    if (os_ret != OS_NO_ERR) {
        log_error("%s %d err, os_ret:0x%x", __FUNCTION__, __LINE__, os_ret);
        ASSERT(0);
    }

    broadcast_init();
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 广播资源卸载
 */
/* ----------------------------------------------------------------------------*/
void app_broadcast_uninit(void)
{
    if (!app_bt_hdl.init_ok) {
        return;
    }

    int os_ret = os_mutex_del(&mutex, OS_DEL_NO_PEND);
    if (os_ret != OS_NO_ERR) {
        log_error("%s %d err, os_ret:0x%x", __FUNCTION__, __LINE__, os_ret);
    }

    broadcast_uninit();
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 广播发送端配对事件回调
 *
 * @param event：配对事件
 * @param priv：事件附带的参数
 */
/* ----------------------------------------------------------------------------*/
static void broadcast_pair_tx_event_callback(const PAIR_EVENT event, void *priv)
{
    int pair_flag = 0;
    switch (event) {
    case PAIR_EVENT_TX_PRI_CHANNEL_CREATE_SUCCESS:
        u32 *private_connect_access_addr = (u32 *)priv;
        g_printf("PAIR_EVENT_TX_PRI_CHANNEL_CREATE_SUCCESS:0x%x", *private_connect_access_addr);
        int ret = syscfg_write(VM_WIRELESS_PAIR_CODE0, private_connect_access_addr, sizeof(u32));
        if (ret <= 0) {
            r_printf(">>>>>>wireless pair code save err");
        }
        break;
    case PAIR_EVENT_TX_OPEN_PAIR_MODE_SUCCESS:
        g_printf("PAIR_EVENT_TX_OPEN_PAIR_MODE_SUCCESS");
        broadcast_in_pair_mode  = 1;
        break;
    case PAIR_EVENT_TX_CLOSE_PAIR_MODE_SUCCESS:
        g_printf("PAIR_EVENT_TX_CLOSE_PAIR_MODE_SUCCESS");
        broadcast_in_pair_mode  = 0;
        app_broadcast_open();
        break;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 清楚配对信息
 */
/* ----------------------------------------------------------------------------*/
void app_broadcast_remove_pair(void)
{
    u32 private_connect_access_addr = 0xFFFFFFFF;
    app_broadcast_close(APP_BROADCAST_STATUS_STOP);
    int ret = syscfg_write(VM_WIRELESS_PAIR_CODE0, &private_connect_access_addr, sizeof(u32));
    app_broadcast_open();
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 广播进入配对模式
 *
 * @param mode：0-广播配对，1-连接配对
 */
/* ----------------------------------------------------------------------------*/
void app_broadcast_enter_pair(u8 mode)
{
    if (broadcast_in_pair_mode) {
        return;
    }

    if (get_broadcast_role()) {
        app_broadcast_close(APP_BROADCAST_STATUS_STOP);
    }
    broadcast_enter_pair(BROADCAST_ROLE_TRANSMITTER, (void *)&pair_tx_cb, mode);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 退出广播配对模式
 */
/* ----------------------------------------------------------------------------*/
void app_broadcast_exit_pair(void)
{
    broadcast_exit_pair(BROADCAST_ROLE_TRANSMITTER);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 非蓝牙后台情况下，在其他音源模式开启BIG，前提要先开蓝牙协议栈
 */
/* ----------------------------------------------------------------------------*/
void app_broadcast_open_in_other_mode()
{
    app_broadcast_init();
#if (!PRODUCT_TEST_MODE_ENABLE && BROADCAST_ENTER_PAIR_BEFORE_RUN)
    /* int pair_code = 0; */
    /* int ret = syscfg_read(VM_WIRELESS_PAIR_CODE0, &pair_code, sizeof(int)); */
    /* if ((ret > 0) && (pair_code != 0xFFFFFFFF)) { */
    /*     g_printf("---have pair code---"); */
    /*     app_broadcast_open(); */
    /* } else { */
    /*     r_printf("---no pair code---"); */
    app_broadcast_enter_pair(BROADCAST_ENTER_PAIR_BEFORE_RUN - 1);
    /* } */
#else
    app_broadcast_open();
#endif
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 非蓝牙后台情况下，在其他音源模式关闭BIG
 */
/* ----------------------------------------------------------------------------*/
void app_broadcast_close_in_other_mode()
{
    app_broadcast_close(APP_BROADCAST_STATUS_STOP);
    app_broadcast_uninit();
}

#if defined(WIRELESS_1tN_POLLING_PAIR) && (WIRELESS_1tN_POLLING_PAIR)
struct __pair_polling_info pair_polling_info = {
    .pair_polling_timer = 0,
    .cur_pair_mode = 0,
    .connect_succ_flag = 0,
    .pair_succ_flag = 0,
};
static void pair_polling_handler(void *priv)
{
    u32 private_connect_access_addr;
    int ret = 0;
    if (pair_polling_info.pair_succ_flag || pair_polling_info.connect_succ_flag) {
        if (pair_polling_info.pair_polling_timer) {
            sys_timer_del(pair_polling_info.pair_polling_timer);
            pair_polling_info.pair_polling_timer = 0;
        }
        return;
    }
    ret = syscfg_read(VM_WIRELESS_PAIR_CODE0, &private_connect_access_addr, sizeof(u32));
    if (ret <= 0) {
        app_broadcast_close(APP_BROADCAST_STATUS_STOP);
        app_broadcast_enter_pair(0);
        if (pair_polling_info.pair_polling_timer) {
            sys_timer_del(pair_polling_info.pair_polling_timer);
            pair_polling_info.pair_polling_timer = 0;
        }
    } else {
        if (pair_polling_info.cur_pair_mode) {
            app_broadcast_close(APP_BROADCAST_STATUS_STOP);
            app_broadcast_enter_pair(0);
            pair_polling_info.cur_pair_mode = 0;
        } else {
            app_broadcast_exit_pair();
            app_broadcast_open();
            pair_polling_info.cur_pair_mode = 1;
        }
    }
}
void app_broadcast_pair_flag_set(u8 flag)
{
    pair_polling_info.pair_succ_flag = flag;
}

void app_broadcast_conn_flag_set(u8 flag)
{
    pair_polling_info.connect_succ_flag = flag;
}

void app_broadcast_pair_polling_sw()
{
    if (pair_polling_info.pair_polling_timer == 0) {
        pair_polling_info.pair_polling_timer = sys_timer_add(NULL, pair_polling_handler, 3000);
    } else {
        sys_timer_del(pair_polling_info.pair_polling_timer);
        pair_polling_info.pair_polling_timer = 0;
    }
}
#endif

#endif


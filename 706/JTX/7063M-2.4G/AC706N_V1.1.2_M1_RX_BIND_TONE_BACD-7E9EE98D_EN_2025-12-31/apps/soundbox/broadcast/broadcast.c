/*********************************************************************************************
    *   Filename        : broadcast.c

    *   Description     :

    *   Author          : Weixin Liang

    *   Email           : liangweixin@zh-jieli.com

    *   Last modifiled  : 2022-12-15 14:18

    *   Copyright:(c)JIELI  2011-2022  @ , All Rights Reserved.
*********************************************************************************************/
#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".broadcast_bss")
#pragma data_seg(".broadcast_data")
#pragma code_seg(".broadcast_text")
#pragma const_seg(".broadcast_const")
#endif

#include "app_config.h"
#include "app_cfg.h"
#include "system/includes.h"
#include "btstack/avctp_user.h"
#include "big.h"
#include "broadcast_api.h"
#include "wireless_dev_manager.h"
#include "clock_cfg.h"
#include "bt/bt.h"
#include "app_broadcast.h"

#if TCFG_BROADCAST_ENABLE
#include "live_audio.h"
#include "audio_mode.h"
#include "live_audio_capture.h"
#include "live_audio_player.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/
#define LOG_TAG_CONST       APP
#define LOG_TAG             "[BROADCAST]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

/*! \brief 开关总中断 */
#define BROADCAST_ENTER_CRITICAL()  local_irq_disable()
#define BROADCAST_EXIT_CRITICAL()   local_irq_enable()

/*! \brief 广播解码声道配置
 	JLA_CODING_CHANNEL == 2 即广播数据是双声道时配置有效
    AUDIO_CH_LR = 0,       	//立体声
    AUDIO_CH_L,           	//左声道（单声道）
    AUDIO_CH_R,           	//右声道（单声道）
    AUDIO_CH_DIFF,        	//差分（单声道） 单/双声道输出左右混合时配置
 */
#define BROADCAST_DEC_OUTPUT_CHANNEL    AUDIO_CH_LR

/*! \brief 配置了主机音频声道分离后， BROADCAST_DEC_OUTPUT_CHANNEL只能为AUDIO_CH_LR */
#if BROADCAST_TX_CHANNEL_SEPARATION
#undef BROADCAST_DEC_OUTPUT_CHANNEL
#define BROADCAST_DEC_OUTPUT_CHANNEL    AUDIO_CH_LR
#endif

#define JLA_LOOK_AHEAD_DELAY(frame_len) \
    (frame_len == 75 ? 4000L : 2500L)
/*
 * BIS丢包修复
 */
#define BIS_AUDIO_PLC_ENABLE  1

/*! \brief 一个BIG开启多个编码器 */
#define BIG_MULTIPLE_ENCODER_EN     0

//收到的解码数据状态
enum {
    BIS_RX_PACKET_FLAG_RIGHT = 0b00,
    BIS_RX_PACKET_FLAG_ERROR,
    BIS_RX_PACKET_FLAG_LOST,
};
/**************************************************************************************************
  Data Types
**************************************************************************************************/
/*! \brief 广播状态枚举 */
enum {
    BROADCAST_STATUS_STOP,      /*!< 广播停止 */
    BROADCAST_STATUS_STOPPING,  /*!< 广播正在停止 */
    BROADCAST_STATUS_OPEN,      /*!< 广播开启 */
    BROADCAST_STATUS_START,     /*!< 广播启动 */
};

typedef struct {
    u16 bis_hdl;
    u32 rx_timestamp;
    void *capture;
    void *rx_player;
} bis_hdl_info_t ;

/*! \brief 广播结构体 */
struct broadcast_hdl {
    struct list_head entry; /*!< big链表项，用于多big管理 */
    u8 del;
    u8 big_hdl;
    u8 first_tx_sync;
    bis_hdl_info_t bis_hdl_info[BIG_MAX_BIS_NUMS];
    u32 big_sync_delay;
    u32 look_ahead_delay;
    void *capture;
    void *tx_player;
};

#if BIS_AUDIO_PLC_ENABLE
//丢包修复补包 解码读到 这两个byte 才做丢包处理
static unsigned char errpacket[256] = {
    0x02, 0x00
};
#endif /*BIS_AUDIO_PLC_ENABLE*/
/**************************************************************************************************
  Static Prototypes
**************************************************************************************************/
static void broadcast_rx_iso_callback(const void *const buf, size_t length, void *priv);
static int broadcast_rx_custom_data_callback(const void *const buf, size_t length, u8 big_hdl);
static void broadcast_rx_event_callback(const BIG_EVENT event, void *priv);

/**************************************************************************************************
  Global Variables
**************************************************************************************************/
static DEFINE_SPINLOCK(broadcast_lock);
static OS_MUTEX broadcast_mutex;
static u8 broadcast_role;   /*!< 记录当前广播为接收端还是发送端 */
static u8 broadcast_init_flag;  /*!< 广播初始化标志 */
static u8 g_big_hdl;        /*!< 用于big_hdl获取 */
static u8 broadcast_num;    /*!< 记录当前开启了多少个big广播 */
static u8 product_test_mode;    /*!< 连接的设备为产测设备 */
static struct list_head broadcast_list_head = LIST_HEAD_INIT(broadcast_list_head);
/* static struct broadcast_sync_info broadcast_data_sync;  #<{(|!< 用于接收同步状态的数据 |)}># */
static u8 broadcast_data_sync[BROADCAST_CUSTOM_DATA_LEN];  /*!< 用于接收同步状态的数据 */
static u8 cur_audio_mode = 0;
const big_callback_t big_rx_cb = {
    .receive_packet_cb      = broadcast_rx_iso_callback,
    .event_cb               = broadcast_rx_event_callback,
};

extern const int config_bb_optimized_ctrl;

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

static inline void broadcast_mutex_pend(OS_MUTEX *mutex, u32 line)
{
    int os_ret;
    os_ret = os_mutex_pend(mutex, 0);
    if (os_ret != OS_NO_ERR) {
        log_error("%s err, os_ret:0x%x", __FUNCTION__, os_ret);
        ASSERT(os_ret != OS_ERR_PEND_ISR, "line:%d err, os_ret:0x%x", line, os_ret);
    }
}

static inline void broadcast_mutex_post(OS_MUTEX *mutex, u32 line)
{
    int os_ret;
    os_ret = os_mutex_post(mutex);
    if (os_ret != OS_NO_ERR) {
        log_error("%s err, os_ret:0x%x", __FUNCTION__, os_ret);
        ASSERT(os_ret != OS_ERR_PEND_ISR, "line:%d err, os_ret:0x%x", line, os_ret);
    }
}

static u32 broadcast_clock_time(void *priv, u8 type)
{
    u32 clock_time;
    switch (type) {
    case CURRENT_TIME:
        int err = wireless_dev_get_cur_clk("big_tx", &clock_time);
        break;
    }

    return clock_time;
}

static u32 broadcast_reference_clock_time(void *priv, u8 type, struct reference_time *time)
{
    int ret = 0;
    switch (type) {
    case PLAY_SYNCHRONIZE_CURRENT_TIME:
        wireless_dev_get_cur_clk("big_rx", &ret);
        break;
    case PLAY_SYNCHRONIZE_LATCH_TIME:
        ret = wireless_dev_trigger_latch_time("big_rx", priv);
        break;
    case PLAY_SYNCHRONIZE_GET_TIME:
        ret = wireless_dev_get_latch_time_us("big_rx", &time->micro_parts, &time->clock_us, &time->event, priv);
        break;
    default:
        break;
    }

    return ret;
}

static void *broadcast_audio_player_init(u16 bis_hdl, struct jla_codec_params *codec_params)
{
    struct audio_path ipath = {
        .fmt = {
            .coding_type = codec_params->coding_type,
            .channel = codec_params->nch,
            .sample_rate = codec_params->sample_rate,
            .frame_len = codec_params->frame_size,
            .bit_rate = codec_params->bit_rate,
        },
        .time = {
            .reference_clock = (void *)bis_hdl,
            .request = (u32(*)(void *, u8))broadcast_reference_clock_time,
        }
    };

    struct audio_path opath = {
        .fmt = {
            .coding_type = AUDIO_CODING_PCM,
            .sample_rate = codec_params->sample_rate,
            .channel = BROADCAST_DEC_OUTPUT_CHANNEL,
        },
        .delay_time = 0,
    };

    wireless_dev_audio_sync_enable("big_rx", (void *)bis_hdl, 0);
    return live_audio_player_open(&ipath, &opath);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 分配big_hdl，并检查hdl是否已被使用
 *
 * @param id:希望分配的id
 * @param head:链表头
 *
 * @return hdl:实际分配的id
 */
/* ----------------------------------------------------------------------------*/
static u16 get_available_big_hdl(u8 id, struct list_head *head)
{
    struct broadcast_hdl *p;
    u8 hdl = id;
    if ((hdl == 0) || (hdl > 0xEF)) {
        hdl = 1;
        g_big_hdl = 1;
    }

    broadcast_mutex_pend(&broadcast_mutex, __LINE__);
__again:
    list_for_each_entry(p, head, entry) {
        if (hdl == p->big_hdl) {
            hdl++;
            goto __again;
        }
    }

    if (hdl > 0xEF) {
        hdl = 0;
    }

    if (hdl == 0) {
        hdl++;
        goto __again;
    }

    g_big_hdl = hdl;
    broadcast_mutex_post(&broadcast_mutex, __LINE__);
    return hdl;
}

void broadcast_init(void)
{
    log_info("--func=%s", __FUNCTION__);
    int ret;

    if (broadcast_init_flag) {
        return;
    }

    int os_ret = os_mutex_create(&broadcast_mutex);
    if (os_ret != OS_NO_ERR) {
        log_error("%s %d err, os_ret:0x%x", __FUNCTION__, __LINE__, os_ret);
        ASSERT(0);
    }

    broadcast_init_flag = 1;

#if LEA_BIG_CTRLER_TX_EN
    //初始化bis发送参数及注册回调
    ret = wireless_dev_init("big_tx", NULL);
    if (ret != 0) {
        log_error("wireless_dev_init fail:0x%x\n", ret);
    }
#endif

#if LEA_BIG_CTRLER_RX_EN
    //初始化bis接收参数及注册回调
    ret = wireless_dev_init("big_rx", NULL);
    if (ret != 0) {
        log_error("wireless_dev_init fail:0x%x\n", ret);
    }
#endif
}

void broadcast_uninit(void)
{
    log_info("--func=%s", __FUNCTION__);
    int ret;

    if (!broadcast_init_flag) {
        return;
    }

    int os_ret = os_mutex_del(&broadcast_mutex, OS_DEL_NO_PEND);
    if (os_ret != OS_NO_ERR) {
        log_error("%s %d err, os_ret:0x%x", __FUNCTION__, __LINE__, os_ret);
    }

    broadcast_init_flag = 0;

#if LEA_BIG_CTRLER_TX_EN
    ret = wireless_dev_uninit("big_tx", NULL);
    if (ret != 0) {
        log_error("wireless_dev_uninit fail:0x%x\n", ret);
    }
#endif

#if LEA_BIG_CTRLER_RX_EN
    ret = wireless_dev_uninit("big_rx", NULL);
    if (ret != 0) {
        log_error("wireless_dev_uninit fail:0x%x\n", ret);
    }
#endif
}

int broadcast_receiver_connect_deal(void *priv, int crc16)
{
    u8 bis_num = get_bis_num(BROADCAST_ROLE_RECEIVER);
    int rx_crc = 0;
    struct broadcast_hdl *broadcast_hdl = 0;
    big_hdl_t *hdl = (big_hdl_t *)priv;
    struct jla_codec_params *codec_params;

    log_info("broadcast_receiver_connect_deal");
    log_info("hdl->big_hdl:%d, hdl->bis_hdl:%d", hdl->big_hdl, hdl->bis_hdl[0]);

    codec_params = get_big_codec_params_hdl();
    ASSERT(codec_params->nch);
    ASSERT(codec_params->sample_rate);
    ASSERT(codec_params->frame_size);
    ASSERT(codec_params->bit_rate);

    broadcast_hdl = (struct broadcast_hdl *)zalloc(sizeof(struct broadcast_hdl));
    ASSERT(broadcast_hdl, "broadcast_hdl is NULL");

    broadcast_mutex_pend(&broadcast_mutex, __LINE__);
    for (u8 i = 0; i < bis_num; i++) {
        broadcast_hdl->bis_hdl_info[i].bis_hdl = hdl->bis_hdl[i];
        broadcast_hdl->bis_hdl_info[i].rx_player = broadcast_audio_player_init(hdl->bis_hdl[i], codec_params);
    }
    broadcast_hdl->big_hdl = hdl->big_hdl;
    spin_lock(&broadcast_lock);
    list_add_tail(&broadcast_hdl->entry, &broadcast_list_head);
    spin_unlock(&broadcast_lock);
    broadcast_mutex_post(&broadcast_mutex, __LINE__);

    return 0;
}

int broadcast_receiver_disconnect_deal(void *priv)
{
    u8 bis_num = get_bis_num(BROADCAST_ROLE_RECEIVER);
    struct broadcast_hdl *p, *n;
    u8 big_hdl = (u8)priv;
    void *player = 0;

    log_info("%s, big_hdl:%d", __FUNCTION__, big_hdl);

    broadcast_mutex_pend(&broadcast_mutex, __LINE__);
    list_for_each_entry_safe(p, n, &broadcast_list_head, entry) {
        if (p->big_hdl == big_hdl) {

            spin_lock(&broadcast_lock);
            list_del(&p->entry);
            spin_unlock(&broadcast_lock);

            for (u8 i = 0; i < bis_num; i++) {
                spin_lock(&broadcast_lock);
                if (p->bis_hdl_info[i].rx_player) {
                    player = p->bis_hdl_info[i].rx_player;
                    p->bis_hdl_info[i].rx_player = NULL;
                }
                spin_unlock(&broadcast_lock);

                if (player) {
                    live_audio_player_close(player);
                }
            }

            spin_lock(&broadcast_lock);
            free(p);
            spin_unlock(&broadcast_lock);
        }
    }
    broadcast_mutex_post(&broadcast_mutex, __LINE__);

    return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 广播接收端上层事件处理回调
 *
 * @param event:具体事件
 * @param priv:事件处理用到的相关参数
 */
/* ----------------------------------------------------------------------------*/
static void broadcast_rx_event_callback(const BIG_EVENT event, void *priv)
{
    static u8 rx_big_hdl;
    static u16 sync_hdl;
    static big_hdl_t big_hdl_info;
    struct sys_event e;

    log_info("--func=%s, %d", __FUNCTION__, event);

    switch (event) {
    //bis接收端开启成功后回调事件
    case BIG_EVENT_RECEIVER_CONNECT:
        log_info("BIS_EVENT_RECEIVER_CONNECT\n");
        memcpy(&big_hdl_info, priv, sizeof(big_hdl_t));
        product_test_mode = big_hdl_info.big_custom_mode;
        u16 crc16 = CRC16(&big_hdl_info, sizeof(big_hdl_t));
        e.type = SYS_BT_EVENT;
        e.arg = (void *)SYS_BT_EVENT_FROM_BIG;
        e.u.wireless_trans.event = event;
        e.u.wireless_trans.crc16 = crc16;
        e.u.wireless_trans.value = (int)(&big_hdl_info);
        sys_event_notify(&e);
        break;
    //bis接收端关闭成功后回调事件
    case BIG_EVENT_RECEIVER_DISCONNECT:
        log_info("BIG_EVENT_RECEIVER_DISCONNECT\n");
        rx_big_hdl = *((u8 *)priv);
        e.type = SYS_BT_EVENT;
        e.arg = (void *)SYS_BT_EVENT_FROM_BIG;
        e.u.wireless_trans.event = event;
        e.u.wireless_trans.value = rx_big_hdl;
        sys_event_notify(&e);
        break;
    case BIG_EVENT_PERIODIC_DISCONNECT:
        log_info("BIG_EVENT_PERIODIC_DISCONNECT\n");
        sync_hdl = *((u16 *)priv);
        e.type = SYS_BT_EVENT;
        e.arg = (void *)SYS_BT_EVENT_FROM_BIG;
        e.u.wireless_trans.event = event;
        e.u.wireless_trans.value = sync_hdl;
        sys_event_notify(&e);
        break;
    }
}

#define READ_BT_16( buffer, pos) ( ((uint16_t) buffer[pos]) | (((uint16_t)buffer[pos+1]) << 8))
/* --------------------------------------------------------------------------*/
/**
 * @brief 接收端接收数据回调函数
 *
 * @param buf:数据缓存
 * @param length:数据长度
 * @param priv:时间戳等参数
 */
/* ----------------------------------------------------------------------------*/
static void broadcast_rx_iso_callback(const void *const buf, size_t length, void *priv)
{
    u8 err_flag = 0;
    u8 bis_num = get_bis_num(BROADCAST_ROLE_RECEIVER);
    big_stream_param_t *param = (big_stream_param_t *)priv;
    struct broadcast_hdl *hdl;

    /* log_info("<<- BIS Data Out <<- TS:%d,%d", param->ts, length); */
    /* put_buf(buf, 2); */

    int catch; //写入解码buf 的长度
    spin_lock(&broadcast_lock);
    list_for_each_entry(hdl, &broadcast_list_head, entry) {
        if (hdl->del) {
            continue;
        }

        for (u8 i = 0; i < bis_num; i++) {
            if (hdl->bis_hdl_info[i].bis_hdl == param->bis_hdl) {

#if BIS_AUDIO_PLC_ENABLE
                if (length == 0) {
                    u8 frame_num = get_big_audio_transmit_data_len() / get_big_enc_output_frame_len();
                    for (int i = 0; i < frame_num; i++) {
                        memcpy((u8 *)errpacket + length, errpacket, 2);
                        length += 2;
                    }
                    err_flag = 1;
                }
#endif//BIS_AUDIO_PLC_ENABLE

                if (err_flag) {     //丢包情况

#if BIS_AUDIO_PLC_ENABLE
                    live_audio_player_push_data(hdl->bis_hdl_info[i].rx_player, (void *)errpacket, length, param->ts);
#endif//BIS_AUDIO_PLC_ENABLE

                } else if ((config_bb_optimized_ctrl & BIT(1)) && (((u8 *)buf)[0] == 2)) {      //错包情况

                    u8 *audio_buf = (u8 *)buf;
                    u16 audio_data_len = READ_BT_16(audio_buf, 2);
                    audio_data_len -= BROADCAST_CUSTOM_DATA_LEN * 8;
                    ((u8 *)buf)[2] = audio_data_len & 0xff;
                    ((u8 *)buf)[3] = (audio_data_len >> 8) & 0xff;
                    live_audio_player_push_data(hdl->bis_hdl_info[i].rx_player, (void *)buf, length - 0, param->ts);

                } else {        //正常情况
                    if (product_test_mode) {
                        live_audio_player_push_data(hdl->bis_hdl_info[i].rx_player, (void *)buf, length, param->ts);
                    } else {
                        live_audio_player_push_data(hdl->bis_hdl_info[i].rx_player, (void *)buf, length - BROADCAST_CUSTOM_DATA_LEN, param->ts);
                        broadcast_rx_custom_data_callback((void *)buf + length - BROADCAST_CUSTOM_DATA_LEN, BROADCAST_CUSTOM_DATA_LEN, hdl->big_hdl);

                    }
                }
            }
        }
    }
    spin_unlock(&broadcast_lock);
}

static int broadcast_rx_custom_data_callback(const void *const buf, size_t length, u8 big_hdl)
{
    int ret = 0;
#if (!BROADCAST_DATA_SYNC_EN)
    return -EPERM;
#endif
    if (!length || (length < BROADCAST_CUSTOM_DATA_LEN)) {
        return -EINVAL;
    }
    u8 *temp_buf = buf;
    u8 need_deal_flag = 0;
    if (memcmp(broadcast_data_sync, temp_buf, BROADCAST_CUSTOM_DATA_LEN)) {
        memcpy(broadcast_data_sync, temp_buf, BROADCAST_CUSTOM_DATA_LEN);
        need_deal_flag = 1;
    }

    if (need_deal_flag) {
        struct sys_event e;
        e.type = SYS_BT_EVENT;
        e.arg = (void *)SYS_BT_EVENT_FROM_BIG;
        e.u.wireless_trans.event = BIG_EVENT_CUSTOM_DATA_SYNC;
        e.u.wireless_trans.value = (int)broadcast_data_sync;
        sys_event_notify(&e);
    }

    return ret;
}

/* ***************************************************************************/
/**
 * @brief open broadcast as receiver
 *
 * @return err:-1, success:available_big_hdl
 */
/* *****************************************************************************/
int broadcast_receiver(big_parameter_t *params)
{
    u8 i;
    int ret;

    if (broadcast_num >= BIG_MAX_NUMS) {
        log_error("broadcast_num overflow");
        return -1;
    }

    if (!broadcast_init_flag) {
        return -2;
    }

    u8 available_big_hdl = get_available_big_hdl(++g_big_hdl, &broadcast_list_head);

    log_info("--func=%s", __FUNCTION__);

    set_big_hdl(BROADCAST_ROLE_RECEIVER, available_big_hdl);
    //启动广播
    ret = wireless_dev_open("big_rx", (void *)params);
    if (ret != 0) {
        log_error("wireless_dev_open fail:0x%x\n", ret);
        if (broadcast_num == 0) {
            broadcast_role = BROADCAST_ROLE_UNKNOW;
        }
        return -1;
    }

    //接收方开启广播后关闭经典蓝牙可发现可连接,并默认切回蓝牙模式运行
#if BROADCAST_RECEIVER_CLOSE_EDR_CONN
    bt_close_discoverable_and_connectable();
#endif

    broadcast_role = BROADCAST_ROLE_RECEIVER;

    broadcast_num++;

    clock_add_set(BROADCAST_CLK);

    return available_big_hdl;
}

/* ***************************************************************************/
/**
 * @brief close broadcast function
 *
 * @param big_hdl:need closed of big_hdl
 */
/* *****************************************************************************/
void broadcast_close(u8 big_hdl)
{
    u8 i;
    u8 find = 0;
    int ret;
    void *capture = 0;
    void *tx_player = 0;
    void *rx_player = 0;

    if (!broadcast_init_flag) {
        return;
    }

    log_info("--func=%s", __FUNCTION__);

    struct broadcast_hdl *hdl;
    broadcast_mutex_pend(&broadcast_mutex, __LINE__);
    spin_lock(&broadcast_lock);
    list_for_each_entry(hdl, &broadcast_list_head, entry) {
        if (hdl->big_hdl != big_hdl) {
            continue;
        }
        hdl->del = 1;
    }
    spin_unlock(&broadcast_lock);
    broadcast_mutex_post(&broadcast_mutex, __LINE__);

    //关闭广播
    if (get_broadcast_role() == BROADCAST_ROLE_RECEIVER) {
        ret = wireless_dev_close("big_rx", &big_hdl);
        if (ret != 0) {
            log_error("wireless_dev_close fail:0x%x\n", ret);
        }
    }

    struct broadcast_hdl *p, *n;
    //互斥量保护临界区代码
    broadcast_mutex_pend(&broadcast_mutex, __LINE__);
    list_for_each_entry_safe(p, n, &broadcast_list_head, entry) {
        if (p->big_hdl != big_hdl) {
            continue;
        }

        spin_lock(&broadcast_lock);
        list_del(&p->entry);
        spin_unlock(&broadcast_lock);

        if (broadcast_role == BROADCAST_ROLE_RECEIVER) {
            for (i = 0; i < get_bis_num(BROADCAST_ROLE_RECEIVER); i++) {
                spin_lock(&broadcast_lock);
                if (p->bis_hdl_info[i].rx_player) {
                    rx_player = p->bis_hdl_info[i].rx_player;
                    p->bis_hdl_info[i].rx_player = NULL;
                }
                spin_unlock(&broadcast_lock);

                if (rx_player) {
                    live_audio_player_close(rx_player);
                }
            }
        }

        spin_lock(&broadcast_lock);
        free(p);
        spin_unlock(&broadcast_lock);
    }
    broadcast_mutex_post(&broadcast_mutex, __LINE__);

#if BROADCAST_RECEIVER_CLOSE_EDR_CONN
    if (broadcast_role == BROADCAST_ROLE_RECEIVER) {
        //恢复经典蓝牙可发现可连接
        user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_ENABLE, 0, NULL);
        user_send_cmd_prepare(USER_CTRL_WRITE_CONN_ENABLE, 0, NULL);
    }
#endif

    broadcast_num--;
    if (broadcast_num == 0) {
        broadcast_role = BROADCAST_ROLE_UNKNOW;
        clock_remove_set(BROADCAST_CLK);
    }
}

int broadcast_enter_pair(u8 role, void *pair_event_cb, u8 mode)
{
    int err = -1;
    if (broadcast_role != BROADCAST_ROLE_UNKNOW) {
        return -1;
    }
    if (!broadcast_init_flag) {
        return -2;
    }
    if (role == BROADCAST_ROLE_RECEIVER) {
        err = wireless_dev_enter_pair("big_rx", mode, pair_event_cb);
    } else if (role == BROADCAST_ROLE_TRANSMITTER) {
        err = wireless_dev_enter_pair("big_tx", mode, pair_event_cb);
    }

    return err;
}

int broadcast_exit_pair(u8 role)
{
    int err = -1;
    if (broadcast_role != BROADCAST_ROLE_UNKNOW) {
        return -1;
    }
    if (!broadcast_init_flag) {
        return -2;
    }
    if (role == BROADCAST_ROLE_RECEIVER) {
        err = wireless_dev_exit_pair("big_rx", (void *)0);
    } else if (role == BROADCAST_ROLE_TRANSMITTER) {
        err = wireless_dev_exit_pair("big_tx", (void *)0);
    }

    return err;
}

int broadcast_get_rssi(void)
{
    int rssi = wireless_dev_get_rssi("big_rx", 0);
    return rssi;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief get current broadcast role
 *
 * @return broadcast role
 */
/* ----------------------------------------------------------------------------*/
u8 get_broadcast_role(void)
{
    return broadcast_role;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 初始化同步的状态数据的内容
 *
 * @param data:用来同步的数据
 */
/* ----------------------------------------------------------------------------*/
void broadcast_sync_data_init(void *data)
{
    memcpy(broadcast_data_sync, data, BROADCAST_CUSTOM_DATA_LEN);
}

#endif


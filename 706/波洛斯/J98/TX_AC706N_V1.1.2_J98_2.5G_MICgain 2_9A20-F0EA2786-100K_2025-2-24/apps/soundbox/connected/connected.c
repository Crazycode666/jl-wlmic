/*********************************************************************************************
    *   Filename        : connected.c

    *   Description     :

    *   Author          : Weixin Liang

    *   Email           : liangweixin@zh-jieli.com

    *   Last modifiled  : 2022-12-15 14:26

    *   Copyright:(c)JIELI  2011-2022  @ , All Rights Reserved.
*********************************************************************************************/
#include "app_config.h"
#include "app_cfg.h"
#include "system/includes.h"
#include "app_task.h"
#include "btstack/avctp_user.h"
#include "cig.h"
#include "connected_api.h"
#include "wireless_dev_manager.h"
#include "clock_cfg.h"
#include "bt/bt.h"
#include "btstack/a2dp_media_codec.h"

#if TCFG_CONNECTED_ENABLE
#include "audio_mode.h"
#include "live_audio_capture.h"
#include "live_audio_player.h"

/**************************************************************************************************
  Macros
**************************************************************************************************/
#define LOG_TAG_CONST       APP
#define LOG_TAG             "[CONNECTED]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


/*! \brief 正弦波测试使能 */
#define SINE_DATA_DEBUG_EN  0

/*! \brief 广播音频同步使能 */
#define CONNECTED_AUDIO_TIMESTAMP_ENABLE        1

/*! \brief:PCM数据buf大小 */
#define PCM_BUF_LEN          (10 * 1024)

/*! \brief 同步处理缓存buf大小 */
#define TS_PACK_BUF_LEN      (10 * 4)

/*! \brief 广播解码声道配置
 	JLA_CODING_CHANNEL == 2 即广播数据是双声道时配置有效
    AUDIO_CH_LR = 0,       	//立体声
    AUDIO_CH_L,           	//左声道（单声道）
    AUDIO_CH_R,           	//右声道（单声道）
    AUDIO_CH_DIFF,        	//差分（单声道） 单/双声道输出左右混合时配置
 */
#define CONNECTED_DEC_OUTPUT_CHANNEL  AUDIO_CH_LR

/*! \brief 配置了主机音频声道分离后， CONNECTED_DEC_OUTPUT_CHANNEL只能为AUDIO_CH_LR */
#if CONNECTED_TX_CHANNEL_SEPARATION
#undef CONNECTED_DEC_OUTPUT_CHANNEL
#define CONNECTED_DEC_OUTPUT_CHANNEL  AUDIO_CH_LR
#endif

/*! \brief CIS丢包修复 */
#define CIS_AUDIO_PLC_ENABLE    0

/*! \brief 一个CIG开启多个编码器 */
#define CIG_MULTIPLE_ENCODER_EN     0

#define connected_get_cis_tick_time(cis_txsync)  wireless_dev_get_last_tx_clk((connected_role & CONNECTED_ROLE_PERIP) == CONNECTED_ROLE_PERIP ? "cig_perip" : "cig_central", (void *)(cis_txsync))

/**************************************************************************************************
  Data Types
**************************************************************************************************/

//收到的解码数据状态
enum {
    CIS_RX_PACKET_FLAG_RIGHT = 0b00,
    CIS_RX_PACKET_FLAG_ERROR,
    CIS_RX_PACKET_FLAG_LOST,
};

//time 类型
enum {
    CURRENT_TIME,
    PACKET_RX_TIME,
};

/*! \brief 广播状态枚举 */
enum {
    CONNECTED_STATUS_STOP,      /*!< 广播停止 */
    CONNECTED_STATUS_STOPPING,  /*!< 广播正在停止 */
    CONNECTED_STATUS_OPEN,      /*!< 广播开启 */
    CONNECTED_STATUS_START,     /*!< 广播启动 */
};

enum {
    CONNECTED_STATUS_DISCONNECTED,  /*!< 广播启动 */
    CONNECTED_STATUS_CONNECTED,     /*!< 广播开启 */
};

typedef struct {
    u16 cis_hdl;
    u16 acl_hdl;
    u32 rx_timestamp;
    void *capture;
    void *rx_player;
} cis_hdl_info_t;

/*! \brief 广播结构体 */
struct connected_hdl {
    struct list_head entry; /*!< cig链表项，用于多cig管理 */
    u8 del;
    u8 cig_hdl;
    u8 latch_enable;
    u8 latch_trigger;
    u16 latch_cis;
    u32 cig_sync_delay;
    void *capture;
    void *tx_player;
    cis_hdl_info_t cis_hdl_info[CIG_MAX_CIS_NUMS];
};

struct MULTIPLE_CAPTURE_MANAGE {
    u8 dev_online;
    void *capture;
};

/**************************************************************************************************
  Static Prototypes
**************************************************************************************************/
static int connected_dec_data_receive_handler(void *_hdl, void *data, int len);
static void connected_central_event_callback(const CIG_EVENT event, void *priv);
static void connected_iso_callback(const void *const buf, size_t length, void *priv);
static void connected_perip_event_callback(const CIG_EVENT event, void *priv);

/**************************************************************************************************
  Global Variables
**************************************************************************************************/
static DEFINE_SPINLOCK(connected_lock);
static OS_MUTEX connected_mutex;
static u8 connected_role;   /*!< 记录当前广播为接收端还是发送端 */
static u8 connected_init_flag;  /*!< 广播初始化标志 */
static u8 g_cig_hdl;        /*!< 用于cig_hdl获取 */
static u8 connected_num;    /*!< 记录当前开启了多少个cig广播 */
static u8 *transmit_buf;    /*!< 用于发送端发数 */
static u8 cur_audio_mode = 0;
static struct list_head connected_list_head = LIST_HEAD_INIT(connected_list_head);
static struct MULTIPLE_CAPTURE_MANAGE multiple_capture_manage[LIVE_AUDIO_CAPTURE_MAX_MODE];
const cig_callback_t cig_central_cb = {
    .receive_packet_cb      = connected_iso_callback,
    .event_cb               = connected_central_event_callback,
};
const cig_callback_t cig_perip_cb = {
    .receive_packet_cb      = connected_iso_callback,
    .event_cb               = connected_perip_event_callback,
};

#if CIS_AUDIO_PLC_ENABLE
//丢包修复补包 解码读到 这两个byte 才做丢包处理
static unsigned char errpacket[256] = {
    0x02, 0x00
};
#endif /*CIS_AUDIO_PLC_ENABLE*/

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

static inline void connected_mutex_pend(OS_MUTEX *mutex, u32 line)
{
    int os_ret;
    os_ret = os_mutex_pend(mutex, 0);
    if (os_ret != OS_NO_ERR) {
        log_error("%s err, os_ret:0x%x", __FUNCTION__, os_ret);
        ASSERT(os_ret != OS_ERR_PEND_ISR, "line:%d err, os_ret:0x%x", line, os_ret);
    }
}

static inline void connected_mutex_post(OS_MUTEX *mutex, u32 line)
{
    int os_ret;
    os_ret = os_mutex_post(mutex);
    if (os_ret != OS_NO_ERR) {
        log_error("%s err, os_ret:0x%x", __FUNCTION__, os_ret);
        ASSERT(os_ret != OS_ERR_PEND_ISR, "line:%d err, os_ret:0x%x", line, os_ret);
    }
}

static u32 cis_clock_time(void *priv, u32 type)
{
    u32 clock_time;
    switch (type) {
    case CURRENT_TIME:
        int err = wireless_dev_get_cur_clk((connected_role & CONNECTED_ROLE_PERIP) == CONNECTED_ROLE_PERIP ? "cig_perip" : "cig_central", &clock_time);
        break;
    }

    return clock_time;
}

static u32 cis_reference_clock_time(void *priv, u8 type, struct reference_time *time)
{
    int ret = 0;
    struct connected_hdl *connected_hdl = (struct connected_hdl *)priv;
    spin_lock(&connected_lock);
    switch (type) {
    case PLAY_SYNCHRONIZE_CURRENT_TIME:
        int err = wireless_dev_get_cur_clk((connected_role & CONNECTED_ROLE_PERIP) == CONNECTED_ROLE_PERIP ? "cig_perip" : "cig_central", &ret);
        break;
    case PLAY_SYNCHRONIZE_LATCH_TIME:
        ret = wireless_dev_trigger_latch_time((connected_role & CONNECTED_ROLE_PERIP) == CONNECTED_ROLE_PERIP ? "cig_perip" : "cig_central", (void *)connected_hdl->latch_cis);
        connected_hdl->latch_trigger = 1;
        break;
    case PLAY_SYNCHRONIZE_GET_TIME:
        ret = wireless_dev_get_latch_time_us((connected_role & CONNECTED_ROLE_PERIP) == CONNECTED_ROLE_PERIP ? "cig_perip" : "cig_central", &time->micro_parts, &time->clock_us, &time->event, (void *)connected_hdl->latch_cis);
        connected_hdl->latch_trigger = 0;
        break;
    default:
        break;
    }
    spin_unlock(&connected_lock);

    return ret;
}

static u16 get_available_cig_hdl(u8 id, struct list_head *head)
{
    struct connected_hdl *p;
    u8 hdl = id;
    if ((hdl == 0) || (hdl > 0xEF)) {
        hdl = 1;
        g_cig_hdl = 1;
    }

    connected_mutex_pend(&connected_mutex, __LINE__);
__again:
    list_for_each_entry(p, head, entry) {
        if (hdl == p->cig_hdl) {
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

    g_cig_hdl = hdl;
    connected_mutex_post(&connected_mutex, __LINE__);
    return hdl;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 初始化CIG所需的参数及流程
 *
 * @param role:CIG角色作为主机还是从机
 */
/* ----------------------------------------------------------------------------*/
void connected_init(u8 role)
{
    log_info("--func=%s", __FUNCTION__);
    int ret;

    if (connected_init_flag) {
        return;
    }

    int os_ret = os_mutex_create(&connected_mutex);
    if (os_ret != OS_NO_ERR) {
        log_error("%s %d err, os_ret:0x%x", __FUNCTION__, __LINE__, os_ret);
        ASSERT(0);
    }

    connected_init_flag = 1;

    if (role == CONNECTED_ROLE_CENTRAL) {
#if LEA_CIG_CENTRAL_EN
        //初始化cis发送参数及注册回调
        ret = wireless_dev_init("cig_central", NULL);
        if (ret != 0) {
            log_error("wireless_dev_uninit fail:0x%x\n", ret);
            connected_init_flag = 0;
        }
#endif
    }

    if (role == CONNECTED_ROLE_PERIP) {
#if LEA_CIG_PERIPHERAL_EN
        //初始化cis接收参数及注册回调
        ret = wireless_dev_init("cig_perip", NULL);
        if (ret != 0) {
            log_error("wireless_dev_uninit fail:0x%x\n", ret);
            connected_init_flag = 0;
        }
#endif
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 复位CIG所需的参数及流程
 *
 * @param role:CIG角色作为主机还是从机
 */
/* ----------------------------------------------------------------------------*/
void connected_uninit(u8 role)
{
    log_info("--func=%s", __FUNCTION__);
    int ret;

    if (!connected_init_flag) {
        return;
    }

    int os_ret = os_mutex_del(&connected_mutex, OS_DEL_NO_PEND);
    if (os_ret != OS_NO_ERR) {
        log_error("%s %d err, os_ret:0x%x", __FUNCTION__, __LINE__, os_ret);
    }

    connected_init_flag = 0;

    if (role & CONNECTED_ROLE_CENTRAL) {
#if LEA_CIG_CENTRAL_EN
        ret = wireless_dev_uninit("cig_central", NULL);
        if (ret != 0) {
            log_error("wireless_dev_uninit fail:0x%x\n", ret);
        }
#endif
    }

    if (role & CONNECTED_ROLE_PERIP) {
#if LEA_CIG_PERIPHERAL_EN
        ret = wireless_dev_uninit("cig_perip", NULL);
        if (ret != 0) {
            log_error("wireless_dev_uninit fail:0x%x\n", ret);
        }
#endif
    }
}

static void *cis_audio_capture_init(u8 mode, int sync_delay)
{
#if (!WIRELESS_2T1_DUPLEX_EN)
    if (!live_audio_mode_play_status(mode)) {
        return NULL;
    }

    live_audio_mode_play_stop(mode);
#endif

    struct audio_path ipath;
    struct audio_path opath;
    live_audio_mode_get_capture_params(mode, &ipath);
    ipath.time.request = (u32(*)(void *, u8))cis_clock_time;
#if (WIRELESS_2T1_DUPLEX_EN && (CONNECTED_ROLE_CONFIG == 1))
    ipath.output.path = "capture_mixer0";
#endif

    live_audio_mode_get_cis_params(mode, &opath, sync_delay);

    log_info("cis audio capture delay : %dus, sync delay : %dus", opath.delay_time, sync_delay);
    log_info("--------------cis_audio_capture_init %d----------------", mode);
    return live_audio_capture_open(&ipath, &opath);
}

static void *cis_rx_audio_capture_open(struct audio_path *path)
{
    int err = 0;
    void *player = path->input.priv;

    err = live_audio_capture_from_player(player, path);
    if (err) {
        return NULL;
    }
    return player;
}

static void cis_rx_audio_capture_close(void *player)
{
    live_audio_capture_from_player_close(player);
}

static struct live_audio_mode_ops cis_rx_audio_capture_ops = {
    .capture_open = cis_rx_audio_capture_open,
    .capture_close = cis_rx_audio_capture_close,
};

static void *cis_rx_audio_capture_init(struct connected_hdl *connected_hdl, u8 ch, void *player, struct jla_codec_params *codec_params)
{
    u8 ch_map[2] = {0};
    switch (ch) {
    case 0:
        ch_map[0] = BIT(1);
        break;
    case 1:
        ch_map[0] = BIT(0);
        break;
    }
    /*从接收的player中获取音频再混入转发中*/
    struct audio_path ipath = {
        .fmt = {
            .sample_rate = codec_params->sample_rate,
            .channel = codec_params->nch,
            .coding_type = AUDIO_CODING_PCM,
            .priv = (void *)LIVE_SOUND_SYNCHRONIZE_STREAM,
        },
        .input = {
            .path = &cis_rx_audio_capture_ops,
            .priv = player,
        },
        .output = {
            .path = "capture_mixer0",
            .ch_map = ch_map,
        },
    };

    struct audio_path opath;
    live_audio_mode_get_cis_params(ch ? LIVE_REMOTE_DEV1_CAPTURE_MODE : LIVE_REMOTE_DEV0_CAPTURE_MODE, &opath, connected_hdl->cig_sync_delay);

    printf("live rx audio capture init.\n");
    return live_audio_capture_open(&ipath, &opath);
}

static void *cis_audio_multi_capture_init(struct connected_hdl *connected_hdl,
        u8 ch, void *player,
        struct jla_codec_params *codec_params,
        int sync_delay)
{
    u8 i;
    void *capture = 0;
    for (i = 0; i < LIVE_AUDIO_CAPTURE_MAX_MODE; i++) {
        if (!multiple_capture_manage[i].dev_online) {
            continue;
        }
        if (multiple_capture_manage[i].capture) {
            continue;
        }
        if ((i == LIVE_A2DP_CAPTURE_MODE) ||
            (i == LIVE_FILE_CAPTURE_MODE)) {
            continue;
        }
        if ((i == LIVE_REMOTE_DEV0_CAPTURE_MODE) ||
            (i == LIVE_REMOTE_DEV1_CAPTURE_MODE)) {
            multiple_capture_manage[i].capture = cis_rx_audio_capture_init(connected_hdl, ch, player, codec_params);
        } else {
            multiple_capture_manage[i].capture = cis_audio_capture_init(i, sync_delay);
        }
    }

    for (i = 0; i < LIVE_AUDIO_CAPTURE_MAX_MODE; i++) {
        if (multiple_capture_manage[i].capture) {
            capture = multiple_capture_manage[i].capture;
            break;
        }
    }
#if 0//WIRELESS_2T1_DUPLEX_EN
    if (multiple_capture_manage[LIVE_AUX_CAPTURE_MODE].capture) {
        capture = multiple_capture_manage[LIVE_AUX_CAPTURE_MODE].capture;
        printf("WIRELESS_2T1_DUPLEX_EN  get linein  capture");
    }
#endif

    return capture;
}

static void cis_audio_multi_capture_uninit(void)
{
    u8 i;
    void *capture = 0;
    for (i = 0; i < LIVE_AUDIO_CAPTURE_MAX_MODE; i++) {
        if (!multiple_capture_manage[i].capture) {
            continue;
        }
        capture = multiple_capture_manage[i].capture;
        multiple_capture_manage[i].capture = 0;
        live_audio_capture_close(capture);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 多路音源混合时，设置音源设备上线标志
 *
 * @param mode:对应的音源设备
 */
/* ----------------------------------------------------------------------------*/
void set_cig_audio_dev_online(u8 mode)
{
    multiple_capture_manage[mode].dev_online = 1;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 多路音源混合时，设置音源设备下线标志
 *
 * @param mode:对应的音源设备
 */
/* ----------------------------------------------------------------------------*/
void set_cig_audio_dev_offline(u8 mode)
{
    multiple_capture_manage[mode].dev_online = 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 多路音源混合时，可通过指定模式来打开需要转发的音频获取模块
 *
 * @param mode:需要获取的音源模式
 *
 * @return true:开启成功 false:开启失败
 */
/* ----------------------------------------------------------------------------*/
int cis_audio_multi_capture_open(u8 mode)
{
    void *capture = 0;
    int sync_delay = 0;
    if (!multiple_capture_manage[mode].capture && (connected_role == CONNECTED_ROLE_CENTRAL)) {
        if ((mode == LIVE_AUX_CAPTURE_MODE) ||
            (mode == LIVE_IIS_CAPTURE_MODE) ||
            (mode == LIVE_USB_CAPTURE_MODE)) {
            g_printf("cis_audio_multi_capture_open:%d", mode);
            multiple_capture_manage[mode].capture = cis_audio_capture_init(mode, sync_delay);
            return true;
        }
    }

    return false;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 多路音源混合时，可通过指定模式来关闭不需要转发的音频获取模块
 *
 * @param mode:需要移除的音源模式
 *
 * @return true:开启成功 false:开启失败
 */
/* ----------------------------------------------------------------------------*/
int cis_audio_multi_capture_close(u8 mode)
{
    void *capture = 0;
    if (multiple_capture_manage[mode].capture) {
        capture = multiple_capture_manage[mode].capture;
        multiple_capture_manage[mode].capture = 0;
        r_printf("cis_audio_multi_capture_close:%d", mode);
        live_audio_capture_close(capture);
        return true;
    }

    return false;
}

static void *cis_audio_player_init(struct connected_hdl *connected_hdl, struct jla_codec_params *codec_params)
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
            .reference_clock = (void *)connected_hdl,
            .request = (u32(*)(void *, u8))cis_reference_clock_time,
        },
    };

    struct audio_path opath = {
        .fmt = {
            .coding_type = AUDIO_CODING_PCM,
            .sample_rate = codec_params->sample_rate,
            .channel = CONNECTED_DEC_OUTPUT_CHANNEL,
        },
        .delay_time = 0,
    };

    if (!connected_hdl->latch_enable) {
        wireless_dev_audio_sync_enable((connected_role & CONNECTED_ROLE_PERIP) == CONNECTED_ROLE_PERIP ? "cig_perip" : "cig_central", (void *)connected_hdl->latch_cis, 0);
        connected_hdl->latch_enable = 1;
    }
    return live_audio_player_open(&ipath, &opath);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief CIG主机连接成功处理事件
 *
 * @param priv:连接成功附带的句柄参数
 * @param crc16:校验连接成功事件转发前后的参数是否一致
 * @param mode:当前音源模式
 *
 * @return 是否执行成功 -- 0为成功，其他为失败
 */
/* ----------------------------------------------------------------------------*/
int connected_central_connect_deal(void *priv, int crc16, u8 mode)
{
    u8 i, index;
    u8 find = 0;
    struct connected_hdl *connected_hdl = 0;
    cig_hdl_t *hdl = (cig_hdl_t *)priv;
    struct jla_codec_params *codec_params;

    log_info("connected_central_connect_deal");
    log_info("hdl->cig_hdl:%d, hdl->cis_hdl:%d", hdl->cig_hdl, hdl->cis_hdl);

    cur_audio_mode = mode;

    //真正连上设备后，清除BIT(7)，使外部跑转发流程
    connected_role &= ~BIT(7);

    codec_params = get_cig_codec_params_hdl();

    connected_mutex_pend(&connected_mutex, __LINE__);
    list_for_each_entry(connected_hdl, &connected_list_head, entry) {
        if (connected_hdl->cig_hdl == hdl->cig_hdl) {
            find = 1;
            break;
        }
    }
    connected_mutex_post(&connected_mutex, __LINE__);

    if (!find) {
        connected_hdl = (struct connected_hdl *)zalloc(sizeof(struct connected_hdl));
        ASSERT(connected_hdl, "connected_hdl is NULL");
    }

    for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
        if (!connected_hdl->cis_hdl_info[i].cis_hdl) {
            index = i;
            break;
        }
    }

    if (!connected_hdl->latch_cis) {
        connected_hdl->latch_cis = hdl->cis_hdl;
    }

#if ((CIG_TRANSPORT_MODE == CIG_MODE_CToP) || (CIG_TRANSPORT_MODE == CIG_MODE_DUPLEX))

#if (CIG_TRANSPORT_MODE == CIG_MODE_DUPLEX)
    connected_hdl->cis_hdl_info[index].rx_player = cis_audio_player_init(connected_hdl, codec_params);
#elif CONNECTED_TX_LOCAL_DEC_EN
    connected_hdl->tx_player = cis_audio_player_init(connected_hdl, codec_params);
#endif

#if CIG_MULTIPLE_ENCODER_EN
    if (!connected_hdl->cis_hdl_info[index].capture) {
        connected_hdl->cis_hdl_info[index].capture = cis_audio_capture_init(mode, hdl->cig_sync_delay);
    }
#else

#if (WIRELESS_2T1_DUPLEX_EN)
    set_cig_audio_dev_online(index ? LIVE_REMOTE_DEV1_CAPTURE_MODE : LIVE_REMOTE_DEV0_CAPTURE_MODE);
    connected_hdl->capture = cis_audio_multi_capture_init(connected_hdl, index, connected_hdl->cis_hdl_info[index].rx_player, codec_params, hdl->cig_sync_delay);
#else
    if (!connected_hdl->capture) {
        connected_hdl->capture = cis_audio_capture_init(mode, hdl->cig_sync_delay);
    }
#endif
#endif //#if CIG_MULTIPLE_ENCODER_EN

    if (!connected_hdl->cig_sync_delay) {
        connected_hdl->cig_sync_delay = hdl->cig_sync_delay;
    }

#elif (CIG_TRANSPORT_MODE == CIG_MODE_PToC)
    live_audio_mode_play_stop(mode);
    connected_hdl->cis_hdl_info[index].rx_player = cis_audio_player_init(connected_hdl, codec_params);
#endif

    connected_hdl->cig_hdl = hdl->cig_hdl;
    connected_hdl->cis_hdl_info[index].acl_hdl = hdl->acl_hdl;
    connected_hdl->cis_hdl_info[index].cis_hdl = hdl->cis_hdl;

    connected_mutex_pend(&connected_mutex, __LINE__);
    if (!find) {
        spin_lock(&connected_lock);
        list_add_tail(&connected_hdl->entry, &connected_list_head);
        spin_unlock(&connected_lock);
    }
    connected_mutex_post(&connected_mutex, __LINE__);

    return 0;
}

static void connected_central_rebuild_cis_latch(struct connected_hdl *connected_hdl)
{
    int i;

    if (!connected_hdl->latch_enable) {
        return;
    }
    for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
        if (!connected_hdl->cis_hdl_info[i].cis_hdl) {
            continue;
        }
        connected_hdl->latch_cis = connected_hdl->cis_hdl_info[i].cis_hdl;
        wireless_dev_audio_sync_enable((connected_role & CONNECTED_ROLE_PERIP) == CONNECTED_ROLE_PERIP ? "cig_perip" : "cig_central", (void *)connected_hdl->latch_cis, 0);
        if (connected_hdl->latch_trigger) {
            wireless_dev_trigger_latch_time((connected_role & CONNECTED_ROLE_PERIP) == CONNECTED_ROLE_PERIP ? "cig_perip" : "cig_central", (void *)connected_hdl->latch_cis);
        }
        break;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief CIG主机连接断开成功处理接口口
 *
 * @param priv:断链成功附带的句柄参数
 *
 * @return 是否执行成功 -- 0为成功，其他为失败
 */
/* ----------------------------------------------------------------------------*/
int connected_central_disconnect_deal(void *priv)
{
    u8 i, index;
    u8 cis_connected_num = 0;
    int status = 0;
    struct connected_hdl *p, *connected_hdl;
    cig_hdl_t *hdl = (cig_hdl_t *)priv;
    void *capture;
    void *tx_player;

    log_info("connected_central_disconnect_deal");
    log_info("%s, cig_hdl:%d", __FUNCTION__, hdl->cig_hdl);

    status = live_audio_mode_play_status(cur_audio_mode);

    connected_mutex_pend(&connected_mutex, __LINE__);
    list_for_each_entry(p, &connected_list_head, entry) {
        if (p->cig_hdl == hdl->cig_hdl) {

            for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
                if (p->cis_hdl_info[i].cis_hdl == hdl->cis_hdl) {
                    spin_lock(&connected_lock);
                    p->cis_hdl_info[i].cis_hdl = 0;
                    spin_unlock(&connected_lock);
                    index = i;
                } else if (p->cis_hdl_info[i].cis_hdl) {
                    cis_connected_num++;
                }
            }

            if (!cis_connected_num && p->capture) {
                spin_lock(&connected_lock);
                capture = p->capture;
                p->capture = NULL;
                spin_unlock(&connected_lock);
#if (WIRELESS_2T1_DUPLEX_EN && (CONNECTED_ROLE_CONFIG == 1))
                cis_audio_multi_capture_uninit();
                set_cig_audio_dev_offline(index ? LIVE_REMOTE_DEV1_CAPTURE_MODE : LIVE_REMOTE_DEV0_CAPTURE_MODE);
#else
                live_audio_capture_close(capture);
#endif
            } else if (cis_connected_num && p->capture) {
#if (WIRELESS_2T1_DUPLEX_EN && (CONNECTED_ROLE_CONFIG == 1))
                cis_audio_multi_capture_close(index ? LIVE_REMOTE_DEV1_CAPTURE_MODE : LIVE_REMOTE_DEV0_CAPTURE_MODE);
                set_cig_audio_dev_offline(index ? LIVE_REMOTE_DEV1_CAPTURE_MODE : LIVE_REMOTE_DEV0_CAPTURE_MODE);
#endif
            }

            if (p->cis_hdl_info[index].capture) {
                live_audio_capture_close(p->cis_hdl_info[index].capture);
            }
            if (p->cis_hdl_info[index].rx_player) {
                live_audio_player_close(p->cis_hdl_info[index].rx_player);
            }

            spin_lock(&connected_lock);
            memset(&p->cis_hdl_info[index], 0, sizeof(cis_hdl_info_t));
            spin_unlock(&connected_lock);

            if (hdl->cis_hdl == p->latch_cis) {
                connected_central_rebuild_cis_latch(p);
            }

            connected_hdl = p;
            break;
        }
    }
    connected_mutex_post(&connected_mutex, __LINE__);

    if (!cis_connected_num) {
        connected_hdl->latch_cis = 0;

        connected_role |= BIT(7);   //断开连接后，或上BIT(7)，防止外部流程判断错误

        connected_mutex_pend(&connected_mutex, __LINE__);
        if (connected_hdl->tx_player) {
            spin_lock(&connected_lock);
            tx_player = connected_hdl->tx_player;
            connected_hdl->tx_player = NULL;
            spin_unlock(&connected_lock);
            live_audio_player_close(tx_player);
        }
        connected_mutex_post(&connected_mutex, __LINE__);

        if (status == LOCAL_AUDIO_PLAYER_STATUS_PLAY) {
            live_audio_mode_play_start(cur_audio_mode);
        }
    }

    return 0;
}

static void channel_separation(void *data, void *lch_buf, void *rch_buf, u32 len, u8 packet_num)
{
    u16 single_channel_data_len = (get_cig_enc_output_frame_len() - 2) / 2;
    u8 *read_ptr = (u8 *)data;
    u8 *lch_write_ptr = (u8 *)lch_buf;
    u8 *rch_write_ptr = (u8 *)rch_buf;

    for (u8 i = 0; i < packet_num; i++) {
        //拷贝数据长度信息
        memcpy(lch_write_ptr, &single_channel_data_len, sizeof(single_channel_data_len));
        lch_write_ptr += sizeof(single_channel_data_len);
        memcpy(rch_write_ptr, &single_channel_data_len, sizeof(single_channel_data_len));
        rch_write_ptr += sizeof(single_channel_data_len);

        //拷贝实际数据
        read_ptr += sizeof(single_channel_data_len);
        memcpy(lch_write_ptr, read_ptr, single_channel_data_len);
        read_ptr += single_channel_data_len;
        lch_write_ptr += single_channel_data_len;
        memcpy(rch_write_ptr, read_ptr, single_channel_data_len);
        read_ptr += single_channel_data_len;
        rch_write_ptr += single_channel_data_len;
    }
}

static int connected_tx_align_data_handler(u8 cig_hdl)
{
    struct connected_hdl *connected_hdl = 0;
    cis_hdl_info_t *cis_hdl_info;
    u32 timestamp;
    cis_txsync_t txsync;
    int rlen = 0, i, j;
    int err;
    u8 capture_send_update = 0;
    u8 packet_num;
    u16 single_ch_trans_data_len;
    void *L_buffer, *R_buffer;

#if (CONNECTED_TX_CHANNEL_SEPARATION && (JLA_CODING_CHANNEL == 2))
    packet_num = get_cig_transmit_data_len() / get_cig_enc_output_frame_len();
    single_ch_trans_data_len = get_cig_transmit_data_len() / 2 + packet_num;
    L_buffer = malloc(single_ch_trans_data_len);
    ASSERT(L_buffer);
    R_buffer = malloc(single_ch_trans_data_len);
    ASSERT(R_buffer);
#endif

    spin_lock(&connected_lock);
    list_for_each_entry(connected_hdl, &connected_list_head, entry) {
        if (connected_hdl->cig_hdl != cig_hdl) {
            continue;
        }

        if (connected_hdl->del) {
            //cis 协议栈clos里面有while  允许继续发数据
            /* continue; */
        }

        if (connected_hdl->capture) {
            rlen = live_audio_capture_read_data(connected_hdl->capture, transmit_buf, get_cig_transmit_data_len());
#if (CONNECTED_TX_CHANNEL_SEPARATION && (JLA_CODING_CHANNEL == 2))
            channel_separation(transmit_buf, L_buffer, R_buffer, get_cig_transmit_data_len(), packet_num);
#endif
        }

        for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
            cis_hdl_info = &connected_hdl->cis_hdl_info[i];
            if (connected_hdl->cis_hdl_info[i].cis_hdl) {
                if (!rlen) {
                    if (cis_hdl_info->capture) {
                        rlen = live_audio_capture_read_data(cis_hdl_info->capture, transmit_buf, get_cig_transmit_data_len());
                        if (rlen != get_cig_transmit_data_len()) {
                            putchar('^');
                            continue;
                        }
                    } else {
                        putchar('^');
                        continue;
                    }
                }

                cig_stream_param_t param = {0};
                param.cis_hdl = cis_hdl_info->cis_hdl;
#if (CONNECTED_TX_CHANNEL_SEPARATION && (JLA_CODING_CHANNEL == 2))
                if (!(i % 2)) {
                    err = wireless_dev_transmit((connected_role & CONNECTED_ROLE_PERIP) == CONNECTED_ROLE_PERIP ? "cig_perip" : "cig_central", L_buffer, single_ch_trans_data_len, &param);
                } else {
                    err = wireless_dev_transmit((connected_role & CONNECTED_ROLE_PERIP) == CONNECTED_ROLE_PERIP ? "cig_perip" : "cig_central", R_buffer, single_ch_trans_data_len, &param);
                }
#else
                err = wireless_dev_transmit((connected_role & CONNECTED_ROLE_PERIP) == CONNECTED_ROLE_PERIP ? "cig_perip" : "cig_central", transmit_buf, get_cig_transmit_data_len(), &param);
#endif
                if (err != 0) {
                    log_error("wireless_dev_transmit fail\n");
                }

                txsync.cis_hdl = cis_hdl_info->cis_hdl;
                connected_get_cis_tick_time(&txsync);
                timestamp = (txsync.tx_ts + connected_hdl->cig_sync_delay + /*get_cig_mtl_time() * 1000L + */get_cig_sdu_period_us()) & 0xfffffff;
                if (connected_hdl->capture) {
                    if (!capture_send_update) {
                        if (timestamp - cis_hdl_info->rx_timestamp != get_cig_sdu_period_us()) {
                            /*log_error("cis tx timestamp error : %d, %d\n", timestamp, cis_hdl_info->rx_timestamp);*/
                        }
                        cis_hdl_info->rx_timestamp = timestamp;
#if (WIRELESS_2T1_DUPLEX_EN && (CONNECTED_ROLE_CONFIG == 1))
                        for (j = 0; j < LIVE_AUDIO_CAPTURE_MAX_MODE; j++) {
                            if (!multiple_capture_manage[j].capture) {
                                continue;
                            }
                            live_audio_capture_send_update(multiple_capture_manage[j].capture, get_cig_sdu_period_us(), (timestamp + get_big_sdu_period_us()) & 0xfffffff);
                        }
#else
                        live_audio_capture_send_update(connected_hdl->capture, get_cig_sdu_period_us(), (timestamp + get_cig_sdu_period_us()) & 0xfffffff);
#endif
                        capture_send_update = 1;
                    }
                } else {
                    live_audio_capture_send_update(cis_hdl_info->capture, get_cig_sdu_period_us(), (timestamp + get_cig_sdu_period_us()) & 0xfffffff);
                }
            }
        }
#if CONNECTED_TX_LOCAL_DEC_EN
        if (connected_hdl->tx_player) {
            live_audio_player_push_data(connected_hdl->tx_player, transmit_buf, get_cig_transmit_data_len(), timestamp);
        }
#endif
    }
    spin_unlock(&connected_lock);

#if (CONNECTED_TX_CHANNEL_SEPARATION && (JLA_CODING_CHANNEL == 2))
    free(L_buffer);
    free(R_buffer);
#endif

    return 0;
}

static void connected_central_event_callback(const CIG_EVENT event, void *priv)
{
    u8 i;
    u16 crc16;
    int rlen = 0;
    struct connected_hdl *connected_hdl = 0;
    static cig_hdl_t cig_hdl_info;
    static cis_acl_info_t conn_acl_info, disconn_acl_info;
    struct sys_event e;

    /* log_info("--func=%s, %d", __FUNCTION__, event); */

    switch (event) {
    //cis发射端开启成功后回调事件
    case CIG_EVENT_CIS_CONNECT:
        log_info("%s, CIG_EVENT_CIS_CONNECT\n", __FUNCTION__);
        memcpy(&cig_hdl_info, priv, sizeof(cig_hdl_t));
        crc16 = CRC16(&cig_hdl_info, sizeof(cig_hdl_t));
        e.type = SYS_BT_EVENT;
        e.arg = (void *)SYS_BT_EVENT_FROM_CIG;
        e.u.wireless_trans.event = CIG_EVENT_CENTRAL_CONNECT;
        e.u.wireless_trans.crc16 = crc16;
        e.u.wireless_trans.value = (int)(&cig_hdl_info);
        sys_event_notify(&e);
        /* g_printf("cis_hdl:0x%x, acl_hdl:0x%x", cig_hdl_info.cis_hdl, cig_hdl_info.acl_hdl); */
        break;

    case CIG_EVENT_CIS_DISCONNECT:
        log_info("%s, CIG_EVENT_CIS_DISCONNECT\n", __FUNCTION__);
        memcpy(&cig_hdl_info, priv, sizeof(cig_hdl_t));
        e.type = SYS_BT_EVENT;
        e.arg = (void *)SYS_BT_EVENT_FROM_CIG;
        e.u.wireless_trans.event = CIG_EVENT_CENTRAL_DISCONNECT;
        e.u.wireless_trans.value = (int)(&cig_hdl_info);
        sys_event_notify(&e);
        /* r_printf("cis_hdl:0x%x, acl_hdl:0x%x", cig_hdl_info.cis_hdl, cig_hdl_info.acl_hdl); */
        break;

    case CIG_EVENT_ACL_CONNECT:
        log_info("%s, CIG_EVENT_ACL_CONNECT\n", __FUNCTION__);
        memcpy(&conn_acl_info, priv, sizeof(cis_acl_info_t));
        crc16 = CRC16(&conn_acl_info, sizeof(cis_acl_info_t));
        e.type = SYS_BT_EVENT;
        e.arg = (void *)SYS_BT_EVENT_FROM_CIG;
        e.u.wireless_trans.event = event;
        e.u.wireless_trans.crc16 = crc16;
        e.u.wireless_trans.value = (int)(&conn_acl_info);
        sys_event_notify(&e);
        /* g_printf("acl_hdl:0x%x, pri_ch:", conn_acl_info.acl_hdl); */
        /* put_buf(&conn_acl_info.pri_ch, 8); */
        break;

    case CIG_EVENT_ACL_DISCONNECT:
        log_info("%s, CIG_EVENT_ACL_DISCONNECT\n", __FUNCTION__);
        memcpy(&disconn_acl_info, priv, sizeof(cis_acl_info_t));
        crc16 = CRC16(&disconn_acl_info, sizeof(cis_acl_info_t));
        e.type = SYS_BT_EVENT;
        e.arg = (void *)SYS_BT_EVENT_FROM_CIG;
        e.u.wireless_trans.event = event;
        e.u.wireless_trans.crc16 = crc16;
        e.u.wireless_trans.value = (int)(&disconn_acl_info);
        sys_event_notify(&e);
        /* r_printf("acl_hdl:0x%x, pri_ch:", disconn_acl_info.acl_hdl); */
        /* put_buf(&disconn_acl_info.pri_ch, 8); */
        break;

    //蓝牙取数发射回调事件
    case CIG_EVENT_TRANSMITTER_ALIGN:
        /* WARNING:该事件为中断函数回调, 不要添加过多打印 */
        u8 cig_hdl = *((u8 *)priv);
        connected_tx_align_data_handler(cig_hdl);
        break;

    case CIG_EVENT_TRANSMITTER_READ_TX_SYNC:
        cig_stream_param_t *param = (cig_stream_param_t *)priv;
        connected_mutex_pend(&connected_mutex, __LINE__);
        list_for_each_entry(connected_hdl, &connected_list_head, entry) {
            if (connected_hdl->del) {
                continue;
            }
            for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
                if (connected_hdl->cis_hdl_info[i].cis_hdl == param->cis_hdl) {
                    spin_lock(&connected_lock);
                    connected_hdl->cis_hdl_info[i].rx_timestamp = (param->ts + connected_hdl->cig_sync_delay + get_cig_mtl_time() * 1000L + get_cig_sdu_period_us()) & 0xfffffff;
                    spin_unlock(&connected_lock);
                    break;
                }
            }
        }
        connected_mutex_post(&connected_mutex, __LINE__);
        break;
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief CIG主机启动接口
 *
 * @param params:CIG主机启动所需参数
 *
 * @return 分配的cig_hdl
 */
/* ----------------------------------------------------------------------------*/
int connected_central_open(cig_parameter_t *params)
{
    int ret;

    connected_init(CONNECTED_ROLE_CENTRAL);

    if (!connected_init_flag) {
        return -2;
    }

    if (connected_num >= CIG_MAX_NUMS) {
        log_error("connected_num overflow");
        return -1;
    }

    if ((connected_role & CONNECTED_ROLE_PERIP) == CONNECTED_ROLE_PERIP) {
        log_error("connected_role err");
        return -1;
    }

    log_info("--func=%s", __FUNCTION__);
    u8 available_cig_hdl = get_available_cig_hdl(++g_cig_hdl, &connected_list_head);

    set_cig_hdl(CONNECTED_ROLE_CENTRAL, available_cig_hdl);
    ret = wireless_dev_open("cig_central", (void *)params);
    if (ret != 0) {
        log_error("wireless_dev_open fail:0x%x\n", ret);
        if (connected_num == 0) {
            connected_role = CONNECTED_ROLE_UNKNOW;
        }
        return -1;
    }

#if ((CIG_TRANSPORT_MODE == CIG_MODE_CToP) || (CIG_TRANSPORT_MODE == CIG_MODE_DUPLEX))
    if (transmit_buf) {
        free(transmit_buf);
    }
    transmit_buf = zalloc(get_cig_transmit_data_len());
    ASSERT(transmit_buf, "transmit_buf is NULL");
#endif

    connected_role = CONNECTED_ROLE_CENTRAL | BIT(7);	//或上BIT(7)，防止外部流程判断错误

    connected_num++;

    clock_add_set(CONNECTED_CIG_CLK);

    return available_cig_hdl;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief CIG从机连接成功处理事件
 *
 * @param priv:连接成功附带的句柄参数
 * @param crc16:校验连接成功事件转发前后的参数是否一致
 * @param mode:当前音源模式
 *
 * @return 是否执行成功 -- 0为成功，其他为失败
 */
/* ----------------------------------------------------------------------------*/
int connected_perip_connect_deal(void *priv, int crc16, u8 mode)
{
    u8 i, index;
    u8 find = 0;
    struct connected_hdl *connected_hdl = 0;
    cig_hdl_t *hdl = (cig_hdl_t *)priv;
    struct jla_codec_params *codec_params;

    log_info("connected_perip_connect_deal");
    log_info("hdl->cig_hdl:0x%x, hdl->cis_hdl:0x%x, hdl->acl_hdl:0x%x", hdl->cig_hdl, hdl->cis_hdl, hdl->acl_hdl);
    g_printf("hdl->cig_hdl:0x%x, hdl->cis_hdl:0x%x, hdl->acl_hdl:0x%x", hdl->cig_hdl, hdl->cis_hdl, hdl->acl_hdl);

    cur_audio_mode = mode;

    //真正连上设备后，清除BIT(7)，使外部跑转发流程
    connected_role &= ~BIT(7);

    codec_params = get_cig_codec_params_hdl();

    connected_mutex_pend(&connected_mutex, __LINE__);
    list_for_each_entry(connected_hdl, &connected_list_head, entry) {
        if (connected_hdl->cig_hdl == hdl->cig_hdl) {
            find = 1;
            break;
        }
    }
    connected_mutex_post(&connected_mutex, __LINE__);

    if (!find) {
        connected_hdl = (struct connected_hdl *)zalloc(sizeof(struct connected_hdl));
        ASSERT(connected_hdl, "connected_hdl is NULL");
    }

    for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
        if (!connected_hdl->cis_hdl_info[i].cis_hdl) {
            index = i;
            break;
        }
    }

#if ((CIG_TRANSPORT_MODE == CIG_MODE_PToC) || (CIG_TRANSPORT_MODE == CIG_MODE_DUPLEX))

#if CIG_MULTIPLE_ENCODER_EN
    if (!connected_hdl->cis_hdl_info[index].capture) {
        connected_hdl->cis_hdl_info[index].capture = cis_audio_capture_init(0, hdl->cig_sync_delay);
    }
#else
    if (!connected_hdl->capture) {
        connected_hdl->capture = cis_audio_capture_init(mode, hdl->cig_sync_delay);
    }
#endif //#if CIG_MULTIPLE_ENCODER_EN

#if (CIG_TRANSPORT_MODE == CIG_MODE_DUPLEX)
    connected_hdl->cis_hdl_info[index].rx_player = cis_audio_player_init(connected_hdl, codec_params);
#elif (CONNECTED_TX_LOCAL_DEC_EN)
    connected_hdl->tx_player = cis_audio_player_init(connected_hdl, codec_params);
#endif

    connected_hdl->cig_sync_delay = hdl->cig_sync_delay;

#elif (CIG_TRANSPORT_MODE == CIG_MODE_CToP)
    live_audio_mode_play_stop(mode);
    connected_hdl->cis_hdl_info[index].rx_player = cis_audio_player_init(connected_hdl, codec_params);
#endif

    connected_hdl->cig_hdl = hdl->cig_hdl;
    connected_hdl->cis_hdl_info[index].acl_hdl = hdl->acl_hdl;
    connected_hdl->cis_hdl_info[index].cis_hdl = hdl->cis_hdl;

    connected_mutex_pend(&connected_mutex, __LINE__);
    if (!find) {
        spin_lock(&connected_lock);
        list_add_tail(&connected_hdl->entry, &connected_list_head);
        spin_unlock(&connected_lock);
    }
    connected_mutex_post(&connected_mutex, __LINE__);

    return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief CIG从机连接断开成功处理接口口
 *
 * @param priv:断链成功附带的句柄参数
 *
 * @return 是否执行成功 -- 0为成功，其他为失败
 */
/* ----------------------------------------------------------------------------*/
int connected_perip_disconnect_deal(void *priv)
{
    u8 i, index;
    u8 cis_connected_num = 0;
    int status = 0;
    struct connected_hdl *p, *connected_hdl;
    cig_hdl_t *hdl = (cig_hdl_t *)priv;
    void *capture;
    void *tx_player;

    log_info("connected_perip_disconnect_deal");
    log_info("%s, cig_hdl:%d", __FUNCTION__, hdl->cig_hdl);

    status = live_audio_mode_play_status(cur_audio_mode);

    connected_mutex_pend(&connected_mutex, __LINE__);
    list_for_each_entry(p, &connected_list_head, entry) {
        if (p->cig_hdl == hdl->cig_hdl) {

            for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
                if (p->cis_hdl_info[i].cis_hdl == hdl->cis_hdl) {
                    spin_lock(&connected_lock);
                    p->cis_hdl_info[i].cis_hdl = 0;
                    spin_unlock(&connected_lock);
                    index  = i;
                } else if (p->cis_hdl_info[i].cis_hdl) {
                    cis_connected_num++;
                }
            }

            if (!cis_connected_num && p->capture) {
                spin_lock(&connected_lock);
                capture = p->capture;
                p->capture = NULL;
                spin_unlock(&connected_lock);
                live_audio_capture_close(capture);
            }

            if (p->cis_hdl_info[index].capture) {
                live_audio_capture_close(p->cis_hdl_info[index].capture);
            }
            if (p->cis_hdl_info[index].rx_player) {
                live_audio_player_close(p->cis_hdl_info[index].rx_player);
            }
            spin_lock(&connected_lock);
            memset(&p->cis_hdl_info[index], 0, sizeof(cis_hdl_info_t));
            spin_unlock(&connected_lock);

            connected_hdl = p;
            break;
        }
    }
    connected_mutex_post(&connected_mutex, __LINE__);

    if (!cis_connected_num) {
        connected_hdl->latch_cis = 0;
        connected_role |= BIT(7);   //断开连接后，或上BIT(7)，防止外部流程判断错误

        connected_mutex_pend(&connected_mutex, __LINE__);
        if (connected_hdl->tx_player) {
            spin_lock(&connected_lock);
            tx_player = connected_hdl->tx_player;
            connected_hdl->tx_player = NULL;
            spin_unlock(&connected_lock);
            live_audio_player_close(tx_player);
        }
        connected_mutex_post(&connected_mutex, __LINE__);

        if (status == LOCAL_AUDIO_PLAYER_STATUS_PLAY) {
            live_audio_mode_play_start(cur_audio_mode);
        }
    }

    return 0;
}

static void connected_perip_event_callback(const CIG_EVENT event, void *priv)
{
    u8 i;
    u16 crc16;
    int rlen = 0;
    struct connected_hdl *connected_hdl = 0;
    static cig_hdl_t cig_conn_hdl_info, cig_disconn_hdl_info;
    static cis_acl_info_t conn_acl_info, disconn_acl_info;
    struct sys_event e;

    /* log_info("--func=%s, %d", __FUNCTION__, event); */

    switch (event) {
    //cis接收端连接成功后回调事件
    case CIG_EVENT_CIS_CONNECT:
        log_info("%s, CIG_EVENT_CIS_CONNECT\n", __FUNCTION__);
        memcpy(&cig_conn_hdl_info, priv, sizeof(cig_hdl_t));
        crc16 = CRC16(&cig_conn_hdl_info, sizeof(cig_hdl_t));
        e.type = SYS_BT_EVENT;
        e.arg = (void *)SYS_BT_EVENT_FROM_CIG;
        e.u.wireless_trans.event = CIG_EVENT_PERIP_CONNECT;
        e.u.wireless_trans.crc16 = crc16;
        e.u.wireless_trans.value = (int)(&cig_conn_hdl_info);
        sys_event_notify(&e);
        /* y_printf("cis_hdl:0x%x, acl_hdl:0x%x", cig_conn_hdl_info.cis_hdl, cig_conn_hdl_info.acl_hdl); */
        break;

    //cis接收端断开成功后回调事件
    case CIG_EVENT_CIS_DISCONNECT:
        log_info("%s, CIG_EVENT_CIS_DISCONNECT\n", __FUNCTION__);
        memcpy(&cig_disconn_hdl_info, priv, sizeof(cig_hdl_t));
        e.type = SYS_BT_EVENT;
        e.arg = (void *)SYS_BT_EVENT_FROM_CIG;
        e.u.wireless_trans.event = CIG_EVENT_PERIP_DISCONNECT;
        e.u.wireless_trans.value = (int)(&cig_disconn_hdl_info);
        sys_event_notify(&e);
        /* r_printf("cis_hdl:0x%x, acl_hdl:0x%x", cig_disconn_hdl_info.cis_hdl, cig_disconn_hdl_info.acl_hdl); */
        break;

    case CIG_EVENT_ACL_CONNECT:
        log_info("%s, CIG_EVENT_ACL_CONNECT\n", __FUNCTION__);
        memcpy(&conn_acl_info, priv, sizeof(cis_acl_info_t));
        crc16 = CRC16(&conn_acl_info, sizeof(cis_acl_info_t));
        e.type = SYS_BT_EVENT;
        e.arg = (void *)SYS_BT_EVENT_FROM_CIG;
        e.u.wireless_trans.event = event;
        e.u.wireless_trans.crc16 = crc16;
        e.u.wireless_trans.value = (int)(&conn_acl_info);
        sys_event_notify(&e);
        /* y_printf("acl_hdl:0x%x, pri_ch:", conn_acl_info.acl_hdl); */
        /* put_buf(&conn_acl_info.pri_ch, 8); */
        break;

    case CIG_EVENT_ACL_DISCONNECT:
        log_info("%s, CIG_EVENT_ACL_DISCONNECT\n", __FUNCTION__);
        memcpy(&disconn_acl_info, priv, sizeof(cis_acl_info_t));
        crc16 = CRC16(&disconn_acl_info, sizeof(cis_acl_info_t));
        e.type = SYS_BT_EVENT;
        e.arg = (void *)SYS_BT_EVENT_FROM_CIG;
        e.u.wireless_trans.event = event;
        e.u.wireless_trans.crc16 = crc16;
        e.u.wireless_trans.value = (int)(&disconn_acl_info);
        sys_event_notify(&e);
        /* r_printf("acl_hdl:0x%x, pri_ch:", disconn_acl_info.acl_hdl); */
        /* put_buf(&disconn_acl_info.pri_ch, 8); */
        break;

    //蓝牙取数发射回调事件
    case CIG_EVENT_TRANSMITTER_ALIGN:
        /* WARNING:该事件为中断函数回调, 不要添加过多打印 */
        u8 cig_hdl = *((u8 *)priv);
        connected_tx_align_data_handler(cig_hdl);
        break;

    case CIG_EVENT_TRANSMITTER_READ_TX_SYNC:
        cig_stream_param_t *param = (cig_stream_param_t *)priv;
        connected_mutex_pend(&connected_mutex, __LINE__);
        list_for_each_entry(connected_hdl, &connected_list_head, entry) {
            if (connected_hdl->del) {
                continue;
            }
            for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
                if (connected_hdl->cis_hdl_info[i].cis_hdl == param->cis_hdl) {
                    spin_lock(&connected_lock);
                    connected_hdl->cis_hdl_info[i].rx_timestamp = (param->ts + connected_hdl->cig_sync_delay + get_cig_mtl_time() * 1000L + get_cig_sdu_period_us()) & 0xfffffff;
                    spin_unlock(&connected_lock);
                    break;
                }
            }
        }
        connected_mutex_post(&connected_mutex, __LINE__);
        break;
    }
}

static void connected_iso_callback(const void *const buf, size_t length, void *priv)
{
    u8 err_flag = 0;
    cig_stream_param_t *param;
    struct connected_hdl *hdl;
    struct connected_sync_channel *p;

    param = (cig_stream_param_t *)priv;

    if (param->acl_hdl) {
        //收取同步数据
        list_for_each_cig_sync_channel(p) {
            if (p->rx_events_suss) {
                if (p->rx_events_suss(param->acl_hdl, buf, length)) {
                    break;
                }
            }
        }
        return;
    }
    /* log_info("<<- cis Data Out <<- TS:%d,%d", param->ts, length); */
    /* put_buf(buf, 2); */
    spin_lock(&connected_lock);
    list_for_each_entry(hdl, &connected_list_head, entry) {
        if (hdl->del) {
            continue;
        }
        for (u8 i = 0; i < CIG_MAX_CIS_NUMS; i++) {
            if (hdl->cis_hdl_info[i].cis_hdl == param->cis_hdl) {
                //收取音频数据
#if CIS_AUDIO_PLC_ENABLE
                if (length == 0) {
                    u8 frame_num = get_cig_transmit_data_len() / get_cig_enc_output_frame_len();
                    for (int i = 0; i < frame_num; i++) {
                        memcpy((u8 *)errpacket + length, errpacket, 2);
                        length += 2;
                    }
                    err_flag = 1;
                }
#endif//CIS_AUDIO_PLC_ENABLE

                if (err_flag) {
#if CIS_AUDIO_PLC_ENABLE
                    live_audio_player_push_data(hdl->cis_hdl_info[i].rx_player, (void *)errpacket, length, param->ts);
#endif
                } else {
                    live_audio_player_push_data(hdl->cis_hdl_info[i].rx_player, (void *)buf, length, param->ts);
                }
            }
        }
    }
    spin_unlock(&connected_lock);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief CIG从机启动接口
 *
 * @param params:CIG从机启动所需参数
 *
 * @return 分配的cig_hdl
 */
/* ----------------------------------------------------------------------------*/
int connected_perip_open(cig_parameter_t *params)
{
    u8 i, j;
    int ret;

    connected_init(CONNECTED_ROLE_PERIP);

    if (!connected_init_flag) {
        return -2;
    }

    if (connected_num >= CIG_MAX_NUMS) {
        log_error("connected_num overflow");
        return -1;
    }

    if ((connected_role & CONNECTED_ROLE_CENTRAL) == CONNECTED_ROLE_CENTRAL) {
        log_error("connected_role err");
        return -1;
    }

    log_info("--func=%s", __FUNCTION__);
    u8 available_cig_hdl = 0xFF;

    set_cig_hdl(CONNECTED_ROLE_PERIP, available_cig_hdl);
    ret = wireless_dev_open("cig_perip", (void *)params);
    if (ret != 0) {
        log_error("wireless_dev_open fail:0x%x\n", ret);
        if (connected_num == 0) {
            connected_role = CONNECTED_ROLE_UNKNOW;
        }
        return -1;
    }

#if ((CIG_TRANSPORT_MODE == CIG_MODE_PToC) || (CIG_TRANSPORT_MODE == CIG_MODE_DUPLEX))
    if (transmit_buf) {
        free(transmit_buf);
    }
    transmit_buf = zalloc(get_cig_transmit_data_len());
    ASSERT(transmit_buf, "transmit_buf is NULL");
#endif

    connected_role = CONNECTED_ROLE_PERIP | BIT(7);	//或上BIT(7)，防止外部流程判断错误

    connected_num++;

    clock_add_set(CONNECTED_CIG_CLK);

    return available_cig_hdl;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 关闭对应cig_hdl的CIG连接
 *
 * @param cig_hdl:需要关闭的CIG连接的cig_hdl
 */
/* ----------------------------------------------------------------------------*/
void connected_close(u8 cig_hdl)
{
    int status = 0;
    u8 i;
    int ret;

    if (!connected_init_flag) {
        return;
    }

    log_info("--func=%s", __FUNCTION__);

    status = live_audio_mode_play_status(cur_audio_mode);

    struct connected_hdl *hdl;
    connected_mutex_pend(&connected_mutex, __LINE__);
    spin_lock(&connected_lock);
    list_for_each_entry(hdl, &connected_list_head, entry) {
        if (hdl->cig_hdl != cig_hdl) {
            continue;
        }
        hdl->del = 1;
    }
    spin_unlock(&connected_lock);
    connected_mutex_post(&connected_mutex, __LINE__);

    //关闭CIG
    if ((connected_role & CONNECTED_ROLE_PERIP) == CONNECTED_ROLE_PERIP) {
        ret = wireless_dev_close("cig_perip", &cig_hdl);
        if (ret != 0) {
            log_error("wireless_dev_close fail:0x%x\n", ret);
        }
    } else if ((connected_role & CONNECTED_ROLE_CENTRAL) == CONNECTED_ROLE_CENTRAL) {
        ret = wireless_dev_close("cig_central", &cig_hdl);
        if (ret != 0) {
            log_error("wireless_dev_close fail:0x%x\n", ret);
        }
    }

    //cis 协议栈clos里面有while  允许继续发数据，while结束，才会继续跑下面的代码

    reset_cig_params();

    //释放链表
    struct connected_hdl *p, *n;
    connected_mutex_pend(&connected_mutex, __LINE__);
    list_for_each_entry_safe(p, n, &connected_list_head, entry) {
        if (p->cig_hdl != cig_hdl) {
            continue;
        }

        spin_lock(&connected_lock);
        list_del(&p->entry);
        spin_unlock(&connected_lock);

        if (p->capture) {
#if (WIRELESS_2T1_DUPLEX_EN && (CONNECTED_ROLE_CONFIG == 1))
            p->capture = NULL;
            cis_audio_multi_capture_uninit();
#else
            live_audio_capture_close(p->capture);
#endif
        }

        if (p->tx_player) {
            live_audio_player_close(p->tx_player);
            p->tx_player = NULL;
        }

        for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
            if (p->cis_hdl_info[i].capture) {
                live_audio_capture_close(p->cis_hdl_info[i].capture);
            }
            if (p->cis_hdl_info[i].rx_player) {
                live_audio_player_close(p->cis_hdl_info[i].rx_player);
            }
        }

        spin_lock(&connected_lock);
        free(p);
        spin_unlock(&connected_lock);
    }
    connected_mutex_post(&connected_mutex, __LINE__);

    connected_num--;
    if (connected_num == 0) {
        connected_uninit(connected_role);
        connected_role = CONNECTED_ROLE_UNKNOW;
        clock_remove_set(CONNECTED_CIG_CLK);
    }

    if (status == LOCAL_AUDIO_PLAYER_STATUS_PLAY) {
        live_audio_mode_play_start(cur_audio_mode);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 获取当前CIG是主机还是从机
 *
 * @return CIG角色
 */
/* ----------------------------------------------------------------------------*/
u8 get_connected_role(void)
{
    return connected_role;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief acl数据发送接口
 *
 * @param acl_hdl:acl数据通道句柄
 * @param data:需要发送的数据
 * @param length:数据长度
 *
 * @return 实际发送出去的数据长度
 */
/* ----------------------------------------------------------------------------*/
int connected_send_acl_data(u16 acl_hdl, void *data, size_t length)
{
    int err = -1;
    cig_stream_param_t param = {0};
    param.acl_hdl = acl_hdl;

    if ((connected_role & CONNECTED_ROLE_CENTRAL) == CONNECTED_ROLE_CENTRAL) {
        err = wireless_dev_transmit("cig_central", data, length, &param);
    } else if ((connected_role & CONNECTED_ROLE_PERIP) == CONNECTED_ROLE_PERIP) {
        err = wireless_dev_transmit("cig_perip", data, length, &param);
    }
    if (err != 0) {
        log_error("acl wireless_dev_transmit fail:0x%x\n", err);
        err = 0;
    } else {
        err = length;
    }
    return err;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 供外部手动初始化capture模块
 *
 * @param cis_hdl:capture模块所对应的cis_hdl
 */
/* ----------------------------------------------------------------------------*/
void cis_audio_capture_reset(u16 cis_hdl)
{
    u8 i;
    struct connected_hdl *p;
    void *capture;

    connected_mutex_pend(&connected_mutex, __LINE__);
    list_for_each_entry(p, &connected_list_head, entry) {
        for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
            if (p->cis_hdl_info[i].cis_hdl == cis_hdl) {
#if CIG_MULTIPLE_ENCODER_EN
                if (!p->cis_hdl_info[i].capture) {
                    capture = cis_audio_capture_init(0, p->cig_sync_delay);
                    spin_lock(&connected_lock);
                    p->cis_hdl_info[i].capture = capture;
                    spin_unlock(&connected_lock);
                }
#else
                if (!p->capture) {
                    capture = cis_audio_capture_init(0, p->cig_sync_delay);
                    spin_lock(&connected_lock);
                    p->capture = capture;
                    spin_unlock(&connected_lock);
                }
#endif
            }
        }
    }
    connected_mutex_post(&connected_mutex, __LINE__);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 供外部手动初始化capture模块
 *
 * @param cis_hdl:capture模块所对应的cis_hdl
 */
/* ----------------------------------------------------------------------------*/
void cis_audio_capture_close(u16 cis_hdl)
{
    u8 i;
    struct connected_hdl *p;
    void *capture;

    connected_mutex_pend(&connected_mutex, __LINE__);
    list_for_each_entry(p, &connected_list_head, entry) {
        for (i = 0; i < CIG_MAX_CIS_NUMS; i++) {
            if (p->cis_hdl_info[i].cis_hdl == cis_hdl) {
#if CIG_MULTIPLE_ENCODER_EN
                if (p->cis_hdl_info[i].capture) {
                    live_audio_capture_close(p->cis_hdl_info[i].capture);
                }
#else
                if (p->capture) {
                    spin_lock(&connected_lock);
                    capture = p->capture;
                    p->capture = NULL;
                    spin_unlock(&connected_lock);
                    live_audio_capture_close(capture);
                }
#endif
            }
        }
    }
    connected_mutex_post(&connected_mutex, __LINE__);
}

#endif


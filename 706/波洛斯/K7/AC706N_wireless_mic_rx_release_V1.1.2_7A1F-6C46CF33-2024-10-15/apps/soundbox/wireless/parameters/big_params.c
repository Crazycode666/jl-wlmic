/*********************************************************************************************
    *   Filename        : big_params.c

    *   Description     :

    *   Author          : Weixin Liang

    *   Email           : liangweixin@zh-jieli.com

    *   Last modifiled  : 2022-12-15 14:17

    *   Copyright:(c)JIELI  2011-2022  @ , All Rights Reserved.
*********************************************************************************************/
#include "app_config.h"
#include "app_cfg.h"
#include "app_task.h"
#include "audio_base.h"
#include "broadcast_api.h"
#include "wireless_params.h"
#include "live_audio.h"
#include "user_cfg.h"

#if TCFG_BROADCAST_ENABLE

/**************************************************************************************************
  Macros
**************************************************************************************************/

#if JL_LOW_LATENCY_EN
#define MIC_SDU_PERIOD_US       2500
#else
#define MIC_SDU_PERIOD_US       5000
#endif

/*! \brief Broadcast Code */
#define BIG_BROADCAST_CODE      "JL_BROADCAST"

/*! \配置广播通道数，不同通道可发送不同数据，例如多声道音频  */
#define TX_USED_BIS_NUM     1
#define RX_USED_BIS_NUM     1
#define SCAN_WINDOW_SLOT                16
#define SCAN_INTERVAL_SLOT              28
#define PRIMARY_ADV_INTERVAL_SLOT       192

/**************************************************************************************************
  Global Variables
**************************************************************************************************/
static struct jla_codec_params big_receiver_codec_params = {
#if (WIERLESS_TRANS_CODING_TYPE == LIVE_AUDIO_CODING_JLA)
    .coding_type = AUDIO_CODING_JLA,
#else
    .coding_type = AUDIO_CODING_JLA_LL,
#endif
    .sample_rate = BROADCAST_CODING_SAMPLE_RATE,
    .frame_size = BROADCAST_CODING_FRAME_LEN,
    .bit_rate =  BROADCAST_CODING_BIT_RATE,
    .nch = BROADCAST_CODING_CHANNEL,
};

static big_parameter_t big_rx_param = {
    .num_bis   = RX_USED_BIS_NUM,
    .cb        = &big_rx_cb,
    .ext_phy   = 1,
    .enc       = 0,
    .bc        = BIG_BROADCAST_CODE,

    .rx = {
        .ext_scan_int = SCAN_WINDOW_SLOT,
        .ext_scan_win = SCAN_INTERVAL_SLOT,
        .psync_to_ms    = 2500,
        .bis            = {1},
        .bsync_to_ms    = 2000,
        .ext_scan_int = SCAN_INTERVAL_SLOT,
        .ext_scan_win = SCAN_WINDOW_SLOT,
    },
};

static big_parameter_t *rx_params = NULL;
static struct jla_codec_params *codec_params = NULL;
static u16 big_transmit_data_len = 0;
static u32 enc_output_frame_len = 0;
static u32 dec_input_buf_len = 0;
static u32 enc_output_buf_len = 0;

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/*! \brief 每包编码数据长度 */
/* (int)((JLA_CODING_FRAME_LEN / 10) * (JLA_CODING_BIT_RATE / 1000 / 8) + 2) */
/* 如果码率超过96K,即帧长超过122,就需要将每次传输数据大小 修改为一帧编码长度 */
static u32 calcul_big_enc_output_frame_len(u16 frame_len, u32 bit_rate)
{
    return (frame_len * bit_rate / 1000 / 8 / 10 + 2);
}

u32 get_big_enc_output_frame_len(void)
{
    ASSERT(enc_output_frame_len, "enc_output_frame_len is 0");
    return enc_output_frame_len;
}

static u16 calcul_big_transmit_data_len(u32 encode_output_frame_len, u32 period, u16 codec_frame_len)
{
    return (encode_output_frame_len * (period * 10 / 1000 / codec_frame_len));
}

u16 get_big_audio_transmit_data_len(void)
{
    ASSERT(big_transmit_data_len, "big_transmit_data_len is 0");
    return big_transmit_data_len;
}

static u32 calcul_big_enc_output_buf_len(u32 transmit_data_len)
{
    return (transmit_data_len * 2);
}

u32 get_big_enc_output_buf_len(void)
{
    ASSERT(enc_output_buf_len, "enc_output_buf_len is 0");
    return enc_output_buf_len;
}

static u32 calcul_big_dec_input_buf_len(u32 transmit_data_len)
{
    return (transmit_data_len * 10);
}

u32 get_big_dec_input_buf_len(void)
{
    ASSERT(dec_input_buf_len, "dec_input_buf_len is 0");
    return dec_input_buf_len;
}

u32 get_big_sdu_period_us(void)
{
    return codec_params->sdu_period_us;
}

struct jla_codec_params *get_big_codec_params_hdl(void)
{
    return codec_params;
}

u32 get_big_mtl_time(void)
{
    return 0;
}

u8 get_bis_num(u8 role)
{
    u8 num = 0;
    if (rx_params) {
        num = rx_params->num_bis;
    }
    return num;
}

void set_big_hdl(u8 role, u8 big_hdl)
{
    if (rx_params) {
        rx_params->big_hdl = big_hdl;
    }
}

int get_big_audio_coding_bit_rate(void)
{
    if (codec_params) {
        return codec_params->bit_rate;
    }
    return 0;
}

int get_big_audio_coding_frame_duration(void)
{
    if (codec_params) {
        return codec_params->frame_size;
    }

    return 0;
}

int get_big_tx_rtn(void)
{
    return 0;
}

int get_big_tx_delay(void)
{
    return 0;
}

void update_receiver_big_codec_params(BROADCAST_SYNC_INFO sync_data)
{
#if 0
    struct broadcast_sync_info *data_sync = (struct broadcast_sync_info *)sync_data;
    if (codec_params) {
        codec_params->sound_input = data_sync->sound_input;
        codec_params->nch = data_sync->nch;
        codec_params->coding_type = data_sync->coding_type;
        codec_params->sample_rate = data_sync->sample_rate;
        codec_params->frame_size = data_sync->frame_size;
        codec_params->bit_rate = data_sync->bit_rate;
        enc_output_frame_len = calcul_big_enc_output_frame_len(codec_params->frame_size, codec_params->bit_rate);
        big_transmit_data_len = calcul_big_transmit_data_len(enc_output_frame_len, codec_params->sdu_period_us, codec_params->frame_size);
        dec_input_buf_len = calcul_big_dec_input_buf_len(big_transmit_data_len);
    }
#endif
}

big_parameter_t *set_big_params(u8 app_task, u8 role, u8 big_hdl)
{
    u32 pair_code;
    int ret;

    rx_params = &big_rx_param;
    codec_params = &big_receiver_codec_params;
    switch (app_task) {
    case APP_LIVE_MIC_TASK:
        codec_params->sdu_period_us = MIC_SDU_PERIOD_US;
        break;
    default:
        ASSERT(0);
        break;
    }
    memcpy(rx_params->pair_name, bt_get_ble_name(), strlen(bt_get_ble_name()) + 1);
    enc_output_frame_len = calcul_big_enc_output_frame_len(codec_params->frame_size, codec_params->bit_rate);
    big_transmit_data_len = calcul_big_transmit_data_len(enc_output_frame_len, codec_params->sdu_period_us, codec_params->frame_size);
    dec_input_buf_len = calcul_big_dec_input_buf_len(big_transmit_data_len);
    rx_params->big_hdl = big_hdl;
    ret = syscfg_read(VM_WIRELESS_PAIR_CODE0, &pair_code, sizeof(u32));
    if ((ret <= 0) || (pair_code == 0xFFFFFFFF)) {
        //配对码需要根据一定规则生成，尽量不要自定义
        /* pair_code = (0x706 << 16) | bt_get_tws_device_indicate(NULL); */
        pair_code = 0;
    }
    rx_params->pri_ch = pair_code;
    return rx_params;
}

#endif


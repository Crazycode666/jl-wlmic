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
#define AUX_SDU_PERIOD_MS       2500
#define MIC_SDU_PERIOD_US       2500
#define IIS_SDU_PERIOD_US       2500
#else
#define AUX_SDU_PERIOD_MS       5000
#define MIC_SDU_PERIOD_US       5000
#define IIS_SDU_PERIOD_US       5000
#endif

/*! \brief Broadcast Code */
#define BIG_BROADCAST_CODE      "JL_BROADCAST"

/*! \配置广播通道数，不同通道可发送不同数据，例如多声道音频  */
#define TX_USED_BIS_NUM     1
#define RX_USED_BIS_NUM     1
#define SCAN_WINDOW_SLOT                16
#define SCAN_INTERVAL_SLOT              28

#if PRODUCT_TEST_MODE_ENABLE
#define PRIMARY_ADV_INTERVAL_SLOT       32
#else
#define PRIMARY_ADV_INTERVAL_SLOT       96
#endif

/**************************************************************************************************
  Global Variables
**************************************************************************************************/
static struct jla_codec_params big_aux_codec_params = {
    .sdu_period_us = AUX_SDU_PERIOD_MS,
    .sound_input = LIVE_SOUDN_AUX,
#if (WIERLESS_TRANS_CODING_TYPE == LIVE_AUDIO_CODING_JLA)
    .coding_type = AUDIO_CODING_JLA,
#else
    .coding_type = AUDIO_CODING_JLA_LL,
#endif
    .nch = BROADCAST_CODING_CHANNEL,
    .sample_rate = BROADCAST_CODING_SAMPLE_RATE,
    .frame_size = BROADCAST_CODING_FRAME_LEN,
    .bit_rate = BROADCAST_CODING_BIT_RATE,
};

static struct jla_codec_params big_mic_codec_params = {
    .sdu_period_us = MIC_SDU_PERIOD_US,
    .sound_input = LIVE_SOUND_MIC,
#if (WIERLESS_TRANS_CODING_TYPE == LIVE_AUDIO_CODING_JLA)
    .coding_type = AUDIO_CODING_JLA,
#else
    .coding_type = AUDIO_CODING_JLA_LL,
#endif
    .nch = BROADCAST_CODING_CHANNEL,
    .sample_rate = BROADCAST_CODING_SAMPLE_RATE,
    .frame_size = BROADCAST_CODING_FRAME_LEN,
    .bit_rate = BROADCAST_CODING_BIT_RATE,
};

static struct jla_codec_params big_iis_codec_params = {
#if (TCFG_AUDIO_INPUT_IIS || TCFG_AUDIO_OUTPUT_IIS)
    .sdu_period_us = IIS_SDU_PERIOD_US,
    .sound_input = LIVE_SOUND_IIS,
#if (WIERLESS_TRANS_CODING_TYPE == LIVE_AUDIO_CODING_JLA)
    .coding_type = AUDIO_CODING_JLA,
#else
    .coding_type = AUDIO_CODING_JLA_LL,
#endif
    .nch = BROADCAST_CODING_CHANNEL,
    .sample_rate = BROADCAST_CODING_SAMPLE_RATE,
    .frame_size = BROADCAST_CODING_FRAME_LEN,
    .bit_rate = BROADCAST_CODING_BIT_RATE,
#endif
};

static big_parameter_t big_aux_tx_param = {
    .cb        = &big_tx_cb,
    .num_bis   = TX_USED_BIS_NUM,
    .ext_phy   = 1,
    .enc       = 0,
    .bc        = BIG_BROADCAST_CODE,
    .form      = 1,

    .tx = {
        .phy            = BIT(1),
        .aux_phy        = 2,
        .eadv_int_slot  = PRIMARY_ADV_INTERVAL_SLOT,
        .padv_int_slot  = PRIMARY_ADV_INTERVAL_SLOT,
        .vdr = {
            .tx_delay   = 300,
        },
    },
};

static big_parameter_t big_mic_tx_param = {
    .cb        = &big_tx_cb,
    .num_bis   = TX_USED_BIS_NUM,
    .ext_phy   = 1,
    .enc       = 0,
    .bc        = BIG_BROADCAST_CODE,
    .form      = 1,
    .hb_rx_timeout = 200,

    .tx = {
        .phy            = BIT(1),
        .aux_phy        = 2,
        .eadv_int_slot  = PRIMARY_ADV_INTERVAL_SLOT,
        .padv_int_slot  = PRIMARY_ADV_INTERVAL_SLOT,
        .vdr = {
            .tx_delay   = 300,
        },
    },
};

static big_parameter_t big_iis_tx_param = {
    .cb        = &big_tx_cb,
    .num_bis   = TX_USED_BIS_NUM,
    .ext_phy   = 1,
    .enc       = 0,
    .bc        = BIG_BROADCAST_CODE,
    .form      = 1,

    .tx = {
        .phy            = BIT(1),
        .aux_phy        = 2,
        .eadv_int_slot  = PRIMARY_ADV_INTERVAL_SLOT,
        .padv_int_slot  = PRIMARY_ADV_INTERVAL_SLOT,
        .vdr = {
            .tx_delay   = 500,
        },
    },
};

static big_parameter_t *tx_params = NULL;
static struct jla_codec_params *codec_params = NULL;
static u16 big_transmit_data_len = 0;
static u32 enc_output_frame_len = 0;
static u32 dec_input_buf_len = 0;
static u32 enc_output_buf_len = 0;

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/
//此接口用于获取jla_ll 编码一帧的长读。如果对应编解码库有改动，这里需要同步改动
static int  get_jla_ll_enc_out_len(int in_points, int cr_config, int bitv)
{
    int len = 0;

    if (cr_config > in_points) {
        cr_config = in_points;
    } else if (cr_config < 0) {
        cr_config = 0;
    }
    if (JLA_LL_CODING_ORDER_TYPE == JLA_LL_ORDER2) {
        len = ((cr_config * 4 + (in_points - cr_config) * 3 + 7) >> 3) + 6;
    } else {
        if (cr_config == in_points) {
            len = (((cr_config - 1) * 4 + 7) >> 3) + 3;
        } else {
            len = (((cr_config - 1) * 4 + (in_points - cr_config - 1) * 3 + 7) >> 3) + 3;
        }
    }
    /* y_printf("jla_ll enc output frame len = %d\n", len); */

    return len;
}

/*! \brief 每包编码数据长度 */
/* (int)((JLA_CODING_FRAME_LEN / 10) * (JLA_CODING_BIT_RATE / 1000 / 8) + 2) */
/* 如果码率超过96K,即帧长超过122,就需要将每次传输数据大小 修改为一帧编码长度 */
static u32 calcul_big_enc_output_frame_len(u16 frame_len, u32 bit_rate, u32 sample_rate)
{
    int len = 0;
#if (WIERLESS_TRANS_CODING_TYPE == LIVE_AUDIO_CODING_JLA)
    len = (frame_len * bit_rate / 1000 / 8 / 10 + 2);
#if JL_CC_CODED_EN
    len += (len & 1); //开fec 编码时会微调码率，使编出来一帧的数据长度是偶数,故如果计算出来是奇数需要+1;
#endif/*JL_CC_CODED_EN*/
#else
    len = get_jla_ll_enc_out_len(JLA_LL_CODEC_INPUT_POINT, JLA_LL_CODEC_CR_CONFIG, 3);
#endif

    return len;
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
    return tx_params->tx.mtl;
}

u8 get_bis_num(u8 role)
{
    u8 num = 0;
    if (tx_params) {
        num = tx_params->num_bis;
    }
    return num;
}

void set_big_hdl(u8 role, u8 big_hdl)
{
    if (tx_params) {
        tx_params->big_hdl = big_hdl;
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
    if (tx_params) {
        return tx_params->tx.rtn;
    }

    return 0;
}

int get_big_tx_delay(void)
{
    if (tx_params) {
        return tx_params->tx.vdr.tx_delay;
    }

    return 0;
}

big_parameter_t *set_big_params(u8 app_task, u8 role, u8 big_hdl)
{
    u32 pair_code;
    int ret;
    switch (app_task) {
    case APP_LINEIN_TASK:
        tx_params = &big_aux_tx_param;
        codec_params = &big_aux_codec_params;
        break;
    case APP_LIVE_MIC_TASK:
        tx_params = &big_mic_tx_param;
        codec_params = &big_mic_codec_params;
        break;
    case APP_LIVE_IIS_TASK:
        tx_params = &big_iis_tx_param;
        codec_params = &big_iis_codec_params;
        break;
    default:
        tx_params = NULL;
        codec_params = NULL;
        break;
    }
    enc_output_frame_len = calcul_big_enc_output_frame_len(codec_params->frame_size, codec_params->bit_rate, codec_params->sample_rate);
    big_transmit_data_len = calcul_big_transmit_data_len(enc_output_frame_len, codec_params->sdu_period_us, codec_params->frame_size);
    dec_input_buf_len = calcul_big_dec_input_buf_len(big_transmit_data_len);
    enc_output_buf_len = calcul_big_enc_output_buf_len(big_transmit_data_len);
    if (tx_params) {
        memcpy(tx_params->pair_name, bt_get_ble_name(), strlen(bt_get_ble_name()) + 1);
        tx_params->big_hdl = big_hdl;
        tx_params->tx.mtl = 0;

#if (WIERLESS_TRANS_CODING_TYPE == LIVE_AUDIO_CODING_JLA)
#if JL_LOW_LATENCY_EN
        tx_params->tx.rtn = 5;
#else
        tx_params->tx.rtn = 9;
#endif
#endif

#if (WIERLESS_TRANS_CODING_TYPE == LIVE_AUDIO_CODING_JLA_LL)
        tx_params->tx.rtn = 4;
#endif

#if JL_CC_CODED_EN
#if JL_LOW_LATENCY_EN
        tx_params->tx.rtn = 3;
#else
        tx_params->tx.rtn = 6;
#endif
#endif

        tx_params->tx.sdu_int_us = codec_params->sdu_period_us;
#if (CONNECTED_TX_CHANNEL_SEPARATION && (JLA_CODING_CHANNEL == 2))
        u8 packet_num;
        u16 single_ch_trans_data_len;
        packet_num = big_transmit_data_len / enc_output_frame_len;
        single_ch_trans_data_len = big_transmit_data_len / 2 + packet_num;
        tx_params->tx.max_sdu = single_ch_trans_data_len + BROADCAST_CUSTOM_DATA_LEN;
#else
        tx_params->tx.max_sdu = big_transmit_data_len + BROADCAST_CUSTOM_DATA_LEN;
#endif
        if ((big_transmit_data_len + BROADCAST_CUSTOM_DATA_LEN) > 251) {
            ASSERT(0);
        }
    }
    ret = syscfg_read(VM_WIRELESS_PAIR_CODE0, &pair_code, sizeof(u32));
    if ((ret <= 0) || (pair_code == 0xFFFFFFFF)) {
        //配对码需要根据一定规则生成，尽量不要自定义
        /* pair_code = (0x706 << 16) | bt_get_tws_device_indicate(NULL); */
        pair_code = 0;
    }

    tx_params->pri_ch = pair_code;
    return tx_params;
}

#endif


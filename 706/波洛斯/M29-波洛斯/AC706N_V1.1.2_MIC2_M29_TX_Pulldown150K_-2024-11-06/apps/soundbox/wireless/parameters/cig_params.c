/*********************************************************************************************
    *   Filename        : cig_params.c

    *   Description     :

    *   Author          : Weixin Liang

    *   Email           : liangweixin@zh-jieli.com

    *   Last modifiled  : 2022-12-15 14:31

    *   Copyright:(c)JIELI  2011-2022  @ , All Rights Reserved.
*********************************************************************************************/
#include "app_config.h"
#include "app_task.h"
#include "audio_base.h"
#include "connected_api.h"
#include "wireless_params.h"
#include "live_audio.h"
#include "user_cfg.h"

#if TCFG_CONNECTED_ENABLE

/**************************************************************************************************
  Macros
**************************************************************************************************/
/*! \brief 各个模式发包间隔 */
#define AUX_SDU_PERIOD_US       10000
#define MIC_SDU_PERIOD_US       10000
#define IIS_SDU_PERIOD_US       10000

/*! \brief 各个模式重发次数 */
#define AUX_RTN_CToP            3
#define MIC_RTN_CToP            3//4
#define IIS_RTN_CToP            3

#define AUX_RTN_PToC            3
#define MIC_RTN_PToC            3//4
#define IIS_RTN_PToC            3

/*! \breief 配置连接设备数，最大配置为2  */
#define USED_CIS_NUM            LEA_CIG_CONNECTION_NUM

/**************************************************************************************************
  Global Variables
**************************************************************************************************/
static struct jla_codec_params cig_aux_codec_params = {
    .sdu_period_us = AUX_SDU_PERIOD_US,
    .sound_input = LIVE_SOUDN_AUX,
    .nch = BROADCAST_CODING_CHANNEL,
#if (WIERLESS_TRANS_CODING_TYPE == LIVE_AUDIO_CODING_JLA)
    .coding_type = AUDIO_CODING_JLA,
#else
    .coding_type = AUDIO_CODING_JLA_LL,
#endif
    .sample_rate = BROADCAST_CODING_SAMPLE_RATE,
    .frame_size = BROADCAST_CODING_FRAME_LEN,
    .bit_rate = BROADCAST_CODING_BIT_RATE,
};

static struct jla_codec_params cig_mic_codec_params = {
    .sdu_period_us = MIC_SDU_PERIOD_US,
    .sound_input = LIVE_SOUND_MIC,
    .nch = BROADCAST_CODING_CHANNEL,
#if (WIERLESS_TRANS_CODING_TYPE == LIVE_AUDIO_CODING_JLA)
    .coding_type = AUDIO_CODING_JLA,
#else
    .coding_type = AUDIO_CODING_JLA_LL,
#endif
    .sample_rate = BROADCAST_CODING_SAMPLE_RATE,
    .frame_size = BROADCAST_CODING_FRAME_LEN,
    .bit_rate = BROADCAST_CODING_BIT_RATE,
};

static struct jla_codec_params cig_iis_codec_params = {
#if (TCFG_AUDIO_INPUT_IIS || TCFG_AUDIO_OUTPUT_IIS)
    .sdu_period_us = IIS_SDU_PERIOD_US,
    .sound_input = LIVE_SOUND_IIS,
    .nch = BROADCAST_CODING_CHANNEL,
#if (WIERLESS_TRANS_CODING_TYPE == LIVE_AUDIO_CODING_JLA)
    .coding_type = AUDIO_CODING_JLA,
#else
    .coding_type = AUDIO_CODING_JLA_LL,
#endif
    .sample_rate = BROADCAST_CODING_SAMPLE_RATE,
    .frame_size = BROADCAST_CODING_FRAME_LEN,
    .bit_rate = BROADCAST_CODING_BIT_RATE,
#endif
};

static struct jla_codec_params cig_receiver_codec_params = {
    .nch = BROADCAST_CODING_CHANNEL,
#if (WIERLESS_TRANS_CODING_TYPE == LIVE_AUDIO_CODING_JLA)
    .coding_type = AUDIO_CODING_JLA,
#else
    .coding_type = AUDIO_CODING_JLA_LL,
#endif
    .sample_rate = BROADCAST_CODING_SAMPLE_RATE,
    .frame_size = BROADCAST_CODING_FRAME_LEN,
    .bit_rate = BROADCAST_CODING_BIT_RATE,
};

static cig_parameter_t cig_aux_central_param = {
    .cb             = &cig_central_cb,
    .pair_en        = 0,
    .num_cis        = USED_CIS_NUM,
    .phy            = BIT(1),

    .cis[0] = {
        .rtnCToP    = AUX_RTN_CToP,
        .rtnPToC    = AUX_RTN_PToC,
    },

    .cis[1] = {
        .rtnCToP    = AUX_RTN_CToP,
        .rtnPToC    = AUX_RTN_PToC,
    },

    .vdr = {
        .tx_delay   = 1500,
        .cig_offset = 1500,
        .aclMaxPduCToP = 36,
        .aclMaxPduPToC = 27,
    },
};

static cig_parameter_t cig_mic_central_param = {
    .cb             = &cig_central_cb,
    .pair_en        = 0,
    .num_cis        = USED_CIS_NUM,
    .phy            = BIT(1),

    .cis[0] = {
        .rtnCToP    = MIC_RTN_CToP,
        .rtnPToC    = MIC_RTN_PToC,
    },

    .cis[1] = {
        .rtnCToP    = MIC_RTN_CToP,
        .rtnPToC    = MIC_RTN_PToC,
    },

    .vdr = {
        .tx_delay   = 1500,
        .cig_offset = 1500,
        .aclMaxPduCToP = 36,
        .aclMaxPduPToC = 27,
    },
};

static cig_parameter_t cig_iis_central_param = {
    .cb             = &cig_central_cb,
    .pair_en        = 0,
    .num_cis        = USED_CIS_NUM,
    .phy            = BIT(1),

    .cis[0] = {
        .rtnCToP    = IIS_RTN_CToP,
        .rtnPToC    = IIS_RTN_PToC,
    },

    .cis[1] = {
        .rtnCToP    = IIS_RTN_CToP,
        .rtnPToC    = IIS_RTN_PToC,
    },

    .vdr = {
        .tx_delay   = 1500,
        .cig_offset = 1500,
        .aclMaxPduCToP = 36,
        .aclMaxPduPToC = 27,
    },
};

static cig_parameter_t cig_perip_param = {
    .cb        = &cig_perip_cb,
    .pair_en   = 0,

    .vdr = {
        .tx_delay   = 150,  //perip 150, central 300
        .aclMaxPduCToP = 36,
        .aclMaxPduPToC = 27,
    },
};


static cig_parameter_t *central_params = NULL;
static cig_parameter_t *perip_params = NULL;
static struct jla_codec_params *codec_params = NULL;
static u16 cig_transmit_data_len = 0;
static u32 enc_output_frame_len = 0;
static u32 dec_input_buf_len = 0;
static u32 enc_output_buf_len = 0;
static u8 cur_app_task = APP_TASK_MAX_INDEX;
static u8 cur_cig_role = 0;

/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/* --------------------------------------------------------------------------*/
/**
 * @brief 此接口用于获取jla_ll 编码一帧的长读。如果对应编解码库有改动，这里需要同步改动
 *
 * @param in_points
 * @param cr_config
 * @param bitv
 *
 * @return
 */
/* ----------------------------------------------------------------------------*/
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
/* (int)((BROADCAST_CODING_FRAME_LEN / 10) * (BROADCAST_CODING_BIT_RATE / 1000 / 8) + 2) */
/* 如果码率超过96K,即帧长超过122,就需要将每次传输数据大小 修改为一帧编码长度 */
static u32 calcul_cig_enc_output_frame_len(u16 frame_len, u32 bit_rate)
{
    int len = 0;
#if (WIERLESS_TRANS_CODING_TYPE == LIVE_AUDIO_CODING_JLA)
    len = (frame_len * bit_rate / 1000 / 8 / 10 + 2);
#else
    len = get_jla_ll_enc_out_len(JLA_LL_CODEC_INPUT_POINT, JLA_LL_CODEC_CR_CONFIG, 3);
#endif

    return len;
}

u32 get_cig_enc_output_frame_len(void)
{
    ASSERT(enc_output_frame_len, "enc_output_frame_len is 0");
    return enc_output_frame_len;
}

static u16 calcul_cig_transmit_data_len(u32 encode_output_frame_len, u16 period, u16 codec_frame_len)
{
    return (encode_output_frame_len * (period * 10 / 1000 / codec_frame_len));
}

u16 get_cig_transmit_data_len(void)
{
    ASSERT(cig_transmit_data_len, "cig_transmit_data_len is 0");
    return cig_transmit_data_len;
}

static u32 calcul_cig_enc_output_buf_len(u32 transmit_data_len)
{
    return (transmit_data_len * 2);
}

u32 get_cig_enc_output_buf_len(void)
{
    ASSERT(enc_output_buf_len, "enc_output_buf_len is 0");
    return enc_output_buf_len;
}

static u32 calcul_cig_dec_input_buf_len(u32 transmit_data_len)
{
    return (transmit_data_len * 10);
}

u32 get_cig_dec_input_buf_len(void)
{
    ASSERT(dec_input_buf_len, "dec_input_buf_len is 0");
    return dec_input_buf_len;
}

u32 get_cig_sdu_period_us(void)
{
    if (codec_params) {
        return codec_params->sdu_period_us;
    }

    return 0;
}

struct jla_codec_params *get_cig_codec_params_hdl(void)
{
    return codec_params;
}

u32 get_cig_mtl_time(void)
{
    return 0;
}

void set_cig_hdl(u8 role, u8 cig_hdl)
{
    if ((role == CONNECTED_ROLE_CENTRAL) && central_params) {
        central_params->cig_hdl = cig_hdl;
    } else if ((role == CONNECTED_ROLE_PERIP) && perip_params) {
        perip_params->cig_hdl = cig_hdl;
    }
}

int get_cig_audio_coding_bit_rate(void)
{
    if (codec_params) {
        return codec_params->bit_rate;
    }
    return 0;
}

int get_cig_audio_coding_frame_duration(void)
{
    if (codec_params) {
        return codec_params->frame_size;
    }

    return 0;
}

int get_cig_tx_rtn(void)
{
    int rtn = 0;

    switch (cur_app_task) {
    case APP_LINEIN_TASK:
        if ((cur_cig_role == CONNECTED_ROLE_CENTRAL) && central_params) {
            rtn = AUX_RTN_CToP;
        } else if ((cur_cig_role == CONNECTED_ROLE_PERIP) && perip_params) {
            rtn = AUX_RTN_PToC;
        }
        break;
    case APP_LIVE_MIC_TASK:
        if ((cur_cig_role == CONNECTED_ROLE_CENTRAL) && central_params) {
            rtn = MIC_RTN_CToP;
        } else if ((cur_cig_role == CONNECTED_ROLE_PERIP) && perip_params) {
            rtn = MIC_RTN_PToC;
        }
        break;
    case APP_LIVE_IIS_TASK:
        if ((cur_cig_role == CONNECTED_ROLE_CENTRAL) && central_params) {
            rtn = IIS_RTN_CToP;
        } else if ((cur_cig_role == CONNECTED_ROLE_PERIP) && perip_params) {
            rtn = IIS_RTN_PToC;
        }
        break;
    }

    return rtn;
}

int get_cig_tx_delay(void)
{
    int tx_delay = 0;

    if ((cur_cig_role == CONNECTED_ROLE_CENTRAL) && central_params) {
        tx_delay = central_params->vdr.tx_delay;
    } else if ((cur_cig_role == CONNECTED_ROLE_PERIP) && perip_params) {
        tx_delay = perip_params->vdr.tx_delay;
    }

    return tx_delay;
}

cig_parameter_t *set_cig_params(u8 app_task, u8 role, u8 pair_without_addr)
{
    int ret;
    u64 pair_addr;
    cur_app_task = app_task;
    cur_cig_role = role;
    if (role == CONNECTED_ROLE_CENTRAL) {
        switch (app_task) {
        case APP_LINEIN_TASK:
            central_params = &cig_aux_central_param;
            codec_params = &cig_aux_codec_params;
            break;
        case APP_LIVE_MIC_TASK:
            central_params = &cig_mic_central_param;
            codec_params = &cig_mic_codec_params;
            break;
        case APP_LIVE_IIS_TASK:
            central_params = &cig_iis_central_param;
            codec_params = &cig_iis_codec_params;
            break;
        default:
            central_params = NULL;
            codec_params = NULL;
            break;
        }
#if (CONNECTED_TX_CHANNEL_SEPARATION && (BROADCAST_CODING_CHANNEL == 2))
        codec_params->nch = 1;
        codec_params->bit_rate /= 2;
        enc_output_frame_len = calcul_cig_enc_output_frame_len(codec_params->frame_size, BROADCAST_CODING_BIT_RATE);
#else
        enc_output_frame_len = calcul_cig_enc_output_frame_len(codec_params->frame_size, codec_params->bit_rate);
#endif
        cig_transmit_data_len = calcul_cig_transmit_data_len(enc_output_frame_len, codec_params->sdu_period_us, codec_params->frame_size);
        dec_input_buf_len = calcul_cig_dec_input_buf_len(cig_transmit_data_len);
        enc_output_buf_len = calcul_cig_enc_output_buf_len(cig_transmit_data_len);
        if (central_params) {

            memcpy(central_params->pair_name, bt_get_ble_name(), strlen(bt_get_ble_name()) + 1);

#if (CIG_TRANSPORT_MODE == CIG_MODE_CToP)
            central_params->mtlCToP = codec_params->sdu_period_us / 1000;
            central_params->sduIntUsCToP = codec_params->sdu_period_us;
            central_params->cis[0].maxSduCToP = cig_transmit_data_len;
            central_params->cis[1].maxSduCToP = cig_transmit_data_len;
#endif  //#if (CIG_TRANSPORT_MODE == CIG_MODE_CToP)

#if (CIG_TRANSPORT_MODE == CIG_MODE_PToC)
            central_params->mtlPToC = codec_params->sdu_period_us / 1000;
            central_params->sduIntUsPToC = codec_params->sdu_period_us;
            central_params->cis[0].maxSduPToC = cig_transmit_data_len;
            central_params->cis[1].maxSduPToC = cig_transmit_data_len;
#endif  //#if (CIG_TRANSPORT_MODE == CIG_MODE_PToC)

#if (CIG_TRANSPORT_MODE == CIG_MODE_DUPLEX)
            central_params->mtlCToP = codec_params->sdu_period_us / 1000;
            central_params->mtlPToC = codec_params->sdu_period_us / 1000;
            central_params->sduIntUsCToP = codec_params->sdu_period_us;
            central_params->sduIntUsPToC = codec_params->sdu_period_us;
#if (CONNECTED_TX_CHANNEL_SEPARATION && (BROADCAST_CODING_CHANNEL == 2))
            u8 packet_num;
            u16 single_ch_trans_data_len;
            packet_num = cig_transmit_data_len / enc_output_frame_len;
            single_ch_trans_data_len = cig_transmit_data_len / 2 + packet_num;
            central_params->cis[0].maxSduCToP = single_ch_trans_data_len;
            central_params->cis[0].maxSduPToC = single_ch_trans_data_len;
            central_params->cis[1].maxSduCToP = single_ch_trans_data_len;
            central_params->cis[1].maxSduPToC = single_ch_trans_data_len;
#else
            central_params->cis[0].maxSduCToP = cig_transmit_data_len;
            central_params->cis[0].maxSduPToC = cig_transmit_data_len;
            central_params->cis[1].maxSduCToP = cig_transmit_data_len;
            central_params->cis[1].maxSduPToC = cig_transmit_data_len;
#endif
#endif  //#if (CIG_TRANSPORT_MODE == CIG_MODE_DUPLEX)

        }
        if (pair_without_addr) {
            central_params->cis[0].pri_ch = 0;
            central_params->cis[1].pri_ch = 0;
        } else {
            ret = syscfg_read(VM_WIRELESS_PAIR_CODE0, &pair_addr, sizeof(pair_addr));
            if (ret <= 0) {
                pair_addr = 0;
            }
            central_params->cis[0].pri_ch = pair_addr;
            g_printf("cis0.pri_ch:");
            put_buf(&central_params->cis[0].pri_ch, sizeof(central_params->cis[0].pri_ch));
            ret = syscfg_read(VM_WIRELESS_PAIR_CODE1, &pair_addr, sizeof(pair_addr));
            if (ret <= 0) {
                pair_addr = 0;
            }
            central_params->cis[1].pri_ch = pair_addr;
            g_printf("cis1.pri_ch:");
            put_buf(&central_params->cis[1].pri_ch, sizeof(central_params->cis[1].pri_ch));
        }
        return central_params;
    }

    if (role == CONNECTED_ROLE_PERIP) {
        perip_params = &cig_perip_param;
        codec_params = &cig_receiver_codec_params;
        if (app_task == APP_LINEIN_TASK) {
            codec_params->sdu_period_us = AUX_SDU_PERIOD_US;
        } else if (app_task == APP_LIVE_MIC_TASK) {
            codec_params->sdu_period_us = MIC_SDU_PERIOD_US;
        } else if (app_task == APP_LIVE_IIS_TASK) {
            codec_params->sdu_period_us = IIS_SDU_PERIOD_US;
        }
        memcpy(perip_params->pair_name, bt_get_ble_name(), strlen(bt_get_ble_name()) + 1);
        enc_output_frame_len = calcul_cig_enc_output_frame_len(codec_params->frame_size, codec_params->bit_rate);
        cig_transmit_data_len = calcul_cig_transmit_data_len(enc_output_frame_len, codec_params->sdu_period_us, codec_params->frame_size);
        dec_input_buf_len = calcul_cig_dec_input_buf_len(cig_transmit_data_len);
        if (pair_without_addr) {
            perip_params->perip.pri_ch = 0;
        } else {
            ret = syscfg_read(VM_WIRELESS_PAIR_CODE0, &pair_addr, sizeof(pair_addr));
            if (ret <= 0) {
                pair_addr = 0;
            }
            perip_params->perip.pri_ch = pair_addr;
            g_printf("perip.pri_ch:");
            put_buf(&perip_params->perip.pri_ch, sizeof(perip_params->perip.pri_ch));
        }
        return perip_params;
    }

    return NULL;
}

void reset_cig_params(void)
{
    cur_app_task = APP_TASK_MAX_INDEX;
    cur_cig_role = 0;
    central_params = 0;
    perip_params = 0;
}

#endif


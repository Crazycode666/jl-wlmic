#include "app_config.h"
#include "media/includes.h"
#include "audio_way_dac.h"
#include "asm/audio_common.h"


#ifndef AUDIO_OUT_WAY_TYPE
#error "no defined AUDIO_OUT_WAY_TYPE"
#endif

#if (AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_DAC)

#if (AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_DONGLE)
s16 dac_buff[1 * 1024] SEC(.dac_buff);
#elif (AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_IIS)
s16 dac_buff[1 * 1024] SEC(.dac_buff);
#elif (AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_DAC)
s16 dac_buff[1 * 1024] SEC(.dac_buff);
#endif

void *audio_dac_get_buf(int *len)
{
    if (len) {
        *len = sizeof(dac_buff);
    }
    return dac_buff;
}

static int audio_dac_trim_value(struct audio_dac_trim *trim)
{
    int len = syscfg_read(CFG_DAC_TRIM_INFO, (void *)trim, sizeof(struct audio_dac_trim));
    if (len != sizeof(struct audio_dac_trim) || __builtin_abs(trim->left) > 800 || __builtin_abs(trim->right) > 800) {
        return -EINVAL;
    }
    return 0;
}

static void audio_dac_trim_end_handler(struct audio_dac_trim *trim)
{
    syscfg_write(CFG_DAC_TRIM_INFO, (void *)trim, sizeof(struct audio_dac_trim));
}

const struct audio_dac_platform_data sound_dac_data = {
    .output = TCFG_AUDIO_DAC_CONNECT_MODE,
    .mode   = TCFG_AUDIO_DAC_MODE,
    .performance_mode = TCFG_DAC_PERFORMANCE_MODE,
    .power_level = TCFG_DAC_POWER_LEVEL,
    .vbg_trim_value = 5,
    .vcm_cap_en = TCFG_VCM_CAP_EN,
    .dcc_level = 14,
    .dcc_rd_sel = 0,
    .fade_en = 0,
    .fade_points_step = 1,
    .fade_volume_step = 1,
    .clock_mode = 0,
    .bit24_en = 0,
    .power = {
        .trim_poweron_time = 500,
        .trim_value = audio_dac_trim_value,
        .trim_begin = NULL,
        .trim_end   = audio_dac_trim_end_handler,
    },
    .codec = {
        .dsm_clk = DAC_DSM_6MHz,
    },
};


void audio_common_param_int()
{
    audio_common_dac_init(&sound_dac_data);
}

void audio_way_dac_init(void)
{
    struct sound_pcm_platform_data data;
    data.dma_addr = (void *)dac_buff;
    data.dma_bytes = sizeof(dac_buff);
    data.fifo_bytes = sizeof(dac_buff);
    data.private_data = (void *)&sound_dac_data;
    sound_platform_load("dac", &data);
    audio_common_dac_init(&sound_dac_data);
}

__attribute__((weak))
void audio_dac_wakeup_irq_handler(void *priv)
{
    /* putchar('R'); */
    audio_way_resume();
}

struct audio_way *audio_way_dac_open(void)
{
    struct audio_way *audio_hdl;
    struct sound_pcm_stream *dac = NULL;
    int err = sound_pcm_create(&dac, "dac", 0);
    if (err) {
        log_e("Create dac sound pcm error.\n");
        return NULL;
    }
    sound_pcm_ctl_ioctl(dac, SNDCTL_IOCTL_SET_BIAS_TRIM, 0);
    sound_pcm_set_irq_handler(dac, NULL, audio_dac_wakeup_irq_handler);
    audio_hdl = zalloc(sizeof(struct audio_way));
    ASSERT(audio_hdl);
    audio_hdl->way_type = AUDIO_WAY_TYPE_DAC;
    audio_hdl->stream = dac;
    audio_hdl->state = AUDIO_WAY_STATE_IDLE;
    switch (sound_dac_data.output) {
    case DAC_OUTPUT_MONO_L:
        audio_hdl->out_ch = 1;
        break;
    default :
        audio_hdl->out_ch = 2;
        break;
    }
    return audio_hdl;
}


#endif /*(AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_DAC)*/



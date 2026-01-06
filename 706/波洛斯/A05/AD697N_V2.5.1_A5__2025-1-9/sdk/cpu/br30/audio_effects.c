#include "audio_effects.h"


#define LOG_TAG_CONST EFFECTS
#define LOG_TAG     "[EFFECTS]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_DEBUG_ENABLE
#include "debug.h"


#if TCFG_AEC_UL_EQ_ENABLE
struct eq_tab ul_eq_tab = {0};
#endif

#if TCFG_PHONE_EQ_ENABLE
struct eq_tab dl_eq_tab = {0};
#endif

struct eq_tab music_eq_tab = {0};
#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  != EFFECTS_BASE)
struct eq_tab post_eq_tab = {0};
#endif

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_BASE)&& TCFG_BT_MUSIC_DRC_ENABLE
struct drc_ch music_drc_param = {0};
#endif

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  != EFFECTS_BASE)
virtual_bass_param_tool_set  virtual_bass_param = {0};

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_ULTRA) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_ULTRA)
dynamic_eq_pro_param_tool_set dyeq_pro_param = {0};
#endif

struct multiband_limiter_param_tool_set mblimiter_param = {0};
#endif

struct eq_tab music_eq_tab_tmp = {0};
struct eq_tab post_eq_tab_tmp = {0};

#if TCFG_AUDIO_OUT_EQ_ENABLE
//bass treble 复用MUSIC_EQ的后两段
struct sof_eq_seg_info audio_out_eq_tab[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 125,   0, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 12000, 0, 0.3f},
};

int audio_out_eq_spec_set_gain(u8 idx, int gain)
{
    if (idx > 1) {
        log_error(" bass treble set error \n");
        return -1;
    }
    //idx 0:低音  1:高音
    struct sof_eq_seg_info *seg = &audio_out_eq_tab[idx];
    seg->gain = gain;

    struct eq_fade_parm fade_parm = {0};
    fade_parm.fade_time = 1;
    fade_parm.fade_step = 0.5f;
    int seg_num = ARRAY_SIZE(audio_out_eq_tab);
    struct sof_eq_tool *tab = zalloc(sizeof(struct sof_eq_tool) + sizeof(struct sof_eq_seg_info) * seg_num);
    if (tab) {
        tab->is_bypass    = 0;
        tab->global_gain = 0;
        tab->seg_num      = seg_num;
        memcpy(tab->seg, audio_out_eq_tab, sizeof(audio_out_eq_tab));
        struct audio_sof_eq *bass_treble = get_cur_sofeq_hdl_by_name(AEID_MUSIC_BASS_TREBLE_EQ);
        if (bass_treble) {
            audio_sof_eq_update(bass_treble, fade_parm, tab);
        }
        free(tab);
    }
}
struct audio_sof_eq *audio_bass_treble_open(u32 sample_rate, u8 ch_num, int in_bit_width, int out_bit_width)
{
    struct audio_sof_eq_param param = {0};
    param.seg = audio_out_eq_tab;
    param.nsection = ARRAY_SIZE(audio_out_eq_tab);
    param.global_gain = 0;
    param.sample_rate = sample_rate;
    param.in_bit_width  = in_bit_width;
    param.out_bit_width = out_bit_width ;
    param.channel = ch_num;	//通道数
    param.name = AEID_MUSIC_BASS_TREBLE_EQ;
    param.qval = 15;
    return  audio_sof_eq_open(&param);
}

void audio_bass_treble_run(struct audio_sof_eq *hdl, s16 *in, s16 *out, int len)
{
    audio_sof_eq_run(hdl, in, out, len);
}

void audio_bass_treble_close(struct audio_sof_eq *hdl)
{
    audio_sof_eq_close(hdl);
}
#endif


struct audio_eq *music_eq_open(u32 sample_rate, u8 ch_num, int out_32bit)
{
    memcpy(&music_eq_tab_tmp, &music_eq_tab, sizeof(music_eq_tab_tmp));
    struct audio_eq_param param = {0};
    param.channels = ch_num;
    param.out_32bit = out_32bit;
    param.max_nsection = music_eq_tab_tmp.seg_num;
    param.nsection = music_eq_tab_tmp.seg_num;
    param.cb = eq_get_filter_info;
    param.eq_name = AEID_MUSIC_EQ;
    param.sr = sample_rate;
    param.seg = music_eq_tab_tmp.seg;
    param.global_gain = music_eq_tab_tmp.global_gain;
    param.bypass = music_eq_tab_tmp.is_bypass;

#if !TCFG_EQ_ONLINE_ENABLE
    param.fade = 10;
    param.f_fade_step = 100;
    param.q_fade_step = 0.1f;
    param.g_fade_step = 0.1f;
    param.fade_step = 0.1f;
#endif

    return audio_dec_eq_open(&param);
}

void music_eq_close(struct audio_eq *eq)
{
    if (eq) {
        audio_dec_eq_close(eq);
    }
}

void music_eq_run(struct audio_eq *eq, s16 *in, s16 *out, int len)
{
    if (eq) {
        audio_dec_eq_run(eq, in, out, len);
    }
}

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_BASE)&& TCFG_BT_MUSIC_DRC_ENABLE
struct audio_drc *music_drc_open(u32 sample_rate, u8 ch_num, int in_bit_width, int out_bit_width)
{
    struct audio_drc_param drc_param = {0};
    drc_param.sr = sample_rate;
    drc_param.channels = ch_num;
    drc_param.out_32bit = out_bit_width;
    drc_param.cb = drc_get_filter_info;
    drc_param.name = AEID_MUSIC_DRC;
    drc_param.cfg = &music_drc_param;
    return audio_dec_drc_open(&drc_param);
}

void music_drc_close(struct audio_drc *drc)
{
    if (drc) {
        audio_dec_drc_close(drc);
    }
}

void music_drc_run(struct audio_drc *drc, s16 *in, s16 *out, int len)
{
    if (drc) {
        audio_dec_drc_run(drc, in, len);
    }
}
#endif



vbass_hdl *music_virtual_bass_open(u32 sample_rate, u8 ch_num, int in_bit_width, int out_bit_width)
{
    VirtualBassParam vbass_param = {0};
    vbass_param.ratio = virtual_bass_param.parm.ratio;
    vbass_param.boost = virtual_bass_param.parm.boost;
    vbass_param.fc    = virtual_bass_param.parm.fc;
    vbass_param.ReserveLowFreqEnable = virtual_bass_param.parm.ReserveLowFreqEnable;
    vbass_param.channel = ch_num;
    vbass_param.SampleRate = sample_rate;
    vbass_param.pcm_info.IndataBit  = in_bit_width;
    vbass_param.pcm_info.OutdataBit = out_bit_width;
    vbass_param.pcm_info.IndataInc = ch_num;
    vbass_param.pcm_info.OutdataInc = ch_num;
    vbass_param.pcm_info.Qval = 15;

    struct virtual_bass_open_param param = {0};
    memcpy(&param.parm, &vbass_param, sizeof(vbass_param));
    param.bypass = virtual_bass_param.is_bypass;
    param.name = AEID_MUSIC_VIRTUAL_BASS;
    return audio_vbass_open(&param);
}

void music_virtual_bass_close(vbass_hdl *hdl)
{
    if (hdl) {
        audio_vbass_close(hdl);
    }
}


void music_virtual_bass_run(vbass_hdl *hdl, s16 *in, s16 *out, int len)
{
    if (hdl) {
        audio_vbass_run(hdl, in, out, len);
    }
}

void music_virtual_bass_update(u32 name, virtual_bass_param_tool_set *cfg)
{
    vbass_hdl *hdl = get_cur_vbass_hdl_by_name(name);
    if (!hdl) {
        return;
    }
    audio_vbass_update_parm(hdl, &cfg->parm);
    audio_vbass_bypass(hdl, cfg->is_bypass);
}

struct sof_eq_seg_info sof_seg[1] = {
    {0, EQ_IIR_TYPE_HIGH_PASS_ADVANCE, 0, 0, 0},
};


#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_ULTRA) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_ULTRA)
static int iir_type[] = {EQ_IIR_TYPE_LOW_PASS_ADVANCE, EQ_IIR_TYPE_HIGH_PASS_ADVANCE, EQ_IIR_TYPE_BAND_PASS};
void dynamic_eq_pro_ext_det_printf(dynamic_eq_pro_param_tool_set *dy_parm)
{
    log_debug("dy_parm->is_bypass %d\n", dy_parm->is_bypass);
    log_debug("delayms %d\n", dy_parm->DelayMs);
    log_debug("DetFrequency %d\n", dy_parm->DetFrequency);
    log_debug("DetType %d\n", dy_parm->DetType);
    log_debug("Order %d\n", dy_parm->Order);
    log_debug("nSection %d\n", dy_parm->nSection);
    for (int i = 0; i < dy_parm->nSection; i++) {
        log_debug("----------------nsection %d----------------\n", i);
        log_debug("fc %d\n ", dy_parm->effectParam[i].fc);
        log_debug("Q 0x%x\n ", *(int *)&dy_parm->effectParam[i].Q);
        log_debug("gain:0x%x\n", *(int *)&dy_parm->effectParam[i].gain);
        log_debug("attacktime %d\n ", dy_parm->effectParam[i].attackTime);
        log_debug("releaseTime %d\n ", dy_parm->effectParam[i].releaseTime);
        log_debug("low_threshold 0x%x\n ", *(int *)&dy_parm->effectParam[i].low_thr);
        log_debug("high_threshold 0x%x\n ", *(int *)&dy_parm->effectParam[i].high_thr);
        log_debug("type %d\n", dy_parm->effectParam[i].type);
        log_debug("rms 0x%x\n", *(int *)&dy_parm->effectParam[i].rmsTime);
        log_debug("outgain 0x%x\n", *(int *)&dy_parm->effectParam[i].OutputGain);
    }
}
struct dynamic_eq_pro_det *music_dynamic_eq_pro_det_open(u32 sample_rate, u8 ch_num, int in_bit_width, int out_bit_width)
{
    struct dynamic_eq_pro_det *hdl = zalloc(sizeof(struct dynamic_eq_pro_det));

    {
        sof_seg[0].iir_type = iir_type[dyeq_pro_param.DetType];
        sof_seg[0].gain     = (iir_type[dyeq_pro_param.DetType] == EQ_IIR_TYPE_BAND_PASS) ? 0 : dyeq_pro_param.Order;
        sof_seg[0].freq     = dyeq_pro_param.DetFrequency;
        sof_seg[0].q        = 0.7f;

        struct audio_sof_eq_param param = {0};
        param.seg = sof_seg;
        param.nsection = ARRAY_SIZE(sof_seg);
        param.global_gain = 0;
        param.sample_rate = sample_rate;
        param.in_bit_width  = 0;
        param.out_bit_width = out_bit_width ;
        param.channel = ch_num;	//通道数
        param.name = AEID_MUSIC_DY_EQ_PRO_DET;
        param.qval = 15;
        hdl->filter_eq = audio_sof_eq_open(&param);
    }

    {
        u8 channel = ch_num;
        DynamicEQProEffectParam *effectParam = zalloc(sizeof(DynamicEQProEffectParam) * 2);
        DynamicEQProParam param = {0};
        param.channel = channel;
        param.nSection = dyeq_pro_param.nSection;//EQ_NSECTION;
        param.SampleRate = sample_rate;
        param.DetectdataInc = (channel == 1) ? 1 : 2;
        param.DetectdataBit = in_bit_width;
        param.pcm_info.IndataBit = in_bit_width;
        param.pcm_info.OutdataBit = out_bit_width;
        param.pcm_info.IndataInc = (channel == 1) ? 1 : 2;
        param.pcm_info.OutdataInc = (channel == 1) ? 1 : 2;
        param.pcm_info.Qval = 15;
        for (int i = 0; i < param.nSection; i++) {
            effectParam[i].bypass           = dyeq_pro_param.effectParam[i].bypass;
            effectParam[i].fc               = dyeq_pro_param.effectParam[i].fc;
            effectParam[i].Q                = dyeq_pro_param.effectParam[i].Q;
            effectParam[i].gain             = dyeq_pro_param.effectParam[i].gain;
            effectParam[i].attackTime       = dyeq_pro_param.effectParam[i].attackTime;
            effectParam[i].releaseTime      = dyeq_pro_param.effectParam[i].releaseTime;
            effectParam[i].low_thr          = dyeq_pro_param.effectParam[i].low_thr;
            effectParam[i].high_thr         = dyeq_pro_param.effectParam[i].high_thr;
            effectParam[i].type             = dyeq_pro_param.effectParam[i].type;
            effectParam[i].rmsTime          = dyeq_pro_param.effectParam[i].rmsTime;
            effectParam[i].OutputGain       = dyeq_pro_param.effectParam[i].OutputGain;
        }

        hdl->dy_eq_pro = dynamic_eq_pro_open((DynamicEQProEffectParam *)effectParam, (DynamicEQProParam *)&param, AEID_MUSIC_DY_EQ_PRO_DET);
        free(effectParam);
    }


    {
        float ch_delay[2];
        ch_delay[0] = dyeq_pro_param.DelayMs;
        ch_delay[1] = dyeq_pro_param.DelayMs;
#define PCM_DELAY_MAX 25
        delay_parm_context parm = {0};
        parm.maxdelay_ms = PCM_DELAY_MAX;
        parm.delaydest_ms_array = ch_delay;//dyeq_pro_param.DelayMs;
        parm.speedv = 10;
        parm.quality = 5;
        parm.dataTypeobj.IndataBit  = in_bit_width;
        parm.dataTypeobj.OutdataBit = out_bit_width;
        parm.dataTypeobj.IndataInc = ch_num;
        parm.dataTypeobj.OutdataInc = ch_num;
        parm.dataTypeobj.Qval = 15;

        struct pcm_delay_open_parm param = {0};
        memcpy(&param.parm, &parm, sizeof(parm));
        param.bypass = dyeq_pro_param.is_bypass;
        param.sample_rate = sample_rate;
        param.ch_num = ch_num;
        param.name = AEID_MUSIC_DY_EQ_PRO_DET;
        hdl->pcm_delay = audio_pcm_delay_open(&param);
    }
    return hdl;
}

void music_dynamic_eq_pro_det_close(struct dynamic_eq_pro_det *hdl)
{
    if (!hdl) {
        return ;
    }
    if (hdl->filter_eq) {
        audio_sof_eq_close(hdl->filter_eq);
    }
    if (hdl->dy_eq_pro) {
        dynamic_eq_pro_close(hdl->dy_eq_pro);
    }
    if (hdl->pcm_delay) {
        audio_pcm_delay_close(hdl->pcm_delay);
    }
    free(hdl);
}
void music_dynamic_eq_pro_det_update(u32 name, dynamic_eq_pro_param_tool_set *dparam)
{
    struct audio_sof_eq *filter_eq = get_cur_sofeq_hdl_by_name(name);
    struct eq_fade_parm fade_parm = {0};

    if (iir_type[dparam->DetType] != EQ_IIR_TYPE_BAND_PASS) {
        int seg_num = 1;
        struct sof_eq_tool *tab = zalloc(sizeof(struct sof_eq_tool) + sizeof(struct sof_eq_seg_info) * seg_num);
        if (tab) {
            tab->global_gain = 0;
            tab->seg_num = seg_num;
            tab->is_bypass       = dparam->is_bypass;
            tab->seg[0].iir_type = iir_type[dparam->DetType];
            tab->seg[0].gain     = dparam->Order;
            tab->seg[0].freq     = dparam->DetFrequency;
            tab->seg[0].q        = 0.7f;
            audio_sof_eq_update(filter_eq, fade_parm, tab);
            free(tab);
        }
    }

    struct dynamic_eq_pro *dy_eq = get_cur_dy_eq_pro_hdl_by_name(name);
    dynamic_eq_pro_update(dy_eq, dparam);

    pcm_delay_hdl *pcm_delay = get_cur_pcm_delay_hdl_by_name(name);
    struct pcm_delay_update_parm param = {0};
    param.link = 1;
    param.speedv = 10;
    param.quality = 5;
    param.ch0_delay = dparam->DelayMs;
    param.ch1_delay = dparam->DelayMs;
    param.ch2_delay = dparam->DelayMs;
    param.ch3_delay = dparam->DelayMs;
    /* printf("==== param.ch0_delay %d\n", dparam->DelayMs); */
    audio_pcm_delay_update_parm(pcm_delay, &param);
}
void music_dynamic_eq_pro_det_run(struct dynamic_eq_pro_det *hdl, s16 *det_source, s16 *in, s16 *out, int len)
{
    struct effects_ultra *ultra = hdl->ultra;
    if (dyeq_pro_param.is_bypass) {
        if (in != out) {
            memcpy(out, in, len);
        }
        return;
    }
    if (iir_type[dyeq_pro_param.DetType] == EQ_IIR_TYPE_BAND_PASS) { //16->32
        audio_convert_data_16bit_to_32bit(det_source, (s32 *)ultra->filter_eq_detout, ultra->inlen / 2);
    } else {
        audio_sof_eq_run(hdl->filter_eq, det_source, ultra->filter_eq_detout, ultra->inlen);//16->32
    }
    audio_pcm_delay_run(hdl->pcm_delay, in, out, len);//32->32
    dynamic_eq_pro_run(hdl->dy_eq_pro, (int *)ultra->filter_eq_detout, (int *)out, len);//32->32
}
#endif

struct multiband_limiter_hdl *music_multiband_limiter_open(u32 sample_rate, u8 ch_num, int in_bit_width, int out_bit_width)
{
    struct multiband_limiter_open_param open_param = {0};
    memcpy(&open_param.param, &mblimiter_param, sizeof(mblimiter_param));
    open_param.sample_rate = sample_rate;
    open_param.channel     = ch_num;
    open_param.run32bit     = in_bit_width;
    open_param.Qval = 15;
    open_param.name = AEID_MUSIC_MBLIMITER;
    return audio_multiband_limiter_open(&open_param);
}


void music_multiband_limiter_close(struct multiband_limiter_hdl *hdl)
{
    if (!hdl) {
        return;
    }
    audio_multiband_limiter_close(hdl);
}

void music_multiband_limiter_run(struct multiband_limiter_hdl *hdl, s16 *in, s16 *out, int len)
{
    if (!hdl) {
        return;
    }
    audio_multiband_limiter_run(hdl, in, out, len);
}


void music_multiband_limiter_update(u32 name,  struct multiband_limiter_param_tool_set *update_param)
{
    struct multiband_limiter_hdl *hdl = get_cur_mblimiter_hdl_by_name(name);
    if (!hdl) {
        return;
    }
    audio_multiband_limiter_update_parm(hdl, update_param);
}

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  != EFFECTS_BASE)
struct audio_eq *music_post_eq_open(u32 sample_rate, u8 ch_num, int out_32bit)
{
    memcpy(&post_eq_tab_tmp, &post_eq_tab, sizeof(post_eq_tab_tmp));
    struct audio_eq_param param = {0};
    param.channels = ch_num;
    param.out_32bit = out_32bit;
    param.max_nsection = post_eq_tab_tmp.seg_num;
    param.nsection = post_eq_tab_tmp.seg_num;
    param.cb = eq_get_filter_info;
    param.eq_name = AEID_MUSIC_POST_EQ;
    param.sr = sample_rate;
    param.seg = post_eq_tab_tmp.seg;
    param.global_gain = post_eq_tab_tmp.global_gain;
    param.bypass = post_eq_tab_tmp.is_bypass;
    return audio_dec_eq_open(&param);
}

void music_post_eq_close(struct audio_eq *eq)
{
    if (eq) {
        audio_dec_eq_close(eq);
    }
}

void music_post_eq_run(struct effects_ultra *hdl, s16 *in, s16 *out, int len)
{
    if (!hdl) {
        return;
    }

    struct audio_eq *eq = hdl->post_eq;
    if (eq) {
        audio_convert_data_32bit_to_16bit((s32 *)in, (s16 *)in, (len >> 2)); //32->16
        audio_dec_eq_run(eq, in, out, len >> 1);	//16->16
    }
}
#endif

struct effects_ultra *music_effects_ultra_open(u32 sample_rate, u8 ch_num)
{
    struct effects_ultra *hdl = zalloc(sizeof(struct effects_ultra));

    hdl->ch_num = ch_num;
    int out_bit_width = 0;
#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_BASE)&& TCFG_BT_MUSIC_DRC_ENABLE//base模式下，drc
    out_bit_width = 1;
#elif defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  != EFFECTS_BASE)//advance 与ultra模式
    out_bit_width = 1;
#endif
    hdl->eq = music_eq_open(sample_rate, ch_num, out_bit_width);

#if TCFG_AUDIO_OUT_EQ_ENABLE
    hdl->bass_treble = audio_bass_treble_open(sample_rate, ch_num, out_bit_width, out_bit_width);
#endif
#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_BASE) && TCFG_BT_MUSIC_DRC_ENABLE
    hdl->drc = music_drc_open(sample_rate, ch_num, 1, 1);
#else

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  != EFFECTS_BASE)
    hdl->vbass = music_virtual_bass_open(sample_rate, ch_num, 1, 1);

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_ULTRA) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_ULTRA)
    hdl->dyeq_pro_det = music_dynamic_eq_pro_det_open(sample_rate, ch_num, 1, 1);
    hdl->dyeq_pro_det->ultra = hdl;
#endif

    hdl->mlimiter = music_multiband_limiter_open(sample_rate, ch_num, 1, 1);

    hdl->post_eq = music_post_eq_open(sample_rate, ch_num, 0);
#endif

#endif
    return hdl;
}


void music_effects_ultra_run(struct effects_ultra *hdl, s16 *in, s16 *out, int len)
{
    if (!hdl) {
        return;
    }

    if (hdl->inlen < len) {
        if (hdl->detout) {
            free(hdl->detout);
            hdl->detout = NULL;
        }
        if (hdl->filter_eq_detout) {
            free(hdl->filter_eq_detout);
            hdl->filter_eq_detout = NULL;
        }
    }

    if (!hdl->detout) {
        hdl->inlen = len;
        hdl->detlen = hdl->inlen * 2;
        hdl->detout = malloc(hdl->detlen);
#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_ULTRA) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_ULTRA)
        hdl->filter_eq_detout = malloc(hdl->detlen);
#endif
    }

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_BASE)&& !TCFG_BT_MUSIC_DRC_ENABLE//base模式下，无drc
    music_eq_run(hdl->eq, in, out, len);//16->16
#if TCFG_AUDIO_OUT_EQ_ENABLE
    audio_bass_treble_run(hdl->bass_treble, out, out, len);
#endif
#else
    music_eq_run(hdl->eq, in, hdl->detout, len);//16->32
#if TCFG_AUDIO_OUT_EQ_ENABLE
    audio_bass_treble_run(hdl->bass_treble, hdl->detout, hdl->detout, 2 * len); //32->32
#endif
#endif

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_BASE)&& TCFG_BT_MUSIC_DRC_ENABLE
    music_drc_run(hdl->drc, hdl->detout, hdl->detout, 2 * len); //32->32
    audio_convert_data_32bit_to_16bit((s32 *)hdl->detout, (s16 *)out, ((2 * len) >> 2)); //32->16
#endif


#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  != EFFECTS_BASE)
    music_virtual_bass_run(hdl->vbass, hdl->detout, hdl->detout, 2 * len); //32->32
#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_ULTRA) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_ULTRA)
    s16 *det_source = in;
    music_dynamic_eq_pro_det_run(hdl->dyeq_pro_det, det_source, hdl->detout, hdl->detout, 2 * len); //32->32
#endif

    music_multiband_limiter_run(hdl->mlimiter, hdl->detout, hdl->detout, 2 * len); //32->32

    music_post_eq_run(hdl, hdl->detout, out, 2 * len); //32->16
#endif
}


void music_effects_ultra_close(struct effects_ultra *hdl)
{
    if (!hdl) {
        return;
    }

    music_eq_close(hdl->eq);
#if TCFG_AUDIO_OUT_EQ_ENABLE
    audio_bass_treble_close(hdl->bass_treble);
#endif

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_BASE)&& TCFG_BT_MUSIC_DRC_ENABLE
    music_drc_close(hdl->drc);
#endif

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  != EFFECTS_BASE)
    music_virtual_bass_close(hdl->vbass);

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_ULTRA) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_ULTRA)
    music_dynamic_eq_pro_det_close(hdl->dyeq_pro_det);
#endif
    music_multiband_limiter_close(hdl->mlimiter);

    music_post_eq_close(hdl->post_eq);
#endif

    if (hdl->detout) {
        free(hdl->detout);
        hdl->detout = NULL;
    }

    if (hdl->filter_eq_detout) {
        free(hdl->filter_eq_detout);
        hdl->filter_eq_detout = NULL;
    }

    free(hdl);
}


//默认参数初始化
void audio_effects_ultra_init()
{
#if TCFG_AEC_UL_EQ_ENABLE
    //ul eq
    ul_eq_tab.is_bypass = 0;
    ul_eq_tab.global_gain = 0;
    ul_eq_tab.seg_num = ARRAY_SIZE(ul_eq_tab_normal);
    if (ul_eq_tab.seg_num > EQ_SECTION_MAX) {
        ul_eq_tab.seg_num = EQ_SECTION_MAX;
    }
    memcpy(ul_eq_tab.seg, ul_eq_tab_normal, sizeof(struct eq_seg_info)*ul_eq_tab.seg_num);
#endif
#if TCFG_PHONE_EQ_ENABLE
    //dl eq
    dl_eq_tab.is_bypass = 0;
    dl_eq_tab.global_gain = 0;
    dl_eq_tab.seg_num = ARRAY_SIZE(phone_eq_tab_normal);
    if (dl_eq_tab.seg_num > EQ_SECTION_MAX) {
        dl_eq_tab.seg_num = EQ_SECTION_MAX;
    }
    memcpy(dl_eq_tab.seg, phone_eq_tab_normal, sizeof(struct eq_seg_info)*dl_eq_tab.seg_num);
#endif

    //music eq
    music_eq_tab.is_bypass = 0;
    music_eq_tab.global_gain = 0;
    music_eq_tab.seg_num = ARRAY_SIZE(eq_tab_normal);
    if (music_eq_tab.seg_num > EQ_SECTION_MAX) {
        music_eq_tab.seg_num = EQ_SECTION_MAX;
    }
    memcpy(music_eq_tab.seg, eq_tab_normal, sizeof(struct eq_seg_info)*music_eq_tab.seg_num);

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  != EFFECTS_BASE)
    //post eq
    post_eq_tab.is_bypass = 0;
    post_eq_tab.global_gain = 0;
    post_eq_tab.seg_num = ARRAY_SIZE(eq_tab_normal);
    if (post_eq_tab.seg_num > EQ_SECTION_MAX) {
        post_eq_tab.seg_num = EQ_SECTION_MAX;
    }
    memcpy(post_eq_tab.seg, eq_tab_normal, sizeof(struct eq_seg_info)*post_eq_tab.seg_num);
#endif

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_BASE)&& TCFG_BT_MUSIC_DRC_ENABLE
    //drc
    int th = 0;//db -60db~0db
    int threshold = roundf(powf(10.0f, th / 20.0f) * 32768); // 0db:32768, -60db:33
    music_drc_param.nband = 1;
    music_drc_param.type = 1;
    music_drc_param._p.limiter[0].attacktime = 5;
    music_drc_param._p.limiter[0].releasetime = 500;
    music_drc_param._p.limiter[0].threshold[0] = threshold;
    music_drc_param._p.limiter[0].threshold[1] = 32768;
#endif

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  != EFFECTS_BASE)
    //virtual bass
    virtual_bass_param.is_bypass = 0;
    virtual_bass_param.parm.ratio = 10;
    virtual_bass_param.parm.boost = 1;
    virtual_bass_param.parm.fc = 100;
    virtual_bass_param.parm.ReserveLowFreqEnable = 0;

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_ULTRA) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_ULTRA)
    //dy eq pro det
    dyeq_pro_param.is_bypass = 0;
    dyeq_pro_param.DelayMs = 5;
    dyeq_pro_param.DetFrequency = 200;
    dyeq_pro_param.DetType = 1;//hp
    dyeq_pro_param.Order = 6;//阶数
    dyeq_pro_param.nSection = 1;
    for (int i = 0; i < dyeq_pro_param.nSection; i++) {
        dyeq_pro_param.effectParam[i].bypass       = 0;
        dyeq_pro_param.effectParam[i].fc           = 1000;
        dyeq_pro_param.effectParam[i].Q            = 0.7f;
        dyeq_pro_param.effectParam[i].gain         = 0.0f;
        dyeq_pro_param.effectParam[i].attackTime   = 5;
        dyeq_pro_param.effectParam[i].releaseTime  = 300;
        dyeq_pro_param.effectParam[i].low_thr      = -10.0f;
        dyeq_pro_param.effectParam[i].high_thr     = 0;
        dyeq_pro_param.effectParam[i].type         = 0;//peaking
        dyeq_pro_param.effectParam[i].rmsTime      = 0.02;
        dyeq_pro_param.effectParam[i].OutputGain   = 0;
    }
#endif

    //mb limiter
    mblimiter_param.common_param.is_bypass = 0;
    mblimiter_param.common_param.way_num = 2;
    mblimiter_param.common_param.order = 4;
    mblimiter_param.common_param.low_freq = 200;
    for (int i = 0; i < 3; i++) {
        mblimiter_param.parm[i].is_bypass = 0;
        mblimiter_param.parm[i].parm.attack_time = 1;
        mblimiter_param.parm[i].parm.release_time = 400;
        mblimiter_param.parm[i].parm.threshold = 0 * 1000;
        mblimiter_param.parm[i].parm.hold_time = 400;
        mblimiter_param.parm[i].parm.detect_time = 1 * 100;
        mblimiter_param.parm[i].parm.input_gain = 0 * 1000;
        mblimiter_param.parm[i].parm.output_gain = 0 * 1000;
        mblimiter_param.parm[i].parm.precision = 1;
    }
#endif

}

void audio_effects_ultra_update_base(u8 eff_module)
{
    if (eff_module == AEID_MUSIC_EQ) {
        //音乐配置
        cur_eq_set_global_gain(AEID_MUSIC_EQ, music_eq_tab.global_gain);
        for (int i = 0; i < music_eq_tab.seg_num; i++) {
            struct eq_seg_info *seg = &music_eq_tab.seg[i];
            cur_eq_set_update(AEID_MUSIC_EQ, seg, music_eq_tab.seg_num);
        }
    }

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_BASE)&& TCFG_BT_MUSIC_DRC_ENABLE
    if (eff_module == AEID_MUSIC_DRC) {
        struct audio_drc *shdl = get_cur_drc_hdl_by_name(AEID_MUSIC_DRC);
        if (shdl) {
            audio_drc_update_parm(shdl, (struct drc_ch *)&music_drc_param);
        }
    }
#endif

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  != EFFECTS_BASE)
    if (eff_module == AEID_MUSIC_VIRTUAL_BASS) {
        music_virtual_bass_update(AEID_MUSIC_VIRTUAL_BASS, &virtual_bass_param);
    }
    if (eff_module == AEID_MUSIC_MBLIMITER) {
        music_multiband_limiter_update(AEID_MUSIC_MBLIMITER,  &mblimiter_param);
    }
#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_ULTRA) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_ULTRA)
    if (eff_module == AEID_MUSIC_DY_EQ_PRO_DET) {
        music_dynamic_eq_pro_det_update(AEID_MUSIC_DY_EQ_PRO_DET, &dyeq_pro_param);
    }
#endif

    if (eff_module == AEID_MUSIC_POST_EQ) {
        cur_eq_set_global_gain(AEID_MUSIC_POST_EQ, post_eq_tab.global_gain);
        for (int i = 0; i < post_eq_tab.seg_num; i++) {
            struct eq_seg_info *seg = &post_eq_tab.seg[i];
            cur_eq_set_update(AEID_MUSIC_POST_EQ, seg, post_eq_tab.seg_num);
        }
    }
#endif

#if TCFG_PHONE_EQ_ENABLE
    if (eff_module == AEID_DL_EQ) {
        //通话配置
        cur_eq_set_global_gain(AEID_DL_EQ, dl_eq_tab.global_gain);
        cur_eq_set_update(AEID_DL_EQ, NULL, dl_eq_tab.seg_num);
    }
#endif
#if TCFG_AEC_UL_EQ_ENABLE
    if (eff_module == AEID_UL_EQ) {
        cur_eq_set_global_gain(AEID_UL_EQ, ul_eq_tab.global_gain);
        cur_eq_set_update(AEID_UL_EQ, NULL, ul_eq_tab.seg_num);
    }
#endif

}
void audio_effects_ultra_update(int id)
{
    log_debug("==========================music_effects_ultral_update===========\n");
    if (song_eq_mode == id) {
        audio_effects_ultra_update_base(AEID_MUSIC_EQ);
        audio_effects_ultra_update_base(AEID_MUSIC_DRC);
        audio_effects_ultra_update_base(AEID_MUSIC_VIRTUAL_BASS);
        audio_effects_ultra_update_base(AEID_MUSIC_MBLIMITER);
        audio_effects_ultra_update_base(AEID_MUSIC_DY_EQ_PRO_DET);
        audio_effects_ultra_update_base(AEID_MUSIC_POST_EQ);
    } else {
        audio_effects_ultra_update_base(AEID_DL_EQ);
        audio_effects_ultra_update_base(AEID_UL_EQ);
    }
    log_debug("==========================music_effects_ultral_update end ===========    \n");
}


void vbass_printf(void *p)
{
    virtual_bass_param_tool_set *cfg = (virtual_bass_param_tool_set *)p;
    log_debug("vbass, cfg.is_bypass %d, cfg.parm.ratio %d, cfg.parm.boost:%d, cfg.parm.fc:%d\n",
              cfg->is_bypass, cfg->parm.ratio, cfg->parm.boost, cfg->parm.fc);
}



void eq_printf(struct eq_tab *tab, u32 id)
{
    log_debug("id:  %x\n", id);
    log_debug("eq is_bypass %d\n", tab->is_bypass);
    log_debug("eq global_gain 0x%x\n", *(int *)&tab->global_gain);
    log_debug("eq seg_num %d\n", tab->seg_num);
    for (int i = 0; i < tab->seg_num; i++) {
        struct eq_seg_info *seg = (struct eq_seg_info *)&tab->seg[i];
        /* log_debug("eq idx:%d, iir:%d, freq:%d, gain:0x%08x, q:0x%08x ", seg->index, seg->iir_type, seg->freq, *(int *)&seg->gain, *(int *)&seg->q); */
        log_debug("eq idx:%d, iir:%d, freq:%d, gain:%d, gain x10:%d, q:%d , q x10:%d\n", seg->index, seg->iir_type, seg->freq, seg->gain, (10 * seg->gain) >> 20, seg->q, (10 * seg->q) >> 24);
    }

}




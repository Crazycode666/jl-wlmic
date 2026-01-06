#include "audio_dec_eff.h"
#include "effects_adj.h"

extern struct audio_dac_hdl dac_hdl;
extern const int const_surround_en;
void user_sat16(s32 *in, s16 *out, u32 npoint);
void a2dp_surround_set(u8 eff);
int audio_out_eq_get_filter_info(void *eq, int sr, struct audio_eq_filter_info *info);
int audio_out_eq_spec_set_info(struct audio_eq *eq, u8 idx, int freq, float gain);
void *dec_eq_drc_setup_new(void *priv, int (*eq_output_cb)(void *, void *, int), u32 sample_rate, u8 channel, u8 async, u8 drc_en);
void eq_cfg_default_init(EQ_CFG *eq_cfg);
void drc_default_init(EQ_CFG *eq_cfg, u8 mode);
int dec_drc_get_filter_info(void *drc, struct audio_drc_filter_info *info);

void eq_32bit_out(struct dec_eq_drc *eff)
{
    if (!config_eq_lite_en) {
        int wlen = 0;
        if (eff->priv && eff->out_cb) {
            wlen = eff->out_cb(eff->priv, &eff->eq_out_buf[eff->eq_out_points], (eff->eq_out_total - eff->eq_out_points) * 2);
        }
        eff->eq_out_points += wlen / 2;
    }
}

static int eq_output(void *priv, void *buf, u32 len)
{
    if (!config_eq_lite_en) {
        int wlen = 0;
        int rlen = len;
        s16 *data = (s16 *)buf;
        struct dec_eq_drc *eff = priv;
        if (!eff->async) {
            return rlen;
        }

        if (eff->drc && eff->async) {
            if (eff->eq_out_buf && (eff->eq_out_points < eff->eq_out_total)) {
                eq_32bit_out(eff);
                if (eff->eq_out_points < eff->eq_out_total) {
                    return 0;
                }
            }

            audio_drc_run(eff->drc, data, len);

            if ((!eff->eq_out_buf) || (eff->eq_out_buf_len < len / 2)) {
                if (eff->eq_out_buf) {
                    free(eff->eq_out_buf);
                }
                eff->eq_out_buf_len = len / 2;
                eff->eq_out_buf = malloc(eff->eq_out_buf_len);
                ASSERT(eff->eq_out_buf);
            }
            user_sat16((s32 *)data, (s16 *)eff->eq_out_buf, len / 4);
            eff->eq_out_points = 0;
            eff->eq_out_total = len / 4;

            eq_32bit_out(eff);
            return len;
        }

        int out_len = 0;
        if (eff->priv && eff->out_cb) {
            out_len = eff->out_cb(eff->priv, data, len);
        }
        return out_len;
    } else {
        return len;
    }
}



int wdrc_get_filter_info(void *drc, struct audio_drc_filter_info *info)
{
    static struct drc_ch wdrc_p = {0};
    struct threshold_group threshold[5] = {{-100 + 90.3f, -91.9f + 90.3f}, {-87 + 90.3f, -72.5f + 90.3f}, {-58.6f + 90.3f, -60.3f + 90.3f}, {-43.5f + 90.3f, -29.7f + 90.3f}, {0 + 90.3f, -10.9f + 90.3f}};
    /* struct threshold_group threshold[3] = {{0x42166666, 0x0}, {0x42580000, 0x42a40000},{0x42f00000, 0x42c50000}}; */

    wdrc_p.nband = 1;
    wdrc_p.type = 3;//wdrc

    //left
    int i = 0;
    wdrc_p._p.wdrc[i][0].attacktime = 1;
    wdrc_p._p.wdrc[i][0].releasetime = 500;
    memcpy(wdrc_p._p.wdrc[i][0].threshold, threshold, sizeof(threshold));
    wdrc_p._p.wdrc[i][0].threshold_num = ARRAY_SIZE(threshold);
    wdrc_p._p.wdrc[i][0].rms_time = 25;
    wdrc_p._p.wdrc[i][0].algorithm = 0;
    wdrc_p._p.wdrc[i][0].mode = 1;
    //right
    wdrc_p._p.wdrc[i][1].attacktime = 1;
    wdrc_p._p.wdrc[i][1].releasetime = 500;
    memcpy(wdrc_p._p.wdrc[i][1].threshold, threshold, sizeof(threshold));
    wdrc_p._p.wdrc[i][1].threshold_num = ARRAY_SIZE(threshold);
    wdrc_p._p.wdrc[i][1].rms_time = 25;
    wdrc_p._p.wdrc[i][1].algorithm = wdrc_p._p.wdrc[i][0].algorithm;
    wdrc_p._p.wdrc[i][1].mode = wdrc_p._p.wdrc[i][0].mode;

    info->R_pch = info->pch = &wdrc_p;
    return 0;
}

void dec_eq_drc_free(void *eff)
{
#if TCFG_EQ_ENABLE
    struct dec_eq_drc *eff_hdl = (struct dec_eq_drc *)eff;
    if (!eff_hdl) {
        return;
    }
    u8 tmp = 0;
    if (eff_hdl->drc_bef_eq) {
        tmp = 1;
    }
    if (eff_hdl->drc_prev) {
        audio_dec_drc_close(eff_hdl->drc_prev);
        eff_hdl->drc_prev = NULL;
    }

    if (eff_hdl->eq) {
        audio_dec_eq_close(eff_hdl->eq);
        eff_hdl->eq = NULL;
    }

    if (eff_hdl->drc) {
        audio_dec_drc_close(eff_hdl->drc);
        eff_hdl->drc = NULL;
    }
    if (eff_hdl->eq_out_buf) {
        free(eff_hdl->eq_out_buf);
        eff_hdl->eq_out_buf = NULL;
    }

    free(eff_hdl);
#endif//TCFG_EQ_ENABLE

}

struct drc_ch esco_drc_p = {0};
int esco_drc_get_filter_info(void *drc, struct audio_drc_filter_info *info)
{
    float th = -0.5f;//db -60db~0db,限幅器阈值
    int threshold = roundf(powf(10.0f, th / 20.0f) * 32768);
    esco_drc_p.nband = 1;
    esco_drc_p.type = 1;
    esco_drc_p._p.limiter[0].attacktime = 5;
    esco_drc_p._p.limiter[0].releasetime = 500;
    esco_drc_p._p.limiter[0].threshold[0] = threshold;
    esco_drc_p._p.limiter[0].threshold[1] = 32768;
    info->R_pch = info->pch = &esco_drc_p;
    return 0;
}
void *esco_eq_drc_setup(void *priv, int (*eq_output_cb)(void *, void *, int), u32 sample_rate, u8 channel, u8 async, u8 drc_en)
{
#if TCFG_EQ_ENABLE && TCFG_PHONE_EQ_ENABLE

    if (config_eq_lite_en) { //仅支持同步方式eq处理
        async = 0;//
    }

    struct dec_eq_drc *eff = zalloc(sizeof(struct dec_eq_drc));
    struct audio_eq_param eq_param = {0};

    eff->priv = priv;
    eff->out_cb = eq_output_cb;
    eq_param.channels = channel;
    eq_param.online_en = 1;
    eq_param.mode_en = 0;
    eq_param.remain_en = 0;
    eq_param.no_wait = async;
    if (drc_en) {
        eq_param.out_32bit = 1;
    }
    eq_param.max_nsection = dl_eq_tab.seg_num;
    eq_param.nsection = dl_eq_tab.seg_num;
    eq_param.cb = eq_get_filter_info;
    eq_param.eq_name = AEID_DL_EQ;
    eq_param.seg = dl_eq_tab.seg;
    eq_param.global_gain = dl_eq_tab.global_gain;
    eq_param.sr = sample_rate;
    eq_param.priv = eff;
    eq_param.output = eq_output;
    eq_param.bypass = dl_eq_tab.is_bypass;
    eff->eq = audio_dec_eq_open(&eq_param);

#if TCFG_DRC_ENABLE
    if (drc_en) {
        struct audio_drc_param drc_param = {0};
        drc_param.sr = sample_rate;
        drc_param.channels = channel;
        drc_param.online_en = 0;
        drc_param.remain_en = 0;
        drc_param.out_32bit = 1;
        drc_param.cb = esco_drc_get_filter_info;
        drc_param.name = AEID_DL_DRC;
        drc_param.cfg = &esco_drc_p;
        eff->drc = audio_dec_drc_open(&drc_param);
        eff->async = async;
    }
#endif//TCFG_DRC_ENABLE

    return eff;
#else
    return NULL;
#endif//TCFG_EQ_ENABLE && TCFG_PHONE_EQ_ENABLE

}
void esco_eq_drc_free(void *eff)
{
    dec_eq_drc_free(eff);
}


__attribute__((always_inline))
void sat16_to_sat32(s16 *in, s32 *out, u32 npoint, int factor)
{
#if 0
    for (int i = 0; i < npoint; i ++) {
        out[i] = in[i] * factor;
    }
#else
    s64 tmp;
    __asm__ volatile(
        "1:											\n\t"
        "rep %2 {									\n\t"        //循环
        "%3 = h[%1 ++= 2]*%4(s)                              \n\t"
        "[%0 ++= 4] = %3.l                            \n\t"   //%3.l意思是取tmp的低32位
        "}										    \n\t"
        "if(%2 != 0) goto 1b			            \n\t"
        :
        "=&r"(out),
        "=&r"(in),
        "=&r"(npoint),
        "=&r"(tmp)
        :
        "r"(factor),
        "0"(out),
        "1"(in),
        "2"(npoint),
        "3"(tmp)
        :
    );

#endif
}

int eq_drc_run(void *priv, void *data, u32 len)
{
#if TCFG_EQ_ENABLE
    struct dec_eq_drc *eff = (struct dec_eq_drc *)priv;
    if (!eff) {
        return 0;
    }

#if TCFG_DRC_ENABLE
    if (eff->drc && !eff->async) {//同步32bit eq drc 处理
        if ((!eff->eq_out_buf) || (eff->eq_out_buf_len < len * 2)) {
            if (eff->eq_out_buf) {
                free(eff->eq_out_buf);
            }
            eff->eq_out_buf_len = len * 2;
            eff->eq_out_buf = malloc(eff->eq_out_buf_len);
            ASSERT(eff->eq_out_buf);
        }
        audio_eq_set_output_buf(eff->eq, eff->eq_out_buf, len);

        if ((eff->drc_bef_eq == V1_GAME_EFF) && eff->drc_prev) {
            sat16_to_sat32((short *)data, (int *)eff->eq_out_buf, len >> 1, 1);
            audio_drc_run(eff->drc_prev, eff->eq_out_buf, len * 2);//32bit drc
        } else if ((eff->drc_bef_eq == NOR_GAME_EFF) && eff->drc_prev) {
            audio_drc_run(eff->drc_prev, data, len);//16bit drc
        }
    }
#endif//TCFG_DRC_ENABLE

    int eqlen = 0;
    if (eff->drc_bef_eq == V1_GAME_EFF) {
        eqlen = audio_eq_run(eff->eq, eff->eq_out_buf, len * 2);//32bit in out
    } else {
        eqlen = audio_eq_run(eff->eq, data, len);
    }

#if TCFG_DRC_ENABLE
    if (eff->drc && !eff->async) {//同步32bit eq drc 处理
        audio_drc_run(eff->drc, eff->eq_out_buf, len * 2);
        user_sat16((s32 *)eff->eq_out_buf, (s16 *)data, (len * 2) / 4);
    }
#endif//TCFG_DRC_ENABLE

    if (eff->drc && !eff->async) {
        return len;
    }
    return eqlen;
#else
    return len;
#endif
}


static struct audio_drc *mix_out_drc = NULL;


static float mix_out_drc_threadhold = -0.5f;//db -60db~0db,限幅器阈值
static struct drc_ch mix_out_drc_p = {0};
int mix_out_drc_get_filter_info(void *drc, struct audio_drc_filter_info *info)
{
    float th = mix_out_drc_threadhold;//db -60db~0db,限幅器阈值
    int threshold = roundf(powf(10.0f, th / 20.0f) * 32768);
    mix_out_drc_p.nband = 1;
    mix_out_drc_p.type = 1;
    mix_out_drc_p._p.limiter[0].attacktime = 5;
    mix_out_drc_p._p.limiter[0].releasetime = 500;
    mix_out_drc_p._p.limiter[0].threshold[0] = threshold;
    mix_out_drc_p._p.limiter[0].threshold[1] = 32768;
    info->R_pch = info->pch = &mix_out_drc_p;
    return 0;
}

void mix_out_drc_open(u16 sample_rate)
{
    u8 ch_num;
#if TCFG_APP_FM_EMITTER_EN
    ch_num = 2;
#else
    u8 dac_connect_mode = audio_dac_get_channel(&dac_hdl);
    if (dac_connect_mode == DAC_OUTPUT_LR) {
        ch_num =  2;
    } else {
        ch_num =  1;
    }
#endif//TCFG_APP_FM_EMITTER_EN

#if TCFG_DRC_ENABLE
    struct audio_drc_param drc_param = {0};
    drc_param.sr = sample_rate;
    drc_param.channels = ch_num;
    drc_param.online_en = 0;
    drc_param.remain_en = 0;
    drc_param.out_32bit = 0;
    drc_param.cb = mix_out_drc_get_filter_info;
    drc_param.name = AEID_MIX_OUT_DRC;
    drc_param.cfg = &mix_out_drc_p;
    mix_out_drc = audio_dec_drc_open(&drc_param);
#endif//TCFG_DRC_ENABLE

}

void mix_out_drc_close()
{
#if TCFG_DRC_ENABLE
    if (mix_out_drc) {
        audio_dec_drc_close(mix_out_drc);
        mix_out_drc = NULL;
    }
#endif//TCFG_DRC_ENABLE

}

void mix_out_drc_run(s16 *data, u32 len)
{
#if TCFG_DRC_ENABLE
    if (mix_out_drc) {
        audio_drc_run(mix_out_drc, data, len);
    }
#endif//TCFG_DRC_ENABLE

}

/*----------------------------------------------------------------------------*/
/**@brief    mix_out后限幅器系数更新
   @param    threadhold限幅器阈值，-60~0,单位db
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void mix_out_drc_threadhold_update(float threadhold)
{
#if TCFG_DRC_ENABLE
    mix_out_drc_threadhold = threadhold;

    local_irq_disable();
    if (mix_out_drc) {
        mix_out_drc->updata = 1;
    }
    local_irq_enable();
#endif//TCFG_DRC_ENABLE

}
#if defined(EQ_CORE_V1)
struct audio_eq *audio_dec_eq_open_new(struct audio_eq_param *parm, struct eq_parm_new *par_new)
{
    struct audio_eq_param *eq_param = parm;
    struct audio_eq *eq = zalloc(sizeof(struct audio_eq) + sizeof(struct hw_eq_ch));
    if (eq) {
        eq->eq_ch = (struct hw_eq_ch *)((int)eq + sizeof(struct audio_eq));

        audio_eq_open(eq, eq_param);
        audio_eq_set_samplerate(eq, eq_param->sr);
        audio_eq_set_info_new(eq, eq_param->channels, par_new->in_mode, eq_param->out_32bit, par_new->run_mode, par_new->data_in_mode, par_new->data_out_mode);
        /* log_info("eq_param->sr %d, eq_param->channels %d\n", eq_param->sr,  eq_param->channels); */
        audio_eq_set_output_handle(eq, eq_param->output, eq_param->priv);
        audio_eq_start(eq);
        /* log_info("audio_dec_eq_open name %d\n", eq_param->eq_name); */
    }
    return eq;
}
#endif/*defined(EQ_CORE_V1)*/

#if defined(AUDIO_GAME_EFFECT_CONFIG) && AUDIO_GAME_EFFECT_CONFIG

struct drc_ch dec_drc_p = {0};
int dec_drc_get_filter_info(void *drc, struct audio_drc_filter_info *info)
{
    float th = -0.5f;//db -60db~0db,限幅器阈值
    int threshold = roundf(powf(10.0f, th / 20.0f) * 32768);
    dec_drc_p.nband = 1;
    dec_drc_p.type = 1;
    dec_drc_p._p.limiter[0].attacktime = 5;
    dec_drc_p._p.limiter[0].releasetime = 500;
    dec_drc_p._p.limiter[0].threshold[0] = threshold;
    dec_drc_p._p.limiter[0].threshold[1] = 32768;
    info->R_pch = info->pch = &dec_drc_p;
    return 0;
}


#if	defined(EQ_CORE_V1)
void *dec_eq_drc_setup_new(void *priv, int (*eq_output_cb)(void *, void *, int), u32 sample_rate, u8 channel, u8 async, u8 drc_en)
{
#if  TCFG_EQ_ENABLE

    struct dec_eq_drc *eff = zalloc(sizeof(struct dec_eq_drc));
    if (config_eq_lite_en) { //仅支持同步方式eq处理
        async = 0;//
    }

#if TCFG_DRC_ENABLE
    if (drc_en & BIT(1)) { //game eff drc
        if (!eq_file_get_cfg(get_eq_cfg_hdl(), (u8 *)SDFILE_RES_ROOT_PATH"eq_game_eff.bin")) { //加载游戏音效
            EQ_CFG *eq_cfg = get_eq_cfg_hdl();
            eq_cfg->eq_type = EQ_TYPE_FILE;
        }
        eff->drc_bef_eq = V1_GAME_EFF;
        async = 0;
        struct audio_drc_param drc_param = {0};
        drc_param.sr = sample_rate;
        drc_param.channels = channel;
        drc_param.online_en = 1;
        drc_param.remain_en = 1;
        drc_param.out_32bit = 1;
        drc_param.cb = drc_get_filter_info;
        drc_param.drc_name = AEID_MUSIC_DRC;
        drc_param.cfg = &music_drc_param;
        eff->drc_prev = audio_dec_drc_open(&drc_param);
    }
#endif//TCFG_DRC_ENABLE

    eff->priv = priv;
    eff->out_cb = eq_output_cb;

    struct audio_eq_param eq_param = {0};
    eq_param.channels = channel;
    eq_param.online_en = 1;
    eq_param.mode_en = 1;
    eq_param.remain_en = 1;
    eq_param.no_wait = async;
    if (drc_en) {
        eq_param.out_32bit = 1;
    }
    eq_param.max_nsection = EQ_SECTION_MAX;
    eq_param.cb = eq_get_filter_info;
    eq_param.eq_name = AEID_MUSIC_EQ;
    eq_param.sr = sample_rate;
    eq_param.priv = eff;
    eq_param.output = eq_output;

    struct eq_parm_new par_new = {0};
    par_new.in_mode = DATI_INT;
    par_new.run_mode = NORMAL;
    par_new.data_in_mode = SEQUENCE_DAT_IN;
    par_new.data_out_mode = SEQUENCE_DAT_OUT;
    eff->eq = audio_dec_eq_open_new(&eq_param, &par_new);
    eff->async = async;
#if TCFG_DRC_ENABLE
    if (drc_en) {
        struct audio_drc_param drc_param = {0};
        drc_param.sr = sample_rate;
        drc_param.channels = channel;
        if (eff->drc_bef_eq == V1_GAME_EFF) {
            drc_param.online_en = 0;
            drc_param.remain_en = 0;
            drc_param.out_32bit = 1;
            drc_param.cb = dec_drc_get_filter_info;
            drc_param.drc_name = mic_eq_mode;
        } else {
            drc_param.online_en = 1;
            drc_param.remain_en = 1;
            drc_param.out_32bit = 1;
            drc_param.cb = drc_get_filter_info;
            drc_param.drc_name = song_eq_mode;
        }
        eff->drc = audio_dec_drc_open(&drc_param);
    }
#endif//TCFG_DRC_ENABLE

    return eff;
#else
    return NULL;
#endif//TCFG_EQ_ENABLE

}
#endif/*EQ_CORE_V1*/
#endif /*AUDIO_GAME_EFFECT_CONFIG*/

#if defined(AUDIO_SPK_EQ_CONFIG) && AUDIO_SPK_EQ_CONFIG





/*
 *spk_eq系数表
 * */
static struct eq_seg_info_float spk_eq_seg_tab[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    0, 0.7f},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,    0, 0.7f},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,   0, 0.7f},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   0, 0.7f},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   0, 0.7f},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  0, 0.7f},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  0, 0.7f},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,  0, 0.7f},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,  0, 0.7f},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000, 0, 0.7f}
};
/*
 *spk_eq系数表运算后存放的buf
 * */
static int spk_eq_coeff_tab[10][5];
/*
 *spk_eq 总增益
 * */
static float spk_eq_global_gain = 0;
static u32 seg_sel = 0;
/*
 *spk_eq系数回调函数
 * */
int spk_eq_get_filter_info(void *_eq, int sr, struct audio_eq_filter_info *info)
{
    if (!sr) {
        sr = 44100;
    }
    local_irq_disable();
    u8 nsection = ARRAY_SIZE(spk_eq_seg_tab);
    for (int i = 0; i < nsection; i++) {
        if (seg_sel & BIT(i)) {
            seg_sel &= ~BIT(i);
            //转换系数为br30的硬件格式
            struct eq_seg_info seg = {0};
            seg.index = spk_eq_seg_tab[i].index;
            seg.iir_type = spk_eq_seg_tab[i].iir_type;
            seg.freq = spk_eq_seg_tab[i].freq;
            seg.gain = (int)(spk_eq_seg_tab[i].gain * (1 << 20));
            seg.q    = (int)(spk_eq_seg_tab[i].q * (1 << 24));
            eq_seg_design(&seg, sr, spk_eq_coeff_tab[i]);
            /* float *tar_buf = spk_eq_coeff_tab[i]; */
            /* printf("cal coeff:0x%x, 0x%x, 0x%x, 0x%x, 0x%x " */
            /* , tar_buf[0] */
            /* , tar_buf[1] */
            /* , tar_buf[2] */
            /* , tar_buf[3] */
            /* , tar_buf[4] */
            /* ); */
        }
    }
    /* printf("spk_eq_global_gain %d\n",(int)spk_eq_global_gain); */
    local_irq_enable();
    info->L_coeff = info->R_coeff = (void *)spk_eq_coeff_tab;//系数指针赋值
    info->L_gain = info->R_gain = spk_eq_global_gain;//总增益填写，用户可修改（-20~20db）
    info->nsection = nsection;//eq段数，根据提供给的系数表来填写，例子是2
    return 0;
}

static struct audio_eq *spk_eq = NULL;
/*
 *spk_eq 系数更新接口,更新第几段eq系数
 *parm: *seg
 *seg->index:第几段(0~9)
 *seg->iir_type:滤波器类型(EQ_IIR_TYPE)
 *seg->freq:中心截止频率(20~20kHz)
 *seg->gain:增益（-12~13dB）
 * */
void spk_eq_seg_update(struct eq_seg_info_float *seg)
{
    if (!seg) {
        log_e("spk_eq seg null\n");
        return ;
    }
    u8 i = seg->index;
    if (i >= ARRAY_SIZE(spk_eq_seg_tab)) {
        log_e("spk_eq index err: %d >= %d\n", seg->index, ARRAY_SIZE(spk_eq_seg_tab));
        return ;
    }
    local_irq_disable();
    memcpy(&spk_eq_seg_tab[i], seg, sizeof(struct eq_seg_info_float));
    seg_sel |= BIT(i);
    if (spk_eq) {
        struct audio_eq *eq = spk_eq;
        eq->updata = 1;//设置更新标志
    }
    local_irq_enable();
}
/*
 *spk_eq 总增益更新
 * */
void spk_eq_global_gain_udapte(float global_gain)
{
    spk_eq_global_gain = global_gain;
    if (spk_eq) {
        /* printf("spk_eq_global_gain %d\n",(int)spk_eq_global_gain); */
        local_irq_disable();
        struct audio_eq *eq = spk_eq;
        eq->updata = 1;//设置更新标志
        local_irq_enable();
    }
}
/*
 *spk_eq 打开
 * */
void spk_eq_open(u32 sample_rate)
{
    if (spk_eq) {
        return ;
    }
    seg_sel = 0xffff;
    u8 ch_num;
#if TCFG_APP_FM_EMITTER_EN
    ch_num = 2;
#else
    u8 dac_connect_mode = audio_dac_get_channel(&dac_hdl);
    if (dac_connect_mode == DAC_OUTPUT_LR) {
        ch_num =  2;
    } else {
        ch_num =  1;
    }
#endif//TCFG_APP_FM_EMITTER_EN


    struct audio_eq_param eq_param = {0};

    eq_param.channels = ch_num;
    eq_param.max_nsection = ARRAY_SIZE(spk_eq_seg_tab);
    eq_param.nsection = ARRAY_SIZE(spk_eq_seg_tab);
    eq_param.cb = spk_eq_get_filter_info;
    eq_param.eq_name = AEID_SPEAKER_EQ;
    eq_param.sr = sample_rate;
    eq_param.seg = NULL;
    eq_param.global_gain = 0;

    spk_eq = audio_dec_eq_open(&eq_param);

    /* sys_timer_add(NULL, spk_eq_global_gain_udpate_test, 5000); */
    /* sys_timer_add(NULL,spk_eq_seg_udpate_test , 5000); */
}
/*
 *spk_eq 关闭
 * */
void spk_eq_close()
{
    if (spk_eq) {
        audio_dec_eq_close(spk_eq);
        spk_eq = NULL;
    }
}

/*
 *spk_eq 数据处理
 * */
void spk_eq_run(s16 *data, u16 len)
{
    if (!spk_eq) {
        return ;
    }
    u16 sample_rate = audio_mixer_get_sample_rate(&mixer);
    struct audio_eq *eq = spk_eq;
    if (eq && (sample_rate != eq->sr)) {
        audio_eq_set_samplerate(eq, sample_rate);
    }
    audio_eq_run(eq, data, len);
}
/*
 *spk_eq 系数表保存到vm
 * */
int spk_eq_save_to_vm(void)
{
    int ret_tmp = 0;
    /* puts("====jj===========to save \n"); */
    /* printf("sizeof(spk_eq_seg_tab) %d, %d\n", sizeof(spk_eq_seg_tab), sizeof(float)); */
    /* printf("%d %d\n", CFG_SPK_EQ_SEG_SAVE, CFG_SPK_EQ_GLOBAL_GAIN_SAVE); */
    int ret = syscfg_write(CFG_SPK_EQ_SEG_SAVE, spk_eq_seg_tab, sizeof(spk_eq_seg_tab));
    if (ret <= 0) {
        printf("spk_eq tab write to vm err, ret %d\n", ret);
        ret_tmp = -1;
    }
    ret = syscfg_write(CFG_SPK_EQ_GLOBAL_GAIN_SAVE, &spk_eq_global_gain, sizeof(float));
    if (ret <= 0) {
        printf("spk_eq global gain write to vm err ret %d\n", ret);
        ret_tmp = -1;
    }
    /* puts("===============to save end\n"); */
    return ret_tmp;
}
/*
 *spk_eq 系数表从vm中读取
 * */
void spk_eq_read_from_vm()
{
    int ret = syscfg_read(CFG_SPK_EQ_SEG_SAVE, spk_eq_seg_tab, sizeof(spk_eq_seg_tab));
    if (ret <= 0) {
        printf("skp_eq read from vm err\n");
    }
    ret = syscfg_read(CFG_SPK_EQ_GLOBAL_GAIN_SAVE, &spk_eq_global_gain, sizeof(float));
    if (ret <= 0) {
        printf("spk_eq global gain read from vm err\n");
    }
}


typedef struct {
    u16 magic;     //0x3344
    u16 crc;       //data crc
    u8 data[32];   //data
} SPK_EQ_PACK;

#define CMD_SEG    0x1
#define CMD_GLOBAL 0x2
#define CMD_SAVE_PARM 0x3
#define CMD_RESET_PARM 0x4

#define CMD_SUPPORT_GLOBAL_GAIN 0x7

#define CMD_READ_SEG_L   0x8//左声道或者单声道获取系数表
#define SPK_EQ_CRC_EN  0//是否使能crc校验
static u8 parse_seq = 0;
int spk_eq_spp_rx_packet(u8 *packet, u8 len)
{

    SPK_EQ_PACK pack = {0};//packet;
    memcpy(&pack, packet, len);
    if (pack.magic != 0x3344) {
        printf("magic err 0x%x\n", pack.magic);
        return -1;
    }
    u8 cmd = pack.data[0];
    u16 crc;
    switch (cmd) {
    case CMD_SEG:
#if SPK_EQ_CRC_EN
        crc = CRC16(pack.data, len - 4);
        if (crc != pack.crc) {
            printf("spk_seg pack crc err %x, %x\n", crc, pack.crc);
            return -1;
        }
#endif
        struct eq_seg_info_float seg = {0};
        memcpy(&seg, &pack.data[1], sizeof(struct eq_seg_info_float));
        spk_eq_seg_update(&seg);

        printf("idx:%d, iir:%d, frq:%d, gain:0x%x, q:0x%x \n", seg.index, seg.iir_type, seg.freq, *(int *)&seg.gain, *(int *)&seg.q);
        break;
    case CMD_GLOBAL:
#if SPK_EQ_CRC_EN
        crc = CRC16(pack.data, len - 4);
        if (crc != pack.crc) {
            printf("spk global gain info pack crc err %x, %x\n", crc, pack.crc);
            return -1;
        }
#endif
        float global_gain = 0;
        memcpy(&global_gain, &pack.data[1], sizeof(float));
        spk_eq_global_gain_udapte(global_gain);
        printf("global_gain 0x%x\n", *(int *)&global_gain);
        break;
    case  CMD_SAVE_PARM:
        return spk_eq_save_to_vm();
        break;
    case CMD_RESET_PARM:
        local_irq_disable();
        for (int i = 0; i < ARRAY_SIZE(spk_eq_seg_tab); i++) {
            spk_eq_seg_tab[i].index = i;
            spk_eq_seg_tab[i].freq = 100;
            spk_eq_seg_tab[i].iir_type = EQ_IIR_TYPE_BAND_PASS;
            spk_eq_seg_tab[i].gain = 0.0f;
            spk_eq_seg_tab[i].q = 0.7f;
            seg_sel |= BIT(i);
        }
        local_irq_enable();
        spk_eq_global_gain_udapte(0);
        return spk_eq_save_to_vm();
        break;
    case CMD_SUPPORT_GLOBAL_GAIN:
        //support global_gain
        break;
    case CMD_READ_SEG_L:
        app_online_db_ack(parse_seq, spk_eq_seg_tab, sizeof(spk_eq_seg_tab));
        return 1;
    default:
        printf("crc %d\n", pack.crc);
        printf("cmd err %x\n", cmd);
        return -1;
    }
    return 0;
}

static int spk_eq_app_online_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size)
{
    parse_seq = ext_data[1];
    printf("parse_seq %x\n", parse_seq);
    int ret = spk_eq_spp_rx_packet(packet, size);
    if (!ret) {
        u8 ack[] = "OK";
        app_online_db_ack(parse_seq, ack, sizeof(ack));
        /* app_online_db_ack(parse_seq, packet, size); */
    } else if (ret != 1) {
        u8 ack[] = "ER";
        app_online_db_ack(parse_seq, ack, sizeof(ack));
        /* app_online_db_ack(parse_seq, packet, size); */
    }
    return 0;
}


int spk_eq_init(void)
{
#if APP_ONLINE_DEBUG
    app_online_db_register_handle(DB_PKT_TYPE_SPK_EQ, spk_eq_app_online_parse);
#endif
    return 0;
}
__initcall(spk_eq_init);


#endif/*AUDIO_SPK_EQ_CONFIG*/


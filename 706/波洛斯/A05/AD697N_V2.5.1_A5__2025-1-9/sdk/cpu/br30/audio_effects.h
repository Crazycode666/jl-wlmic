#ifndef __AUDIO_EFFECTS_H__
#define __AUDIO_EFFECTS_H__


#include "effects_adj.h"

struct dynamic_eq_pro_det {
    struct audio_sof_eq *filter_eq;
    pcm_delay_hdl *pcm_delay;
    struct dynamic_eq_pro *dy_eq_pro;
    void *ultra;
};

struct effects_ultra {
    s16 inlen;
    s16 *detout;//tmp buf
    s16 *filter_eq_detout;//dyeq_pro det data
    s16 detlen;

    struct audio_eq *eq;
    struct audio_sof_eq *bass_treble;
    struct audio_drc *drc;

    vbass_hdl *vbass;
#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_ULTRA) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_ULTRA)
    //dynamic eq pro det
    struct dynamic_eq_pro_det *dyeq_pro_det;
#endif

    struct multiband_limiter_hdl *mlimiter;

    struct audio_eq *post_eq;
    u8 ch_num;

};


void audio_effects_init();
void music_virtual_bass_update(u32 name, virtual_bass_param_tool_set *cfg);
void dynamic_eq_pro_ext_det_printf(dynamic_eq_pro_param_tool_set *dy_parm);
void music_dynamic_eq_pro_det_update(u32 name, dynamic_eq_pro_param_tool_set *dparam);
void music_multiband_limiter_update(u32 name,  struct multiband_limiter_param_tool_set *update_param);
void vbass_printf(void *p);
void audio_effects_ultra_init();

struct effects_ultra *music_effects_ultra_open(u32 sample_rate, u8 ch_num);
void music_effects_ultra_run(struct effects_ultra *hdl, s16 *in, s16 *out, int len);
void music_effects_ultra_close(struct effects_ultra *hdl);
void audio_effects_ultra_update(int id);

#endif

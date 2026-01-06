#ifndef __EFFECT_ADJ__H
#define __EFFECT_ADJ__H

#include "system/includes.h"
#include "app_config.h"
#include "asm/hw_eq.h"
#include "application/audio_drc.h"
#include "media/audio_vbass.h"
#include "media/audio_sof_eq.h"
#include "media/audio_multiband_limiter.h"
#include "media/dynamic_eq_pro.h"
#include "media/pcm_delay.h"
#include "application/audio_eq.h"
#include "application/eq_config.h"
#include "online_db_deal.h"

enum {
    AEID_MUSIC_EQ = 0x1,
    AEID_MUSIC_PRE_DRC,
    AEID_MUSIC_DRC,
    AEID_MIX_OUT_DRC,
    AEID_UL_EQ,
    AEID_DL_EQ,
    AEID_DL_DRC,
    AEID_DCC_EQ,
    AEID_SPEAKER_EQ,
    AEID_MUSIC_MBLIMITER,
    AEID_AUDIO_OUT_EQ,
    AEID_AUDIO_OUT_DRC,
    AEID_MUSIC_DY_EQ_PRO_DET,
    AEID_MUSIC_VIRTUAL_BASS,
    AEID_MUSIC_POST_EQ,
    AEID_MUSIC_BASS_TREBLE_EQ,
    AEID_MUSIC_BOOST_DRC,

};

#define mSECTION_MAX EQ_SECTION_MAX

#ifndef TCFG_CALL_DL_EQ_SECTION
#define TCFG_CALL_DL_EQ_SECTION  3
#endif
#ifndef TCFG_CALL_UL_EQ_SECTION
#define TCFG_CALL_UL_EQ_SECTION  3
#endif

#define EQ_GLOBAL_GAIN_CMD (65535)
#define EQ_BYPASS_CMD      (65534)

struct eq_tab {
    int is_bypass;  //1:bypass 0:no_bypass
    float global_gain;    //总增益
    int seg_num;          //eq效果文件存储的段数
    int enable_section;   //
    struct eq_seg_info seg[EQ_SECTION_MAX];
};

void eq_printf(struct eq_tab *tab, u32 id);
void audio_convert_data_32bit_to_16bit(s32 *in, s16 *out, u32 npoint);

int ci_send_cmd(void *priv, u32 id, u8 *packet, int size);
int eq_app_online_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size);
void audio_effects_ultra_update_base(u8 eff_module);

extern struct eq_tab ul_eq_tab;
extern struct eq_tab dl_eq_tab;
extern struct eq_tab dcc_eq_tab;
extern struct eq_tab music_eq_tab;
extern struct eq_tab post_eq_tab;

extern struct drc_ch music_drc_param;
extern virtual_bass_param_tool_set  virtual_bass_param;
extern dynamic_eq_pro_param_tool_set dyeq_pro_param;
extern struct multiband_limiter_param_tool_set mblimiter_param;

extern const struct eq_seg_info eq_tab_normal[10];
extern const struct eq_seg_info *eq_type_tab[EQ_MODE_MAX];
extern float global_gain_tab[EQ_MODE_MAX];

//eq_cfg_hw.bin中播歌eq曲线，当作用户自定义模式，参与效果切换.
#define EQ_FILE_CP_TO_CUSTOM  0


#if EQ_FILE_CP_TO_CUSTOM
extern struct eq_seg_info ul_eq_tab_normal[TCFG_CALL_UL_EQ_SECTION];
extern struct eq_seg_info phone_eq_tab_normal[TCFG_CALL_DL_EQ_SECTION];
#else
extern const struct eq_seg_info ul_eq_tab_normal[TCFG_CALL_UL_EQ_SECTION];
extern const struct eq_seg_info phone_eq_tab_normal[TCFG_CALL_DL_EQ_SECTION];
#endif

extern u8 eq_get_table_nsection(EQ_MODE mode);
#endif/*__EFFECTS_ADJ__H*/

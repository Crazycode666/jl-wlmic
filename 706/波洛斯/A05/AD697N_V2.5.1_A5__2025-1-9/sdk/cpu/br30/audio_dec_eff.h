#ifndef _AUD_DEC_EFF_H
#define _AUD_DEC_EFF_H
#include "typedef.h"
#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "application/eq_config.h"
#include "app_config.h"

#include "app_main.h"
#include "application/audio_eq.h"
typedef int (*eq_output_cb)(void *, void *, int);

struct dec_eq_drc {
    s16 *eq_out_buf;
    int eq_out_buf_len;
    int eq_out_points;
    int eq_out_total;

    void *priv;
    eq_output_cb  out_cb;
    void *drc_prev;
    void *eq;
    void *drc;
    u8 async;
    u8 drc_bef_eq;
    u8 remain;
};

struct eq_parm_new {
    u8 in_mode: 2;
    u8 run_mode: 2;
    u8 data_in_mode: 2;
    u8 data_out_mode: 2;
};
void *esco_eq_drc_setup(void *priv, int (*eq_output_cb)(void *, void *, int), u32 sample_rate, u8 channel, u8 async, u8 drc_en);
void esco_eq_drc_free(void *eff);

int eq_drc_run(void *priv, void *data, u32 len);

void mix_out_drc_open(u16 sample_rate);
void mix_out_drc_close();
void mix_out_drc_run(s16 *data, u32 len);
/*----------------------------------------------------------------------------*/
/**@brief    mix_out后限幅器系数更新
   @param    threadhold限幅器阈值，-60~0,单位db
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void mix_out_drc_threadhold_update(float threadhold);
#define V1_GAME_EFF  1
#define NOR_GAME_EFF 2

#if defined(AUDIO_SPK_EQ_CONFIG) && AUDIO_SPK_EQ_CONFIG
#include "online_db_deal.h"
#endif

struct eq_seg_info_float {
    u16 index;
    u16 iir_type; ///<EQ_IIR_TYPE
    int freq;
    float gain;
    float q;
};

void spk_eq_seg_update(struct eq_seg_info_float *seg);
void spk_eq_global_gain_udapte(float global_gain);
void spk_eq_open(u32 sample_rate);
void spk_eq_close();
void spk_eq_run(s16 *data, u16 len);
int spk_eq_save_to_vm(void);
void spk_eq_read_from_vm();

extern struct audio_dac_hdl dac_hdl;
extern struct audio_mixer mixer;
#endif


#ifndef __AUDIO_MULTIBAND_LIMITER__
#define __AUDIO_MULTIBAND_LIMITER__

#include "AudioEffect_DataType.h"
#include "media/audio_limiter.h"
#include "media/audio_sof_eq.h"
#include "multi_ch_mix.h"
#include "media/audio_crossover.h"
#include "media/audio_hw_crossover.h"
#include "audio_drc_common.h"
#include "media/audio_multiband_crossover.h"
#include "os/os_api.h"


struct multiband_limiter_param_tool_set {//多带 LIMITER界面参数
    struct mdrc_common_param common_param;
    struct limiter_param_tool_set parm[3];//[0]低频 [1]高频  [2]全频
};

struct multiband_limiter_open_param {
    struct multiband_limiter_param_tool_set param;
    u32 sample_rate;
    u32 name;
    u8 channel;
    u8 run32bit;                                  //运行位宽32bit使能
    u8 Qval;                                      //数据饱和值15：16bit  23:24bit
    u8 eq_core;                                   //硬件分频器的所使用的eq核
};
struct multiband_limiter_hdl {
    struct list_head hentry;                        //
    struct audio_limiter *limiter[4];              //DRC句柄
    struct multiband_limiter_param_tool_set param;
    struct multiband_crossover mcross;
    OS_MUTEX mutex;
    u32 name;
    u8 mlimiter_init;
};


struct multiband_limiter_hdl *audio_multiband_limiter_open(struct multiband_limiter_open_param *param);
int audio_multiband_limiter_close(struct multiband_limiter_hdl *hdl);
int audio_multiband_limiter_run(struct multiband_limiter_hdl *hdl, s16 *indata, s16 *outdata, u32 len);
void audio_multiband_limiter_update_parm(struct multiband_limiter_hdl *hdl, struct multiband_limiter_param_tool_set *update_param);

extern void butterworth_hp_design(int fc, int fs, int nSOS, int *coeff);
extern void  butterworth_lp_design(int fc, int fs, int nSOS, int *coeff);

struct multiband_limiter_hdl *get_cur_mblimiter_hdl_by_name(u32 name);

#endif

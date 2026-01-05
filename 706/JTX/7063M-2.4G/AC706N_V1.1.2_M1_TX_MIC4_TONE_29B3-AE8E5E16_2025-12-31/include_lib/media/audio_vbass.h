#ifndef CONFIG_EFFECT_CORE_V2_ENABLE
#include "application/audio_vbass.h"
#else
#ifndef _AUDIO_VBASS_API_H_
#define _AUDIO_VBASS_API_H_
#include "system/includes.h"
#include "media/audio_stream.h"
#include "media/VirtualBass_api.h"

typedef struct _VirtualBassUdateParam {
    int ratio;
    int boost;
    int fc;
} VirtualBassUdateParam;


typedef struct _vbass_hdl {
    struct audio_stream_entry entry;	// 音频流入口
    struct list_head hentry;                         //
    void *workbuf;           //vbass 运行句柄及buf
    VirtualBassParam parm;
    u32 vbass_name;
    u8 status;                           //内部运行状态机
    u8 update;                           //设置参数更新标志
    /******32bit位宽输出***********/
    int bit_wide;// 0:runout16bit 1:runout32bit
    u16 dat_len;
    u16 dat_total;
    u16 buf_len;
    u16 *buf;
} vbass_hdl;


vbass_hdl *audio_vbass_open(u32 vbass_name, VirtualBassParam *parm);
int audio_vbass_close(vbass_hdl *hdl);

void audio_vbass_enable_32bit_out(vbass_hdl *hdl, int enable_32bit_out, u32 out_buf_size);
int audio_vbass_parm_update(u32 vbass_name, VirtualBassUdateParam *parm);
void audio_vbass_bypass(u32 vbass_name, u8 bypass);
vbass_hdl *get_cur_vbass_hdl_by_name(u32 vbass_name);

#ifndef RUN_NORMAL
#define RUN_NORMAL  0
#endif

#ifndef RUN_BYPASS
#define RUN_BYPASS  1
#endif


#endif
#endif /*CONFIG_EFFECT_CORE_V2_ENABLE*/


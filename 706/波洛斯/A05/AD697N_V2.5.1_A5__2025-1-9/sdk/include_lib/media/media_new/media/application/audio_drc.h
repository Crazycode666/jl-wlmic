
#ifndef _DRC_API_H_
#define _DRC_API_H_

#include "typedef.h"
#include "sw_drc.h"

struct audio_drc_filter_info {
    struct drc_ch *pch;
    struct drc_ch *R_pch;
};

#if (defined(EQ_CORE_V1)|| defined(EQ_CORE_V2))
typedef int (*audio_drc_filter_cb)(void *drc, struct audio_drc_filter_info *info);
#else
typedef int (*audio_drc_filter_cb)(struct audio_drc_filter_info *info);
#endif

struct audio_drc_param {
    u8 channels;           //通道数 (channels|(L_wdrc))或(channels|(R_wdrc))
    u8 online_en;          //是否支持在线调试
    u8 remain_en;          //写1
    u8 stero_div;          //是否左右声道 拆分的  drc效果,一般写0
#if (defined(EQ_CORE_V1)|| defined(EQ_CORE_V2))
    u8 ver;                     //0:旧版本  1:新版本，新旧版本参数结构有差异
    u8 out_32bit;               //是否支持32bit 的输入数据处理  1:使能  0：不使能
    u32 sr;                     //数据采样率
#endif
    u32 reserved;
    u32 name;
    audio_drc_filter_cb cb;     //系数更新的回调函数，用户赋值
    struct drc_ch *cfg;    //软件drc 系数地址
};

struct audio_drc {
    struct list_head hentry;                        //
    struct drc_ch sw_drc[2];    //软件drc 系数地址
    u32 sr;                    //采样率
    u8 channels;           //通道数(channels|(L_wdrc))或(channels|(R_wdrc))
    u8 remain_flag;        //输出数据支持remain
    u8 updata;             //系数更标志
    u8 online_en;          //是否支持在线更新系数
    u8 remain_en;          //是否支持remain输出
    u8 start;              //无效
    u8 run32bit;           //是否使能32bit位宽数据处理1:使能  0：不使能
    u8 need_restart;       //是否需要重更新系数 1:是  0：否
    u8 stero_div;          //是否左右声道拆分的drc效果,1：是  0：否
    u8 ver;
    audio_drc_filter_cb cb;     //系数更新回调
    void *hdl;                  //软件drc句柄
    void *output_priv;          //输出回调私有指针
    int (*output)(void *priv, void *data, u32 len);//输出回调函数
    u32 name;
    struct drc_ch *cfg;    //软件drc 系数地址
};


int audio_drc_open(struct audio_drc *drc, struct audio_drc_param *param);

void audio_drc_set_output_handle(struct audio_drc *drc, int (*output)(void *priv, void *data, u32 len), void *output_priv);

void audio_drc_set_samplerate(struct audio_drc *drc, int sr);

int audio_drc_set_32bit_mode(struct audio_drc *drc, u8 run_32bit);

int audio_drc_start(struct audio_drc *drc);
int audio_drc_run(struct audio_drc *drc, s16 *data, u32 len);
int audio_drc_close(struct audio_drc *drc);
/*----------------------------------------------------------------------------*/
/**@brief    audio_drc_open重新封装，简化使用
   @param    *parm: drc参数句柄,参数详见结构体struct audio_drc_param
   @return   eq句柄
   @note
*/
/*----------------------------------------------------------------------------*/
struct audio_drc *audio_dec_drc_open(struct audio_drc_param *parm);

void audio_drc_update_parm(struct audio_drc *drc, struct drc_ch *wdrc_parm);
/*----------------------------------------------------------------------------*/
/**@brief    audio_drc_close重新封装，简化使用
   @param    drc句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_dec_drc_close(struct audio_drc *drc);

int audio_dec_drc_run(struct audio_drc *drc, s16 *data, u32 len);

struct audio_drc *get_cur_drc_hdl_by_name(u32 name);

#if (defined(EQ_CORE_V1)|| defined(EQ_CORE_V2))
#else
void drc_app_run_check(struct audio_drc *drc);
int drc_get_filter_info(struct audio_drc_filter_info *info);
#endif

#endif


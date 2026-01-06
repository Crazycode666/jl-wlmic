
#ifndef _SOF_EQ_API_H_
#define _SOF_EQ_API_H_

#include "typedef.h"
#include "media/audio_stream.h"
#include "system/timer.h"
#include "system/init.h"
// #include "media/audio_eq.h"
#include "media/audio_sof_eq.h"
// #include "media/effects_adj.h"
#include "media/fix_iir_filter_api.h"
#include "asm/hw_eq.h"

//64位的位或处理
#define     MASK_BIT(n)              (u64)((u64)1 << (n))

struct sof_eq_seg_info {
    u16 index;
    u16 iir_type; ///<EQ_IIR_TYPE
    int freq;
    float gain;
    float q;
};

struct  sof_eq_tool { //配置项存储结构
    float global_gain;           //总增益
    int is_bypass: 8;                //0 关闭， 1：打开bypass
    int seg_num: 8;                //eq效果文件存储的段数 +2
    int reserve: 16;
    struct sof_eq_seg_info seg[0];   //eq系数存储地址,高阶高通低通滤波器追加存储在尾部
};


struct audio_sof_eq ;
struct eq_fade_parm {
    u8 fade_time;       //系数更新是否需要淡入，淡入timer循环时间建议值(10~100ms)
    u8 f_fade_step;     //滤波器中心截止频率淡入步进（10~100Hz）
    float fade_step;    //滤波器增益淡入步进（0.01f~1.0f）
    float q_fade_step;  //滤波器q值淡入步进（0.01f~1.0f）
    float g_fade_step;  //总增益淡入步进（0.01f~1.0f）
    void *priv;
    int (*callback)(struct audio_sof_eq *eq, void *priv);//淡出结束时的回调函数

};
struct audio_sof_eq_fade {
    u8 type_en;
    u8 nsection;
    u16 timer;
    u32 cur_seg_size;
    float cur_global_gain;
    float use_global_gain;
    struct sof_eq_seg_info *cur_seg;
    struct sof_eq_seg_info *use_seg;
    u8 *gain_flag;
    void *priv;
    int (*callback)(struct audio_sof_eq *eq, void *priv);

} ;

struct audio_sof_eq_param {
    void *coeff;//直接输入计算好的系数表,与seg二选1
    struct sof_eq_seg_info *seg; //系数表
    float global_gain;       //总增益
    u32 sample_rate;         //采样率，更根据当前数据实际采样率填写
    u32 name;
    u8 in_bit_width;         //输入数据位宽 0:short  1:int  2:float
    u8 out_bit_width;        //输出数据位宽 0:short  1:int  2:float
    u8 nsection;             //滤波器段数
    u8 channel;              //通道数,1：单声道  2：立体声
    u8 qval;                 //数据饱和位宽
    u8 cur_coeff_nsection;//滤波器计算后的段数（多阶滤波器段数可变，导致系数段数与滤波器个数会存在不一样的情况）
};

struct audio_sof_eq {
    void *work_buf;                          //sof eq句柄
    int *coeff;                            //系数
    struct audio_sof_eq_fade *fade;
    struct eq_fade_parm fade_parm;
    // struct audio_sof_eq_param oparam;
    struct iir_filter_param iir_param;
    struct sof_eq_seg_info *cur_seg;
    void *file;
    float global_gain;
    int sample_rate;
    struct list_head hentry;                        //
    u64 mask;
    u32 name;
    u8 channel;
    u8 cur_seg_nsection;//滤波器个数
    u8 cur_coeff_nsection;//滤波器计算后的段数（多阶滤波器段数可变，导致系数段数与滤波器个数会存在不一样的情况）
    u8 update;
};


extern int eq_db2mag(int db, int dbQ, int magDQ);
struct audio_sof_eq *audio_sof_eq_open(struct audio_sof_eq_param *param);
int audio_sof_eq_close(struct audio_sof_eq *hdl);
int audio_sof_eq_run(struct audio_sof_eq *hdl, s16 *indata, s16 *outdata, u32 len);
void audio_sof_eq_update(struct audio_sof_eq *hdl, struct eq_fade_parm fade_parm, struct sof_eq_tool *update_param);
void audio_sof_eq_hp_lp_adv_reset(struct sof_eq_seg_info *seg, u8 seg_num);

//分频器接口
int audio_sof_coeff_eq_run(struct audio_sof_eq *hdl, s16 *indata, s16 *outdata, u32 len);
void audio_sof_coeff_eq_update(struct audio_sof_eq *hdl, void *coeff);

struct audio_sof_eq *get_cur_sofeq_hdl_by_name(u32 name);
void audio_convert_data_16bit_to_32bit(s16 *indata, s32 *outdata, int npoint);
void audio_convert_data_32bit_to_16bit(s32 *in, s16 *out, u32 npoint);

#define debug_digital(x)  __builtin_abs((int)((x - (int)x) * 100)) //此处使用该宏定义，获取浮点两位小数，转int型后打印

#endif


#ifndef _EQ_API_H_
#define _EQ_API_H_

#include "typedef.h"
// #include "asm/audio_platform.h"
#include "asm/hw_eq.h"
// #include "app_config.h"

#define EQ_CHANNEL_MAX      2

#define EQ_SR_IDX_MAX       9

#if (defined(EQ_CORE_V1)|| defined(EQ_CORE_V2))
#else
#define EQ_SECTION_MAX_DEFAULT   10
#define AUDIO_SONG_EQ_NAME  0
#define AUDIO_CALL_EQ_NAME 1
#define AUDIO_AEC_EQ_NAME  3
#endif

/*eq type*/
typedef enum {
    EQ_MODE_NORMAL = 0,
    EQ_MODE_ROCK,
    EQ_MODE_POP,
    EQ_MODE_CLASSIC,
    EQ_MODE_JAZZ,
    EQ_MODE_COUNTRY,
    EQ_MODE_CUSTOM,//自定义
    EQ_MODE_MAX,
} EQ_MODE;

/*eq type*/
typedef enum {
    EQ_TYPE_FILE = 0x01,
    EQ_TYPE_ONLINE,
    EQ_TYPE_MODE_TAB,
} EQ_TYPE;

#define audio_eq_filter_info 	eq_coeff_info

#if (defined(EQ_CORE_V1)|| defined(EQ_CORE_V2))
typedef int (*audio_eq_filter_cb)(void *eq, int sr, struct audio_eq_filter_info *info);



struct audio_eq_param {
///////////////
    u32 online_en : 1;   //是否支持在线调试  1:支持 0：不支持
    u32 mode_en : 1;     //支持默认系数表写1
    u32 remain_en : 1;   //数据输出支持remain,写1
///////////////

    u8 no_wait : 1;     //是否使能异步eq, 1:使能  0：不使能
    u8 max_nsection : 6;//最大的eq段数,根据使用填写，要小于等于EQ_SECTION_MAX
    u8 nsection : 6;    //实际需要的eq段数，需小于等于max_nsection
    u8 out_32bit : 1;    //是否支持32bit eq输出，仅在 no_wait 写1时，out_32bit 才能写1
    u8 channels : 2;    //通道数
    u8 bypass;
    u8 fade;            //系数更新是否需要淡入，淡入timer循环时间建议值(10~100ms)
    u8 f_fade_step;     //滤波器中心截止频率淡入步进（10~100Hz）
    float fade_step;    //滤波器增益淡入步进（0.01f~1.0f）
    float q_fade_step;  //滤波器q值淡入步进（0.01f~1.0f）
    float g_fade_step;  //总增益淡入步进（0.01f~1.0f）
    audio_eq_filter_cb cb;//获取eq系数的回调函数
    u32 eq_name;         //eq名字，在线调试时，用于区分不同eq更新系数 播歌一般写song_eq_mode
    u32 sr;              //采样率，更根据当前数据实际采样率填写
    float global_gain;
    struct eq_seg_info *seg;//
    void *priv;          //私有指针
    int (*output)(void *priv, void *data, u32 len);//异步eq输出回调
    void (*irq_callback)(void *priv);//总段回调函数，数据流激活
};


#else
typedef int (*audio_eq_filter_cb)(int sr, struct audio_eq_filter_info *info);


/*EQ_ONLINE_CMD_PARAMETER_SEG*/
typedef struct eq_seg_info EQ_ONLINE_PARAMETER_SEG;

/*-----------------------------------------------------------*/
typedef struct eq_seg_info EQ_CFG_SEG;


struct audio_eq_param {
    u32 channels : 2;
    u32 online_en : 1;
    u32 mode_en : 1;
    u32 remain_en : 1;
    u32 no_wait : 1;
    u32 max_nsection : 6;
    u32 reserved : 20;

    u32 nsection : 6;
    u32 drc_en: 1;
    u32 eq_name;
    audio_eq_filter_cb cb;
};

#endif//EQ_CORE_V1



struct audio_eq_fade {
    u8 nsection;
    u16 timer;
    float cur_global_gain;
    float use_global_gain;
    struct eq_seg_info *cur_seg;
    struct eq_seg_info *use_seg;
    u8 *gain_flag;
} ;


#ifdef CONFIG_EQ_SUPPORT_ASYNC
struct audio_eq_async {
    u16 ptr;
    u16 len;
    u16 buf_len;
    u8 clear;
    u8 out_stu;
    char *buf;
};
#endif

struct audio_eq {
    void *eq_ch;                          //硬件eq句柄
    u8 updata;                            //系数是否需要更新
    u8 start;                             //eq start标识
    u8 max_nsection;                      //eq最大段数
    u8 cur_nsection;                      //当前eq段数
    u8 check_hw_running;                  //检测到硬件正在运行时不等待其完成1:设置检查  0：不检查
    u8 async_en;
    u8 out_32bit;
    u8 ch_num;
    u8 core;                              //多硬件支持选配，需芯片支持
    u8 coeff_free;
    u8 ext_seg_num[2];
    //u8 ext_coeff_lp_num[2];
    //u8 ext_coeff_hp_num[2];
    u32 bypass;
    u32 sr;                                    //采样率
    u32 eq_name;                               //eq标识
    u32 mask[2];
    float global_gain;
    float global_gain_r;
    s16 *eq_out_buf;//同步方式，32bit输出，当out为NULL时，内部申请eq_out_buf
    int out_buf_size;
    int eq_out_points;
    int eq_out_total;
    // struct audio_stream_entry entry;	// 音频流入口

#ifdef CONFIG_EQ_SUPPORT_ASYNC
    void *run_buf;
    void *run_out_buf;
    int run_len;
    struct audio_eq_async async;                     //异步eq 输出管理
#endif

// #if AUDIO_EQ_CLEAR_MEM_BY_MUTE_TIME_MS
    u32 mute_cnt_l;
    u32 mute_cnt_r;
    u32 mute_cnt_max;
// #endif

    audio_eq_filter_cb cb;
    void *output_priv;                               //私有指针
    int (*output)(void *priv, void *data, u32 len);  //输出回调
#if (defined(EQ_CORE_V1)|| defined(EQ_CORE_V2))
    struct eq_seg_info *eq_seg_tab;                 //运算前系数表
    struct eq_seg_info *seg;                        //运算前系数表,由audio_eq_param初始化时指定
    int *eq_coeff_tab;                              //运算后系数表
    struct list_head hentry;                        //
#endif
    const char *event_owner;                        //记录data_handler所处的任务
    struct audio_eq_fade *fade;
    float fade_step;    //滤波器增益淡入步进
    float q_fade_step;  //q值淡入步进（0.01f~1.0f）
    float g_fade_step;  //总增益淡入步进（0.01f~1.0f）
    u8 f_fade_step;     //频率淡入步进
    u8 fade_time;       //淡入timer循环时间ms
};

/*----------------------------------------------------------------------------*/
/**@brief    eq模块初始化
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_eq_init(int eq_section_num);
/*----------------------------------------------------------------------------*/
/**@brief    eq 打开
   @param    *param: eq参数句柄,参数详见结构体struct audio_eq_param
   @return   eq句柄
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_eq_open(struct audio_eq *eq, struct audio_eq_param *param);
/*----------------------------------------------------------------------------*/
/**@brief    异步eq设置输出回调
   @param    eq:句柄
   @param    *output:回调函数
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_eq_set_output_handle(struct audio_eq *eq, int (*output)(void *priv, void *data, u32 len), void *output_priv);
/*----------------------------------------------------------------------------*/
/**@brief    同步eq设置输出buf
   @param    eq:句柄
   @param    *output:回调函数
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_eq_set_output_buf(struct audio_eq *eq, s16 *buf, u32 len);
/*----------------------------------------------------------------------------*/
/**@brief    eq设置采样率
   @param    eq:句柄
   @param    sr:采样率
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_eq_set_samplerate(struct audio_eq *eq, int sr);
/*----------------------------------------------------------------------------*/
/**@brief    eq设置输入输出通道数
   @param    eq:句柄
   @param    channel:通道数
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_eq_set_channel(struct audio_eq *eq, u8 channel);

/*----------------------------------------------------------------------------*/
/**@brief    设置检查eq是否正在运行
   @param    eq:句柄
   @param    check_hw_running: 1:设置检查  0：不检查
   @return
   @note     检测到硬件正在运行时不等待其完成，直接返回, 仅异步EQ有效
*/
/*----------------------------------------------------------------------------*/
int audio_eq_set_check_running(struct audio_eq *eq, u8 check_hw_running);
/*----------------------------------------------------------------------------*/
/**@brief    设置eq信息
   @param    eq:句柄
   @param    channels:设置输入输出通道数
   @param    out_32bit:使能eq输出32bit数据，1：输出32bit数据， 0：输出16bit是数据
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_eq_set_info(struct audio_eq *eq, u8 channels, u8 out_32bit);

/*----------------------------------------------------------------------------*/
/**@brief    设置eq信息
   @param    eq:句柄
   @param    channels:设置入输出通道数
   @param    in_mode:使能eq输入32bit数据，2:float, 1：输入32bit数据， 0：输入16bit是数据
   @param    out_mode:使能eq输出32bit数据，2:float, 1：输出32bit数据， 0：输出16bit是数据
   @param    run_mode:运行模式，0：normal, 1:mono, 2:stero
   @param    data_in_mode:输入数据存放方式 0：块模式  1：序列模式
   @param    data_out_mode:输入数据存放方式 0：块模式  1：序列模式
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_eq_set_info_new(struct audio_eq *eq, u8 channels, u8 in_mode, u8 out_mode, u8 run_mode, u8 data_in_mode, u8 data_out_mode);


/*----------------------------------------------------------------------------*/
/**@brief    eq启动
   @param    eq:句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_eq_start(struct audio_eq *eq);
/*----------------------------------------------------------------------------*/
/**@brief    eq的数据处理
   @param    eq:句柄
   @param    *data:输入数据地址
   @param    len:输入数据长度
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_eq_run(struct audio_eq *eq, s16 *data, u32 len);
/*----------------------------------------------------------------------------*/
/**@brief    eq关闭
   @param    eq:句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_eq_close(struct audio_eq *eq);


#ifdef CONFIG_EQ_SUPPORT_ASYNC
/*----------------------------------------------------------------------------*/
/**@brief    清除异步eq剩余的数据
   @param    eq:句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_eq_async_data_clear(struct audio_eq *eq);
#endif
/*----------------------------------------------------------------------------*/
/**@brief    获取异步eq剩余的数据
   @param    eq:句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int audio_eq_data_len(struct audio_eq *eq);

/*----------------------------------------------------------------------------*/
/**@brief    audio_eq_open重新封装，简化使用
   @param    *parm: eq参数句柄,参数详见结构体struct audio_eq_param
   @return   eq句柄
   @note
*/
/*----------------------------------------------------------------------------*/
struct audio_eq *audio_dec_eq_open(struct audio_eq_param *parm);

/*----------------------------------------------------------------------------*/
/**@brief    audio_eq_close重新封装，简化使用
   @param    eq句柄
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_dec_eq_close(struct audio_eq *eq);

void audio_dec_eq_run(struct audio_eq *eq, void *in, void *out, int len);

void cur_eq_set_bypass(u32 eq_name, int bypass);

/*
 *设置声道总增益
 * */
void cur_eq_set_global_gain(u32 eq_name, float g_Gain);
/*
 *设置某一段eq相关系数
 *seg:eq系数
 *nsection:eq的段数
 * */
void cur_eq_set_update(u32 eq_name, struct eq_seg_info *seg, u32 nsection);


/*
 *eq系数回调
 * */
int eq_get_filter_info(void *_eq, int sr, struct audio_eq_filter_info *info);
/*
 *通过eq_name获取当前运行的eq的指针
 * */
struct audio_eq *get_cur_eq_hdl_by_name(u32 eq_name);

#endif



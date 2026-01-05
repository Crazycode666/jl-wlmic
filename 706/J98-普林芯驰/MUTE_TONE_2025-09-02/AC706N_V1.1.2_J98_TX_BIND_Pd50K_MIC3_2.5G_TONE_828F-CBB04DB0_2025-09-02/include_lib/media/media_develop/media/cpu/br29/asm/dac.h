#ifndef __CPU_DAC_H__
#define __CPU_DAC_H__


#include "generic/typedef.h"
#include "generic/atomic.h"
#include "os/os_api.h"
#include "media/audio_stream.h"
#include "media/audio_cfifo.h"


#define DAC_44_1KHZ            0
#define DAC_48KHZ              1
#define DAC_32KHZ              2
#define DAC_22_05KHZ           3
#define DAC_24KHZ              4
#define DAC_16KHZ              5
#define DAC_11_025KHZ          6
#define DAC_12KHZ              7
#define DAC_8KHZ               8

#define DAC_ISEL_FULL_PWR			4
#define DAC_ISEL_HALF_PWR			2
#define DAC_ISEL_THREE_PWR			1

#define DACVDD_LDO_2_40V        0
#define DACVDD_LDO_2_50V        1
#define DACVDD_LDO_2_60V        2
#define DACVDD_LDO_2_70V        3
#define DACVDD_LDO_2_80V        4
#define DACVDD_LDO_2_90V        5
#define DACVDD_LDO_3_00V        6
#define DACVDD_LDO_3_10V        7

/************************************************************************************************************
                                       AUDIO DAC 相关配置

1.  DAC 的输出声道数和输出方式由 TCFG_AUDIO_DAC_CONNECT_MODE 和 TCFG_AUDIO_DAC_MODE 来决定。
    下表是不同组合的支持情况，如果支持该组合配置表格中为使用的DAC输出引脚，不支持的则为 not support。

    +-------------------------------------+------------------+---------------+----------------------------+
    |                                     | DAC_MODE_SINGLE  | DAC_MODE_DIFF | DAC_MODE_VCMO (VCMO就是RL) |
    +-------------------------------------+------------------+---------------+----------------------------+
    |DAC_OUTPUT_MONO_L           (1 声道) | FL               |  FL-FR        |  FL-VCMO                   |
    |DAC_OUTPUT_LR               (2 声道) | FL   FR          |  FL-FR  RL-RR |  FL-VCMO  FR-VCMO          |
    |DAC_OUTPUT_FRONT_LR_REAR_L  (3 声道) | FL   FR   RL     |  not support  |  not support               |
    |DAC_OUTPUT_FRONT_LR_REAR_R  (3 声道) |   not support    |  not support  |  FL-VCMO  FR-VCMO  RR-VCMO |
    |DAC_OUTPUT_FRONT_LR_REAR_LR (4 声道) | FL   FR   RL  RR |  not support  |  not support               |
    +-------------------------------------+------------------+---------------+----------------------------+

2.  DAC 支持高性能模式（DAC_MODE_HIGH_PERFORMANCE）和低功耗模式（DAC_MODE_LOW_POWER ），
    用户可依据实际场景配置 DAC_PERFORMANCE_MODE 选择。

3.  TCFG_DAC_POWER_LEVEL 控制DAC供电电压，外部硬件VCM引脚有挂电容的选择 AUDIO_VCM_CAP_LEVEL1/2/3/4/5，
    VCM引脚没挂电容的选择 AUDIO_VCM_CAPLESS_LEVEL1/2/3/4，数字越大输出幅度越大，功耗越高。
************************************************************************************************************/

// TCFG_AUDIO_DAC_CONNECT_MODE
#define DAC_OUTPUT_MONO_L                  (0)   //左声道
#define DAC_OUTPUT_LR                      (2)   //立体声
#define DAC_OUTPUT_FRONT_LR_REAR_L         (7)   //三声道单端输出 前L+前R+后L (不可设置vcmo公共端)
#define DAC_OUTPUT_FRONT_LR_REAR_R         (8)   //三声道单端输出 前L+前R+后R (可设置vcmo公共端)
#define DAC_OUTPUT_FRONT_LR_REAR_LR        (9)   //四声道单端输出

#define DAC_OUTPUT_MONO_R                  (1)   // 未使用
#define DAC_OUTPUT_MONO_LR_DIFF            (3)   // 未使用
#define DAC_OUTPUT_DUAL_LR_DIFF            (6)   // 未使用


// TCFG_AUDIO_DAC_MODE
#define DAC_MODE_SINGLE                    (0)
#define DAC_MODE_DIFF                      (1)
#define DAC_MODE_VCMO                      (2)

/************************************
             dac性能模式
************************************/
// TCFG_DAC_PERFORMANCE_MODE
#define	DAC_MODE_HIGH_PERFORMANCE          (0)
#define	DAC_MODE_LOW_POWER		           (1)

/************************************
             dac 供电模式
************************************/
// TCFG_DAC_POWER_LEVEL
#define	AUDIO_VCM_CAP_LEVEL1	           (0)	// VCM-cap, VCM = 1.2v
#define	AUDIO_VCM_CAP_LEVEL2	           (1)  // VCM-cap, VCM = 1.3v
#define	AUDIO_VCM_CAP_LEVEL3	           (2)	// VCM-cap, VCM = 1.4v
#define	AUDIO_VCM_CAP_LEVEL4	           (3)	// VCM-cap, VCM = 1.5v

#define AUDIO_DAC_SYNC_IDLE             0
#define AUDIO_DAC_SYNC_PROBE            1
#define AUDIO_DAC_SYNC_START            2
#define AUDIO_DAC_SYNC_NO_DATA          3
#define AUDIO_DAC_ALIGN_NO_DATA         4
#define AUDIO_DAC_SYNC_ALIGN_COMPLETE   5
#define AUDIO_DAC_SYNC_KEEP_RATE_DONE   6

#define AUDIO_SRC_SYNC_ENABLE   1
#define SYNC_LOCATION_FLOAT      1
#if SYNC_LOCATION_FLOAT
#define PCM_PHASE_BIT           0
#else
#define PCM_PHASE_BIT           8
#endif

#define DAC_DSM_6MHz                    0
#define DAC_DSM_12MHz                   1

#define FADE_OUT_IN           1

#define DA_LEFT        0
#define DA_RIGHT       1

#define DA_SOUND_NORMAL                 0x0
#define DA_SOUND_RESET                  0x1
#define DA_SOUND_WAIT_RESUME            0x2

#define DACR32_DEFAULT		8192
#define DA_SYNC_INPUT_BITS              20
#define DA_SYNC_MAX_NUM                 (1 << DA_SYNC_INPUT_BITS)

#define AUDIO_DAC_MULTI_CHANNEL_ENABLE  1

enum {
    DAC_EVENT_START,
    DAC_EVENT_SAMPLE_BEGIN,
    DAC_EVENT_SAMPLE_STOP,
    DAC_EVENT_SAMPLE_MISS_DATA,
    DAC_EVENT_CLOSE,
};

#if ((defined CONFIG_SOUND_PLATFORM_ENABLE) && CONFIG_SOUND_PLATFORM_ENABLE)
struct audio_dac_trim;
struct audio_dac_power_data {
    int trim_poweron_time;
    int (*trim_value)(struct audio_dac_trim *trim);
    void (*trim_begin)(void);
    void (*trim_end)(struct audio_dac_trim *trim);
};

struct audio_dac_codec_data {
    u32 dsm_clk : 1;
    u32 dma_mode : 1;
};

struct audio_dac_platform_data {
    u8 output;
    u8 mode;
    u8 performance_mode;
    u8 power_level;
    u8 bit24_en;
    u8 dcc_level;
    u8 dcc_rd_sel;
    u8 fade_en;
    u8 fade_points_step;
    u8 fade_volume_step;
    struct audio_dac_power_data power;
    struct audio_dac_codec_data codec;
    u8 clock_mode;
    u8 vbg_trim_value;
    u8 vcm_cap_en;
};
#endif

struct dac_platform_data {
    u8 ldo_id;
    u8 ldo_volt;
    u8 pa_mute_port;
    u8 pa_mute_value;
    u8 output;
    u8 vcmo_en;
    u8 keep_vcmo;
    u8 lpf_isel;

    u8 sys_vol_type; //系统音量类型， 0:默认调数字音量  1:默认调模拟音量
    u8 max_ana_vol;
    u16 max_dig_vol;
    s16 *dig_vol_tab;
};


struct analog_module {
    /*模拟相关的变量*/
#if 0
    unsigned int ldo_en : 1;
    unsigned int ldo_volt : 3;
    unsigned int output : 4;
    unsigned int vcmo : 1;
    unsigned int inited : 1;
    unsigned int lpf_isel : 4;
    unsigned int keep_vcmo : 1;
    unsigned int reserved : 17;
#endif
    u8 inited;
    u8 vdd_volt;
    u16 dac_test_volt;
};
struct audio_dac_trim {
    s16 left;
    s16 right;
    s16 rear_left;
    s16 rear_right;
};


struct audio_dac_hdl {
    struct analog_module analog;
    // const
    struct dac_platform_data *pd;   /*DAC 板级配置*/
    OS_SEM sem;
    struct audio_dac_trim trim;     /*DAC trim值*/
    volatile u8 mute;
    volatile u8 state;              /*DAC运行状态*/
    u8 gain_l;
    u8 gain_r;
    u8 vol_l;
    u8 vol_r;

    u8 channel;                     /*DAC声道数*/
    u8 protect_time_dis;			/*关闭DAC延时保护*/
    u16 max_d_volume;               /*DAC数字最大音量*/
    u16 d_volume[2];                /*DAC数字音量*/
    u32 sample_rate;                /*DAC采样率*/
    u16 start_ms;                   /*起始延时(保留待删除)*/
    u16 delay_ms;                   /*最大延时(保留待删除)*/
    u16 start_points;               /*起始样点数(保留待删除*/
    u16 delay_points;               /*最大延时样点数(保留待删除)*/
    u16 unread_samples;             /*未读样点个数*/

    u16 irq_points;                 /*DAC普通中断样点个数*/

    s16 *output_buf;                /*DAC缓冲地址*/
    u16 output_buf_len;             /*DAC缓冲长度*/
    u8 protect_fadein;              /*延时保护淡入*/
    u16 irq_timeout;                /*非模块硬件的超时*/
    s16 read_offset;                /*DAC读设置偏移*/
    u16 protect_pns;                /*DAC延时保护最小样点*/
    s16 fade_vol;                   /*淡入淡出音量*/
    //struct audio_stream_entry entry;
    struct audio_cfifo fifo;        /*DAC cfifo结构管理*/
    struct list_head ch_head;       /*DAC 子设备表头*/
    OS_MUTEX ch_mutex;              /*DAC 子设备锁*/
};

struct audio_dac_channel_attr {
    u8  write_mode;         /*DAC写入模式*/
    u16 delay_time;         /*DAC通道延时*/
    u16 protect_time;       /*DAC延时保护时间*/
};

struct audio_dac_channel {
    u8  state;              /*DAC状态*/
    u8  pause;
    u8  samp_sync_step;     /*数据流驱动的采样同步步骤*/
    u8  wait_resume;        /*数据流激活wait设置*/
    struct audio_dac_channel_attr attr;     /*DAC通道属性*/
    struct audio_sample_sync *samp_sync;    /*样点同步句柄*/
    struct audio_dac_hdl *dac;              /*DAC设备*/
    struct audio_cfifo_channel fifo;        /*DAC cfifo通道管理*/
    struct list_head ch_entry;              /*DAC设备entry*/
    struct audio_stream_entry entry;        /*DAC数据流节点*/
};

/*************************************************************************
 * 创建一个新的DAC
 * INPUT    :  dac - DAC设备句柄, ch - 新的DAC通道
 * OUTPUT   :  0 - 创建成功，非0 - 创建失败.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_dac_new_channel(struct audio_dac_hdl *dac, struct audio_dac_channel *ch);

/*************************************************************************
 * 释放一个DAC
 * INPUT    :  ch - DAC通道
 * OUTPUT   :  无.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
void audio_dac_free_channel(struct audio_dac_channel *ch);

/*************************************************************************
 * 获取DAC的属性配置
 * INPUT    :  ch - DAC通道，attr - DAC通道属性
 * OUTPUT   :  0 - 成功, 非0 - 出错.
 * WARNINGS :  用于设置属性前的初始化配置.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_dac_channel_get_attr(struct audio_dac_channel *ch, struct audio_dac_channel_attr *attr);

/*************************************************************************
 * 设置DAC的属性配置
 * INPUT    :  ch - DAC通道，attr - DAC通道属性
 * OUTPUT   :  0 - 成功, 非0 - 出错.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_dac_channel_set_attr(struct audio_dac_channel *ch, struct audio_dac_channel_attr *attr);

/*************************************************************************
 * 暂停DAC的数据流输出
 * INPUT    :  ch - DAC通道，pause - 暂停/非暂停
 * OUTPUT   :  0 - 成功, 非0 - 出错.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Zhuodengliang.
 *=======================================================================*/
int audio_dac_channel_set_pause(struct audio_dac_channel *ch, u8 pause);

/*************************************************************************
 * DAC播放同步使能
 * INPUT    :  ch - DAC通道，samp_sync - 采样同步句柄
 * OUTPUT   :  0 - 成功, 非0 - 出错.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_dac_channel_sync_enable(struct audio_dac_channel *ch, struct audio_sample_sync *samp_sync);

/*************************************************************************
 * DAC播放同步使能关闭
 * INPUT    :  ch - DAC通道，samp_sync - 采样同步句柄
 * OUTPUT   :  0 - 成功, 非0 - 出错.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_dac_channel_sync_disable(struct audio_dac_channel *ch, struct audio_sample_sync *samp_sync);

/*************************************************************************
 * DAC驱动模块初始化
 * INPUT    :  dac - DAC设备句柄，pd - DAC配置数据
 * OUTPUT   :  0 - 成功, 非0 - 出错.
 * WARNINGS :  1、初始化必须放在DAC设备使用之前；2、仅初始化一次不可重复操作.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_dac_init(struct audio_dac_hdl *dac, const struct dac_platform_data *pd);

/*************************************************************************
 * DAC省电容DTB设置
 * INPUT    :  dac - DAC设备句柄，dacr32 - DTB值
 * OUTPUT   :  无.
 * WARNINGS :  无需理解，由省电容开发人员设置.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
void audio_dac_set_capless_DTB(struct audio_dac_hdl *dac, s16 dacr32);

/*************************************************************************
 * DAC模块输出偏置电压校准
 * INPUT    :  dac - DAC设备句柄，dac_trim - DAC trim校准值参数,
 *             fast_trim - 加速模式，部分芯片不支持加速, 不建议使用，
 *                         详情请咨询开发人员.
 * OUTPUT   :  0 - 成功, 非0 - 出错.
 * WARNINGS :  1、如非必要，trim在第一次上电/或者保存值丢失情况下调用；
 *             2、该函数有时间开销，trim值应保留在存储单元重复使用，可避免上电trim.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_dac_do_trim(struct audio_dac_hdl *dac, struct audio_dac_trim *dac_trim, u8 fast_trim);

/*************************************************************************
 * DAC模块trim值设置
 * INPUT    :  dac - DAC设备句柄，dac_trim - DAC trim校准值参数
 * OUTPUT   :  0 - 成功, 非0 - 出错.
 * WARNINGS :  用于trim后或者读取存储值后设置给DAC模块，不可随意配置.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_dac_set_trim_value(struct audio_dac_hdl *dac, struct audio_dac_trim *dac_trim);

int audio_dac_set_delay_time(struct audio_dac_hdl *dac, int start_ms, int max_ms);

/*************************************************************************
 * DAC中断处理函数
 * INPUT    :  dac - DAC设备句柄
 * OUTPUT   :  无.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
void audio_dac_irq_handler(struct audio_dac_hdl *dac);

/*************************************************************************
 * DAC缓冲区内存设置
 * INPUT    :  dac - DAC设备句柄，buf - 缓冲区地址, len - 缓冲区长度(byte)
 * OUTPUT   :  0 - 成功, 非0 - 出错.
 * WARNINGS :  接口设计为DAC初始化的配置，不建议任意位置使用.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_dac_set_buff(struct audio_dac_hdl *dac, s16 *buf, int len);

int audio_dac_write(struct audio_dac_hdl *dac, void *buf, int len);

s16 *audio_dac_get_write_ptr(struct audio_dac_hdl *dac, int *len);

int audio_dac_update_write_ptr(struct audio_dac_hdl *dac, int len);

/*************************************************************************
 * DAC驱动模块初始化
 * INPUT    :  dac - DAC设备句柄，pd - DAC配置数据
 * OUTPUT   :  0 - 成功, 非0 - 出错.
 * WARNINGS :  1、初始化必须放在DAC设备使用之前；2、仅初始化一次不可重复操作.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_dac_set_sample_rate(struct audio_dac_hdl *dac, int sample_rate);

int audio_dac_get_sample_rate(struct audio_dac_hdl *dac);

int audio_dac_get_channel(struct audio_dac_hdl *dac);

int audio_dac_set_channel(struct audio_dac_hdl *dac, u8 channel);

int audio_dac_get_pd_output(struct audio_dac_hdl *dac);

int audio_dac_set_pd_output(struct audio_dac_hdl *dac, u8 output);

int audio_dac_set_digital_vol(struct audio_dac_hdl *dac, u16 vol);

int audio_dac_set_analog_vol(struct audio_dac_hdl *dac, u16 vol);
int audio_dac_mix_ch_get_datasize(struct audio_dac_hdl *dac);
int audio_dac_start(struct audio_dac_hdl *dac);

int audio_dac_stop(struct audio_dac_hdl *dac);

int audio_dac_idle(struct audio_dac_hdl *dac);

void audio_dac_mute(struct audio_dac_hdl *hdl, u8 mute);

int audio_dac_open(struct audio_dac_hdl *dac);

int audio_dac_close(struct audio_dac_hdl *dac);

int audio_dac_mute_left(struct audio_dac_hdl *dac);

int audio_dac_mute_right(struct audio_dac_hdl *dac);

int audio_dac_set_volume(struct audio_dac_hdl *dac, u8 gain);

int audio_dac_set_L_digital_vol(struct audio_dac_hdl *dac, u16 vol);

int audio_dac_set_R_digital_vol(struct audio_dac_hdl *dac, u16 vol);

void audio_dac_set_fade_handler(struct audio_dac_hdl *dac, void *priv, void (*fade_handler)(u8, u8, u8, u8));

int audio_dac_power_off(struct audio_dac_hdl *dac);

/*************************************************************************
 * DAC采样率匹配
 * INPUT    :  dac - DAC设备句柄，sample_rate - 采样率
 * OUTPUT   :  true - 匹配到, false - 匹配不到.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
bool audio_dac_sample_rate_match(struct audio_dac_hdl *dac, u32 sample_rate);

/*************************************************************************
 * DAC采样率选择
 * INPUT    :  dac - DAC设备句柄，sample_rate - 预期采样率，
 *             hign - 如果DAC不支持预期的采样率选择临近一档高于预期的采样率
 * OUTPUT   :  最终选择到的采样率.
 * WARNINGS :  无.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_dac_sample_rate_select(struct audio_dac_hdl *dac, u32 sample_rate, u8 high);

int audio_dac_sound_reset(struct audio_dac_hdl *dac, u32 msecs);

u8 audio_dac_is_working(struct audio_dac_hdl *dac);

void audio_dac_set_irq_time(struct audio_dac_hdl *dac, int time_ms);

int audio_dac_data_time(struct audio_dac_hdl *dac);

int audio_dac_get_status(struct audio_dac_hdl *dac);

int audio_dac_get_max_channel(void);

int audio_dac_ch_analog_gain_set(struct audio_dac_hdl *dac, u8 ch, u8 again);

int audio_dac_ch_analog_gain_get(struct audio_dac_hdl *dac, u8 ch);

int audio_dac_ch_digital_gain_set(struct audio_dac_hdl *dac, u8 ch, u32 dgain);

int audio_dac_ch_digital_gain_get(struct audio_dac_hdl *dac, u8 ch);

void audio_dac_ch_mute(struct audio_dac_hdl *dac, u8 ch, u8 mute);

void audio_dac_zero_detect_onoff(struct audio_dac_hdl *dac, u8 onoff);

int audio_dac_set_RL_digital_vol(struct audio_dac_hdl *dac, u16 vol);
int audio_dac_set_RR_digital_vol(struct audio_dac_hdl *dac, u16 vol);

/*************************************************************************;
 * DAC采样率选择
 *
 * INPUT    :  dac - DAC设备句柄, volt - DACVDD 电压值(宏定义)
 *
 * OUTPUT   :  无.
 *
 * WARNINGS :  根据struct dac_platform_data的vdd_setting_enable变量控制，
 *             该开关为0，则函数无法真正设置电压到dac电源.
 *
 * HISTORY  :  2021/2/18 by Lichao.
 *=======================================================================*/
void audio_dac_set_vdd_volt(struct audio_dac_hdl *dac, u8 volt);

/*************************************************************************
 * DAC模块电源关闭
 * INPUT    :  无.
 * OUTPUT   :  无.
 * WARNINGS :  若DAC电源关闭后，低功耗或其他应用不应再设置DAC电源等.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
void dac_power_off(void);

/*关闭audio相关模块使能*/
void audio_disable_all(void);

/*******************************************************************
 *          Audio DACVDD上电接口
 * params   : struct audio_dac_hdl结构指针
 *
 * return   : 无
 *
 * warnings : 仅用作Audio初始化上电，不可在任意位置使用
 *
 * example  : audio_dac_vdd_power_on(&dac_hdl);
 *=================================================================*/
void audio_dac_vdd_power_on(struct audio_dac_hdl *dac);
/*******************************************************************
 *          Audio DACVDD断电接口
 * params   : struct audio_dac_hdl结构指针
 *
 * return   : 无
 *
 * warnings : 仅用作Audio DACVDD断电，不可在任意位置使用
 *
 * example  : audio_dac_vdd_power_off(&dac_hdl);
 *=================================================================*/
void audio_dac_vdd_power_off(struct audio_dac_hdl *dac);

int audio_dac_fifo_set_read(struct audio_dac_hdl *dac, int offset);

/*************************************************************************
 * INPUT    :  dac - DAC设备句柄，data - 地址，len - 长度，
 *             channel - 声道(DA_LEFT/DA_RIGHT/DA_ALL_CH).
 * OUTPUT   :  读取长度.
 * WARNINGS :  DAC未开启读不到任何数据.
 * HISTORY  :  2020/12/28 by Lichao.
 *=======================================================================*/
int audio_dac_fifo_read(struct audio_dac_hdl *dac, void *data, int len, u8 channel);

/*
*********************************************************************
*                  Audio Dac Reset Buf
* Description: 重新设置dacbuf
* Arguments  : *dac			dac句柄
*              *buf			缓存地址
*              *len			缓存长度
*              *protect_time_en		DAC延时保护使能
* Return	 : None.
* Note(s)    : 该函数会先停止dac，所以建议在不使用dac或者静音的时候调用
*********************************************************************
*/
void audio_dac_reset_buf(struct audio_dac_hdl *dac, s16 *buf, int len, u8 protect_time_en);

#endif



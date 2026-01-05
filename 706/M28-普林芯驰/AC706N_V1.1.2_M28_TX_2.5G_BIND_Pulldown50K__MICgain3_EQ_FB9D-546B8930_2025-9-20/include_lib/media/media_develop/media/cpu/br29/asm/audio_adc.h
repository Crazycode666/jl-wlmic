#ifndef AUDIO_ADC_H
#define AUDIO_ADC_H

#include "generic/typedef.h"
#include "generic/list.h"
#include "generic/atomic.h"

/*无电容电路*/
#define SUPPORT_MIC_CAPLESS     1

#define LADC_STATE_INIT			1
#define LADC_STATE_OPEN      	2
#define LADC_STATE_START     	3
#define LADC_STATE_STOP      	4

#define FPGA_BOARD          	0

#define LADC_MIC                0
#define LADC_LINEIN             1

/*LINEIN通道定义*/
// #define AUDIO_LIN0L_CH			BIT(0)// PB6
// #define AUDIO_LIN0R_CH			BIT(1)// PA3
// #define AUDIO_LIN1L_CH			BIT(2)// PA4
// #define AUDIO_LIN1R_CH			BIT(3)// PB3
// #define AUDIO_LIN_DACL_CH		BIT(6) //不支持
// #define AUDIO_LIN_DACR_CH		BIT(7)//不支持
// #define AUDIO_LIN0_LR			            (AUDIO_LIN0L_CH | AUDIO_LIN0R_CH)
// #define AUDIO_LIN1_LR			            (AUDIO_LIN1L_CH | AUDIO_LIN1R_CH)
// #define AUDIO_LIN2_LR			            (AUDIO_LIN0L_CH | AUDIO_LIN1R_CH)

/* 通道选择 */
#define AUDIO_ADC_MIC_0					    BIT(0) //PB6
#define AUDIO_ADC_MIC_1					    BIT(1) //PA3
#define AUDIO_ADC_MIC_2					    BIT(2) //PA4
#define AUDIO_ADC_MIC_3					    BIT(3) //PB3
#define AUDIO_ADC_MIC_4					    BIT(4) //PA1
#define PLNK_MIC		            		BIT(5)
#define ALNK_MIC				            BIT(6)

#define AUDIO_ADC_LINE0 					BIT(0) //PB6
#define AUDIO_ADC_LINE1  					BIT(1) //PA3
#define AUDIO_ADC_LINE2  					BIT(2) //PA4
#define AUDIO_ADC_LINE3  					BIT(3) //PB3
#define AUDIO_ADC_LINE4  					BIT(4) //PA1
#define AUDIO_LIN_DACL_CH		            BIT(5)
#define AUDIO_LIN_DACR_CH		            BIT(6)

#define AUDIO_ADC_MIC_CH		            AUDIO_ADC_MIC_4
#define AUDIO_ADC_LINE_CH		            AUDIO_ADC_LINE0

/*mic_mode 工作模式定义*/
#define AUDIO_MIC_CAP_MODE                  0   //单端隔直电容模式
#define AUDIO_MIC_CAP_DIFF_MODE             1   //差分隔直电容模式
#define AUDIO_MIC_CAPLESS_MODE              2   //单端省电容模式

struct ladc_port {
    u8 channel;
};

struct adc_platform_data {
    u8 mic_mode;
    u8 mic_channel;
    u8 ladc_num;
    u8 mic_ldo_pwr;
    /*
     *MIC0内部上拉电阻档位
     *1(0.5K) ~ 9(4.5K), step = 0.5k
     *10(5K) ~ 15(10K), step = 1k
     */
    u8 mic_bias_res;
    u32 adc_dcc;
    u32 mic_capless : 1;  		//MIC0免电容方案
    u32 mic_diff : 1;  			//MIC0差分模式方案
    u32 mic_ldo_isel: 2; 		//MIC0通道电流档位选择
    u32 mic_ldo_vsel : 3;		//MIC0_LDO电压档位选择
    u32 mic_bias_inside : 1;	//MIC0电容隔直模式使用内部mic偏置(PA0)
    u32 mic_bias_keep : 1;		//保持内部MIC0偏置输出
    u32 mic_in_sel : 1;			//MICIN选择[0:MIC0 1:MICEXT(ldo5v)]
    u32 mic_ldo_state : 1;      //当前micldo是否打开
    u32 reserved : 10;
    const struct ladc_port *ladc;
};

struct capless_low_pass {
    u16 bud; //快调边界
    u16 count;
    u16 pass_num;
    u16 tbidx;
    u32 bud_factor;
};

struct adc_linein_ch {
    u8 gain;
    u8 buf_num;
    u8 ch;
    u16 buf_size;
    u16 sample_rate;
    s16 *bufs;
    struct audio_adc_hdl *hdl;
    void (*handler)(struct audio_adc_ch *, s16 *, u16);

};

struct audio_adc_output_hdl {
    struct list_head entry;
    void *priv;
    void (*handler)(void *, s16 *, int);
};

struct capless_trim {
    u8 bias_rsel;
};

struct audio_adc_hdl {
    u8 state;
    u8 channel;
    struct list_head head;
    struct adc_platform_data *pd;
    //atomic_t ref;
    u8 digital_en;
};

struct adc_mic_ch {
    u8 gain;
    u8 buf_num;
    u8 ch;
    u16 buf_size;
    u16 sample_rate;
    s16 *bufs;
    struct audio_adc_hdl *hdl;
    void (*handler)(struct audio_adc_ch *, s16 *, u16);
};

struct audio_adc_ch {
    u8 gain;
    u8 buf_num;
    u8 ch;
    u16 buf_size;
    u16 sample_rate;
    s16 *bufs;
    struct audio_adc_hdl *hdl;
    void (*handler)(struct audio_adc_ch *, s16 *, u16);
};


/*
*********************************************************************
*                  Audio ADC Initialize
* Description: 初始化Audio_ADC模块的相关数据结构
* Arguments  : adc	ADC模块操作句柄
*			   pd	ADC模块硬件相关配置参数
* Note(s)    : None.
*********************************************************************
*/
void audio_adc_init(struct audio_adc_hdl *, const struct adc_platform_data *);

/*
*********************************************************************
*                  Audio ADC Output Callback
* Description: 注册adc采样输出回调函数
* Arguments  : adc		adc模块操作句柄
*			   output  	采样输出回调
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_adc_add_output_handler(struct audio_adc_hdl *, struct audio_adc_output_hdl *);

/*
*********************************************************************
*                  Audio ADC Output Callback
* Description: 删除adc采样输出回调函数
* Arguments  : adc		adc模块操作句柄
*			   output  	采样输出回调
* Return	 : None.
* Note(s)    : 采样通道关闭的时候，对应的回调也要同步删除，防止内存释
*              放出现非法访问情况
*********************************************************************
*/
void audio_adc_del_output_handler(struct audio_adc_hdl *, struct audio_adc_output_hdl *);

/*
*********************************************************************
*                  Audio ADC IRQ Handler
* Description: Audio ADC中断回调函数
* Arguments  : adc  adc模块操作句柄
* Return	 : None.
* Note(s)    : 仅供Audio_ADC中断使用
*********************************************************************
*/
void audio_adc_irq_handler(struct audio_adc_hdl *adc);

/*
*********************************************************************
*                  Audio ADC Mic Open
* Description: 打开mic采样通道
* Arguments  : mic	mic操作句柄
*			   ch	mic通道索引
*			   adc  adc模块操作句柄
* Return	 : 0 成功	其他 失败
* Note(s)    : None.
*********************************************************************
*/
int audio_adc_mic_open(struct adc_mic_ch *mic, int ch, struct audio_adc_hdl *adc);

/*
*********************************************************************
*                  Audio ADC Mic Sample Rate
* Description: 设置mic采样率
* Arguments  : mic			mic操作句柄
*			   sample_rate	采样率
* Return	 : 0 成功	其他 失败
* Note(s)    : None.
*********************************************************************
*/
int audio_adc_mic_set_sample_rate(struct adc_mic_ch *mic, int sample_rate);

/*
*********************************************************************
*                  Audio ADC Mic Gain
* Description: 设置mic增益
* Arguments  : mic	mic操作句柄
*			   gain	mic增益
* Return	 : 0 成功	其他 失败
* Note(s)    : MIC增益范围：
*              单路输入/差分输入(0~4): 0(0dB), 1(3dB), 2(9dB), 3(15dB), 4(21dB);
*              大于等于两路输入(0~2):      0(-6dB), 1(-3dB), 2(3dB);
*********************************************************************
*/
int audio_adc_mic_set_gain(struct adc_mic_ch *mic, int gain);

/*
*********************************************************************
*                  Audio ADC Mic Buffer
* Description: 设置采样buf和采样长度
* Arguments  : mic		mic操作句柄
*			   bufs		采样buf地址
*			   buf_size	采样buf长度，即一次采样中断数据长度
*			   buf_num 	采样buf的数量
* Return	 : 0 成功	其他 失败
* Note(s)    : (1)需要的总buf大小 = buf_size * ch_num * buf_num
* 		       (2)buf_num = 2表示，第一次数据放在buf0，第二次数据放在
*			   buf1,第三次数据放在buf0，依此类推。如果buf_num = 0则表
*              示，每次数据都是放在buf0
*********************************************************************
*/
int audio_adc_mic_set_buffs(struct adc_mic_ch *mic, s16 *bufs, u16 buf_size, u8 buf_num);

/*
*********************************************************************
*                  Audio ADC Mic Start
* Description: 启动audio_adc采样
* Arguments  : mic	mic操作句柄
* Return	 : 0 成功	其他 失败
* Note(s)    : None.
*********************************************************************
*/
int audio_adc_mic_start(struct adc_mic_ch *mic);

/*
*********************************************************************
*                  Audio ADC Mic Digital Enable
* Description: audio_adc数字模块使能
* Arguments  : mic	mic操作句柄
* Return	 : 0 成功	其他 失败
* Note(s)    : None.
*********************************************************************
*/
int audio_adc_mic_digital_enable(struct adc_mic_ch *ch, u8 enable);
/*
*********************************************************************
*                  Audio ADC Mic Close
* Description: 关闭mic采样
* Arguments  : mic	mic操作句柄
* Return	 : 0 成功	其他 失败
* Note(s)    : None.
*********************************************************************
*/
int audio_adc_mic_close(struct adc_mic_ch *mic);

/*
 *en：控制micldo开关
 *pd->mic_bias_keep控制对应对应偏置io输出（micldo经过分压电阻出来的）
 */
int audio_mic_ldo_en(u8 en, struct adc_platform_data *pd);

/*
*********************************************************************
*                  Audio MIC Mute
* Description: mic静音使能控制
* Arguments  : mute 静音使能
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_set_mic_mute(bool mute);


int audio_adc_linein_open(struct audio_adc_ch *adc, int ch, struct audio_adc_hdl *hdl);
int audio_adc_linein_set_sample_rate(struct audio_adc_ch *ch, int sample_rate);
int audio_adc_linein_set_gain(struct audio_adc_ch *ch, int gain);
int audio_adc_linein_set_buffs(struct audio_adc_ch *ch, s16 *bufs, u16 buf_size, u8 buf_num);
int audio_adc_linein_start(struct audio_adc_ch *ch);
int audio_adc_linein_close(struct audio_adc_ch *ch);

int audio_adc_start(struct audio_adc_ch *linein_ch, struct adc_mic_ch *mic_ch);
int audio_adc_close(struct audio_adc_ch *linein_ch, struct adc_mic_ch *mic_ch);

int audio_mic_bias_adjust(struct capless_trim *trim_info);
#endif/*AUDIO_ADC_H*/


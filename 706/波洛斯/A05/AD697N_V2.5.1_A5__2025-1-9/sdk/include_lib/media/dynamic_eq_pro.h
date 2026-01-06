#ifndef __DYNAMIC_EQ_PRO_H__
#define __DYNAMIC_EQ_PRO_H__

#include "system/includes.h"
#include "media/DynamicEQPro_api.h"
#include "media/drc_api.h"
#include "os/os_api.h"
// #include "effects/convert_data.h"

#define RUN_NORMAL  0
#define RUN_BYPASS  1

struct dynamic_eq_pro_tool_update {
    int bypass;
    int fc;
    float Q;
    float gain;
    u16 attackTime;
    u16 releaseTime;
    float low_thr;//小于等于low_thr,输出幅度增加gain(dB),
    float high_thr;//大于等于high_thr,输出幅度增加0(dB), 等于low_thr且小于high_thr,输出幅度按照实际斜率控制
    int type;
    float rmsTime;
    float OutputGain;
};


//动态EQ DynamicEQPro:
typedef struct _dynamic_eq_pro_tool_set {
    int is_bypass;          // 1-> byass 0 -> no bypass
//probe_eq and pcm_delay
    u16 DelayMs;            //命名 Delay Ms      0~25,默认5
    u16 DetFrequency;       //命名 Det Frequency 20~20000 Hz, 默认200
    u16 DetType;            //命名 Det Type      0:LowPass  1:HighPass  2:AllPass 默认:LowPass。界面采用下拉：LowPass/HighPass/AllPass
    u16 Order;              //命名 Order         1~6,默认3, 界面采用下拉 1：6dB/Oct、2：12dB/Oct、3：18dB/Oct、4：24dB/Oct、5：30dB/Oct、6：36dB/Oct

    int nSection;					//段数
    struct dynamic_eq_pro_tool_update  effectParam[2];
} dynamic_eq_pro_param_tool_set ; //实际发送这个结构体


struct dynamic_eq_pro {
    struct list_head hentry;                        //
    // struct audio_stream_entry entry;	//音频流入口
    void *workbuf;                      //算法运行buf
    DynamicEQProEffectParam *effectParam;
    DynamicEQProParam parm;                //算法相关配置参数
    OS_MUTEX mutex;
    u32 name;
    u8 status;                          //内部运行状态机
    u8 update;                          //设置参数更新标志
};
/*
*********************************************************************
*            dynamic_eq_pro_open
* Description: 动态eq打开
* Arguments  :*parm 检测模块相关参数、若有2段，则parm[0], parm[1],参数连续存方
*             nesection:动态eq检测模块支持的段数
*             channel:输入数据通道数
*             sample_rate:输入数据采样率
* Return	 : 模块句柄.
* Note(s)    : None.
*********************************************************************
*/
struct dynamic_eq_pro *dynamic_eq_pro_open(DynamicEQProEffectParam *effectParam, DynamicEQProParam *parm, u32 name);


/*
*********************************************************************
*            dynamic_eq_pro_run
* Description: 动态eq模块数据处理
* Arguments  :*hdl:模块句柄
* 			  ext_det_data:外部检测数据源，配NULL时，使用输入数据
*             data:输入数据地址，32bit位宽
*             len:输入数据长度，byte
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
int dynamic_eq_pro_run(struct dynamic_eq_pro *hdl, int *ext_det_data, int *data, int len);

/*
*********************************************************************
*            dynamic_eq_pro_bypass
* Description: 动态eq模块设置直通、正常处理
* Arguments  :*hdl:模块句柄
*             bypass:设置直通(RUN_BYPASS)、正常处理(RUN_NORMAL)
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void dynamic_eq_pro_bypass(struct dynamic_eq_pro *hdl, u8 bypass);

/*
*********************************************************************
*            dynamic_eq_bypass
* Description: 动态eq模块设置直通、正常处理
* Arguments  :*hdl:模块句柄
*             bypass:设置直通(RUN_BYPASS)、正常处理(RUN_NORMAL)
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void dynamic_eq_pro_update(struct dynamic_eq_pro *hdl, dynamic_eq_pro_param_tool_set *dparam);


/*
*********************************************************************
*            dynamic_eq_close
* Description: 动态eq检块关闭
* Arguments  :*hdl:模块句柄
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void dynamic_eq_pro_close(struct dynamic_eq_pro *hdl);


struct dynamic_eq_pro *get_cur_dy_eq_pro_hdl_by_name(u32 name);



#endif/*__DYNAMIC_EQ_H__*/

#ifndef  __LP_TOUCH_KEY_H__
#define  __LP_TOUCH_KEY_H__

#include "typedef.h"


#define LP_CTMU_CHANNEL_SIZE                        2

#define CTMU_SHORT_CLICK_DELAY_TIME 	            400 	//单击事件后等待下一次单击时间(ms)
#define CTMU_LONG_CLICK_DELAY_TIME 		            1000 	//产生长按事件的触摸时间(ms)
#define CTMU_HOLD_CLICK_DELAY_TIME 		            200 	//long事件产生后, 发hold事件间隔(ms)

//长按开机时间:
#define CFG_M2P_CTMU_SOFTOFF_LONG_TIME 			    5 	//(n + 6) * 100(ms)

//======== CH0是否发送单击事件
#define CFG_EAROUT_NOTIFY_CH0_ONE_CLICK_EVNET 		1  //CH0是否发送单击事件

//======== 在耳外选择发送触摸通道类型
#define CFG_EAROUT_NOTIFY_CH0_ALL_EVENT				0  //在耳外发CH0所有消息
#define CFG_EAROUT_NO_NOTIFY_CH0_ALL_EVENT 			1  //在耳外不发CH0所有消息
#define CFG_EAROUT_NO_NOTIFY_CH0_CLICK_EVENT 		2  //在耳外只发CH0 LONG, HOLD, UP事件

#define CFG_EAROUT_NOTIFY_CH0_EVENT_SEL 			CFG_EAROUT_NOTIFY_CH0_ALL_EVENT

//触摸按键长按复位时间配置
#define CTMU_RESET_TIME_CONFIG			            8000	//长按复位时间(ms), 配置为0关闭

//=======================================================================//
//                             LPCTMU ANA0                               //
//=======================================================================//
//------------------- 上限电压配置: LPCTM_ANA0[7:6]
/*
上限电压表:
	0: 0.65V
	1: 0.70V
	2: 0.75V
	3: 0.80V
*/
enum LPCTM_VH_TABLE {
    LPCTMU_VH_065V = (0 << 6),
    LPCTMU_VH_070V = (1 << 6),
    LPCTMU_VH_075V = (2 << 6),
    LPCTMU_VH_080V = (3 << 6),
};

//------------------- 下限电压配置: LPCTM_ANA0[5:4]
/*
下限电压表:
	0: 0.20V
	1: 0.25V
	2: 0.30V
	3: 0.35V
*/
enum LPCTM_VL_TABLE {
    LPCTMU_VL_020V = (0 << 4),
    LPCTMU_VL_025V = (1 << 4),
    LPCTMU_VL_030V = (2 << 4),
    LPCTMU_VL_035V = (3 << 4),
};

//------------------- 充放电电流配置: LPCTM_ANA0[3:1]
/*
充放电电流表:
	0: 0.8  uA
	1: 2.4  uA
	2: 4.0 uA
	3: 5.6 uA
	4: 7.2 uA
	5: 8.8 uA
	6: 10.4 uA
	7: 12.0 uA
*/
enum LPCTM_ISEL_TABLE {
    LPCTMU_ISEL_008UA = (0 << 1),
    LPCTMU_ISEL_024UA = (1 << 1),
    LPCTMU_ISEL_040UA = (2 << 1),
    LPCTMU_ISEL_056UA = (3 << 1),
    LPCTMU_ISEL_072UA = (4 << 1),
    LPCTMU_ISEL_088UA = (5 << 1),
    LPCTMU_ISEL_104UA = (6 << 1),
    LPCTMU_ISEL_120UA = (7 << 1)
};

u8 get_lpctmu_ana_level(void);


struct ctmu_ch_cfg {
    u8 enable;
    u8 key_value;
    u8 port;
    u8 sensitivity;
};

struct lp_touch_key_platform_data {
    u8 slide_mode_en;
    u8 slide_mode_key_value;
    struct ctmu_ch_cfg ch[LP_CTMU_CHANNEL_SIZE];
};


typedef struct __LP_TOUCH_KEY_CONFIG {
    u8 cfg_en; //配置项有效标志0: 配置项无效, 使用软件默认配置, 1: 配置项有效, 使用配置工具配置值
    u8 touch_key_sensity_class;
    u8 earin_key_sensity_class;
} _GNU_PACKED_ LP_TOUCH_KEY_CONFIG;


enum {
    TOUCH_KEY_EVENT_SLIDE_UP,
    TOUCH_KEY_EVENT_SLIDE_DOWN,
    TOUCH_KEY_EVENT_SLIDE_LEFT,
    TOUCH_KEY_EVENT_SLIDE_RIGHT,
    TOUCH_KEY_EVENT_MAX,
};

/* =========== ctmu API ============= */
//ctmu 初始化
void lp_touch_key_init(const struct lp_touch_key_platform_data *config);
u8 lp_touch_key_power_on_status();

void lp_touch_key_disable(void); //关闭模块
void lp_touch_key_enable(void);  //打开模块


#endif  /*LP_TOUCH_KEY_H*/


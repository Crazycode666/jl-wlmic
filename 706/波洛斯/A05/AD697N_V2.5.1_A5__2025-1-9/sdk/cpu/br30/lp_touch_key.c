#include "includes.h"
#include "asm/power/p11.h"
#include "asm/power/p33.h"
#include "asm/lp_touch_key.h"
#include "asm/lp_touch_key_alog.h"
#include "device/key_driver.h"
#include "lp_touch_key_epd.h"
/* #include "bt_tws.h" */
#include "classic/tws_api.h"
#include "key_event_deal.h"
#include "btstack/avctp_user.h"
#include "app_config.h"

#define LOG_TAG_CONST       LP_KEY
#define LOG_TAG             "[LP_KEY]"
/* #define LOG_ERROR_ENABLE */
/* #define LOG_DEBUG_ENABLE */
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

//需要开debug的情况
//1.内置触摸调试工具
//2.入耳检测
#define CTMU_CH0_MODULE_DEBUG 	1
#define CTMU_CH1_MODULE_DEBUG 	1

#define CTMU_CHECK_LONG_CLICK_BY_RES    1

#define CTMU_TESTBOX_TEST_CH    0

#define TWS_FUNC_ID_VOL_LP_KEY      TWS_FUNC_ID('L', 'K', 'E', 'Y')
#define TWS_BT_SEND_CH0_RES_DATA_ENABLE 	0
#define TWS_BT_SEND_CH1_RES_DATA_ENABLE 	0
#define TWS_BT_SEND_EVENT_ENABLE 			0

#define CTMU_CH0_PORT 	IO_PORTB_01
#define CTMU_CH1_PORT 	IO_PORTB_02

#define CTMU_INIT_CH0_ENABLE 		BIT(0)
#define CTMU_INIT_CH1_ENABLE 		BIT(1)

#define CTMU_INIT_CH0_DEBUG 		BIT(2)
#define CTMU_INIT_CH1_DEBUG 		BIT(3)

#define CTMU_INIT_FALL_ENABLE 		BIT(4)
#define CTMU_INIT_RAISE_ENABLE 		BIT(5)
/* #define CTMU_INIT_SHORT_ENABLE 		BIT(6) */

#define CTMU_INIT_EARTCH_ENABLE     BIT(6)
#define CTMU_INIT_PORT_INV 		    BIT(7)


enum CTMU_P2M_EVENT {
    CTMU_P2M_CH0_DEBUG_EVENT = 0x50,
    CTMU_P2M_CH0_SHORT_EVENT,
    CTMU_P2M_CH0_LONG_EVENT,
    CTMU_P2M_CH0_FALLING_EVENT,
    CTMU_P2M_CH0_RAISING_EVENT,

    CTMU_P2M_CH1_DEBUG_EVENT = 0x58,
    CTMU_P2M_CH1_SHORT_EVENT,
    CTMU_P2M_CH1_LONG_EVENT,
    CTMU_P2M_CH1_FALLING_EVENT,
    CTMU_P2M_CH1_RAISING_EVENT,

    CTMU_P2M_CH1_IN_EVENT,
    CTMU_P2M_CH1_OUT_EVENT,
};


enum CTMU_M2P_CMD {
    CTMU_M2P_INIT = 0x50,
    CTMU_M2P_DISABLE, 		//模块关闭
    CTMU_M2P_ENABLE,  		//模块使能
    CTMU_M2P_CH0_ENABLE, 	//通道0打开
    CTMU_M2P_CH0_DISABLE, 	//通道0关闭
    CTMU_M2P_CH1_ENABLE, 	//通道1打开
    CTMU_M2P_CH1_DISABLE, 	//通道0关闭
    CTMU_M2P_UPDATE_BASE_TIME, 	//更新时基参数
    CTMU_M2P_CHARGE_ENTER_MODE, //进仓充电模式
    CTMU_M2P_CHARGE_EXIT_MODE,  //退出充电模式
};


//================= 关于模块内部的计算 ==================//
#define CTMU_DEFAULT_LRC_FREQ 			32000 			//默认LRC频率(Hz)
#define CTMU_DEFAULT_LRC_PRD 			(1000000 / CTMU_DEFAULT_LRC_FREQ)	//默认LRC周期(ms)
#define CTMU_TIME_BASE 					10 	//ms
#define CTMU_SHORT_CLICK_WINDOW_TIME 	CTMU_LONG_CLICK_DELAY_TIME
#define CTMU_LONG_CLICK_WINDOW_TIME 	1000 	//ms

#define CFG_M2P_CTMU_SHORT_TIME 	    0 //(CTMU_SHORT_CLICK_WINDOW_TIME / 10 - 1)
#define CFG_M2P_CTMU_LONG_TIME 		    (CTMU_LONG_CLICK_DELAY_TIME / 10 - 1)
//时基计算:
#define CFG_M2P_CTMU_BASE_TIME_PRD 		((CTMU_DEFAULT_LRC_FREQ * CTMU_TIME_BASE) / 1000 - 1)

//采样窗口时间:
#define CFG_CTMU_PRD1_TIME 			    4000 	//us
#define CFG_M2P_CTMU_PRD1_VALUE 	    ((CFG_CTMU_PRD1_TIME / CTMU_DEFAULT_LRC_PRD) + 1) 	//换算

#define CTMU_RESET_TIMER_PRD_VALUE 		200 	//ms

#if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
#ifdef CTMU_RESET_TIME_CONFIG
#undef CTMU_RESET_TIME_CONFIG
#define CTMU_RESET_TIME_CONFIG 		0
#endif
#endif


enum {
    BT_CH0_RES_MSG,
    BT_CH1_RES_MSG,
    BT_EVENT_HW_MSG,
    BT_EVENT_SW_MSG,
    BT_EVENT_VDDIO,
};

struct ctmu_key {
    u8 init;
    u8 ch_init;
    u8 click_cnt[2];
    u8 last_key[2];
    u8 key_msg_lock;
    u8 softoff_mode;
    u16 key_msg_lock_timer;
    u16 short_timer[2];
    u16 long_timer[2];
    u16 ear_in_timer;
    u8  last_ear_in_state;
    u8  inear_ok;
    u16 trim_value;
    u16 trim_flag;
    u32 lrc_hz;
    const struct lp_touch_key_platform_data *config;
};

enum ctmu_key_event {
    CTMU_KEY_NULL,
    CTMU_KEY_SHORT_CLICK,
    CTMU_KEY_LONG_CLICK,
    CTMU_KEY_HOLD_CLICK,
};

enum ch1_event_list {
    CH1_EAR_IN,
    CH1_EAR_OUT,
};

enum LP_TOUCH_SOFTOFF_MODE {
    LP_TOUCH_SOFTOFF_MODE_LEGACY  = 0, //普通关机
    LP_TOUCH_SOFTOFF_MODE_ADVANCE = 1, //带触摸关机
};

static struct ctmu_key _ctmu_key = {
    .last_ear_in_state = CH1_EAR_OUT,
    .ear_in_timer = 0xFFFF,
    .click_cnt = {0, 0},
    .short_timer = {0xFFFF, 0xFFFF},
    .long_timer = {0xFFFF, 0XFFFF},
    .last_key = {CTMU_KEY_NULL, CTMU_KEY_NULL},
};
#define __this 		(&_ctmu_key)

struct ch_ana_cfg {
    u8 isel;
    u8 vhsel;
    u8 vlsel;
};

struct ch_adjust_table {
    u16 cfg0;
    u16 cfg1;
    u16 cfg2;
};

//cap(电容)检测灵敏度级数配置
//模具厚度, 触摸片面积有关
//模具越厚, 触摸片面积越大, 触摸时电容变化量
//cap检测灵敏度级数配置建议从级数0开始调, 选取合适的灵敏度;

//======================================================//
// 内置触摸灵敏度表, 由内置触摸灵敏度调试工具生成 		//
// 请将该表替换sdk中lp_touch_key.c中同名的参数表        //
//======================================================//
const static struct ch_adjust_table ch_sensitivity_table[] = {
    /*  cfg0 		cfg1 		cfg2 */
//ch0:
    {15, 		20, 		300}, //cap检测灵敏度级数0
    {15, 		20, 		270}, //cap检测灵敏度级数1
    {15, 		20, 		240}, //cap检测灵敏度级数2
    {15, 		20, 		210}, //cap检测灵敏度级数3
    {15, 		20, 		180}, //cap检测灵敏度级数4
    {15, 		20, 		150}, //cap检测灵敏度级数5
    {15, 		20, 		120}, //cap检测灵敏度级数6
    {15, 		20, 		 90}, //cap检测灵敏度级数7
    {10, 		15, 		 60}, //cap检测灵敏度级数8
    {10, 		15, 		 30}, //cap检测灵敏度级数9

//ch1:
    {15, 		20, 		230}, //cap检测灵敏度级数0
    {15, 		20, 		210}, //cap检测灵敏度级数1
    {15, 		20, 		190}, //cap检测灵敏度级数2
    {15, 		20, 		170}, //cap检测灵敏度级数3
    {15, 		20, 		150}, //cap检测灵敏度级数4
    {15, 		20, 		130}, //cap检测灵敏度级数5
    {15, 		20, 		110}, //cap检测灵敏度级数6
    {15, 		20, 		 80}, //cap检测灵敏度级数7
    {10, 		15, 		 70}, //cap检测灵敏度级数8
    {10, 		15, 		 50}, //cap检测灵敏度级数9
};

#define LPCTMU_VH_LEVEL     3 // 上限电压档位
#define LPCTMU_VL_LEVEL     0 // 下限电压档位
#define LPCTMU_CUR_LEVEL    7 // 充放电电流档位

static const u8 lpctmu_ana_vh_table[4] = {
    LPCTMU_VH_065V,
    LPCTMU_VH_070V,
    LPCTMU_VH_075V,
    LPCTMU_VH_080V,
};
static const u8 lpctmu_ana_vl_table[4] = {
    LPCTMU_VL_020V,
    LPCTMU_VL_025V,
    LPCTMU_VL_030V,
    LPCTMU_VL_035V,
};
static const u8 lpctmu_ana_cur_table[8] = {
    LPCTMU_ISEL_008UA,
    LPCTMU_ISEL_024UA,
    LPCTMU_ISEL_040UA,
    LPCTMU_ISEL_056UA,
    LPCTMU_ISEL_072UA,
    LPCTMU_ISEL_088UA,
    LPCTMU_ISEL_104UA,
    LPCTMU_ISEL_120UA
};

u8 get_lpctmu_ana_level(void)
{
    return ((lpctmu_ana_vh_table[LPCTMU_VH_LEVEL]) | \
            (lpctmu_ana_vl_table[LPCTMU_VL_LEVEL]) | \
            (lpctmu_ana_cur_table[LPCTMU_CUR_LEVEL]));
}

struct lp_touch_key_alog_cfg {
    u16 ready_flag;
    u16 range;
    s32 sigma;
};
static struct lp_touch_key_alog_cfg alog_cfg[2];
static u16 res_stable_cnt[2];
static u8 ch_range_sensity[2] = {7, 7};
static u16 save_alog_cfg_time_out[2] = {0, 0};
static u16 ctmu_res_scan_time_add = 0;
#define TOUCH_RANGE_MIN     50
#define TOUCH_RANGE_MAX     500

#define CTMU_RES_BUF_SIZE   30
static u16 ctmu_res_buf[2][CTMU_RES_BUF_SIZE];
static u16 ctmu_res_buf_in[2] = {0, 0};
static u16 falling_res_avg[2] = {0, 0};
static u16 long_event_res_avg[2] = {0, 0};

const u8 _ch_priv[2] = {0, 1};

int eartch_event_deal_init(void);


#if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
static int lp_touch_key_online_debug_init(void);
static int lp_touch_key_online_debug_send(u8 ch, u16 val);
static int lp_touch_key_online_debug_key_event_handle(u8 ch_index, struct sys_event *event);
#endif /* #if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE */

static volatile u8 is_lpkey_active = 0;

//init io HZ
static void ctmu_port_init(u8 port)
{
    gpio_set_die(port, 0);
    gpio_set_dieh(port, 0);
    gpio_set_direction(port, 1);
    gpio_set_pull_down(port, 0);
    gpio_set_pull_up(port, 0);
}

static void lp_touch_key_send_cmd(enum CTMU_M2P_CMD cmd)
{
    M2P_CTMU_CMD = cmd;
    P11_M2P_INT_SET = BIT(M2P_CTMU_INDEX);
}

#if TCFG_LP_EARTCH_KEY_ENABLE
void eartch_hardware_recover(void)
{
    lp_touch_key_send_cmd(CTMU_M2P_CH1_ENABLE);
}

void eartch_hardware_suspend(void *priv)
{
    __this->ch_init &= ~BIT(1);
    lp_touch_key_send_cmd(CTMU_M2P_CH1_DISABLE);
}
#endif /* #if TCFG_LP_EARTCH_KEY_ENABLE */


#if CTMU_CHECK_LONG_CLICK_BY_RES

static void lp_touch_key_ctmu_res_buf_clear(u8 ch)
{
    ctmu_res_buf_in[ch] = 0;
    for (u8 i = 0; i < CTMU_RES_BUF_SIZE; i ++) {
        ctmu_res_buf[ch][i] = 0;
    }
}

static void lp_touch_key_ctmu_res_all_buf_clear(void)
{
    for (u8 ch = 0; ch < 2; ch ++) {
        lp_touch_key_ctmu_res_buf_clear(ch);
    }
}

static u32 lp_touch_key_ctmu_res_buf_avg(u8 ch)
{
#if !TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
    u32 res_sum = 0;
    u8 j = 0;
    u8 cnt = ctmu_res_buf_in[ch];
    for (u8 i = 0; i < (CTMU_RES_BUF_SIZE - 10); i ++) {
        if (ctmu_res_buf[ch][cnt]) {
            res_sum += ctmu_res_buf[ch][cnt];
            j ++;
        } else {
            return 0;
        }
        cnt ++;
        if (cnt >= CTMU_RES_BUF_SIZE) {
            cnt = 0;
        }
    }
    if (res_sum) {
        return (res_sum / j);
    }
#endif
    return 0;
}

static u8 lp_touch_key_check_long_click_by_ctmu_res(u8 ch)
{
#if !TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
    long_event_res_avg[ch] = lp_touch_key_ctmu_res_buf_avg(ch);
    log_debug("long_event_res_avg: %d\n", long_event_res_avg[ch]);
    if ((falling_res_avg[ch] < 2000)  || \
        (falling_res_avg[ch] > 20000) || \
        (long_event_res_avg[ch] < 2000) || \
        (long_event_res_avg[ch] > 20000)) {
    } else {
        u16 cfg2 = (P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2H + ch * 6) << 8) | P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2L + ch * 6);
        u16 diff = (falling_res_avg[ch] > long_event_res_avg[ch]) ? (falling_res_avg[ch] - long_event_res_avg[ch]) : (long_event_res_avg[ch] - falling_res_avg[ch]);
        if (diff < (cfg2 / 2)) {
            log_debug("long event return ! diff: %d  <  cfg2/2: %d\n", diff, (cfg2 / 2));
            return 1;
        }
    }
#endif
    return 0;
}

#endif

u8 lp_touch_key_alog_range_display(u8 *display_buf)
{
    if (__this->init == 0) {
        return 0;
    }
    u8 tmp_buf[16], i = 0;
    memset(tmp_buf, 0, sizeof(tmp_buf));
    for (u8 ch = 0; ch < LP_CTMU_CHANNEL_SIZE; ch ++) {
        if (__this->config->ch[ch].enable) {
            u16 range = alog_cfg[ch].range % 1000;
            tmp_buf[0 + i * 4] = (range / 100) + '0';
            tmp_buf[1 + i * 4] = ((range % 100) / 10) + '0';
            tmp_buf[2 + i * 4] = (range % 10) + '0';
            tmp_buf[3 + i * 4] = ' ';
            i ++;
        }
    }
    if (i) {
        display_buf[0] = i * 4 + 1;
        display_buf[1] = 0x01;
        memcpy((u8 *)&display_buf[2], tmp_buf, i * 4);
        printf_buf(display_buf, i * 4 + 2);
        return (display_buf[0] + 1);
    }
    return 0;
}

#if (CTMU_CH0_MODULE_DEBUG || CTMU_CH1_MODULE_DEBUG)
#if !TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE

void lp_touch_key_save_alog_cfg(void *priv)
{
    u8 ch = *((u8 *)priv);
    int ret = syscfg_write(VM_LP_TOUCH_KEY0_ALOG_CFG + ch, (void *)&alog_cfg[ch], sizeof(struct lp_touch_key_alog_cfg));
    if (ret != sizeof(struct lp_touch_key_alog_cfg)) {
        log_info("write vm alog cfg ready flag error !\n");
    }
    save_alog_cfg_time_out[ch] = 0;
}

void lp_touch_key_ctmu_cfg2_check_update(u8 ch, u16 cfg2_new)
{
    u16 cfg0 = (P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0H + ch * 6) << 8) | P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0L + ch * 6);
    u16 cfg2_old = (P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2H + ch * 6) << 8) | P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2L + ch * 6);
    if ((cfg2_old != cfg2_new) && (cfg2_new > (3 * cfg0))) {
        printf("ctmu ch%d cfg2_old = %d  cfg2_new = %d\n", ch, cfg2_old, cfg2_new);
        P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2L + ch * 6) = cfg2_new & 0xff;
        P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2H + ch * 6) = (cfg2_new >> 8) & 0xff;
        P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG1L + ch * 6) = P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0L + ch * 6) + 7;
    }
}

static void lp_touch_key_ctmu_res_scan(void *priv)
{
    for (u8 ch = 0; ch < LP_CTMU_CHANNEL_SIZE; ch ++) {
        if (__this->config->ch[ch].enable) {
            if ((ch == 0) && (M2P_CTMU_CH0_RES_SEND)) {
                continue;
            }
            if ((ch == 1) && (M2P_CTMU_CH1_RES_SEND)) {
                continue;
            }
#if TCFG_LP_EARTCH_KEY_ENABLE
            if (ch == 1) {
                continue;
            }
#endif

            u16 ctmu_res0;
            u16 ctmu_res1;
__read_res:
            ctmu_res0 = (P11_P2M_ACCESS(P2M_MASSAGE_CTMU_CH0_H_RES + ch * 2) << 8) | P11_P2M_ACCESS(P2M_MASSAGE_CTMU_CH0_L_RES + ch * 2);
            delay(100);
            ctmu_res1 = (P11_P2M_ACCESS(P2M_MASSAGE_CTMU_CH0_H_RES + ch * 2) << 8) | P11_P2M_ACCESS(P2M_MASSAGE_CTMU_CH0_L_RES + ch * 2);
            if (ctmu_res0 != ctmu_res1) {
                goto __read_res;
            }
#if 1

            if ((ctmu_res0 < 2000) || (ctmu_res0 > 20000)) {
                continue;
            }

            ctmu_res_buf[ch][ctmu_res_buf_in[ch]] = ctmu_res0;
            ctmu_res_buf_in[ch] ++;
            if (ctmu_res_buf_in[ch] >= CTMU_RES_BUF_SIZE) {
                ctmu_res_buf_in[ch] = 0;
            }

            if (alog_cfg[ch].ready_flag == 0) {
                continue;
            }
            if (res_stable_cnt[ch] < 50) {
                res_stable_cnt[ch] ++;
                continue;
            }
            TouchAlgo_Update(ch, ctmu_res0);

            u8 range_valid = 0;
            u16 touch_range = TouchAlgo_GetRange(ch, (u8 *)&range_valid);
            s32 touch_sigma = TouchAlgo_GetSigma(ch);
            /* printf("ch%d res:%d range:%d val:%d sigma:%d\n", ch, ctmu_res0, touch_range, range_valid, touch_sigma); */
            if ((range_valid) && (touch_range > TOUCH_RANGE_MIN) && (touch_range < TOUCH_RANGE_MAX)) {
                if (touch_range != alog_cfg[ch].range) {
                    u16 cfg2_new = touch_range * (10 - ch_range_sensity[ch]) / 10;
                    lp_touch_key_ctmu_cfg2_check_update(ch, cfg2_new);
                    alog_cfg[ch].range = touch_range;
                    alog_cfg[ch].sigma = touch_sigma;
                    if (save_alog_cfg_time_out[ch] == 0) {
                        save_alog_cfg_time_out[ch] = sys_timeout_add((void *)&_ch_priv[ch], lp_touch_key_save_alog_cfg, 1);
                    }
                }
            } else if ((range_valid) && (touch_range >= TOUCH_RANGE_MAX)) {
                TouchAlgo_SetRange(ch, alog_cfg[ch].range);
            }
#endif
        }
    }
}


void lp_touch_key_alog_ready_flag_check_and_set(void)
{
    for (u8 ch = 0; ch < LP_CTMU_CHANNEL_SIZE; ch ++) {
        if (__this->config->ch[ch].enable) {
#if TCFG_LP_EARTCH_KEY_ENABLE
            if (ch == 1) {
                continue;
            }
#endif
            alog_cfg[ch].ready_flag = 0;
            syscfg_read(VM_LP_TOUCH_KEY0_ALOG_CFG + ch, (void *)&alog_cfg[ch], sizeof(struct lp_touch_key_alog_cfg));
            if (alog_cfg[ch].ready_flag == 0) {
                alog_cfg[ch].ready_flag = 1;
                lp_touch_key_save_alog_cfg((void *)&ch);
            }
        }
    }
}

void lp_touch_key_alog_init(u8 ch, u8 cfg2_sensity)
{
#if TCFG_LP_EARTCH_KEY_ENABLE
    if (ch == 1) {
        return;
    }
#endif
    TouchAlgo_Init(ch, TOUCH_RANGE_MIN, TOUCH_RANGE_MAX);
    res_stable_cnt[ch] = 0;
    ch_range_sensity[ch] = cfg2_sensity;
    int ret = syscfg_read(VM_LP_TOUCH_KEY0_ALOG_CFG + ch, (void *)&alog_cfg[ch], sizeof(struct lp_touch_key_alog_cfg));
    if ((ret == (sizeof(struct lp_touch_key_alog_cfg))) && (alog_cfg[ch].range > TOUCH_RANGE_MIN) && (alog_cfg[ch].range < TOUCH_RANGE_MAX) && (alog_cfg[ch].ready_flag)) {
        log_info("vm read ch%d alog ready_flag:%d sigma:%d range:%d\n", ch, alog_cfg[ch].ready_flag, alog_cfg[ch].sigma, alog_cfg[ch].range);
        TouchAlgo_SetSigma(ch, alog_cfg[ch].sigma);
        TouchAlgo_SetRange(ch, alog_cfg[ch].range);
        u16 new_cfg2 = alog_cfg[ch].range * (10 - ch_range_sensity[ch]) / 10;
        P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2L + 6 * ch) = new_cfg2 & 0xff;
        P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2H + 6 * ch) = (new_cfg2 >> 8) & 0xff;
    }
}

#endif
#endif


#define IO_PORT_RESET_PORT_LDOIN   	LINDT_IN//IO编号
#define IO_RESET_PORTB_01 			11

void lp_touch_key_init(const struct lp_touch_key_platform_data *config)
{
    log_info("%s >>>>", __func__);

    ASSERT(config && (__this->init == 0));
    __this->config = config;

    //长按复位检测
    u8 pinr_io;
    if (P33_CON_GET(P3_PINR_CON) & BIT(0)) {
        pinr_io = P33_CON_GET(P3_PORT_SEL0);
        if (pinr_io == IO_RESET_PORTB_01) {
            P33_CON_SET(P3_PINR_CON, 0, 1, 0);
            p33_tx_1byte(P3_PORT_SEL0, IO_PORT_RESET_PORT_LDOIN);
            P33_CON_SET(P3_PINR_CON, 2, 1, 1);
            P33_CON_SET(P3_PINR_CON, 0, 1, 1);
            log_info("reset pin change: old: %d, new: %d, P33_PINR_CON = 0x%x", pinr_io, P33_CON_GET(P3_PORT_SEL0), P33_CON_GET(P3_PINR_CON));
        }
    }

    u8 ch_sensity[2];
    ch_sensity[0] = __this->config->ch[0].sensitivity;
    ch_sensity[1] = __this->config->ch[1].sensitivity;

    LP_TOUCH_KEY_CONFIG tool_cfg;
    int ret = syscfg_read(CFG_LP_TOUCH_KEY_ID, &tool_cfg, sizeof(LP_TOUCH_KEY_CONFIG));
    if (ret > 0) {
        log_info("cfg_en: %d, ch0_sensity_cfg: %d, ch1_sensity_cfg: %d", tool_cfg.cfg_en, tool_cfg.touch_key_sensity_class, tool_cfg.earin_key_sensity_class);
        if (tool_cfg.cfg_en) {
            ch_sensity[0] = tool_cfg.touch_key_sensity_class;
            ch_sensity[1] = tool_cfg.earin_key_sensity_class;
        }
    } else {
        log_error("touch key cfg not exist");
    }

    log_info("ch0_sensity: %d, ch1_sensity: %d", ch_sensity[0], ch_sensity[1]);

    M2P_CTMU_CH0_ANA_SEL = get_lpctmu_ana_level();
    M2P_CTMU_CH1_ANA_SEL = get_lpctmu_ana_level();

#if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
    M2P_CTMU_CH0_RES_SEND = 1;
    M2P_CTMU_CH1_RES_SEND = 1;
#else
    M2P_CTMU_CH0_RES_SEND = 0;
    M2P_CTMU_CH1_RES_SEND = 0;
#endif

    M2P_CTMU_SHORT_TIMEL = (CFG_M2P_CTMU_SHORT_TIME & 0xFF);
    M2P_CTMU_SHORT_TIMEH = ((CFG_M2P_CTMU_SHORT_TIME >> 8) & 0xFF);
    M2P_CTMU_LONG_TIMEL = (CFG_M2P_CTMU_LONG_TIME & 0xFF);
    M2P_CTMU_LONG_TIMEH = ((CFG_M2P_CTMU_LONG_TIME >> 8) & 0xFF);

    //长按开机时间配置
    M2P_CTMU_SOFTOFF_LONG_TIMEL = (CFG_M2P_CTMU_SOFTOFF_LONG_TIME & 0xFF);
    M2P_CTMU_SOFTOFF_LONG_TIMEH = ((CFG_M2P_CTMU_SOFTOFF_LONG_TIME >> 8) & 0xFF);

    //采样窗口时间配置
    M2P_CTMU_PRD1_L = (CFG_M2P_CTMU_PRD1_VALUE & 0xFF);
    M2P_CTMU_PRD1_H = ((CFG_M2P_CTMU_PRD1_VALUE >> 8) & 0xFF);

    M2P_CTMU_MSG = 0;

    M2P_CTMU_MSG |= CTMU_INIT_FALL_ENABLE;
    M2P_CTMU_MSG |= CTMU_INIT_RAISE_ENABLE;
    //M2P_CTMU_MSG |= CTMU_INIT_SHORT_ENABLE;

    log_info("M2P_CTMU_SHORT_TIMEL = 0x%x", M2P_CTMU_SHORT_TIMEL);
    log_info("M2P_CTMU_SHORT_TIMEH = 0x%x", M2P_CTMU_SHORT_TIMEH);
    log_info("M2P_CTMU_LONG_TIMEL  = 0x%x", M2P_CTMU_LONG_TIMEL);
    log_info("M2P_CTMU_LONG_TIMEH  = 0x%x", M2P_CTMU_LONG_TIMEH);

    log_info("M2P_CTMU_SOFTOFF_LONG_TIMEL = 0x%x", M2P_CTMU_SOFTOFF_LONG_TIMEL);
    log_info("M2P_CTMU_SOFTOFF_LONG_TIMEH = 0x%x", M2P_CTMU_SOFTOFF_LONG_TIMEH);
    log_info("M2P_CTMU_PRD1L = 0x%x", M2P_CTMU_PRD1_L);
    log_info("M2P_CTMU_PRD1H = 0x%x", M2P_CTMU_PRD1_H);

    log_info("M2P_CTMU_CH0_ANA_SEL = 0x%x", M2P_CTMU_CH0_ANA_SEL);
    log_info("M2P_CTMU_CH1_ANA_SEL = 0x%x", M2P_CTMU_CH1_ANA_SEL);

    if (__this->config->ch[0].port == IO_PORTB_02) {
        M2P_CTMU_MSG |= CTMU_INIT_PORT_INV; 		//PB1和PB2通道互换IO;
        ASSERT(__this->config->ch[1].port == IO_PORTB_01);
    } else {
        ASSERT(__this->config->ch[0].port == IO_PORTB_01);
        ASSERT(__this->config->ch[1].port == IO_PORTB_02);
    }

    for (u8 ch = 0; ch < LP_CTMU_CHANNEL_SIZE; ch ++) {
        if (__this->config->ch[ch].enable) {
            ctmu_port_init(__this->config->ch[ch].port);

            P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0L + 6 * ch) = ((ch_sensitivity_table[ch_sensity[ch] + ch * 10].cfg0)       & 0xFF);
            P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0H + 6 * ch) = (((ch_sensitivity_table[ch_sensity[ch] + ch * 10].cfg0) >> 8) & 0xFF);
            P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG1L + 6 * ch) = (((ch_sensitivity_table[ch_sensity[ch] + ch * 10].cfg0) + 5)  & 0xFF);
            P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG1H + 6 * ch) = ((((ch_sensitivity_table[ch_sensity[ch] + ch * 10].cfg0) + 5) >> 8) & 0xFF);
            P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2L + 6 * ch) = ((ch_sensitivity_table[ch_sensity[ch] + ch * 10].cfg2)       & 0xFF);
            P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2H + 6 * ch) = (((ch_sensitivity_table[ch_sensity[ch] + ch * 10].cfg2) >> 8) & 0xFF);

            log_info("M2P_CTMU_CH%d_CFG0L = 0x%x", ch, P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0L + ch * 6));
            log_info("M2P_CTMU_CH%d_CFG0H = 0x%x", ch, P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG0H + ch * 6));
            log_info("M2P_CTMU_CH%d_CFG1L = 0x%x", ch, P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG1L + ch * 6));
            log_info("M2P_CTMU_CH%d_CFG1H = 0x%x", ch, P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG1H + ch * 6));
            log_info("M2P_CTMU_CH%d_CFG2L = 0x%x", ch, P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2L + ch * 6));
            log_info("M2P_CTMU_CH%d_CFG2H = 0x%x", ch, P11_M2P_ACCESS(M2P_MASSAGE_CTMU_CH0_CFG2H + ch * 6));

            if (ch == 0) {
                M2P_CTMU_MSG |= CTMU_INIT_CH0_ENABLE;
#if CTMU_CH0_MODULE_DEBUG
                M2P_CTMU_MSG |= CTMU_INIT_CH0_DEBUG;
#if !TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
                if (M2P_CTMU_CH0_RES_SEND == 0) {
                    lp_touch_key_alog_init(0, ch_sensity[0]);
                }
#endif
#endif
            } else {
                M2P_CTMU_MSG |= CTMU_INIT_CH1_ENABLE;
#if CTMU_CH1_MODULE_DEBUG
                M2P_CTMU_MSG |= CTMU_INIT_CH1_DEBUG;
#if !TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
                if (M2P_CTMU_CH1_RES_SEND == 0) {
                    lp_touch_key_alog_init(1, ch_sensity[1]);
                }
#endif
#endif
            }
        }
    }
    log_info("M2P_CTMU_MSG = 0x%x", M2P_CTMU_MSG);
    log_info("M2P_CTMU_CH0_RES_SEND = 0x%x", M2P_CTMU_CH0_RES_SEND);
    log_info("M2P_CTMU_CH1_RES_SEND = 0x%x", M2P_CTMU_CH1_RES_SEND);

#if !TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE

#if 1   //触摸算法要尽可能的在装完整机之后开始跑，而这里是耳机生产流程中获取它第一次在舱内开机的时候，在vm标记一个变量，从此开始跑算法。

    if (get_charge_online_flag()) {
        log_info("check charge online!\n");
        lp_touch_key_alog_ready_flag_check_and_set();//如果有其他好的位置，请将该函数移到那个位置执行。
    }

#endif

    if (ctmu_res_scan_time_add == 0) {
        ctmu_res_scan_time_add = usr_timer_add(NULL, lp_touch_key_ctmu_res_scan, 10, 0);
    }

    lp_touch_key_ctmu_res_all_buf_clear();

#endif

    //CTMU 时基配置
    u16 time_prd = 0;
    if (__this->lrc_hz) {
        time_prd = (__this->lrc_hz * CTMU_TIME_BASE) / 1000 - 1;
    } else {
        time_prd = CFG_M2P_CTMU_BASE_TIME_PRD;
    }
    M2P_CTMU_BASE_TIME_PRD_L = (time_prd & 0xFF);
    M2P_CTMU_BASE_TIME_PRD_H = ((time_prd >> 8) & 0xFF);
    log_info("LRC_HZ = %d", __this->lrc_hz);
    log_info("M2P_CTMU_BASE_TIME_PRD_L = 0x%x", M2P_CTMU_BASE_TIME_PRD_L);
    log_info("M2P_CTMU_BASE_TIME_PRD_H = 0x%x", M2P_CTMU_BASE_TIME_PRD_H);


#if TCFG_LP_EARTCH_KEY_ENABLE

#if TCFG_EARTCH_EVENT_HANDLE_ENABLE
    if (eartch_event_deal_init() == 0) {
        sys_timeout_add(NULL, eartch_hardware_suspend, 500);
    }
#endif

    __this->trim_flag = 0;
    __this->inear_ok = 0;
    u16 trim_value = 0;
    ret = syscfg_read(LP_KEY_CH1_TRIM_VALUE, &trim_value, sizeof(trim_value));
    if (ret > 0) {
        __this->trim_value = trim_value;
        M2P_CTMU_TRIM_VALUE_L = (__this->trim_value & 0xFF);
        M2P_CTMU_TRIM_VALUE_H = ((__this->trim_value >> 8) & 0xFF);
        __this->inear_ok = 1;
        log_info("trim_value = %d", __this->trim_value);
    } else {
        __this->trim_value = 0;//没有trim的情况下用不了
        M2P_CTMU_TRIM_VALUE_L = (10000 & 0xFF);
        M2P_CTMU_TRIM_VALUE_H = ((10000 >> 8) & 0xFF);
    }
    //软件触摸灵敏度调试
    M2P_CTM_INEAR_VALUE_L = TCFG_LP_EARTCH_SOFT_INEAR_VAL & 0xFF;
    M2P_CTM_INEAR_VALUE_H = TCFG_LP_EARTCH_SOFT_INEAR_VAL >> 8;
    M2P_CTM_OUTEAR_VALUE_L = TCFG_LP_EARTCH_SOFT_OUTEAR_VAL & 0xFF;
    M2P_CTM_OUTEAR_VALUE_H = TCFG_LP_EARTCH_SOFT_OUTEAR_VAL >> 8;

    M2P_CTMU_MSG |= CTMU_INIT_EARTCH_ENABLE;

#endif

    //CTMU 初始化命令

    M2P_CTMU_INIT_FLAG = 0;
    lp_touch_key_send_cmd(CTMU_M2P_INIT);
    while (!M2P_CTMU_INIT_FLAG) {
        asm volatile("csync");
    }
    load_p11_bank_code2ram(1, 1);

    __this->softoff_mode = LP_TOUCH_SOFTOFF_MODE_ADVANCE;
    __this->init = 1;

#if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
    lp_touch_key_online_debug_init();
#endif

}


int __attribute__((weak)) lp_touch_key_event_remap(struct sys_event *e)
{
    return true;
}

static void __ctmu_notify_key_event(struct sys_event *event, u8 ch)
{
    event->type = SYS_KEY_EVENT;
    event->u.key.type = KEY_DRIVER_TYPE_CTMU_TOUCH; 	//区分按键类型
    event->arg  = (void *)DEVICE_EVENT_FROM_KEY;

    if (__this->key_msg_lock) {//锁住期间不能发消息
        return;
    }

#if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
    if (lp_touch_key_online_debug_key_event_handle(ch, event)) {
        return;
    }
#endif

    if (lp_touch_key_event_remap(event)) {
        sys_event_notify(event);
    }
}

__attribute__((weak)) u8 remap_ctmu_short_click_event(u8 click_cnt, u8 event)
{
    return event;
}

static void __ctmu_short_click_time_out_handle(void *priv)
{
    u8 ch = *((u8 *)priv);
    struct sys_event e;
    switch (__this->click_cnt[ch]) {
    case 0:
        return;
        break;
    case 1:
        e.u.key.event = KEY_EVENT_CLICK;
        break;
    case 2:
        e.u.key.event = KEY_EVENT_DOUBLE_CLICK;
        break;
    case 3:
        e.u.key.event = KEY_EVENT_TRIPLE_CLICK;
        break;
    case 4:
        e.u.key.event = KEY_EVENT_FOURTH_CLICK;
        break;
    case 5:
        e.u.key.event = KEY_EVENT_FIRTH_CLICK;
        break;
    default:
        e.u.key.event = KEY_EVENT_USER;
        break;
    }
    e.u.key.event = remap_ctmu_short_click_event(__this->click_cnt[ch], e.u.key.event);
    e.u.key.value = __this->config->ch[ch].key_value;

    __this->short_timer[ch] = 0xFFFF;
    __this->last_key[ch] = CTMU_KEY_NULL;
    log_debug("notify key short event, cnt: %d", __this->click_cnt[ch]);
    __ctmu_notify_key_event(&e, ch);
}

static void ctmu_short_click_handle(u8 ch)
{
    __this->last_key[ch] = CTMU_KEY_SHORT_CLICK;
    if (__this->short_timer[ch] == 0xFFFF) {
        __this->click_cnt[ch] = 1;
#if TCFG_LP_SLIDE_KEY_ENABLE
        __this->short_timer[ch] = usr_timeout_add((void *)&_ch_priv[ch], __ctmu_short_click_time_out_handle, CTMU_SHORT_CLICK_DELAY_TIME + 150, 1);
#else
        __this->short_timer[ch] = usr_timeout_add((void *)&_ch_priv[ch], __ctmu_short_click_time_out_handle, CTMU_SHORT_CLICK_DELAY_TIME, 1);
#endif
    } else {
        __this->click_cnt[ch]++;
        usr_timer_modify(__this->short_timer[ch], CTMU_SHORT_CLICK_DELAY_TIME);
    }
}

static void __ctmu_hold_click_time_out_handle(void *priv)
{
    log_debug("notify key hold event");

    u8 ch = *((u8 *)priv);

    struct sys_event e;
    e.u.key.event = KEY_EVENT_HOLD;
    e.u.key.value = __this->config->ch[ch].key_value;

    __ctmu_notify_key_event(&e, ch);
    __this->last_key[ch] = CTMU_KEY_HOLD_CLICK;
}


static void ctmu_long_click_handle(u8 ch)
{
    log_debug("notify key long event");
    __this->last_key[ch] = CTMU_KEY_LONG_CLICK;
    struct sys_event e;
    e.u.key.event = KEY_EVENT_LONG;
    e.u.key.value = __this->config->ch[ch].key_value;
    __ctmu_notify_key_event(&e, ch);
    if (__this->long_timer[ch] == 0xFFFF) {
        __this->long_timer[ch] = usr_timer_add((void *)&_ch_priv[ch], __ctmu_hold_click_time_out_handle, CTMU_HOLD_CLICK_DELAY_TIME, 1);
    }
}


static void ctmu_raise_click_handle(u8 ch)
{
    struct sys_event e = {0};
    if (__this->last_key[ch] >= CTMU_KEY_LONG_CLICK) {
        if (__this->long_timer[ch] != 0xFFFF) {
            usr_timer_del(__this->long_timer[ch]);
            __this->long_timer[ch] = 0xFFFF;
        }
        e.u.key.event = KEY_EVENT_UP;
        e.u.key.value = __this->config->ch[ch].key_value;
        __ctmu_notify_key_event(&e, ch);
        __this->last_key[ch] = CTMU_KEY_NULL;
        log_debug("notify key HOLD UP event");
    } else {
        ctmu_short_click_handle(ch);
    }
}


#if TCFG_LP_EARTCH_KEY_ENABLE

void __attribute__((weak)) ear_lptouch_update_state(u8 state)
{
    return;
}

extern void ear_lptouch_update_state(u8 state);
extern void eartch_state_update(u8 state);
static void __ctmu_ear_in_timeout_handle(void *priv)
{
    u8 state;
    __this->ear_in_timer = 0xFFFF;
    if (__this->config->ch[1].key_value == 0xFF) {//使用外部自定义流程
        if (__this->last_ear_in_state == CH1_EAR_IN) {
            ear_lptouch_update_state(0);
        } else {
            ear_lptouch_update_state(1);
        }
        return;
    }

#if TCFG_EARTCH_EVENT_HANDLE_ENABLE
    if (__this->last_ear_in_state == CH1_EAR_IN) {
        state = EARTCH_STATE_IN;
    } else if (__this->last_ear_in_state == CH1_EAR_OUT) {
        state = EARTCH_STATE_OUT;
    } else {
        return;
    }
    eartch_state_update(state);
#endif
}


static void __ctmu_key_unlock_timeout_handle(void *priv)
{
    __this->key_msg_lock = false;
    __this->key_msg_lock_timer = 0;
}

static void ctmu_ch1_event_handle(u8 ch1_event)
{
    __this->last_ear_in_state = ch1_event;

#if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
    struct sys_event event;
    if (ch1_event == CH1_EAR_IN) {
        event.u.key.event = 10;
    } else if (ch1_event == CH1_EAR_OUT) {
        event.u.key.event = 11;
    }
    if (lp_touch_key_online_debug_key_event_handle(1, &event)) {
        return;
    }
#endif

    __this->key_msg_lock = true;
    if (__this->key_msg_lock_timer == 0) {
        __this->key_msg_lock_timer = sys_hi_timeout_add(NULL, __ctmu_key_unlock_timeout_handle, 3000);
    } else {
        sys_hi_timer_modify(__this->key_msg_lock_timer, 3000);
    }
    __ctmu_ear_in_timeout_handle(NULL);
}


void lp_touch_key_testbox_inear_trim(u8 flag)
{
#if (!TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE)
    if (flag == 1) {
        __this->trim_flag = 1;
        __this->inear_ok = 0;
        __this->trim_value = 0;
        M2P_CTMU_CH1_RES_SEND = 1;
        log_info("__this->trim_flag = %d", __this->trim_flag);
        is_lpkey_active = 1;
    } else {
        __this->trim_flag = 0;
        M2P_CTMU_CH1_RES_SEND = 0;
        log_info("__this->trim_flag = %d", __this->trim_flag);
    }
#endif
}

#endif


u32 __attribute__((weak)) user_send_cmd_prepare(USER_CMD_TYPE cmd, u16 param_len, u8 *param)
{
    return 0;
}

static u8 lp_touch_key_testbox_remote_test(u8 event)
{
    u8 ret = true;
    u8 key_report = 0;

    switch (event) {
    case CTMU_P2M_CH0_FALLING_EVENT:
    case CTMU_P2M_CH1_FALLING_EVENT:
        key_report = 0xF1;
        log_info("Notify testbox CH0 Down");
        break;
    case CTMU_P2M_CH0_RAISING_EVENT:
    case CTMU_P2M_CH1_RAISING_EVENT:
        key_report = 0xF2;
        log_info("Notify testbox CH0 Up");
        break;
    case CTMU_P2M_CH0_LONG_EVENT:
    case CTMU_P2M_CH0_SHORT_EVENT:
        break;
    default:
        ret = false;
        break;
    }

    if (key_report) {
        user_send_cmd_prepare(USER_CTRL_TEST_KEY, 1, &key_report); //音量加
    }

    return ret;
}

#if TCFG_LP_SLIDE_KEY_ENABLE


/******************************双通道滑动触摸识别的基础原理***********************************

1.单击，但如果超过设定的长按时间，那就为长按
chx  __________________________________________
chx  ___________                     __________
                |_______>180________|


2.单击，但如果超过设定的长按时间，那就为长按
chx  ___________                     __________
                |_______>180________|
chx  _______ <30                     <30 ______
            |___________________________|


3.单击，但如果超过设定的长按时间，那就为长按
chx  ___________                     __________
                |_______>180________|
chx  _______ <30                       >30   __
            |_______________________________|


4.单击，但如果超过设定的长按时间，那就为长按
chx  ___________                     __________
                |_______>180________|
chx  ___   >30                       <30 ______
        |_______________________________|


5.单击，但如果超过设定的长按时间，那就为长按
chx  ___________                     __________
                |_______>180________|
chx  ___   >30                         >30   __
        |___________________________________|


6.单击，但如果超过设定的长按时间，那就为长按
chx  ___________                         ______
                |_______________________|
chx  _______ <30        >180         _<30______
            |_______________________|


7.根据前后操作，有可能是滑动，也有可能是连击中的后单击，也有可能是无效操作
chx  ___________                 __________
                |_______________|
chx  _______ <30    <180     _<30__________
            |_______________|


8.滑动，t < 设定的长按的时间
chx  ___________                 __________
                |_______________|
chx  ___   >30       t       _<30__________
        |___________________|


9.滑动，t < 设定的长按的时间
chx  ___________                     ______
                |___________________|
chx  ___   >30       t       __>30_________
        |___________________|


10.滑动，t < 设定的长按的时间
chx  ___________                     ______
                |___________________|
chx  _______ <30     t       __>30_________
            |_______________|

*****************************************************************************/

enum falling_order_type {
    FALLING_NULL,
    FALLING_FIRST,
    FALLING_SECOND,
};
enum raising_order_type {
    RAISING_NULL,
    RAISING_FIRST,
    RAISING_SECOND,
};

enum time_interval_type {
    LONG_TIME,
    SHORT_TIME,
};

enum slide_key_type {
    SHORT_CLICK = 1,
    DOUBLE_CLICK,
    TRIPLE_CLICK,
    FOURTH_CLICK,
    FIRTH_CLICK,
    LONG_CLICK = 8,
    LONG_HOLD_CLICK = 9,
    LONG_UP_CLICK = 10,
    SLIDE_UP = 0x12,
    SLIDE_DOWN = 0x21,
};

static u8 falling_order[2] = {FALLING_NULL, FALLING_NULL};
static u8 raising_order[2] = {RAISING_NULL, RAISING_NULL};
static u8 falling_time_interval = 0;
static u8 raising_time_interval = 0;
static u8 last_key_type = 0;

#define FALLING_TIMEOUT_TIME    30              //两个按下边沿的时间间隔的阈值
static int falling_timeout_add = 0;
static void falling_timeout_handle(void *priv)
{
    falling_timeout_add = 0;
}

#define RAISING_TIMEOUT_TIME    30              //两个抬起边沿的时间间隔的阈值
static int raising_timeout_add = 0;
static void raising_timeout_handle(void *priv)
{
    raising_timeout_add = 0;
}

#define CLICK_IDLE_TIMEOUT_TIME 500             //如果该时间内，没有单击，那么该时间之后的第一次单击，识别为首次单击
static int click_idle_timeout_add = 0;
static void click_idle_timeout_handle(void *priv)
{
    click_idle_timeout_add = 0;
}

#define FIRST_SHORT_CLICK_TIMEOUT_TIME  190     //只针对首次单击，要满足按够那么长时间。连击时，后面的单击没有该时间要求
static int first_short_click_timeout_add = 0;
static void first_short_click_timeout_handle(void *priv)
{
    first_short_click_timeout_add = 0;
}

void __attribute__((weak)) send_keytone_event(void)
{
    //可以在此处发送按键音的消息
    /* struct sys_event e; */
    /* e.u.key.event = 5; */
    /* e.u.key.value = 3; */
    /* __ctmu_notify_key_event(&e, 1); */
}

#define SEND_KEYTONE_TIMEOUT_TIME   250         //长按时的按键音，在按下后的多长时间要响
static u8 send_keytone_flag = 0;
static int send_keytone_timeout_add = 0;
static void send_keytone_timeout_handle(void *priv)
{
    send_keytone_event();
    send_keytone_timeout_add = 0;
    send_keytone_flag = 1;
}

#define SLIDE_CLICK_TIMEOUT_TIME    500         //识别为滑动之后，该时间内，如果有case7.也要识别为滑动
#define FAST_SLIDE_CNT_MAX          4           //一来就连续那么多次的case7.，就会识别为一次滑动。
static u8 fast_slide_cnt = 0;
static u8 fast_slide_type = 0;
static int slide_click_timeout_add = 0;
void slide_click_timeout_handle(void *priv)
{
    slide_click_timeout_add = 0;
}

static void check_channel_falling_info(u8 ch)
{
    falling_order[ch] = FALLING_FIRST;
    if (falling_order[!ch] == FALLING_FIRST) {
        falling_order[ch] = FALLING_SECOND;
    }
    send_keytone_flag = 0;
    if (send_keytone_timeout_add == 0) {
        send_keytone_timeout_add = sys_hi_timeout_add(NULL, send_keytone_timeout_handle, SEND_KEYTONE_TIMEOUT_TIME);
    } else {
        sys_hi_timer_modify(send_keytone_timeout_add, SEND_KEYTONE_TIMEOUT_TIME);
    }
    if ((last_key_type != SHORT_CLICK) || (click_idle_timeout_add == 0)) {
        if (first_short_click_timeout_add == 0) {
            first_short_click_timeout_add = sys_hi_timeout_add(NULL, first_short_click_timeout_handle, FIRST_SHORT_CLICK_TIMEOUT_TIME);
        } else {
            sys_hi_timer_modify(first_short_click_timeout_add, FIRST_SHORT_CLICK_TIMEOUT_TIME);
        }
    }
    if (falling_order[ch] == FALLING_FIRST) {
        if (falling_timeout_add == 0) {
            falling_timeout_add = sys_hi_timeout_add(NULL, falling_timeout_handle, FALLING_TIMEOUT_TIME);
        } else {
            sys_hi_timer_modify(falling_timeout_add, FALLING_TIMEOUT_TIME);
        }
        falling_time_interval = SHORT_TIME;
    } else if (falling_order[ch] == FALLING_SECOND) {
        if (falling_timeout_add) {
            sys_hi_timeout_del(falling_timeout_add);
            falling_timeout_add = 0;
            falling_time_interval = SHORT_TIME;
        } else {
            falling_time_interval = LONG_TIME;
        }
    }
}

static u8 check_channel_raising_info_and_key_type(u8 ch)
{
    u8 key_type = 0;
    fast_slide_type = 0;
    if (falling_order[ch] == FALLING_NULL) {
        return key_type;
    }
    raising_order[ch] = RAISING_FIRST;
    if (raising_order[!ch] == RAISING_FIRST) {
        raising_order[ch] = RAISING_SECOND;
    }
    if (falling_order[ch] > falling_order[!ch]) {
        if (raising_order[ch] == RAISING_FIRST) {
            key_type = SHORT_CLICK;             //case 1~5.
        } else {
            if (falling_time_interval == SHORT_TIME) {
                if (raising_timeout_add) {
                    sys_hi_timeout_del(raising_timeout_add);
                    raising_timeout_add = 0;
                    key_type = SHORT_CLICK;     //case 6~7.
                    if (ch == 0) {
                        fast_slide_type = SLIDE_DOWN;
                    } else {
                        fast_slide_type = SLIDE_UP;
                    }
                } else if (ch == 0) {
                    key_type = SLIDE_DOWN;      //case 10.
                } else {
                    key_type = SLIDE_UP;        //case 10.
                }
            } else if (ch == 0) {
                key_type = SLIDE_DOWN;          //caee 8~9.
            } else {
                key_type = SLIDE_UP;            //case 8~9.
            }
        }
    } else {
        if (raising_order[ch] == RAISING_FIRST) {
            if (falling_time_interval == SHORT_TIME) {
                if (raising_timeout_add == 0) {
                    raising_timeout_add = sys_hi_timeout_add(NULL, raising_timeout_handle, RAISING_TIMEOUT_TIME);
                } else {
                    sys_hi_timer_modify(raising_timeout_add, RAISING_TIMEOUT_TIME);
                }
            } else if (ch == 0) {
                key_type = SLIDE_UP;            //case 8~9.
            } else {
                key_type = SLIDE_DOWN;          //case 8~9.
            }
        } else {
            if (falling_time_interval == SHORT_TIME) {
                if (raising_timeout_add) {
                    sys_hi_timeout_del(raising_timeout_add);
                    raising_timeout_add = 0;
                    key_type = SHORT_CLICK;     //case 6~7
                    if (ch == 0) {
                        fast_slide_type = SLIDE_UP;
                    } else {
                        fast_slide_type = SLIDE_DOWN;
                    }
                } else if (ch == 0) {
                    key_type = SLIDE_UP;        //case 10.
                } else {
                    key_type = SLIDE_DOWN;      //case 10.
                }
            } else if (ch == 0) {
                key_type = SLIDE_UP;            //case 8~9.
            } else {
                key_type = SLIDE_DOWN;          //case 8~9.
            }
        }
    }
    return key_type;
}

static u8 check_slide_key_type(u8 event, u8 ch)
{
    u8 key_type = 0;
    u8 event_ch = 0;
    if (event == (CTMU_P2M_CH0_FALLING_EVENT + ch * 8)) {
        if (ch == event_ch) {
            check_channel_falling_info(0);
        } else {
            check_channel_falling_info(1);
        }
    } else if (event == (CTMU_P2M_CH0_RAISING_EVENT + ch * 8)) {
        if (ch == event_ch) {
            if ((last_key_type == LONG_CLICK) || (last_key_type == LONG_HOLD_CLICK)) {
                key_type = LONG_UP_CLICK;       //一定是长按抬起
            } else {
                key_type = check_channel_raising_info_and_key_type(0);
            }
        } else {
            key_type = check_channel_raising_info_and_key_type(1);
        }
    } else if (event == (CTMU_P2M_CH0_LONG_EVENT + event_ch)) {//长按只判断通道号小的那个按键
        key_type = LONG_CLICK;
        /* } else if (event == (CTMU_P2M_CH0_HOLD_EVENT + event_ch)) {//长按只判断通道号小的那个按键 */
        /* key_type = LONG_HOLD_CLICK; */
    }

    if (key_type) {
        falling_order[0] = FALLING_NULL;
        falling_order[1] = FALLING_NULL;
        raising_order[0] = RAISING_NULL;
        raising_order[1] = RAISING_NULL;
        falling_time_interval = 0;
        last_key_type = key_type;
        if (send_keytone_timeout_add) {
            sys_hi_timeout_del(send_keytone_timeout_add);
            send_keytone_timeout_add = 0;
        }
        if (key_type == SHORT_CLICK) {                  //case 6~7 的进一步处理
            if (first_short_click_timeout_add) {
                sys_hi_timeout_del(first_short_click_timeout_add);
                first_short_click_timeout_add = 0;

                if (slide_click_timeout_add) {
                    sys_hi_timeout_del(slide_click_timeout_add);
                    slide_click_timeout_add = 0;
                    if (fast_slide_type) {
                        key_type = fast_slide_type;
                        last_key_type = key_type;
                    } else {
                        key_type = 0xff;
                        last_key_type = key_type;
                    }
                    fast_slide_cnt = 0;
                } else {
                    if (fast_slide_type) {
                        fast_slide_cnt ++;
                        if (fast_slide_cnt >= FAST_SLIDE_CNT_MAX) {
                            fast_slide_cnt = 0;
                            key_type = fast_slide_type;
                            last_key_type = key_type;
                        } else {
                            key_type = 0xff;
                            last_key_type = key_type;
                        }
                    } else {
                        fast_slide_cnt = 0;
                        key_type = 0xff;
                        last_key_type = key_type;
                    }
                }
            } else {
                if (send_keytone_flag == 0) {
                    send_keytone_event();
                }
                if (slide_click_timeout_add) {
                    sys_hi_timeout_del(slide_click_timeout_add);
                    slide_click_timeout_add = 0;
                }
                fast_slide_cnt = 0;
            }
            if (click_idle_timeout_add == 0) {
                click_idle_timeout_add = sys_hi_timeout_add(NULL, click_idle_timeout_handle, CLICK_IDLE_TIMEOUT_TIME);
            } else {
                sys_hi_timer_modify(click_idle_timeout_add, CLICK_IDLE_TIMEOUT_TIME);
            }
        } else {
            fast_slide_cnt = 0;
        }

        if ((key_type == SLIDE_UP) || (key_type == SLIDE_DOWN)) {
            if (slide_click_timeout_add == 0) {
                slide_click_timeout_add = sys_hi_timeout_add(NULL, slide_click_timeout_handle, SLIDE_CLICK_TIMEOUT_TIME);
            } else {
                sys_hi_timer_modify(slide_click_timeout_add, SLIDE_CLICK_TIMEOUT_TIME);
            }
        }
    }

    return key_type;
}

static void ctmu_send_slide_key_type_event(u8 key_type)
{
    struct sys_event e;
    u8 event_ch = 0;
    switch (key_type) {
    case SHORT_CLICK:           //单击
        ctmu_short_click_handle(event_ch);
        break;
    case LONG_CLICK:            //长按

#if CTMU_CHECK_LONG_CLICK_BY_RES
        if (lp_touch_key_check_long_click_by_ctmu_res(event_ch)) {
            last_key_type = 0;
            return;
        }
#endif

        ctmu_long_click_handle(event_ch);
        break;
    //case LONG_HOLD_CLICK:       //长按保持
    //    ctmu_hold_click_handle(event_ch);
    //    break;
    case LONG_UP_CLICK:         //长按抬起
        ctmu_raise_click_handle(event_ch);
        break;
    case SLIDE_UP:              //向上滑动
        e.u.key.event = TOUCH_KEY_EVENT_SLIDE_UP;
        e.u.key.value = __this->config->slide_mode_key_value;
        printf("key.event = TOUCH_KEY_EVENT_SLIDE_UP\n");
        printf("key.value = %d\n", e.u.key.value);
        __ctmu_notify_key_event(&e, event_ch);
        break;
    case SLIDE_DOWN:            //向下滑动
        e.u.key.event = TOUCH_KEY_EVENT_SLIDE_DOWN;
        e.u.key.value = __this->config->slide_mode_key_value;
        printf("key.event = TOUCH_KEY_EVENT_SLIDE_DOWN\n");
        printf("key.value = %d\n", e.u.key.value);
        __ctmu_notify_key_event(&e, event_ch);
        break;
    default:
        break;
    }
}

#endif

u8 last_state = CTMU_P2M_CH1_OUT_EVENT;
extern u8 testbox_get_key_action_test_flag(void *priv);
static int lp_touch_key_testbox_test_res_handle(u8 ctmu_event);
void p33_ctmu_key_event_irq_handler()
{
    u8 ret = 0;
    u8 ctmu_event = P2M_CTMU_KEY_EVENT;
    u16 ch0_res = 0, ch1_res = 0, ch1_diff = 0;

    u8 ch_num = 0;
    if ((ctmu_event >= 0x58) && (ctmu_event <= 0x5e)) {
        ch_num = 1;
    }

    if (testbox_get_key_action_test_flag(NULL)) {
        ret = lp_touch_key_testbox_remote_test(ctmu_event);
        if (ret == true) {
            return;
        }
    }

    if (lp_touch_key_testbox_test_res_handle(ctmu_event)) {
        return;
    }

    /*log_debug("ctmu msg: 0x%x", ctmu_event);*/
    switch (ctmu_event) {
    case CTMU_P2M_CH0_DEBUG_EVENT:
        ch0_res = ((P2M_CTMU_CH0_H_RES << 8) | (P2M_CTMU_CH0_L_RES));

#if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
        lp_touch_key_online_debug_send(0, ch0_res);
#endif
        break;

    case CTMU_P2M_CH1_DEBUG_EVENT:
        ch1_res = ((P2M_CTMU_CH1_H_RES << 8) | P2M_CTMU_CH1_L_RES);

#if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
        lp_touch_key_online_debug_send(1, ch1_res);
#endif

#if TCFG_LP_EARTCH_KEY_ENABLE

#if !TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE

        if (__this->trim_flag) {

            ch1_diff = ((P2M_CTMU_H_DIFF_VALUE << 8) | P2M_CTMU_L_DIFF_VALUE);

            if (__this->trim_value == 0) {
                __this->trim_value = ch1_diff;
            } else {
                __this->trim_value = ((ch1_diff + __this->trim_value) >> 1);
            }
            if (__this->trim_flag++ > 20) {
                __this->trim_flag = 0;
                M2P_CTMU_CH1_RES_SEND = 0;
                int ret = syscfg_write(LP_KEY_CH1_TRIM_VALUE, &(__this->trim_value), sizeof(__this->trim_value));
                log_info("write ret = %d", ret);
                if (ret > 0) {
                    M2P_CTMU_TRIM_VALUE_L = (__this->trim_value & 0xFF);
                    M2P_CTMU_TRIM_VALUE_H = ((__this->trim_value >> 8) & 0xFF);
                    __this->inear_ok = 1;
#if TCFG_EARTCH_EVENT_HANDLE_ENABLE
                    eartch_state_update(EARTCH_STATE_TRIM_OK);
#endif
                    log_info("trim: %d\n", __this->inear_ok);
                    is_lpkey_active = 0;
                } else {
                    __this->inear_ok = 0;
#if TCFG_EARTCH_EVENT_HANDLE_ENABLE
                    eartch_state_update(EARTCH_STATE_TRIM_ERR);
#endif
                    log_info("trim: %d\n", __this->inear_ok);
                    is_lpkey_active = 0;
                }
            }
            log_debug("ch1 trim value: %d, res = %d", __this->trim_value, ch1_diff);
        }

#endif

        if (P2M_CTMU_CTMU_KEY_CNT != last_state) {
            last_state = P2M_CTMU_CTMU_KEY_CNT;
            if (last_state == CTMU_P2M_CH1_IN_EVENT) {
                if (__this->inear_ok) {
                    ctmu_ch1_event_handle(CH1_EAR_IN);
                }
            } else if (last_state == CTMU_P2M_CH1_OUT_EVENT) {
                if (__this->inear_ok) {
                    ctmu_ch1_event_handle(CH1_EAR_OUT);
                }
            }
        }

#endif

        break;

    case CTMU_P2M_CH0_LONG_EVENT:
        log_debug("CH%d: Long click", ch_num);
        is_lpkey_active = 0;
#if !TCFG_LP_SLIDE_KEY_ENABLE

#if CTMU_CHECK_LONG_CLICK_BY_RES
        if (lp_touch_key_check_long_click_by_ctmu_res(ch_num)) {
            return;
        }
#endif

        ctmu_long_click_handle(ch_num);
#endif
        break;
    case CTMU_P2M_CH1_FALLING_EVENT:
#if TCFG_LP_EARTCH_KEY_ENABLE
        log_debug("CH%d: FALLING", ch_num);
        break;
#endif
    case CTMU_P2M_CH0_FALLING_EVENT:
        log_debug("CH%d: FALLING", ch_num);
        is_lpkey_active = 1;

#if CTMU_CHECK_LONG_CLICK_BY_RES
        falling_res_avg[ch_num] = lp_touch_key_ctmu_res_buf_avg(ch_num);
        log_debug("falling_res_avg: %d", falling_res_avg[ch_num]);
#endif
        break;
    case CTMU_P2M_CH0_RAISING_EVENT:
    case CTMU_P2M_CH1_RAISING_EVENT:
        log_debug("CH%d: RAISING", ch_num);
        is_lpkey_active = 0;

#if CTMU_CHECK_LONG_CLICK_BY_RES
        lp_touch_key_ctmu_res_buf_clear(ch_num);
#endif

        if (!(__this->ch_init & BIT(ch_num))) {
            __this->ch_init |= BIT(ch_num);
            if (__this->ch_init != 0b11) {
                //load_p11_bank_code2ram(1, 1);
            }
            return;
        }
#if !TCFG_LP_SLIDE_KEY_ENABLE
        ctmu_raise_click_handle(ch_num);
#endif
        break;
    default:
        break;
    }

#if TCFG_LP_SLIDE_KEY_ENABLE
    u8 key_type = check_slide_key_type(ctmu_event, ch_num);
    if (key_type) {
        printf("CH%d: key_type = 0x%x\n", ch_num, key_type);
        ctmu_send_slide_key_type_event(key_type);
    }
#endif

}


void lp_touch_key_trace_lrc_hook(u32 lrc_hz)
{
    static u8 first_time = 0;
    if (first_time) {
        return;
    }
    first_time = 1;
    log_debug("%s", __func__);
    __this->lrc_hz = lrc_hz;

    u16 cnt = (CTMU_RESET_TIMER_PRD_VALUE * lrc_hz) / (1000L * 64);
#if CTMU_RESET_TIME_CONFIG
    M2P_LCTM_RESET_PCNT_PRD1 = cnt >> 8;
    M2P_LCTM_RESET_PCNT_PRD0 = cnt & 0xff;
    M2P_LCTM_RESET_PCNT_VALUE = (CTMU_RESET_TIME_CONFIG -  CTMU_LONG_CLICK_DELAY_TIME) / CTMU_RESET_TIMER_PRD_VALUE;
#else
    M2P_LCTM_RESET_PCNT_VALUE = 0;
#endif /* #if CTMU_RESET_TIME_CONFIG */
    log_debug("lp timer cnt = %d, lrc_hz = %d, RESET_PCNT = %d", cnt, lrc_hz, M2P_LCTM_RESET_PCNT_VALUE);

    if (__this->init) {
        u16 time_prd = 0;
        time_prd = (__this->lrc_hz * CTMU_TIME_BASE) / 1000 - 1;
        M2P_CTMU_BASE_TIME_PRD_L = (time_prd & 0xFF);
        M2P_CTMU_BASE_TIME_PRD_H = ((time_prd >> 8) & 0xFF);

        lp_touch_key_send_cmd(CTMU_M2P_UPDATE_BASE_TIME);
    }
}

u8 lp_touch_key_power_on_status()
{
    extern u8 power_reset_flag;
    u8 sfr = power_reset_flag;
    static u8 power_on_flag = 0;

    log_debug("P3_RST_SRC = %x, P2M_CTMU_CTMU_WKUP_MSG = 0x%x", sfr, P2M_CTMU_CTMU_WKUP_MSG);
    if ((sfr & BIT(0)) || (sfr & BIT(1))) {
        return 0;
    }

    if (P2M_CTMU_CTMU_WKUP_MSG & BIT(0)) {
        power_on_flag = 1;
        P2M_CTMU_CTMU_WKUP_MSG &= (~(BIT(0)));
    }

    return power_on_flag;
}

int lp_touch_key_reinit(void)
{
    for (u8 ch = 0; ch < LP_CTMU_CHANNEL_SIZE; ch ++) {
        if (__this->short_timer[ch] != 0xFFFF) {
            usr_timer_del(__this->short_timer[ch]);
            __this->short_timer[ch] = 0xFFFF;
        }
        if (__this->long_timer[ch] != 0xFFFF) {
            usr_timer_del(__this->long_timer[ch]);
            __this->long_timer[ch] = 0xFFFF;
        }
        __this->last_key[ch] = CTMU_KEY_NULL;
    }

    __this->ch_init = 0;

    load_p11_bank_code2ram(1, 0);

    //CTMU 初始化命令

    M2P_CTMU_INIT_FLAG = 0;
    lp_touch_key_send_cmd(CTMU_M2P_INIT);
    while (!M2P_CTMU_INIT_FLAG) {
        asm volatile("csync");
    }
    load_p11_bank_code2ram(1, 1);

    return 0;
}

void lp_touch_key_disable(void)
{
    log_debug("%s", __func__);

    u8 wait_cnt = 0;
    while (!(__this->ch_init & BIT(0))) {
        asm volatile("nop");
        os_time_dly(1);
        wait_cnt ++;
        if (wait_cnt > 3) {
            return;
        }
    }

    P2M_CTMU_CTMU_WKUP_MSG &= (~(BIT(1)));
    lp_touch_key_send_cmd(CTMU_M2P_DISABLE);
    wait_cnt = 0;
    while (!(P2M_CTMU_CTMU_WKUP_MSG & BIT(1))) {
        os_time_dly(1);
        wait_cnt ++;
        if (wait_cnt > 3) {
            break;
        }
    }
    __this->softoff_mode = LP_TOUCH_SOFTOFF_MODE_LEGACY;
}


void lp_touch_key_enable(void)
{
    log_debug("%s", __func__);
    lp_touch_key_send_cmd(CTMU_M2P_ENABLE);
    __this->softoff_mode = LP_TOUCH_SOFTOFF_MODE_ADVANCE;
}

void lp_touch_key_charge_mode_enter()
{
#if (!LP_TOUCH_KEY_CHARGE_MODE_SW_DISABLE)
    log_debug("%s", __func__);

    u8 wait_cnt = 0;
    while (!(__this->ch_init & BIT(0))) {
        asm volatile("nop");
        os_time_dly(1);
        wait_cnt ++;
        if (wait_cnt > 3) {
            return;
        }
    }
    //__this->ch_init = 0;
    P2M_CTMU_CTMU_WKUP_MSG &= (~(BIT(1)));
    lp_touch_key_send_cmd(CTMU_M2P_CHARGE_ENTER_MODE);
    wait_cnt = 0;
    while (!(P2M_CTMU_CTMU_WKUP_MSG & BIT(1))) {
        os_time_dly(1);
        wait_cnt ++;
        if (wait_cnt > 3) {
            break;
        }
    }
    __this->softoff_mode = LP_TOUCH_SOFTOFF_MODE_LEGACY;
#endif
}

void lp_touch_key_charge_mode_exit()
{
    lp_touch_key_reinit();

#if (!LP_TOUCH_KEY_CHARGE_MODE_SW_DISABLE)
    log_debug("%s", __func__);
    lp_touch_key_send_cmd(CTMU_M2P_CHARGE_EXIT_MODE);
    __this->softoff_mode = LP_TOUCH_SOFTOFF_MODE_ADVANCE;
#endif
}

//=============================================//
//NOTE: 该函数为进关机时被库里面回调
//在板级配置struct low_power_param power_param中变量lpctmu_en配置为TCFG_LP_TOUCH_KEY_ENABLE时:
//该函数在决定softoff进触摸模式还是普通模式:
//	return 1: 进触摸模式关机(LP_TOUCH_SOFTOFF_MODE_ADVANCE);
//	return 0: 进普通模式关机(触摸关闭)(LP_TOUCH_SOFTOFF_MODE_LEGACY);
//使用场景:
// 	1)在充电舱外关机, 需要触摸开机, 进带触摸关机模式;
// 	2)在充电舱内关机，可以关闭触摸模块, 进普通关机模式, 关机功耗进一步降低.
//=============================================//
u8 lp_touch_key_softoff_mode_query(void)
{
    return __this->softoff_mode;
}

//==================================================//
//==============  在线调节灵敏度参数表    ==========//
//==================================================//
#if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE
#include "online_db_deal.h"

//LP KEY在线调试工具版本号管理
const u8 lp_key_sdk_name[16] = "AC697N";
const u8 lp_key_bt_ver[4] 	     = {0, 0, 1, 0};
struct lp_key_ver_info {
    char sdkname[16];
    u8 lp_key_ver[4];
};

#if TCFG_LP_EARTCH_KEY_ENABLE
//版本号                     按键事件个数  							   按键事件个数
const char ch_content[] = {0x01, 'c', 'h', '0', '\0', 6, 0, 1, 2, 3, 4, 5, 'c', 'h', '1', '\0', 2, 10, 11}; //ch0 & ch1
#else
const char ch_content[] = {0x01, 'c', 'h', '0', '\0', 6, 0, 1, 2, 3, 4, 5, 'c', 'h', '1', '\0', 6, 0, 1, 2, 3, 4, 5};
#endif /* #if TCFG_LP_EARTCH_KEY_ENABLE */

enum {
    TOUCH_RECORD_GET_VERSION = 0x05,
    TOUCH_RECORD_GET_CH_SIZE = 0x0B,
    TOUCH_RECORD_GET_CH_CONTENT = 0x0C,
    TOUCH_RECORD_CHANGE_MODE = 0x0E,
    TOUCH_RECORD_COUNT = 0x200,
    TOUCH_RECORD_START,
    TOUCH_RECORD_STOP,
    ONLINE_OP_QUERY_RECORD_PACKAGE_LENGTH,
    TOUCH_CH_CFG_UPDATE  = 0x3000,
    TOUCH_CH_VOL_CFG_UPDATE  = 0x3001,
    TOUCH_CH_CFG_CONFIRM = 0x3100,
};

enum {
    LP_KEY_ONLINE_ST_INIT = 0,
    LP_KEY_ONLINE_ST_READY,
    LP_KEY_ONLINE_ST_CH_RES_DEBUG_START,
    LP_KEY_ONLINE_ST_CH_RES_DEBUG_STOP,
    LP_KEY_ONLINE_ST_CH_KEY_DEBUG_START,
    LP_KEY_ONLINE_ST_CH_KEY_DEBUG_CONFIRM,
};

//小机接收命令包格式, 使用DB_PKT_TYPE_TOUCH通道接收
typedef struct {
    int cmd_id;
    int mode;
    int data[];
} touch_receive_cmd_t;

//小机发送按键消息格式, 使用DB_PKT_TYPE_TOUCH通道发送
typedef struct {
    u32 cmd_id;
    u32 mode;
    u32 key_event;
} lp_touch_online_key_event_t;

typedef struct {
    u8 state;
    u8 current_record_ch;
    u16 res_packet;
    struct ch_adjust_table debug_cfg;
    lp_touch_online_key_event_t online_key_event;
    struct ch_ana_cfg ch_ana;
} lp_touch_key_online;

static lp_touch_key_online lp_key_online = {0};

static int lp_touch_key_online_debug_key_event_handle(u8 ch_index, struct sys_event *event)
{
    int err = 0;
    if ((lp_key_online.state == LP_KEY_ONLINE_ST_CH_KEY_DEBUG_START) && (lp_key_online.current_record_ch == ch_index)) {
        lp_key_online.online_key_event.cmd_id = 0x3100;
        lp_key_online.online_key_event.mode = 0;
        lp_key_online.online_key_event.key_event = event->u.key.event;
        log_debug("send %d event to PC", lp_key_online.online_key_event.key_event);
        err = app_online_db_send(DB_PKT_TYPE_TOUCH, (u8 *)(&(lp_key_online.online_key_event)), sizeof(lp_touch_online_key_event_t));
    }

    if ((lp_key_online.state == LP_KEY_ONLINE_ST_CH_KEY_DEBUG_CONFIRM) ||
        (lp_key_online.state <= LP_KEY_ONLINE_ST_READY)) {
        return 0;
    }

    return 1;
}

static int lp_touch_key_debug_reinit(u8 update_state)
{
    log_debug("%s, current_record_ch = %d", __func__, lp_key_online.current_record_ch);

    switch (update_state) {
    case LP_KEY_ONLINE_ST_CH_RES_DEBUG_START:
        if (lp_key_online.current_record_ch == 0) {
            M2P_CTMU_MSG &= ~(CTMU_INIT_CH1_DEBUG);
            M2P_CTMU_MSG |= CTMU_INIT_CH0_DEBUG;
            if (lp_key_online.ch_ana.isel) {
                M2P_CTMU_CH0_ANA_SEL = (lp_key_online.ch_ana.vhsel << 6) | (lp_key_online.ch_ana.vlsel << 4) | (lp_key_online.ch_ana.isel << 1);
            } else {
                M2P_CTMU_CH0_ANA_SEL = get_lpctmu_ana_level();
            }
            /* printf("M2P_CTMU_CH0_ANA_SEL = 0x%x\n", M2P_CTMU_CH0_ANA_SEL); */
        } else if (lp_key_online.current_record_ch == 1) {
            M2P_CTMU_MSG &= ~(CTMU_INIT_CH0_DEBUG);
            M2P_CTMU_MSG |= CTMU_INIT_CH1_DEBUG;
            if (lp_key_online.ch_ana.isel) {
                M2P_CTMU_CH1_ANA_SEL = (lp_key_online.ch_ana.vhsel << 6) | (lp_key_online.ch_ana.vlsel << 4) | (lp_key_online.ch_ana.isel << 1);
            } else {
                M2P_CTMU_CH1_ANA_SEL = get_lpctmu_ana_level();
            }
            /* printf("M2P_CTMU_CH1_ANA_SEL = 0x%x\n", M2P_CTMU_CH1_ANA_SEL); */
        }
        break;
    case LP_KEY_ONLINE_ST_CH_RES_DEBUG_STOP:
        M2P_CTMU_MSG &= ~(CTMU_INIT_CH0_DEBUG);
        M2P_CTMU_MSG &= ~(CTMU_INIT_CH1_DEBUG);
        break;
    case LP_KEY_ONLINE_ST_CH_KEY_DEBUG_START:
        M2P_CTMU_MSG &= ~(CTMU_INIT_CH0_DEBUG);
        M2P_CTMU_MSG &= ~(CTMU_INIT_CH1_DEBUG);
        if (lp_key_online.current_record_ch == 0) {
            M2P_CTMU_CH0_CFG0L = ((lp_key_online.debug_cfg.cfg0) & 0xFF);
            M2P_CTMU_CH0_CFG0H = (((lp_key_online.debug_cfg.cfg0) >> 8) & 0xFF);
            M2P_CTMU_CH0_CFG1L = (((lp_key_online.debug_cfg.cfg0) + 5) & 0xFF);
            M2P_CTMU_CH0_CFG1H = ((((lp_key_online.debug_cfg.cfg0) + 5) >> 8) & 0xFF);
            M2P_CTMU_CH0_CFG2L = ((lp_key_online.debug_cfg.cfg2) & 0xFF);
            M2P_CTMU_CH0_CFG2H = (((lp_key_online.debug_cfg.cfg2) >> 8) & 0xFF);
        } else if (lp_key_online.current_record_ch == 1) {
            M2P_CTMU_CH1_CFG0L = ((lp_key_online.debug_cfg.cfg0) & 0xFF);
            M2P_CTMU_CH1_CFG0H = (((lp_key_online.debug_cfg.cfg0) >> 8) & 0xFF);
            M2P_CTMU_CH1_CFG1L = (((lp_key_online.debug_cfg.cfg0) + 5) & 0xFF);
            M2P_CTMU_CH1_CFG1H = ((((lp_key_online.debug_cfg.cfg0) + 5) >> 8) & 0xFF);
            M2P_CTMU_CH1_CFG2L = ((lp_key_online.debug_cfg.cfg2) & 0xFF);
            M2P_CTMU_CH1_CFG2H = (((lp_key_online.debug_cfg.cfg2) >> 8) & 0xFF);
        }
        break;
    case LP_KEY_ONLINE_ST_CH_KEY_DEBUG_CONFIRM:
        M2P_CTMU_MSG &= ~(CTMU_INIT_CH0_DEBUG);
        M2P_CTMU_MSG &= ~(CTMU_INIT_CH1_DEBUG);
        break;
    default:
        break;
    }

    lp_touch_key_reinit();

    return 0;
}

static int lp_touch_key_online_debug_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size)
{
    int res_data = 0;
    touch_receive_cmd_t *touch_cmd;
    int err = 0;
    u8 parse_seq = ext_data[1];
    struct ch_adjust_table *receive_cfg;
    struct ch_ana_cfg *ana_cfg;
    struct lp_key_ver_info ver_info = {0};

    res_data = 4;
    log_debug("%s", __func__);
    put_buf(packet, size);
    put_buf(ext_data, ext_size);
    //memcpy(&touch_cmd, packet, sizeof(touch_receive_cmd_t));
    touch_cmd = (touch_receive_cmd_t *)packet;
    switch (touch_cmd->cmd_id) {
    /* case TOUCH_RECORD_COUNT: */
    /* log_debug("TOUCH_RECORD_COUNT"); */
    /* err = app_online_db_ack(parse_seq, (u8 *)&res_data, 4); */
    /* break; */
    case TOUCH_RECORD_START:
        log_debug("TOUCH_RECORD_START");
        err = app_online_db_ack(parse_seq, (u8 *)&res_data, 1); //该命令随便ack一个byte即可
        lp_key_online.state = LP_KEY_ONLINE_ST_CH_RES_DEBUG_START;
        lp_touch_key_debug_reinit(lp_key_online.state);
        break;
    case TOUCH_RECORD_STOP:
        log_debug("TOUCH_RECORD_STOP");
        app_online_db_ack(parse_seq, (u8 *)&res_data, 1); //该命令随便ack一个byte即可
        lp_key_online.state = LP_KEY_ONLINE_ST_CH_RES_DEBUG_STOP;
        lp_touch_key_debug_reinit(lp_key_online.state);
        break;
    /* case ONLINE_OP_QUERY_RECORD_PACKAGE_LENGTH: */
    /* log_debug("ONLINE_OP_QUERY_RECORD_PACKAGE_LENGTH"); */
    /* err = app_online_db_ack(parse_seq, (u8 *)&res_data, 4); //回复对应的通道数据长度 */
    /* break; */
    case TOUCH_CH_CFG_UPDATE:
        log_debug("TOUCH_CH_CFG_UPDATE");
        app_online_db_ack(parse_seq, (u8 *)"OK", 2); //回"OK"字符串
        lp_key_online.state = LP_KEY_ONLINE_ST_CH_KEY_DEBUG_START;

        receive_cfg = (struct ch_adjust_table *)(touch_cmd->data);
        lp_key_online.debug_cfg.cfg0 = receive_cfg->cfg0;
        lp_key_online.debug_cfg.cfg1 = receive_cfg->cfg1;
        lp_key_online.debug_cfg.cfg2 = receive_cfg->cfg2;
        log_debug("update, cfg0 = %d, cfg1 = %d, cfg2 = %d", lp_key_online.debug_cfg.cfg0, lp_key_online.debug_cfg.cfg1, lp_key_online.debug_cfg.cfg2);
        lp_touch_key_debug_reinit(lp_key_online.state);
        break;
    case TOUCH_CH_VOL_CFG_UPDATE:
        log_debug("TOUCH_CH_VOL_CFG_UPDATE");
        app_online_db_ack(parse_seq, (u8 *)"OK", 2); //回"OK"字符串
        ana_cfg = (struct ch_ana_cfg *)(touch_cmd->data);
        lp_key_online.ch_ana.isel = ana_cfg->isel;
        lp_key_online.ch_ana.vhsel = ana_cfg->vhsel;
        lp_key_online.ch_ana.vlsel = ana_cfg->vlsel;
        log_debug("update, isel = %d, vhsel = %d, vlsel = %d", lp_key_online.ch_ana.isel, lp_key_online.ch_ana.vhsel, lp_key_online.ch_ana.vlsel);
        break;
    case TOUCH_CH_CFG_CONFIRM:
        log_debug("TOUCH_CH_CFG_CONFIRM");
        app_online_db_ack(parse_seq, (u8 *)"OK", 2); //回"OK"字符串
        lp_key_online.state = LP_KEY_ONLINE_ST_CH_KEY_DEBUG_CONFIRM;
        break;
    case TOUCH_RECORD_GET_VERSION:
        log_debug("TOUCH_RECORD_GET_VERSION");
        memcpy(ver_info.sdkname, lp_key_sdk_name, sizeof(lp_key_sdk_name));
        memcpy(ver_info.lp_key_ver, lp_key_bt_ver, sizeof(lp_key_bt_ver));
        app_online_db_ack(parse_seq, (u8 *)(&ver_info), sizeof(ver_info)); //回复版本号数据结构
        break;
    case TOUCH_RECORD_GET_CH_SIZE:
        log_debug("TOUCH_RECORD_GET_CH_SIZE");
        res_data = sizeof(ch_content);
        err = app_online_db_ack(parse_seq, (u8 *)&res_data, 4); //回复对应的通道数据长度
        break;
    case TOUCH_RECORD_GET_CH_CONTENT:
        log_debug("TOUCH_RECORD_GET_CH_CONTENT");
        app_online_db_ack(parse_seq, (u8 *)(&ch_content), sizeof(ch_content));
        break;
    case TOUCH_RECORD_CHANGE_MODE:
        log_debug("TOUCH_RECORD_CHANGE_MODE, cmd_mode = %d", touch_cmd->mode);
        lp_key_online.current_record_ch = touch_cmd->mode;
        lp_key_online.ch_ana.isel = 0;
        app_online_db_ack(parse_seq, (u8 *)"OK", 2); //回"OK"字符串
        break;
    default:
        break;
    }

    return 0;
}

static int lp_touch_key_online_debug_send(u8 ch, u16 val)
{
    int err = 0;

    putchar('s');
    if (lp_key_online.state == LP_KEY_ONLINE_ST_CH_RES_DEBUG_START) {
        lp_key_online.res_packet = val;
        err = app_online_db_send(DB_PKT_TYPE_DAT_CH0, (u8 *)(&(lp_key_online.res_packet)), 2);
    }

    return err;
}

static int lp_touch_key_online_debug_init(void)
{
    log_debug("%s", __func__);
    app_online_db_register_handle(DB_PKT_TYPE_TOUCH, lp_touch_key_online_debug_parse);
    lp_key_online.state = LP_KEY_ONLINE_ST_READY;

    return 0;
}

int lp_touch_key_online_debug_exit(void)
{
    return 0;
}

#endif /* #if TCFG_LP_TOUCH_KEY_BT_TOOL_ENABLE */

static u8 lpkey_idle_query(void)
{
    return !is_lpkey_active;
}
#if TCFG_LP_TOUCH_KEY_ENABLE
REGISTER_LP_TARGET(key_lp_target) = {
    .name = "lpkey",
    .is_idle = lpkey_idle_query,
};
#endif /* #if !TCFG_LP_TOUCH_KEY_ENABLE */


//======================================================//
//              测试盒变化量测试命令接收                //
//======================================================//
#define LP_TOUCH_TEST_TIMEOUT_CONFIG 			8000 //ms
#define LP_TOUCH_TEST_END_DELAY_COUNTER 		20

extern int lp_touch_key_testbox_test_cmd_send(void *priv);

enum LP_TOUCH_KEY_TESTBOX_CMD_TABLE {
    LP_TOUCH_KEY_TESTBOX_CMD_TESTMODE_NONE = 0,
    LP_TOUCH_KEY_TESTBOX_CMD_TESTMODE_ENTER = 'T', //84
    LP_TOUCH_KEY_TESTBOX_CMD_TEST_KEY,			   //85
    LP_TOUCH_KEY_TESTBOX_CMD_TEST_TIMEOUT_REPORT,  //86, 测试盒超时, 请求测试结果
    LP_TOUCH_KEY_TESTBOX_CMD_TESTMODE_EXIT,        //87
};

enum LP_TOUCH_KEY_TESTBOX_ERR_TABLE {
    LP_TOUCH_KEY_TESTBOX_ERR_NONE = 'O', 			//79, 测试通过
    LP_TOUCH_KEY_TESTBOX_ERR_DOUBLE_KEY_NOT_HIT, 	//80, 未检测到双击
    LP_TOUCH_KEY_TESTBOX_ERR_DETLA, 				//81, 触摸变化量太小
};

typedef struct lp_touch_key_test_cmd {
    u8 cmd;
} LP_TOUCH_TESTBOX_CMD;


struct lp_touch_key_test_report {
    u8 result;
    u16 res_max;
    u16 res_min;
    u16 res_delta;
    u16 res_percent;
    u8 fall_cnt;
    u8 raise_cnt;
};


struct lp_touch_key_test_statis {
    u16 last_value;
    u16 probe_max;
    u16 probe_min;
    u16 res_max0;
    u16 res_min0;
    u16 res_max1;
    u16 res_min1;
    u16 end_cnt;
    u8 double_key_hit;
    u8 cur_status;
    u8 fall_cnt;
    u8 raise_cnt;
};

struct lp_touch_key_test_handle {
    u8 cur_test_status;
    struct lp_touch_key_test_statis statis;
    u32 timeout_timer;
};

static struct lp_touch_key_test_handle testbox_test_handle = {0};

static int lp_touch_key_testbox_reinit(u8 mode)
{
    static u8 ctmu_msg_old = 0;
    static u8 ch0_res_send_old = 0;
    static u8 ch1_res_send_old = 0;

    if (mode) {
        ctmu_msg_old = M2P_CTMU_MSG;
        ch0_res_send_old = M2P_CTMU_CH0_RES_SEND;
        ch1_res_send_old = M2P_CTMU_CH1_RES_SEND;
#if CTMU_TESTBOX_TEST_CH
        M2P_CTMU_MSG &= ~CTMU_INIT_CH0_DEBUG;
        M2P_CTMU_MSG |=  CTMU_INIT_CH1_DEBUG;
        M2P_CTMU_CH0_RES_SEND = 0;
        M2P_CTMU_CH1_RES_SEND = 1;
#else
        M2P_CTMU_MSG |=  CTMU_INIT_CH0_DEBUG;
        M2P_CTMU_MSG &= ~CTMU_INIT_CH1_DEBUG;
        M2P_CTMU_CH0_RES_SEND = 1;
        M2P_CTMU_CH1_RES_SEND = 0;
#endif
    } else {
        M2P_CTMU_MSG = ctmu_msg_old;
        M2P_CTMU_CH0_RES_SEND = ch0_res_send_old;
        M2P_CTMU_CH1_RES_SEND = ch1_res_send_old;
    }

    for (u8 ch = 0; ch < LP_CTMU_CHANNEL_SIZE; ch ++) {
        if (__this->short_timer[ch] != 0xFFFF) {
            usr_timer_del(__this->short_timer[ch]);
            __this->short_timer[ch] = 0xFFFF;
        }
        if (__this->long_timer[ch] != 0xFFFF) {
            usr_timer_del(__this->long_timer[ch]);
            __this->long_timer[ch] = 0xFFFF;
        }
        __this->last_key[ch] = CTMU_KEY_NULL;
    }

    __this->ch_init = 0;

    load_p11_bank_code2ram(1, 0);

    //CTMU 初始化命令
    M2P_CTMU_INIT_FLAG = 0;
    lp_touch_key_send_cmd(CTMU_M2P_INIT);
    while (!M2P_CTMU_INIT_FLAG) {
        asm volatile("csync");
    }
    load_p11_bank_code2ram(1, 1);

    return 0;
}

static void lp_touch_key_testbox_test_timeout_handle(void *priv)
{
    log_info("==== lp key test local timeout ====");
    if ((testbox_test_handle.cur_test_status == LP_TOUCH_KEY_TESTBOX_CMD_TESTMODE_ENTER) ||
        (testbox_test_handle.cur_test_status == LP_TOUCH_KEY_TESTBOX_CMD_TEST_KEY)) {
        testbox_test_handle.timeout_timer = 0;
        lp_touch_key_testbox_reinit(0);
        testbox_test_handle.cur_test_status = LP_TOUCH_KEY_TESTBOX_CMD_TESTMODE_NONE;
    }

    return;
}

/* --------------------------------------------------------------------------
 * @brief 蓝牙lmp提供该接口, 用于给测试盒发送测试结果
 * @param priv
 * @return
---------------------------------------------------------------------------- */
int __attribute__((weak)) lp_touch_key_testbox_test_cmd_send(void *priv)
{
    return 0;
}

static int lp_touch_key_testbox_test_report(void)
{
    struct lp_touch_key_test_report report;
    u16 detla0, detla1, target_delta;
    u32 max, min;
    if (testbox_test_handle.statis.double_key_hit) {
        detla0 = testbox_test_handle.statis.res_max0 - testbox_test_handle.statis.res_min0;
        detla1 = testbox_test_handle.statis.res_max1 - testbox_test_handle.statis.res_min1;
        if (detla1 > detla0) {
            report.res_max = testbox_test_handle.statis.res_max1;
            report.res_min = testbox_test_handle.statis.res_min1;
        } else {
            report.res_max = testbox_test_handle.statis.res_max0;
            report.res_min = testbox_test_handle.statis.res_min0;
        }
    } else {
        report.res_max = testbox_test_handle.statis.probe_max;
        report.res_min = testbox_test_handle.statis.probe_min;
    }

    report.res_delta = report.res_max - report.res_min;
    max = report.res_max;
    min = report.res_min;
    if ((max && min) && (max > min)) {
        report.res_percent = (max - min) * 100 / max;
    } else {
        log_info("max err");
        report.res_percent = 0;
    }
    report.fall_cnt = testbox_test_handle.statis.fall_cnt;
    report.raise_cnt = testbox_test_handle.statis.raise_cnt;
    //
    //u8 index = __this->config->ch0.sensitivity;
    target_delta = ch_sensitivity_table[2 + CTMU_TESTBOX_TEST_CH * 10].cfg2; //取工具生成的1级灵敏度, 标定过良好样机的30%, 小于30%, 确认为不良
    if (/* (report.res_delta < 100) || */((report.res_delta) < target_delta)) {
        report.result = LP_TOUCH_KEY_TESTBOX_ERR_DETLA;
    } else if (testbox_test_handle.statis.double_key_hit == 0) {
        report.result = LP_TOUCH_KEY_TESTBOX_ERR_DOUBLE_KEY_NOT_HIT;
    } else {
        report.result = LP_TOUCH_KEY_TESTBOX_ERR_NONE;
    }

    log_info("======= touch test info report =======");
    log_info("result    = %d", report.result);
    log_info("res_max   = %d", report.res_max);
    log_info("res_min   = %d", report.res_min);
    log_info("res_delta = %d", report.res_delta);
    log_info("res_percent       = %d", report.res_percent);
    log_info("report->fall_cnt  = %d", report.fall_cnt);
    log_info("report->raise_cnt = %d", report.raise_cnt);

    lp_touch_key_testbox_test_cmd_send(&report);

    return 0;
}

static void lp_touch_key_testbox_test_res_statis(u16 value)
{
    if (testbox_test_handle.statis.probe_max == 0) {
        testbox_test_handle.statis.probe_max = value;
        testbox_test_handle.statis.probe_min = value;
    } else {
        testbox_test_handle.statis.probe_min = MIN(value, testbox_test_handle.statis.probe_min);
        testbox_test_handle.statis.probe_max = MAX(value, testbox_test_handle.statis.probe_max);
    }

    if (testbox_test_handle.statis.cur_status == 'L') {
        if (testbox_test_handle.statis.fall_cnt == 1) {
            if (testbox_test_handle.statis.res_min0 == 0) {
                testbox_test_handle.statis.res_min0 = value;
            } else {
                testbox_test_handle.statis.res_min0 = MIN(value, testbox_test_handle.statis.res_min0);
            }
        } else if (testbox_test_handle.statis.fall_cnt == 2) {
            if (testbox_test_handle.statis.res_min1 == 0) {
                testbox_test_handle.statis.res_min1 = value;
            } else {
                testbox_test_handle.statis.res_min1 = MIN(value, testbox_test_handle.statis.res_min1);
            }
        }
    } else if (testbox_test_handle.statis.cur_status == 'H') {
        if (testbox_test_handle.statis.raise_cnt == 1) {
            if (testbox_test_handle.statis.res_max0 == 0) {
                testbox_test_handle.statis.res_max0 = value;
            } else {
                testbox_test_handle.statis.res_max0 = MAX(value, testbox_test_handle.statis.res_max0);
            }
        } else if (testbox_test_handle.statis.fall_cnt == 2) {
            testbox_test_handle.statis.end_cnt++;
            if (testbox_test_handle.statis.end_cnt == LP_TOUCH_TEST_END_DELAY_COUNTER) {
                testbox_test_handle.statis.double_key_hit = 1;
                lp_touch_key_testbox_test_report();
            }

            if (testbox_test_handle.statis.end_cnt > LP_TOUCH_TEST_END_DELAY_COUNTER) {
                return;
            }

            if (testbox_test_handle.statis.res_max1 == 0) {
                testbox_test_handle.statis.res_max1 = value;
            } else {
                testbox_test_handle.statis.res_max1 = MAX(value, testbox_test_handle.statis.res_max1);
            }
        }
    }
}

static int lp_touch_key_testbox_test_res_handle(u8 ctmu_event)
{
    u16 res = 0;

    putchar('+');

    if (testbox_test_handle.cur_test_status == LP_TOUCH_KEY_TESTBOX_CMD_TESTMODE_NONE) {
        return 0;
    }
    if (testbox_test_handle.cur_test_status != LP_TOUCH_KEY_TESTBOX_CMD_TEST_KEY) {
        return 1;
    }
    switch (ctmu_event) {
    case CTMU_P2M_CH0_FALLING_EVENT:
    case CTMU_P2M_CH1_FALLING_EVENT:
        testbox_test_handle.statis.fall_cnt++;
        testbox_test_handle.statis.cur_status = 'L';
        log_info("Key Down");
        break;
    case CTMU_P2M_CH0_RAISING_EVENT:
    case CTMU_P2M_CH1_RAISING_EVENT:
        testbox_test_handle.statis.raise_cnt++;
        testbox_test_handle.statis.cur_status = 'H';
        log_info("Key Up");
        break;
    case CTMU_P2M_CH0_DEBUG_EVENT:
        res = ((P2M_CTMU_CH0_H_RES << 8) | (P2M_CTMU_CH0_L_RES));
        break;
    case CTMU_P2M_CH1_DEBUG_EVENT:
        res = ((P2M_CTMU_CH1_H_RES << 8) | (P2M_CTMU_CH1_L_RES));
        break;
    default:
        break;
    }

    if (res) {
        lp_touch_key_testbox_test_res_statis(res);
    }

    return 1;
}

/* ---------------------------------------------------------------------------- */
/**
 * @brief 蓝牙lmp层回调函数, 用于接收测试盒
 *
 * @param priv
 *
 * @return
 */
/* ---------------------------------------------------------------------------- */
int lp_touch_key_receive_cmd_from_testbox(void *priv)
{
    LP_TOUCH_TESTBOX_CMD *test_cmd = (LP_TOUCH_TESTBOX_CMD *)priv;
    if (__this->init == 0) {
        return 0;
    }

    log_info("%s: cmd = %d", __func__, test_cmd->cmd);

    switch (test_cmd->cmd) {
    case LP_TOUCH_KEY_TESTBOX_CMD_TESTMODE_ENTER:
        if (testbox_test_handle.cur_test_status == LP_TOUCH_KEY_TESTBOX_CMD_TESTMODE_NONE) {
            lp_touch_key_testbox_reinit(1);
            testbox_test_handle.cur_test_status = LP_TOUCH_KEY_TESTBOX_CMD_TESTMODE_ENTER;
        }
        break;
    case LP_TOUCH_KEY_TESTBOX_CMD_TEST_KEY:
        if ((testbox_test_handle.cur_test_status == LP_TOUCH_KEY_TESTBOX_CMD_TESTMODE_ENTER) ||
            (testbox_test_handle.cur_test_status == LP_TOUCH_KEY_TESTBOX_CMD_TEST_KEY)) {
            local_irq_disable();
            memset((u8 *)(&(testbox_test_handle.statis)), 0, sizeof(testbox_test_handle.statis));
            testbox_test_handle.cur_test_status = LP_TOUCH_KEY_TESTBOX_CMD_TEST_KEY;
            if (testbox_test_handle.timeout_timer) {
                sys_timer_modify(testbox_test_handle.timeout_timer, LP_TOUCH_TEST_TIMEOUT_CONFIG);
            } else {
                testbox_test_handle.timeout_timer = sys_timeout_add(NULL, lp_touch_key_testbox_test_timeout_handle, LP_TOUCH_TEST_TIMEOUT_CONFIG);
            }
            local_irq_enable();
        }
        break;
#if 0
    case LP_TOUCH_KEY_TESTBOX_CMD_TEST_KEY_RETRY:
        if (testbox_test_handle.cur_test_status == LP_TOUCH_KEY_TESTBOX_CMD_TEST_KEY) {
            local_irq_disable();
            memset((u8 *)(&(testbox_test_handle.statis)), 0, sizeof(testbox_test_handle.statis));
            if (testbox_test_handle.timeout_timer) {
                sys_timer_modify(testbox_test_handle.timeout_timer, LP_TOUCH_TEST_TIMEOUT_CONFIG);
            } else {
                testbox_test_handle.timeout_timer = sys_timeout_add(NULL, lp_touch_key_testbox_test_timeout_handle, LP_TOUCH_TEST_TIMEOUT_CONFIG);
            }
            local_irq_enable();
        }
        break;
#endif /* #if 0 */
    case LP_TOUCH_KEY_TESTBOX_CMD_TESTMODE_EXIT:
        if ((testbox_test_handle.cur_test_status == LP_TOUCH_KEY_TESTBOX_CMD_TESTMODE_ENTER) ||
            (testbox_test_handle.cur_test_status == LP_TOUCH_KEY_TESTBOX_CMD_TEST_KEY)) {
            lp_touch_key_testbox_reinit(0);
            testbox_test_handle.cur_test_status = LP_TOUCH_KEY_TESTBOX_CMD_TESTMODE_NONE;
            if (testbox_test_handle.timeout_timer) {
                sys_timeout_del(testbox_test_handle.timeout_timer);
                testbox_test_handle.timeout_timer = 0;
            }
        }
        break;
    case LP_TOUCH_KEY_TESTBOX_CMD_TEST_TIMEOUT_REPORT:
        if (testbox_test_handle.cur_test_status == LP_TOUCH_KEY_TESTBOX_CMD_TEST_KEY) {
            testbox_test_handle.cur_test_status = LP_TOUCH_KEY_TESTBOX_CMD_TEST_TIMEOUT_REPORT;
            lp_touch_key_testbox_test_report();
            testbox_test_handle.cur_test_status = LP_TOUCH_KEY_TESTBOX_CMD_TEST_KEY;
        }
        break;
    default:
        break;
    }

    return 0;
}


/* ---------------------------------------------------------------------------- */
/**
 * @brief 模拟测试盒流程
 */
/* ---------------------------------------------------------------------------- */
#if 0// for test

#define LP_TEST_TASK_NAME 			"lp_test"
static u16 timeout_id = 0;
static u8 retry_cnt = 0;

enum TESTBOX_LOCAL_CMD {
    TESTBOX_LOCAL_CMD_REPORT,
    TESTBOX_LOCAL_CMD_TEST_END,
};

static void lp_touch_key_testbox_test_report_get(void *priv)
{
    LP_TOUCH_TESTBOX_CMD test_cmd;
    if (priv == NULL) {
        return;
    }

    struct lp_touch_key_test_report *info = (struct lp_touch_key_test_report *)priv;

    os_taskq_post_msg(LP_TEST_TASK_NAME, 8,
                      TESTBOX_LOCAL_CMD_REPORT,
                      info->result,
                      info->res_max,
                      info->res_min,
                      info->res_delta,
                      info->res_percent,
                      info->fall_cnt,
                      info->raise_cnt);
}

static void lp_touch_key_testbox_timeout(void *priv)
{
    LP_TOUCH_TESTBOX_CMD test_cmd;

    log_info("===== Touch Key Test Timeout =====");
    //查询测试结果:
    test_cmd.cmd = LP_TOUCH_KEY_TESTBOX_CMD_TEST_TIMEOUT_REPORT;
    lp_touch_key_receive_cmd_from_testbox(&test_cmd);
}

static void lp_touch_key_testbox_testmode(void *priv)
{
    LP_TOUCH_TESTBOX_CMD test_cmd;
    test_cmd.cmd = LP_TOUCH_KEY_TESTBOX_CMD_TEST_KEY;
    lp_touch_key_receive_cmd_from_testbox(&test_cmd);
    log_info("===== Please Double Click =====");
    timeout_id = sys_timeout_add(NULL, lp_touch_key_testbox_timeout, 4000);
}

static void lp_touch_key_testbox_enter(void)
{
    LP_TOUCH_TESTBOX_CMD test_cmd;
    test_cmd.cmd = LP_TOUCH_KEY_TESTBOX_CMD_TESTMODE_ENTER;
    log_info("===== Touch Key Test Enter =====");
    lp_touch_key_receive_cmd_from_testbox(&test_cmd);
    sys_timeout_add(NULL, lp_touch_key_testbox_testmode, 200);
}

static void testbox_report(void *priv)
{
    u32 *info = (u32 *)priv;
    LP_TOUCH_TESTBOX_CMD test_cmd;

    if (timeout_id) {
        sys_timeout_del(timeout_id);
        timeout_id = 0;
    }

    struct lp_touch_key_test_report report;
    report.result 		= info[0];
    report.res_max 		= info[1];
    report.res_min 		= info[2];
    report.res_delta 	= info[3];
    report.res_percent 	= info[4];
    report.fall_cnt 	= info[5];
    report.raise_cnt 	= info[6];

    log_info("===== Touch Key Test Report =====");
    log_info("info->result  = %d", report.result);
    log_info("info->res_max = %d", report.res_max);
    log_info("info->res_min = %d", report.res_min);
    log_info("info->res_delta   = %d", report.res_delta);
    log_info("info->res_percent = %d", report.res_percent);
    log_info("info->fall_cnt  = %d", report.fall_cnt);
    log_info("info->raise_cnt = %d", report.raise_cnt);

    if (report.result == LP_TOUCH_KEY_TESTBOX_ERR_NONE) {
        log_info("LP Key Test OK");
    } else if (report.result == LP_TOUCH_KEY_TESTBOX_ERR_DOUBLE_KEY_NOT_HIT) {
        log_info("Double Key Not Hit");
    } else if (report.result == LP_TOUCH_KEY_TESTBOX_ERR_DETLA) {
        log_info("Touch Delta Less");
    }

    if ((report.result != LP_TOUCH_KEY_TESTBOX_ERR_NONE) && (retry_cnt < 2)) {
        log_info("test err, retry_cnt = %d, try again", retry_cnt);
        test_cmd.cmd = LP_TOUCH_KEY_TESTBOX_CMD_TEST_KEY;
        lp_touch_key_receive_cmd_from_testbox(&test_cmd);
        log_info("===== Please Double Click =====");
        timeout_id = sys_timeout_add(NULL, lp_touch_key_testbox_timeout, 4000);
        retry_cnt++;
    } else {
        log_info("===== Touch Key Test End =====");
        retry_cnt = 0;
        test_cmd.cmd = LP_TOUCH_KEY_TESTBOX_CMD_TESTMODE_EXIT;
        lp_touch_key_receive_cmd_from_testbox(&test_cmd);
        os_taskq_post_msg(LP_TEST_TASK_NAME, 1, TESTBOX_LOCAL_CMD_TEST_END);
    }
}

static void touch_key_test(void *priv)
{
    int res = 0;
    int msg[30];

    log_info("==== Touch Key Test Task ====");

    os_time_dly(1000);

    lp_touch_key_testbox_enter();

    while (1) {
        res = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        if (res == OS_TASKQ) {
            switch (msg[1]) {
            case TESTBOX_LOCAL_CMD_REPORT:
                testbox_report(&(msg[2]));
                break;
            case TESTBOX_LOCAL_CMD_TEST_END:
                os_time_dly(500);
                lp_touch_key_testbox_enter();
                break;
            default:
                break;
            }
        }
    }
}

void lp_touch_key_testbox_test(void)
{
    os_task_create(touch_key_test, NULL, 1, 256, 256, LP_TEST_TASK_NAME);
}

#endif /* #if 0// for test */



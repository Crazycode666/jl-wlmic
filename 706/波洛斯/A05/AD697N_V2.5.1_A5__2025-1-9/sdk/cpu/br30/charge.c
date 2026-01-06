#include "asm/clock.h"
#include "timer.h"
#include "asm/power/p33.h"
#include "asm/charge.h"
#include "asm/adc_api.h"
#include "uart.h"
#include "device/device.h"
#include "asm/power_interface.h"
/*#include "power/power_hw.h"*/
#include "system/event.h"
#include "asm/efuse.h"
#include "gpio.h"
#include "syscfg_id.h"
#include "app_config.h"
#if TCFG_CHARGE_CALIBRATION_ENABLE
#include "asm/charge_calibration.h"
#endif

#define LOG_TAG_CONST   CHARGE
#define LOG_TAG         "[CHARGE]"
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#include "debug.h"

#define IO_PORT_LDOIN   LINDT_IN//IO编号

typedef struct _CHARGE_VAR {
    struct charge_platform_data *data;
    volatile u8 charge_online_flag;
    volatile u8 init_ok;
    volatile u8 need_trim_vbat;
    volatile int ldo5v_timer;
    volatile int charge_timer;
    volatile int power_sw_timer;
    volatile int cc_timer;      //涓流切恒流的usr timer
    volatile int full_timer;    //满电过压检测定时器
#if TCFG_CHARGE_CALIBRATION_ENABLE
    calibration_result result;
    u8 result_flag; //校准数据有效标记
    u8 enter_flag;  //进入过模式标记
    u8 full_time;   //标记第几次触发充满
    u8 full_lev;    //满电档位记录
    int vbg_lvl;    //记录原始的VBG档位
    int vddio_lvl;  //记录原始的VDDIO档位
    int vdc13_lvl;  //记录原始的VDC13档位
    int evdd_lvl;   //记录原始的EVDD档位
    int rvdd_lvl;   //记录原始的RVDD档位
    int dvdd_lvl;   //记录原始的DVDD档位
#endif
} CHARGE_VAR;

#define __this 	(&charge_var)
static CHARGE_VAR charge_var;
extern void charge_set_callback(void (*wakup_callback)(void), void (*sub_callback)(void));
static u8 charge_flag;

#define BIT_LDO5V_IN		BIT(0)
#define BIT_LDO5V_OFF		BIT(1)
#define BIT_LDO5V_ERR		BIT(2)
void charge_wakeup_isr(void);

u8 get_charge_poweron_en(void)
{
    return __this->data->charge_poweron_en;
}

void charge_check_and_set_pinr(u8 level)
{
    u8 pinr_io;
    if (P33_CON_GET(P3_PINR_CON) & BIT(0)) {
        pinr_io = P33_CON_GET(P3_PORT_SEL0);
        if (pinr_io == IO_PORT_LDOIN) {
            if (level == 0) {
                P33_CON_SET(P3_PINR_CON, 2, 1, 0);
            } else {
                P33_CON_SET(P3_PINR_CON, 2, 1, 1);
            }
        }
    }
}

static void udelay(u32 usec)
{
    JL_TIMER0->CON = BIT(14);
    JL_TIMER0->CNT = 0;
    JL_TIMER0->PRD = clk_get("timer") / 1000000L  * usec; //1us
    JL_TIMER0->CON = BIT(0) | BIT(2) | BIT(6); //sys clk
    while ((JL_TIMER0->CON & BIT(15)) == 0);
    JL_TIMER0->CON = BIT(14);
}

static u8 check_charge_state(void)
{
    u8 online_cnt = 0;
    u8 i = 0;

    __this->charge_online_flag = 0;

    for (i = 0; i < 20; i++) {
        if (LVCMP_DET_GET() || LDO5V_DET_GET()) {
            online_cnt++;
        }
        udelay(1000);
    }
    log_info("online_cnt = %d\n", online_cnt);
    if (online_cnt > 5) {
        __this->charge_online_flag = 1;
    }

    return __this->charge_online_flag;
}

void set_charge_online_flag(u8 flag)
{
    __this->charge_online_flag = flag;
}

u8 get_charge_online_flag(void)
{
    return __this->charge_online_flag;
}

u8 get_ldo5v_online_hw(void)
{
    return LDO5V_DET_GET();
}

u8 get_lvcmp_det(void)
{
    return LVCMP_DET_GET();
}

u8 get_ldo5v_pulldown_en(void)
{
    if (__this->data) {
        return __this->data->ldo5v_pulldown_en;
    }
    return 0;
}

u8 get_ldo5v_pulldown_res(void)
{
    if (__this->data) {
        return __this->data->ldo5v_pulldown_lvl;
    }
    return CHARGE_PULLDOWN_200K;
}
void charge_udelay(u32 us)
{
    udelay(us);
}

void charge_event_to_user(u8 event)
{
    struct sys_event e;
    e.type = SYS_DEVICE_EVENT;
    e.arg  = (void *)DEVICE_EVENT_FROM_CHARGE;
    e.u.dev.event = event;
    e.u.dev.value = 0;
    sys_event_notify(&e);
}

#if TCFG_CHARGE_CALIBRATION_ENABLE

//恒流模式才需要校准电流
static void charge_enter_calibration_mode(void)
{
    u8 is_charging;
    int vbg_t, vddio_t, vdc13_t, evdd_t, rvdd_t, dvdd_t, vbat_trim, vbg_aim;
    if (!__this->result_flag) {
        return;
    }

    if (__this->enter_flag) {
        return;
    }

    __this->enter_flag = 1;

    //调整电压前,先关闭时钟切换
    clk_set_en(0);

    is_charging = IS_CHARGE_EN();
    //先关闭充电
    CHGBG_EN(0);
    CHARGE_EN(0);

    vbg_t   = __this->vbg_lvl     = MVBG_GET() * 10;
    vddio_t = __this->vddio_lvl   = GET_VDDIOM_VOL() * 10;
    vdc13_t = __this->vdc13_lvl   = GET_VD13_VOL_SEL() * 10;
    evdd_t  = __this->evdd_lvl    = GET_EVD_VOL() * 10;
    rvdd_t  = __this->rvdd_lvl    = GET_RVDD_VOL_SEL() * 10;
    dvdd_t  = __this->dvdd_lvl    = GET_SYSVDD_VOL_SEL() * 10;
    __this->full_lev = GET_CHARGE_FULL_SET();
    vbg_aim = __this->result.vbg_lev * 10;

    log_info("charge_enter_calibration_mode\n");
    log_info("vbg: %d, vddio: %d, vdc13: %d, evdd: %d, rvdd: %d, dvdd: %d\n",
             MVBG_GET(), GET_VDDIOM_VOL(), GET_VD13_VOL_SEL(),
             GET_EVD_VOL(), GET_RVDD_VOL_SEL(), GET_SYSVDD_VOL_SEL());

    while (vbg_aim != vbg_t) {
        if (vbg_aim > vbg_t) {
            vbg_t = charge_voltage_calc(vbg_t, VBG_LEV_DIFF, VBG_LEV_MAX, 0);
            vddio_t = charge_voltage_calc(vddio_t, -VDDIO_LEV_DIFF, VDDIO_LEV_MAX, 0);
            vdc13_t = charge_voltage_calc(vdc13_t, -VDC13_LEV_DIFF, VDC13_LEV_MAX, 0);
            evdd_t = charge_voltage_calc(evdd_t, -EVDD_LEV_DIFF, EVDD_LEV_MAX, 0);
            rvdd_t = charge_voltage_calc(rvdd_t, -RVDD_LEV_DIFF, RVDD_LEV_MAX, 0);
            dvdd_t = charge_voltage_calc(dvdd_t, -DVDD_LEV_DIFF, DVDD_LEV_MAX, 0);
        } else if (vbg_aim < vbg_t) {
            vbg_t = charge_voltage_calc(vbg_t, -VBG_LEV_DIFF, VBG_LEV_MAX, 0);
            vddio_t = charge_voltage_calc(vddio_t, VDDIO_LEV_DIFF, VDDIO_LEV_MAX, 0);
            vdc13_t = charge_voltage_calc(vdc13_t, VDC13_LEV_DIFF, VDC13_LEV_MAX, 0);
            evdd_t = charge_voltage_calc(evdd_t, EVDD_LEV_DIFF, EVDD_LEV_MAX, 0);
            rvdd_t = charge_voltage_calc(rvdd_t, RVDD_LEV_DIFF, RVDD_LEV_MAX, 0);
            dvdd_t = charge_voltage_calc(dvdd_t, DVDD_LEV_DIFF, DVDD_LEV_MAX, 0);
        }
        MVBG_SEL((vbg_t + 5) / 10);
        VDDIOM_VOL_SEL((vddio_t + 5) / 10);
        VDC13_VOL_SEL((vdc13_t + 5) / 10);
        EVD_VOL_SEL((evdd_t + 5) / 10);
        RVDD_VOL_SEL((rvdd_t + 5) / 10);
        SYSVDD_VOL_SEL((dvdd_t + 5) / 10);
        udelay(1000);
    }

    log_info("vbg: %d, vddio: %d, vdc13: %d, evdd: %d, rvdd: %d, dvdd: %d\n",
             MVBG_GET(), GET_VDDIOM_VOL(), GET_VD13_VOL_SEL(),
             GET_EVD_VOL(), GET_RVDD_VOL_SEL(), GET_SYSVDD_VOL_SEL());

    //根据充电电流设置对应第一次判满的档位
    if (__this->result.curr_lev == CHARGE_mA_20) {
        CHARGE_FULL_mA_SEL(CHARGE_FULL_mA_15);
    } else if (__this->result.curr_lev == CHARGE_mA_30) {
        CHARGE_FULL_mA_SEL(CHARGE_FULL_mA_25);
    } else {
        CHARGE_FULL_mA_SEL(CHARGE_FULL_mA_30);
    }
    set_charge_mA(__this->result.curr_lev);
    CHARGE_FULL_V_SEL(__this->result.vbat_lev);

    adc_reset();
    __this->full_time = 0;
    //再打开充电
    if (is_charging) {
        CHGBG_EN(1);
        CHARGE_EN(1);
    }
}

static void charge_exit_calibration_mode(void)
{
    u8 is_charging;
    int vbg_t, vddio_t, vdc13_t, evdd_t, rvdd_t, dvdd_t, vbat_trim, vbg_aim;
    if (!__this->result_flag) {
        return;
    }

    if (!__this->enter_flag) {
        return;
    }

    __this->enter_flag = 0;

    is_charging = IS_CHARGE_EN();
    //先关闭充电
    CHGBG_EN(0);
    CHARGE_EN(0);

    log_info("charge_exit_calibration_mode\n");
    log_info("vbg: %d, vddio: %d, vdc13: %d, evdd: %d, rvdd: %d, dvdd: %d\n",
             MVBG_GET(), GET_VDDIOM_VOL(), GET_VD13_VOL_SEL(),
             GET_EVD_VOL(), GET_RVDD_VOL_SEL(), GET_SYSVDD_VOL_SEL());

    vbg_t   = MVBG_GET() * 10;
    vddio_t = GET_VDDIOM_VOL() * 10;
    vdc13_t = GET_VD13_VOL_SEL() * 10;
    evdd_t  = GET_EVD_VOL() * 10;
    rvdd_t  = GET_RVDD_VOL_SEL() * 10;
    dvdd_t  = GET_SYSVDD_VOL_SEL() * 10;
    while (__this->vbg_lvl != vbg_t) {
        if (__this->vbg_lvl > vbg_t) {
            vbg_t = charge_voltage_calc(vbg_t, VBG_LEV_DIFF, VBG_LEV_MAX, 0);
            vddio_t = charge_voltage_calc(vddio_t, -VDDIO_LEV_DIFF, VDDIO_LEV_MAX, __this->vddio_lvl);
            vdc13_t = charge_voltage_calc(vdc13_t, -VDC13_LEV_DIFF, VDC13_LEV_MAX, __this->vdc13_lvl);
            evdd_t = charge_voltage_calc(evdd_t, -EVDD_LEV_DIFF, EVDD_LEV_MAX, __this->evdd_lvl);
            rvdd_t = charge_voltage_calc(rvdd_t, -RVDD_LEV_DIFF, RVDD_LEV_MAX, __this->rvdd_lvl);
            dvdd_t = charge_voltage_calc(dvdd_t, -DVDD_LEV_DIFF, DVDD_LEV_MAX, __this->dvdd_lvl);
        } else if (__this->vbg_lvl < vbg_t) {
            vbg_t = charge_voltage_calc(vbg_t, -VBG_LEV_DIFF, VBG_LEV_MAX, 0);
            vddio_t = charge_voltage_calc(vddio_t, VDDIO_LEV_DIFF, __this->vddio_lvl, 0);
            vdc13_t = charge_voltage_calc(vdc13_t, VDC13_LEV_DIFF, __this->vdc13_lvl, 0);
            evdd_t = charge_voltage_calc(evdd_t, EVDD_LEV_DIFF, __this->evdd_lvl, 0);
            rvdd_t = charge_voltage_calc(rvdd_t, RVDD_LEV_DIFF, __this->rvdd_lvl, 0);
            dvdd_t = charge_voltage_calc(dvdd_t, DVDD_LEV_DIFF, __this->dvdd_lvl, 0);
        }
        MVBG_SEL((vbg_t + 5) / 10);
        VDDIOM_VOL_SEL((vddio_t + 5) / 10);
        VDC13_VOL_SEL((vdc13_t + 5) / 10);
        EVD_VOL_SEL((evdd_t + 5) / 10);
        RVDD_VOL_SEL((rvdd_t + 5) / 10);
        SYSVDD_VOL_SEL((dvdd_t + 5) / 10);
        udelay(1000);
    }
    //修正
    MVBG_SEL(__this->vbg_lvl / 10);
    VDDIOM_VOL_SEL(__this->vddio_lvl / 10);
    VDC13_VOL_SEL(__this->vdc13_lvl / 10);
    EVD_VOL_SEL(__this->evdd_lvl / 10);
    RVDD_VOL_SEL(__this->rvdd_lvl / 10);
    SYSVDD_VOL_SEL(__this->dvdd_lvl / 10);

    log_info("vbg: %d, vddio: %d, vdc13: %d, evdd: %d, rvdd: %d, dvdd: %d\n",
             MVBG_GET(), GET_VDDIOM_VOL(), GET_VD13_VOL_SEL(),
             GET_EVD_VOL(), GET_RVDD_VOL_SEL(), GET_SYSVDD_VOL_SEL());

    //调整完成所有的电压才能够使能时钟切换
    clk_set_en(1);

    //1、减小了一个充电档位,充电电流应该设置为(配置值-1档)
    //2、增大了一个充电档位,充电电流应该设置为(配置档)
    //3、只是增大了VBG,充电电流应该设置为(配置档)
    //4、只是减小了VBG,充电电流应该设置为(配置档-1)
    CHARGE_FULL_mA_SEL(__this->data->charge_full_mA);
    CHARGE_FULL_V_SEL(__this->full_lev);
    if (__this->data->charge_mA > 0) {
        if (__this->result.curr_lev < __this->data->charge_mA) {
            set_charge_mA(__this->data->charge_mA - 1);
        } else if (__this->result.curr_lev > __this->data->charge_mA) {
            set_charge_mA(__this->data->charge_mA);
        } else {
            if (__this->result.vbg_lev > __this->vbg_lvl) {
                set_charge_mA(__this->data->charge_mA);
            } else {
                set_charge_mA(__this->data->charge_mA - 1);
            }
        }
    } else {
        set_charge_mA(__this->data->charge_mA);
    }

    adc_reset();

    //再打开充电
    if (is_charging) {
        CHGBG_EN(1);
        CHARGE_EN(1);
    }
}

#endif

void charge_power_switch_timer(void *priv)
{
    u32 vbat_voltage;
    vbat_voltage = adc_get_voltage(AD_CH_VBAT) * 4 / 10;
    if (vbat_voltage > 300) {
        power_set_charge_mode(0);
        power_set_mode(PWR_DCDC15);
        power_set_charge_mode(1);
        usr_timer_del(__this->power_sw_timer);
        __this->power_sw_timer = 0;
        adc_set_sample_freq(AD_CH_VBAT, 5000);
    }
}

void power_enter_charge_mode(void)
{
    if (TCFG_LOWPOWER_POWER_SEL == PWR_DCDC15) {
        u8 chip_id = get_chip_version() & 0x0f;
        u32 vbat_voltage;
        if ((chip_id >= 0x0C) || (chip_id == 0x01) || (chip_id == 0x02)) {
            vbat_voltage = adc_get_voltage(AD_CH_VBAT) * 4 / 10;
            if (vbat_voltage > 300) {
                power_set_charge_mode(0);
                power_set_mode(PWR_DCDC15);
                power_set_charge_mode(1);
            } else {
                power_set_charge_mode(0);
                power_set_mode(PWR_LDO15);
                power_set_charge_mode(1);
                if (__this->power_sw_timer == 0) {
                    //低压充电不进低功耗
                    adc_set_sample_freq(AD_CH_VBAT, 20);
                    __this->power_sw_timer = usr_timer_add(NULL, charge_power_switch_timer, 1000, 1);
                }
            }
        } else if ((chip_id == 0x03) || (chip_id == 0x04) || (chip_id == 0x06)) {
            power_set_charge_mode(1);
            power_set_mode(PWR_DCDC15_FOR_CHARGE);
        }
    }
}

void power_exit_charge_mode(void)
{
    if (TCFG_LOWPOWER_POWER_SEL == PWR_DCDC15) {
        power_set_charge_mode(0);
        power_set_mode(TCFG_LOWPOWER_POWER_SEL);
        if (__this->power_sw_timer) {
            usr_timer_del(__this->power_sw_timer);
            __this->power_sw_timer = 0;
        }
    }
}

/*检测是否满足进入恒流充电条件*/
static void charge_cc_check(void *priv)
{
    if ((adc_get_voltage(AD_CH_VBAT) * 4 / 10) > CHARGE_CCVOL_V) {
        /*满足进入恒流充电条件 设置恒流充电电流大小*/
        set_charge_mA(__this->data->charge_mA);
        usr_timer_del(__this->cc_timer);
        __this->cc_timer = 0;
        /*触发一次满电唤醒服务函数 将满电使能打开*/
#if TCFG_CHARGE_CALIBRATION_ENABLE
        charge_enter_calibration_mode();
#endif
        power_wakeup_enable_with_port(IO_CHGFL_DET);
        charge_wakeup_isr();
    }
}

static void charge_vbat_full_check(void *priv)
{
    u32 vbat_vol, config_full_voltage;
    vbat_vol = adc_get_voltage(AD_CH_VBAT) * 4;
    config_full_voltage = get_charge_full_value();
    log_info("vbat_vol : %d mV\n", vbat_vol);
    if (vbat_vol > (config_full_voltage + 50)) {
        log_info("is vbat check full!!!\n");
        power_wakeup_disable_with_port(IO_CHGFL_DET);
        charge_event_to_user(CHARGE_EVENT_CHARGE_FULL);
    }
}

void charge_start(void)
{
    u8 check_full = 0;
    log_info("%s\n", __func__);

    if (__this->charge_timer) {
        usr_timer_del(__this->charge_timer);
        __this->charge_timer = 0;
    }

    /*进入恒流充电之后(VBAT > 3V),才开启充满检测*/
    if ((adc_get_voltage(AD_CH_VBAT) * 4 / 10) > CHARGE_CCVOL_V) {
        /*设置恒流充电电流大小*/
        set_charge_mA(__this->data->charge_mA);
        /*恒流充电开启满电唤醒使能*/
        power_wakeup_enable_with_port(IO_CHGFL_DET);
        check_full = 1;
#if TCFG_CHARGE_CALIBRATION_ENABLE
        charge_enter_calibration_mode();
#endif
    } else {
        /*设置涓流电流大小*/
        set_charge_mA(CHARGE_mA_20);
        if (!__this->cc_timer) {
            /*每1分钟检测一次是否具备进入恒流充电的条件*/
            __this->cc_timer = usr_timer_add(NULL, charge_cc_check, 1000, 1);
        }
    }

    power_enter_charge_mode();

    CHGBG_EN(1);
    CHARGE_EN(1);

    charge_event_to_user(CHARGE_EVENT_CHARGE_START);
    if (check_full && CHARGE_FULL_FLAG_GET()) {
        charge_wakeup_isr();
    }

    if (__this->need_trim_vbat) {
        //VBAT没有校准过,注册定时器判断是否超压
        if (__this->full_timer == 0) {
            __this->full_timer = sys_timer_add(NULL, charge_vbat_full_check, 10000);
        }
    }
}

void charge_close(void)
{
    log_info("%s\n", __func__);
#if TCFG_CHARGE_CALIBRATION_ENABLE
    charge_exit_calibration_mode();
#endif

    CHGBG_EN(0);
    CHARGE_EN(0);

    power_exit_charge_mode();

    power_wakeup_disable_with_port(IO_CHGFL_DET);

    charge_event_to_user(CHARGE_EVENT_CHARGE_CLOSE);

    if (__this->charge_timer) {
        usr_timer_del(__this->charge_timer);
        __this->charge_timer = 0;
    }
    if (__this->cc_timer) {
        usr_timer_del(__this->cc_timer);
        __this->cc_timer = 0;
    }
    if (__this->full_timer) {
        sys_timer_del(__this->full_timer);
        __this->full_timer = 0;
    }
}

static void charge_write_vbat_trim_value(int trim_val)
{
    u8 vbat_trim_val = trim_val;
    syscfg_write(VM_CHARGE_VBAT_TRIM, (void *)&vbat_trim_val, 1);
    log_info("charge write vm; vbat_trim_val = %d\n", vbat_trim_val);
}

static u8 charge_full_check_vbat_trim(void)
{
    u8 offset, trim_val, cur_lev;
    u32 vbat_vol, config_full_voltage;

    //CP校准过就不管了
    if (__this->need_trim_vbat == 0) {
        return 0;
    }

    cur_lev = GET_CHARGE_FULL_SET();
    vbat_vol = adc_get_voltage(AD_CH_VBAT) * 4;

    //判满时,电池电压过低,可能是没接电池时插入5V,不进行校准操作
    if (vbat_vol < 3700) {
        log_info("battery mabe offline! %d mV\n", vbat_vol);
        return 0;
    }

    config_full_voltage = get_charge_full_value();

    log_info("config_full_voltage: %d\n", config_full_voltage);
    log_info("vbat_vol: %d\n", vbat_vol);
    //能够充到配置值的正负50mV之间就不用校准了
    if ((vbat_vol > (config_full_voltage - 50)) && (vbat_vol < (config_full_voltage + 50))) {
        return 0;
    }

    if (vbat_vol >= config_full_voltage) {
        offset = (vbat_vol - config_full_voltage + 21) / 43;
        if (offset > cur_lev) {
            trim_val = 0;
        } else {
            trim_val = cur_lev - offset;
        }
    } else {
        offset = (config_full_voltage - vbat_vol + 21) / 43;
        if (cur_lev + offset > 15) {
            trim_val = 15;
        } else {
            trim_val = cur_lev + offset;
        }
    }

    //没有调整
    if (trim_val == cur_lev) {
        return 0;
    }

    CHARGE_FULL_V_SEL(trim_val);

    int msg[3];
    msg[0] = (int)charge_write_vbat_trim_value;
    msg[1] = 1;
    msg[2] = (int)trim_val;
    os_taskq_post_type((char *)"app_core", Q_CALLBACK, 3, msg);
    return 1;
}

static void charge_full_detect(void *priv)
{
    static u16 charge_full_cnt = 0;
    static u16 charge_not_full_cnt = 0;
    u16 vbat_vol, vpwr_vol;
    u8 chip_id = get_chip_version() & 0x0f;

    if (CHARGE_FULL_FLAG_GET() && LVCMP_DET_GET()) {
        /* putchar('F'); */
        //AB版可能出现误判满
        charge_not_full_cnt = 0;
#if (TCFG_CHARGE_CALIBRATION_ENABLE)
        //电流校准后初次判满前不进行电池电压检测
        if (__this->result_flag && (__this->full_time == 0)) {
            goto __skip_volt_detect;
        }
#endif

        vbat_vol = adc_get_voltage(AD_CH_VBAT) * 4 / 10;
        vpwr_vol = adc_get_voltage(AD_CH_LDO5V) * 4 / 10;
        if ((vpwr_vol < 450) || (vbat_vol < ((get_charge_full_value() - 100) / 10))) {
            /* putchar('P'); */
            charge_full_cnt = 0;
            return;
        }

#if (TCFG_CHARGE_CALIBRATION_ENABLE)
__skip_volt_detect:
#endif
        if (charge_full_cnt < 10) {
            charge_full_cnt++;
        } else {
            charge_full_cnt = 0;
#if TCFG_CHARGE_CALIBRATION_ENABLE
            if (__this->result_flag && (__this->full_time == 0)) {
                //第一次判满退出校准模式
                __this->full_time = 1;
                charge_exit_calibration_mode();
                return;
            }
#endif
            if (charge_full_check_vbat_trim()) {
                //第一次充电校准,可能调整电压后需要进行再充电
                log_info("need charge again\n");
                return;
            }
            power_wakeup_disable_with_port(IO_CHGFL_DET);
            usr_timer_del(__this->charge_timer);
            __this->charge_timer = 0;
            charge_event_to_user(CHARGE_EVENT_CHARGE_FULL);
        }
    } else {
        /* putchar('K'); */
        charge_full_cnt = 0;
        if (charge_not_full_cnt < 10) {
            charge_not_full_cnt++;
        } else {
            charge_not_full_cnt = 0;
            usr_timer_del(__this->charge_timer);
            __this->charge_timer = 0;
        }
    }
}

static void ldo5v_detect(void *priv)
{
    /* log_info("%s\n",__func__); */

    static u16 ldo5v_in_normal_cnt = 0;
    static u16 ldo5v_in_err_cnt = 0;
    static u16 ldo5v_off_cnt = 0;

    if (LVCMP_DET_GET()) {	//ldoin > vbat
        /* putchar('X'); */
        if (ldo5v_in_normal_cnt < 50) {
            ldo5v_in_normal_cnt++;
        } else {
            /* printf("ldo5V_IN\n"); */
            set_charge_online_flag(1);
            ldo5v_off_cnt = 0;
            ldo5v_in_normal_cnt = 0;
            ldo5v_in_err_cnt = 0;
            usr_timer_del(__this->ldo5v_timer);
            __this->ldo5v_timer = 0;
            if ((charge_flag & BIT_LDO5V_IN) == 0) {
                charge_flag = BIT_LDO5V_IN;
                charge_event_to_user(CHARGE_EVENT_LDO5V_IN);
                power_wakeup_set_edge(IO_VBTCH_DET, FALLING_EDGE);//检测ldoin比vbat电压低的情况(充电仓给电池充满后会关断，此时电压会掉下来)
            }
        }
    } else if (LDO5V_DET_GET() == 0) {	//ldoin<拔出电压（0.6）
        /* putchar('Q'); */
        if (ldo5v_off_cnt < (__this->data->ldo5v_off_filter + 10)) {
            ldo5v_off_cnt++;
        } else {
            /* printf("ldo5V_OFF\n"); */
            set_charge_online_flag(0);
            ldo5v_off_cnt = 0;
            ldo5v_in_normal_cnt = 0;
            ldo5v_in_err_cnt = 0;
            usr_timer_del(__this->ldo5v_timer);
            __this->ldo5v_timer = 0;
            if ((charge_flag & BIT_LDO5V_OFF) == 0) {
                charge_flag = BIT_LDO5V_OFF;
                power_wakeup_set_edge(IO_VBTCH_DET, RISING_EDGE);//拔出后重新检测插入
                charge_event_to_user(CHARGE_EVENT_LDO5V_OFF);
            }
        }
    } else {	//拔出电压（0.6左右）< ldoin < vbat
        /* putchar('E'); */
        if (ldo5v_in_err_cnt < 220) {
            ldo5v_in_err_cnt++;
        } else {
            /* printf("ldo5V_ERR\n"); */
            set_charge_online_flag(1);
            ldo5v_off_cnt = 0;
            ldo5v_in_normal_cnt = 0;
            ldo5v_in_err_cnt = 0;
            usr_timer_del(__this->ldo5v_timer);
            __this->ldo5v_timer = 0;
            if ((charge_flag & BIT_LDO5V_ERR) == 0) {
                charge_flag = BIT_LDO5V_ERR;
                power_wakeup_set_edge(IO_VBTCH_DET, RISING_EDGE);
                if (__this->data->ldo5v_off_filter) {
                    charge_event_to_user(CHARGE_EVENT_CHARGE_ERR);
                }
            }
        }
    }
}

void sub_wakeup_isr(void)
{
    /* printf(" %s \n", __func__); */
    if (__this->ldo5v_timer == 0) {
        __this->ldo5v_timer = usr_timer_add(0, ldo5v_detect, 2, 1);
    }
}

void charge_wakeup_isr(void)
{
    /* printf(" %s \n", __func__); */
    if (__this->charge_timer == 0) {
        __this->charge_timer = usr_timer_add(0, charge_full_detect, 2, 1);
    }
}

#if 0
static void test_func(void *priv)
{
    /* if (P33_CON_GET(P3_CHG_READ) & BIT(0)) { */
    /* JL_PORTA->DIR &= ~BIT(3); */
    /* JL_PORTA->OUT |= BIT(3); */
    /* } else { */
    /* JL_PORTA->OUT &= ~BIT(3); */
    /* } */
}
#endif

u8 get_charge_mA_config(void)
{
    return __this->data->charge_mA;
}

void set_charge_mA(u8 charge_mA)
{
    static u8 charge_mA_old = 0xff;
    if (charge_mA_old != charge_mA) {
        charge_mA_old = charge_mA;
        CHARGE_mA_SEL(charge_mA);
    }
}

const u16 full_table[CHARGE_FULL_V_MAX] = {
    3940, 3980, 4022, 4064, 4108, 4153, 4200, 4248,
    4286, 4327, 4369, 4412, 4450, 4495, 4542, 4589
};
u16 get_charge_full_value(void)
{
    ASSERT(__this->init_ok, "charge not init ok!\n");
    ASSERT(__this->data->charge_full_V < CHARGE_FULL_V_MAX);
    return full_table[__this->data->charge_full_V];
}

const u16 current_table[CHARGE_mA_MAX] = {
    20,  30,  40,  50,  60,  70,  80,  90,
    100, 110, 120, 140, 160, 180, 200, 220,
};
u16 get_charge_current_value(u8 cur_lvl)
{
    ASSERT(__this->data->charge_mA < CHARGE_mA_MAX);
    return current_table[cur_lvl];
}

static void charge_config(void)
{
    int len;
    u8 charge_4202_trim_val = CHARGE_FULL_V_4222;
    u8 offset = 0;
    u8 charge_full_v_val = 0;

    __this->need_trim_vbat = 0;
    if (get_vbat_trim() == 0xf) {
        log_info("vbat not trim, use default config!!!!!!");
        __this->need_trim_vbat = 1;
        len = syscfg_read(VM_CHARGE_VBAT_TRIM, (void *)&charge_4202_trim_val, 1);
        if (len != 1) {
            log_info("vbat vm not trim, use default config!!!!!!");
            charge_4202_trim_val = CHARGE_FULL_V_4222;
        }
    } else {
        charge_4202_trim_val = get_vbat_trim();		//4.2V对应的trim出来的实际档位
    }

    log_info("charge_4202_trim_val = %d\n", charge_4202_trim_val);

    if (__this->data->charge_full_V >= CHARGE_FULL_V_4222) {
        offset = __this->data->charge_full_V - CHARGE_FULL_V_4222;
        charge_full_v_val = charge_4202_trim_val + offset;
        if (charge_full_v_val > 0xf) {
            charge_full_v_val = 0xf;
        }
    } else {
        offset = CHARGE_FULL_V_4222 - __this->data->charge_full_V;
        if (charge_4202_trim_val >= offset) {
            charge_full_v_val = charge_4202_trim_val - offset;
        } else {
            charge_full_v_val = 0;
        }
    }

    log_info("charge_full_v_val = %d\n", charge_full_v_val);

    CHARGE_FULL_V_SEL(charge_full_v_val);
    CHARGE_FULL_mA_SEL(__this->data->charge_full_mA);
    /* CHARGE_mA_SEL(__this->data->charge_mA); */
    CHARGE_mA_SEL(CHARGE_mA_20);
}

static int charge_init(const struct dev_node *node, void *arg)
{
    log_info("%s\n", __func__);

    __this->data = (struct charge_platform_data *)arg;

    ASSERT(__this->data);

    __this->init_ok = 0;
    __this->charge_online_flag = 0;

    /*先关闭充电使能，后面检测到充电插入再开启*/
    power_wakeup_disable_with_port(IO_CHGFL_DET);
    CHGBG_EN(0);
    CHARGE_EN(0);

    /*LDO5V的100K下拉电阻使能*/
    L5V_RES_DET_S_SEL(__this->data->ldo5v_pulldown_lvl);
    L5V_LOAD_EN(__this->data->ldo5v_pulldown_en);

    charge_config();

    if (check_charge_state()) {
        if (__this->ldo5v_timer == 0) {
            __this->ldo5v_timer = usr_timer_add(0, ldo5v_detect, 2, 1);
        }
    } else {
        charge_flag = BIT_LDO5V_OFF;
    }
    charge_set_callback(charge_wakeup_isr, sub_wakeup_isr);

    /* usr_timer_add(0, test_func, 10); */

    adc_add_sample_ch(AD_CH_LDO5V);

#if TCFG_CHARGE_CALIBRATION_ENABLE
    int ret = syscfg_read(VM_CHARGE_CALIBRATION, (void *)&__this->result, sizeof(calibration_result));
    if (ret == sizeof(calibration_result)) {
        __this->result_flag = 1;
        log_info("calibration result:\n");
        log_info_hexdump((u8 *)&__this->result, sizeof(calibration_result));
        if (__this->result.vbg_lev == MVBG_GET()) {
            //VBG一样则在充电中不需要调各种电压
            __this->result_flag = 0;
            log_info("update charge current lev: %d, %d\n", __this->data->charge_mA, __this->result.curr_lev);
            __this->data->charge_mA = __this->result.curr_lev;
        }
    } else {
        log_info("not calibration\n");
    }
#endif

    __this->init_ok = 1;

    return 0;
}

void charge_module_stop(void)
{
    if (!__this->init_ok) {
        return;
    }
    charge_set_callback(NULL, NULL);
    charge_close();
    power_wakeup_disable_with_port(IO_LDOIN_DET);
    power_wakeup_disable_with_port(IO_VBTCH_DET);
    if (__this->ldo5v_timer) {
        usr_timer_del(__this->ldo5v_timer);
        __this->ldo5v_timer = 0;
    }
}

void charge_module_restart(void)
{
    if (!__this->init_ok) {
        return;
    }
    if (!__this->ldo5v_timer) {
        __this->ldo5v_timer = usr_timer_add(NULL, ldo5v_detect, 2, 1);
    }
    charge_set_callback(charge_wakeup_isr, sub_wakeup_isr);
    power_wakeup_enable_with_port(IO_LDOIN_DET);
    power_wakeup_enable_with_port(IO_VBTCH_DET);
}

const struct device_operations charge_dev_ops = {
    .init  = charge_init,
};

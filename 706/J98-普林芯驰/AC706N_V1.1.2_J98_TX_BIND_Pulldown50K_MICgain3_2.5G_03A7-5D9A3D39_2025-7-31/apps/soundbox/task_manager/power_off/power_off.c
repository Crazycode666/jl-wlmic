
/*************************************************************
   此文件函数主 poweroff 关机流程

   如果蓝牙模式有开启，就会等待蓝牙流程关闭完成才会进入软关机

	power_set_soft_poweroff();
	软关机


**************************************************************/


#include "common/power_off.h"
#include "bt_tws.h"
#include <stdlib.h>
#include "app_config.h"
#include "app_task.h"
#include "system/includes.h"
#include "media/includes.h"
#include "app_power_manage.h"
#include "app_chargestore.h"
#include "btstack/avctp_user.h"
#include "app_main.h"
#include "ui/ui_api.h"
#include "ui_manage.h"
#include "tone_player.h"
#include "user_cfg.h"
#include "bt_tws.h"
#include "bt.h"
#include "app_broadcast.h"
#include "broadcast_api.h"

#define LOG_TAG_CONST       APP_ACTION
#define LOG_TAG             "[APP_ACTION]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


#define POWER_OFF_CNT       6

static u8 goto_poweroff_cnt = 0;
static u8 goto_poweroff_flag = 0;
static u8 power_off_tone_play_flag = 0;

//*----------------------------------------------------------------------------*/
/**@brief   poweroff 长按等待 关闭蓝牙
  @param    无
  @return   无
  @note
 */
/*----------------------------------------------------------------------------*/
void power_off_deal(struct sys_event *event, u8 step)
{
    switch (step) {
    case 0:
        goto_poweroff_cnt = 0;
        goto_poweroff_flag = 0;
        goto_poweroff_flag = 1;
        break;

    case 1:
        log_info("poweroff flag:%d cnt:%d\n", goto_poweroff_flag, goto_poweroff_cnt);

        if (goto_poweroff_flag) {
            goto_poweroff_cnt++;

            if (goto_poweroff_cnt == POWER_OFF_CNT) {
                goto_poweroff_cnt = 0;
                app_var.goto_poweroff_flag = 1;
                app_task_switch_to(APP_POWEROFF_TASK);
            }
        }
        break;
    }
}


//*----------------------------------------------------------------------------*/
/**@brief   poweroff 流程消息事件处理
  @param    无
  @return   无
  @note     只处理提示音消息
 */
/*----------------------------------------------------------------------------*/
static int poweroff_sys_event_handler(struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        return true;
    case SYS_BT_EVENT:
        return true;
    case SYS_DEVICE_EVENT:
        return true;
    default:
        return true;
    }
}

//*----------------------------------------------------------------------------*/
/**@brief   poweroff 等待关机显示完毕
  @param    无
  @return   无
  @note      */
/*----------------------------------------------------------------------------*/
static inline void poweroff_wait_ui()
{
#if TCFG_UI_ENABLE
    u8 count = 0;
__retry:
    if (UI_GET_WINDOW_ID() != ID_WINDOW_POWER_OFF) {
        os_time_dly(10);//增加延时防止没有关显示
        if (count < 3) {
            goto __retry;
        }
        count++;
    }
    UI_HIDE_CURR_WINDOW();
#if TCFG_SPI_LCD_ENABLE
    extern void ui_backlight_close(void);
    ui_backlight_close();
#endif
#endif
}

static void poweroff_done(void)
{
    poweroff_wait_ui();//等待关机显示完毕
    while (get_ui_busy_status()) {
    }
#if (TCFG_CHARGE_ENABLE && TCFG_CHARGE_POWERON_ENABLE)
    extern u8 get_charge_online_flag(void);
    if (get_charge_online_flag()) {
        cpu_reset();
    } else {
        power_set_soft_poweroff();
    }
#else
    power_set_soft_poweroff();
#endif
}



void poweroff_tone_end(void *priv, int flag)
{
    if (app_var.goto_poweroff_flag) {
        log_info("audio_play_event_end,enter soft poweroff");
        poweroff_done();
    }
}

//*----------------------------------------------------------------------------*/
/**@brief   poweroff 流程启动
  @param    无
  @return   无
  @note
 */
/*----------------------------------------------------------------------------*/
static void poweroff_app_start()
{
    int ret = false;
    if (app_var.goto_poweroff_mode == 1) {
        log_info("cpu_reset!!!\n");
        cpu_reset();
    }
    if (app_var.goto_poweroff_flag) {
        UI_SHOW_WINDOW(ID_WINDOW_POWER_OFF);
        syscfg_write(CFG_MUSIC_VOL, &app_var.music_volume, 1);
        extern const char vm_ram_storage_enable;

        if (vm_ram_storage_enable) {
            //如果开启了VM配置项暂存RAM功能则在关机前保存数据到vm_flash
            if (get_vm_ram_storage_enable()) {
                vm_flush2flash();
            }
        }


        os_taskq_flush();
        poweroff_tone_end(NULL, 0);
        /* ret = tone_play_with_callback_by_name(tone_table[IDEX_TONE_POWER_OFF], 1, */
        /*                                       poweroff_tone_end, (void *)IDEX_TONE_POWER_OFF); */
        if (ret) {
            y_printf("power_off tone play err,enter soft poweroff");
            poweroff_done();
        }
    }
}

//*----------------------------------------------------------------------------*/
/**@brief   poweroff 主任务
  @param    无
  @return   无
  @note
 */
/*----------------------------------------------------------------------------*/
void app_poweroff_task()
{
    int res;
    int msg[32];
    poweroff_app_start();
    while (1) {
        app_task_get_msg(msg, ARRAY_SIZE(msg), 1);

        switch (msg[0]) {
        case APP_MSG_SYS_EVENT:
            if (poweroff_sys_event_handler((struct sys_event *)(&msg[1])) == false) {
                app_default_event_deal((struct sys_event *)(&msg[1]));    //由common统一处理
            }
            break;
        default:
            break;
        }

        /* if (app_task_exitting()) {
            poweroff_task_close();
            return;
        } */
    }
}

extern void sdx_dev_entry_lowpower(const char *sdx_name);
u8 poweroff_entry_cbfun(void)
{
#if TCFG_SD0_ENABLE
    sdx_dev_entry_lowpower("sd0");
#endif
#if TCFG_SD1_ENABLE
    sdx_dev_entry_lowpower("sd1");
#endif
    return 0;
}



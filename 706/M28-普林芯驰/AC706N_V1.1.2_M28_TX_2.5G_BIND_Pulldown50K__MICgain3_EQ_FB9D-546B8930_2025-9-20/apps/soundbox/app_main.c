#include "system/includes.h"
#include "app_config.h"
#include "asm/pwm_led.h"
#include "tone_player.h"
#include "ui_manage.h"
#include "app_main.h"
#include "app_task.h"
#include "asm/charge.h"
#include "app_power_manage.h"
#include "app_charge.h"
#include "user_cfg.h"
#include "power_on.h"
#include "bt.h"
#include "soundcard/peripheral.h"
#include "audio.h"
#include "vm.h"
#include "rtc/alarm.h"

#define LOG_TAG_CONST       APP
#define LOG_TAG             "[APP]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"



APP_VAR app_var;


void app_entry_idle()
{
    app_task_switch_to(APP_IDLE_TASK);
}



void app_task_loop()
{
    while (1) {
        switch (app_curr_task) {
        case APP_POWERON_TASK:
            log_info("APP_POWERON_TASK \n");
            app_poweron_task();
            app_task_exit_ok();
            break;
        case APP_POWEROFF_TASK:
            log_info("APP_POWEROFF_TASK \n");
            app_poweroff_task();
            app_task_exit_ok();
            break;
#if TCFG_APP_LINEIN_EN
        case APP_LINEIN_TASK:
            log_info("APP_LINEIN_TASK \n");
            app_linein_task();
            app_task_exit_ok();
            break;
#endif
#if TCFG_APP_PC_EN
        case APP_PC_TASK:
            log_info("APP_PC_TASK \n");
            app_pc_task();
            app_task_exit_ok();
            break;
#endif
        case APP_IDLE_TASK:
            log_info("APP_IDLE_TASK \n");
            app_idle_task();
            app_task_exit_ok();
            break;
#if TCFG_APP_LIVE_MIC_EN
        case APP_LIVE_MIC_TASK:
            app_live_mic_task();
            app_task_exit_ok();
            break;
#endif
#if TCFG_APP_LIVE_IIS_EN
        case APP_LIVE_IIS_TASK:
            log_info("APP_LIVE_IIS_ACTION_TASK \n");
            app_live_iis_task();
            app_task_exit_ok();
            break;
#endif
        default:
            log_info(" %d mode not enabled", app_curr_task);
            break;
        }
        app_task_clear_key_msg();//清理按键消息



//需要根据具体开发需求选择是否开启切mode也需要缓存flash
//1.需要注意的缓存到flash需要较长一段时间（1~3s），特别是从蓝牙后台模式切换到蓝牙前台模式，会出现用户明显感知卡顿现象，因此需要具体需求选择性开启。
#if 0
        extern const char vm_ram_storage_enable;
        if (vm_ram_storage_enable) {
            //如果开启了VM配置项暂存RAM功能则在每次切模式的时候保存数据到vm_flash,避免丢失数据
            if (get_vm_ram_storage_enable()) {
                vm_flush2flash();
            }
        }
#endif //#if 0
//

        extern const char vm_ram_storage_enable;
        if (vm_ram_storage_enable == 0) {
            //如果开启把vm配置项暂存到ram的功能,则不需要定期整理vm，增加操作flash的时间
            //检查整理VM
            vm_check_all(0);
        }

    }
}

void app_main()
{
    log_info("app_main \n");

    app_var.start_time = timer_get_ms();

    if (get_charge_online_flag()) {

        app_var.poweron_charge = 1;

#if (TCFG_SYS_LVD_EN == 1)
        vbat_check_init();
#endif

#ifndef  PC_POWER_ON_CHARGE
        app_curr_task = APP_IDLE_TASK;
#else
        app_curr_task = APP_POWERON_TASK;
#endif
    } else {
#if TCFG_HOST_AUDIO_ENABLE
        void usb_host_audio_init(int (*put_buf)(void *ptr, u32 len), int *(*get_buf)(void *ptr, u32 len));
        usb_host_audio_init(usb_audio_play_put_buf, usb_audio_record_get_buf);
#endif

        ui_update_status(STATUS_POWERON);

#if TCFG_APP_LIVE_MIC_EN
        app_curr_task = APP_LIVE_MIC_TASK;
#elif TCFG_APP_LIVE_IIS_EN
        app_curr_task = APP_LIVE_IIS_TASK;
#elif TCFG_APP_LINEIN_EN
        app_curr_task = APP_LINEIN_TASK;
#else
        app_curr_task = APP_IDLE_TASK;
#endif
    }

#if TCFG_CHARGE_BOX_ENABLE
    app_curr_task = APP_IDLE_TASK;
#endif

#if TCFG_CHARGE_ENABLE
    set_charge_event_flag(1);
#endif

    app_task_loop();
}




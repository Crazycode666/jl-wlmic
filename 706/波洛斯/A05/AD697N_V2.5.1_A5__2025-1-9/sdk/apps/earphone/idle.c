#include "system/includes.h"
#include "media/includes.h"
#include "app_config.h"
#include "app_action.h"
#include "tone_player.h"
#include "asm/charge.h"
#include "app_charge.h"
#include "app_main.h"
#include "ui_manage.h"
#include "vm.h"
#include "app_chargestore.h"
#include "user_cfg.h"
#include "default_event_handler.h"
#include "key_event_deal.h"
#include "btstack/avctp_user.h"
#include "asm/pwm_led.h"

#if TCFG_ANC_BOX_ENABLE
#include "app_ancbox.h"
#endif
#if TCFG_CHARGE_CALIBRATION_ENABLE
#include "app_charge_calibration.h"
#endif

#define LOG_TAG_CONST       APP_IDLE
#define LOG_TAG             "[APP_IDLE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#if CHARGING_CLEAN_PHONE_INFO
#include "key_event_deal.h"
#include "btstack/btstack_typedef.h"
#endif
static void app_idle_enter_softoff(void)
{
    //ui_update_status(STATUS_POWEROFF);
    while (get_ui_busy_status()) {
        log_info("ui_status:%d\n", get_ui_busy_status());
    }
#if TCFG_CHARGE_ENABLE
    if (get_lvcmp_det() && (0 == get_charge_full_flag())) {
        log_info("charge inset, system reset!\n");
        cpu_reset();
    }
#endif
    power_set_soft_poweroff();
}

static int app_idle_tone_event_handler(struct device_event *dev)
{
    int ret = false;

    switch (dev->event) {
    case AUDIO_PLAY_EVENT_END:
        if (app_var.goto_poweroff_flag) {
            log_info("audio_play_event_end,enter soft poweroff");
            app_idle_enter_softoff();
        }
        break;
    }

    return ret;
}

#if 1//CHARGING_CLEAN_PHONE_INFO

#define IDLE_CLEAN_INFO_CNT     20

static u8 idle_key_table[KEY_NUM_MAX][KEY_EVENT_MAX] = {
    //SHORT      LONG                   HOLD                        UP                       DOUBLE    TRIPLE
    {KEY_NULL,   KEY_NULL,  KEY_POWEROFF_HOLD,  KEY_NULL, KEY_IDLE_TO_BT, KEY_NULL},   //KEY_0
};
int app_idle_key_event_handler(struct sys_event *event)
{
    int ret = false;
    struct key_event *key = &event->u.key;
    u8 key_event = idle_key_table[key->value][key->event];
    static u16 key_hold_cnt = 0;
    static u8 key_press_flag = 0;

    switch (key_event) {
    case KEY_CLEAN_PHONE_INFO:
        log_info("KEY_CLEAN_PHONE_INFO");
        key_hold_cnt = 0;
        key_press_flag = 1;
        P33_CON_SET(P3_PINR_CON, 0, 1, 0);
        break;
    case KEY_CLEAN_PHONE_INFO_HOLD:
        key_hold_cnt++;
        /*if (key_press_flag) {
            if (key_hold_cnt == IDLE_CLEAN_INFO_CNT) {
                log_info("KEY_CLEAN_PHONE_INFO_HOLD");
                extern void delete_link_key_for_app(bd_addr_t *bd_addr, u8 id);
                delete_link_key_for_app(NULL, 0);
                tone_play(TONE_SIN_NORMAL, 1);
                key_press_flag = 0;
            }
        }*/
        break;
    case KEY_CLEAN_PHONE_INFO_UP:
        log_info("KEY_CLEAN_PHONE_INFO_UP");
        key_hold_cnt = 0;
        key_press_flag = 0;
        P33_CON_SET(P3_PINR_CON, 0, 1, 1);
        break;
    case KEY_POWEROFF_HOLD:
        power_set_soft_poweroff();
        break;
    case KEY_IDLE_TO_BT:
        gpio_set_direction(IO_PORTB_08,0);
        gpio_direction_output(IO_PORTB_08,0);
        //os_time_dly(10);
        task_switch_to_bt();

        //user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_ENABLE, 0, NULL);
        //user_send_cmd_prepare(USER_CTRL_WRITE_CONN_ENABLE, 0, NULL);
        break;
    }

    return ret;
}

#endif

static int idle_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
#if 1//CHARGING_CLEAN_PHONE_INFO
        /* log_info("idle_event_handler:SYS_KEY_EVENT\n"); */
        app_idle_key_event_handler(event);
#endif
        return 0;
    case SYS_BT_EVENT:
        return 0;
    case SYS_DEVICE_EVENT:
        if ((u32)event->arg == DEVICE_EVENT_FROM_CHARGE) {
#if TCFG_CHARGE_ENABLE
            return app_charge_event_handler(&event->u.dev);
#endif
        }
        if ((u32)event->arg == DEVICE_EVENT_FROM_TONE) {
            return app_idle_tone_event_handler(&event->u.dev);
        }
#if TCFG_CHARGESTORE_ENABLE || TCFG_TEST_BOX_ENABLE
        if ((u32)event->arg == DEVICE_EVENT_CHARGE_STORE) {
            app_chargestore_event_handler(&event->u.chargestore);
        }
#endif
#if TCFG_ANC_BOX_ENABLE
        if ((u32)event->arg == DEVICE_EVENT_FROM_ANC) {
            return app_ancbox_event_handler(&event->u.ancbox);
        }
#endif
#if TCFG_CHARGE_CALIBRATION_ENABLE
        if ((u32)event->arg == DEVICE_EVENT_CHARGE_CALIBRATION) {
            return app_charge_calibration_event_handler(&event->u.charge_calibration);
        }
#endif
        break;
    default:
        return false;
    }

#if CONFIG_BT_BACKGROUND_ENABLE
    default_event_handler(event);
#endif

    return false;
}

static int idle_state_machine(struct application *app, enum app_state state,
                              struct intent *it)
{
    int ret;
    switch (state) {
    case APP_STA_CREATE:
        //tone_play_index(IDEX_TONE_ADAPTER_MODE, 1);
        break;
    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_IDLE_MAIN:
            log_info("ACTION_IDLE_MAIN\n");
            if (app_var.goto_poweroff_flag) {
                syscfg_write(CFG_MUSIC_VOL, &app_var.music_volume, 1);
                /* tone_play(TONE_POWER_OFF); */
                os_taskq_flush();
                STATUS *p_tone = get_tone_config();
                ret = 1;//tone_play_index(p_tone->power_off, 1);
                printf("power_off tone play ret:%d", ret);
                if (ret) {
                    if (app_var.goto_poweroff_flag) {
                        log_info("power_off tone play err,enter soft poweroff");
                        app_idle_enter_softoff();
                    }
                }
            }
            //user_send_cmd_prepare(USER_CTRL_INQUIRY_CANCEL, 0, NULL);
            //user_send_cmd_prepare(USER_CTRL_CONNECTION_CANCEL, 0, NULL);
            //user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_DISABLE, 0, NULL);
            //user_send_cmd_prepare(USER_CTRL_WRITE_CONN_DISABLE, 0, NULL);
            //pwm_led_mode_set(PWM_LED1_FAST_FLASH);
            //tone_play_index(IDEX_TONE_ADAPTER_MODE, 1);
            //sys_key_event_enable();
#if CHARGING_CLEAN_PHONE_INFO
            else {
                sys_key_event_enable();
            }
#endif
            break;
        case ACTION_IDLE_POWER_OFF:
            os_taskq_flush();
            syscfg_write(CFG_MUSIC_VOL, &app_var.music_volume, 1);
            break;
        }
        break;
    case APP_STA_PAUSE:
        break;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        break;
    case APP_STA_DESTROY:
        break;
    }

    return 0;
}

static const struct application_operation app_idle_ops = {
    .state_machine  = idle_state_machine,
    .event_handler 	= idle_event_handler,
};

REGISTER_APPLICATION(app_app_idle) = {
    .name 	= "idle",
    .action	= ACTION_IDLE_MAIN,
    .ops 	= &app_idle_ops,
    .state  = APP_STA_DESTROY,
};



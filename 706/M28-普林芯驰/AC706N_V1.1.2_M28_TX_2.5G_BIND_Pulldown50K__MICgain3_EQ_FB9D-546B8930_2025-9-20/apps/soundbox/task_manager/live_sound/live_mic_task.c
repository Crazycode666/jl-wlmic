/*************************************************************
  此文件函数主要是 mic 模式按键处理和事件处理

  void app_live_mic_task
  mic模式主函数

  static int live_mic_sys_event_handler(struct sys_event *event)
  linein模式系统事件所有处理入口

  static void live_mic_task_close(void)
  linein模式退出

 **************************************************************/

#include "system/app_core.h"
#include "system/includes.h"
#include "server/server_core.h"

#include "app_config.h"
#include "app_cfg.h"
#include "app_task.h"
#include "live_mic.h"
#include "linein/linein.h"
#include "bt/bt.h"
#include "btstack/avctp_user.h"
#include "media/includes.h"
#include "tone_player.h"

#include "app_charge.h"
#include "app_main.h"
#include "app_online_cfg.h"
#include "app_power_manage.h"

#include "audio_dec_iis.h"

#include "key_event_deal.h"
#include "user_cfg.h"
#include "clock_cfg.h"

#include "app_connected.h"
#include "connected_api.h"
#include "app_broadcast.h"
#include "broadcast_api.h"
#include "audio_mode.h"

#if TCFG_APP_LIVE_MIC_EN

static void connected_on_off_detect(void *priv)
{
    static bool detect = false;
    static bool on_off = true;

    if (on_off != get_connected_on_off()) {
        on_off = !on_off;
        detect = true;
    }
    if (detect) {
        if (!on_off) {
            user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_ENABLE, 0, NULL);
            user_send_cmd_prepare(USER_CTRL_WRITE_CONN_ENABLE, 0, NULL);
        } else {
            bt_close_discoverable_and_connectable();
        }
        detect = false;
    }
}



//*----------------------------------------------------------------------------*/
/**@brief    broadcast mic 入口
   @param    无
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void live_mic_app_init(void)
{
    sys_key_event_enable();
    clock_idle(LINEIN_IDLE_CLOCK);
#if (TCFG_BROADCAST_ENABLE || TCFG_CONNECTED_ENABLE)
#if WIRELESS_2T1_DUPLEX_EN
    extern void wireless_multiple_audio_codec_interface_register(void);
    wireless_multiple_audio_codec_interface_register();
#else
    mic_wireless_audio_codec_interface_register();
#endif

#if (!TCFG_BLUETOOTH_BACK_MODE)
    btstack_init_in_other_mode();
#endif
#endif
}

//*----------------------------------------------------------------------------*/
/**@brief    broadcast mic 提示音播放结束回调函数
   @param    无
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void  live_mic_tone_play_end_callback(void *priv, int flag)
{
    u32 index = (u32)priv;
    if (app_next_task) {
        printf("\n-- error: curr task isn't APP_LIVE_MIC_TASK!\n");
        return;
    }
    switch (index) {
    case IDEX_TONE_MIC:
        app_task_put_key_msg(KEY_MIC_START, 0);
        break;
    default:
        break;
    }
}

static void live_mic_start(void)
{
#if TCFG_NEED_DEC_ENABLE

#if (TCFG_BROADCAST_ENABLE || TCFG_CONNECTED_ENABLE)
    mic_start(TCFG_AUDIO_ADC_MIC_CHA, JLA_CODING_SAMPLERATE, 6);
#else
    mic_start(TCFG_AUDIO_ADC_MIC_CHA, 44100, 6);
#endif

#endif
}

static void multiple_dev_start(void)
{
#if (CONNECTED_ROLE_CONFIG == 2)
    mic_start(TCFG_AUDIO_ADC_MIC_CHA, JLA_CODING_SAMPLERATE, 6);
#endif

#if TCFG_LINEIN_ENABLE
    linein_start();
#endif

#if TCFG_AUDIO_INPUT_IIS
    iis_in_dec_open(TCFG_IIS_SR);
#endif //#if TCFG_AUDIO_INPUT_IIS

}

//*----------------------------------------------------------------------------*/
/**@brief   bc mic 按键消息入口
   @param    无
   @return   1、消息已经处理，不需要发送到common  0、消息发送到common处理
   @note
*/
/*----------------------------------------------------------------------------*/
static int live_mic_key_msg_deal(struct sys_event *event)
{
    int result = 0;
    int ret = true;
    int err = 0;
    u8 vol;
    int key_event = event->u.key.event;
    int key_value = event->u.key.value;
    switch (key_event) {
    case KEY_MIC_START:
        break;
    case KEY_MUSIC_PP:
        live_mic_volume_pp();
        break;
    default:
        ret = false;
        break;
    }
    return ret;
}



//*----------------------------------------------------------------------------*/
/**@brief    bc mic 模式活跃状态 所有消息入口
   @param    无
   @return   1、当前消息已经处理，不需要发送comomon 0、当前消息不是linein处理的，发送到common统一处理
   @note
*/
/*----------------------------------------------------------------------------*/
static int live_mic_sys_event_handler(struct sys_event *event)
{
    int ret = TRUE;
    switch (event->type) {
    case SYS_KEY_EVENT:
        return live_mic_key_msg_deal(event);
        break;
    default:
        return false;
    }
    return false;
}

static void live_mic_task_close(void)
{
#if TCFG_BROADCAST_ENABLE
    app_broadcast_deal(BROADCAST_APP_MODE_EXIT);
#if (!TCFG_BLUETOOTH_BACK_MODE)
    app_broadcast_close_in_other_mode();
#endif
#endif

#if TCFG_CONNECTED_ENABLE
    app_connected_deal(CONNECTED_APP_MODE_EXIT);
#if (!TCFG_BLUETOOTH_BACK_MODE)
    app_connected_close_in_other_mode();
#endif
#endif

#if (!TCFG_BLUETOOTH_BACK_MODE)
    btstack_exit_in_other_mode();
#endif

    mic_stop();
}

//*----------------------------------------------------------------------------*/
/**@brief    broadcast mic 任务主体
   @param    无
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void app_live_mic_task()
{
    int res;
    int msg[32];
    live_mic_app_init();
    sys_auto_shut_down_enable();
    while (1) {
        app_task_get_msg(msg, ARRAY_SIZE(msg), 1);
        switch (msg[0]) {
        case APP_MSG_SYS_EVENT:
            if (live_mic_sys_event_handler((struct sys_event *)(&msg[1])) == false) {
                app_default_event_deal((struct sys_event *)(&msg[1]));
            }
            break;
        default:
            break;
        }

        if (app_task_exitting()) {
            live_mic_task_close();
            return;
        }
    }
}

#else
void app_live_mic_task(void)
{

}

#endif /* TCFG_APP_live_mic_EN */



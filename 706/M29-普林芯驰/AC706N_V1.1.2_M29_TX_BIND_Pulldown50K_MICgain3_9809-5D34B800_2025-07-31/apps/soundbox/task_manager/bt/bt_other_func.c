#include "system/includes.h"
#include "app_config.h"
#include "btstack/avctp_user.h"
#include "btstack/btstack_task.h"
#include "btstack/bluetooth.h"
#include "btstack/btstack_error.h"
#include "btctrler/btctrler_task.h"
#include "classic/hci_lmp.h"
#include "bt/bt_ble.h"
#include "bt/bt.h"
#include "bt_common.h"
#include "app_main.h"
#include "soundbox.h"
#include "app_task.h"
#include "app_broadcast.h"
#include "app_connected.h"

#if BLUETOOTH_TOGGLE

struct app_bt_opr app_bt_hdl;

#define __this 	(&app_bt_hdl)

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙模式协议栈功能配置
   @param    无
   @return   无
   @note
*/
/*----------------------------------------------------------------------------*/
void bt_function_select_init()
{
    u8 tmp_ble_addr[6];

    ////设置蓝牙加密的level
    //io_capabilities ; /*0: Display only 1: Display YesNo 2: KeyboardOnly 3: NoInputNoOutput*/
    //authentication_requirements: 0:not protect  1 :protect
    __set_simple_pair_param(3, 0, 2);

#if (TCFG_BT_SNIFF_ENABLE == 0)
    void lmp_set_sniff_disable(void);
    lmp_set_sniff_disable();
#endif

    /*
                TX     RX
       AI800x   PA13   PA12
       AC692x   PA13   PA12
       AC693x   PA8    PA9
       AC695x   PA9    PA10
       AC696x   PA9    PA10
       AC694x   PB1    PB2
       AC697x   PC2    PC3
       AC631x   PA7    PA8

    */
#if TCFG_BT_RF_USE_EXT_PA_ENABLE
    //设置TX和RX引脚
    extern void bt_rf_PA_control_io_remap(u16 tx_io, u16 rx_io);
    bt_rf_PA_control_io_remap(TCFG_RF_PA_TX_PORT, TCFG_RF_PA_RX_PORT);
    ////设置蓝牙接收状态io输出，可以外接pa
    bt_set_rxtx_status_enable(1);
#endif

    lib_make_ble_address(tmp_ble_addr, (void *)bt_get_mac_addr());
    le_controller_set_mac((void *)tmp_ble_addr);
    printf("\n-----edr + ble 's address-----");
    put_buf((void *)bt_get_mac_addr(), 6);
    put_buf((void *)tmp_ble_addr, 6);

    set_bt_enhanced_power_control(1);
}

/*----------------------------------------------------------------------------*/
/**@brief   自动关机开关
   @param
   @return
   @note    开启后，如果一段时间内没有连接等操作就会进入关机
*/
/*----------------------------------------------------------------------------*/
void sys_auto_shut_down_enable(void)
{
#if TCFG_AUTO_SHUT_DOWN_TIME
    //log_debug("sys_auto_shut_down_enable\n");

    if (app_var.auto_shut_down_timer == 0) {
        app_var.auto_shut_down_timer = sys_timeout_add(NULL, sys_enter_soft_poweroff, (app_var.auto_off_time * 1000));
    } else {//在切换到蓝牙任务APP_STA_START中，current_app为空
       //printf("****************** enter poweroff **********************\n");
        sys_auto_shut_down_disable();
        app_var.auto_shut_down_timer = sys_timeout_add(NULL, sys_enter_soft_poweroff, (app_var.auto_off_time * 1000));
    }
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief   关闭自动关机
   @param
   @return
   @note    链接上设备后会调用关闭
*/
/*----------------------------------------------------------------------------*/

//static int disable_flag = 0;
void sys_auto_shut_down_disable(void)
{
#if TCFG_AUTO_SHUT_DOWN_TIME
    //disable_flag++;
    //printf("sys_auto_shut_down_disable[%d] \n",disable_flag);
    if (app_var.auto_shut_down_timer) {
        sys_timeout_del(app_var.auto_shut_down_timer);
        app_var.auto_shut_down_timer = 0;
    }
#endif
}

void app_bt_task()
{
}

u8 bt_app_exit_check()
{
    return 1;
}

int bt_background_event_handler_filter(struct sys_event *event)
{
    if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
        switch (event->u.bt.event) {
        case BT_STATUS_INIT_OK:
            __this->init_ok = 1;
#if TCFG_NORMAL_SET_DUT_MODE
            puts("ble set dut mode\n");
            extern void ble_standard_dut_test_init(void);
            ble_standard_dut_test_init();
#else

#if TCFG_BROADCAST_ENABLE
            app_broadcast_open_in_other_mode();
#endif

#if TCFG_CONNECTED_ENABLE
            app_connected_open_in_other_mode();
#endif

#endif
            break;
        default:
            break;
        }
    }
    return 0;
}

int bt_background_event_handler(struct sys_event *event)
{
    bt_background_event_handler_filter(event);
    return 0;
}

u8 get_call_status()
{
    return BT_CALL_HANGUP;
}

u32 bt_tws_master_slot_clk(void)
{
    return 0;
}

void sys_enter_soft_poweroff(void *priv)
{
    printf("%s\n", __func__);

    if (app_var.goto_poweroff_flag) {
        return;
    }

    app_var.goto_poweroff_flag = 1;
    app_var.goto_poweroff_cnt = 0;
    app_var.goto_poweroff_mode = (u8)priv;

    if (priv == NULL) {
        app_task_switch_to(APP_POWEROFF_TASK);
    } else if (priv == (void *)1) {
        printf("cpu_reset!!!\n");
        cpu_reset();
    }
}

void soft_poweroff_mode(u8 mode)
{
    __this->force_poweroff = mode;
}

void btstack_init_in_other_mode(void)
{
    bt_function_select_init();
    /* bredr_handle_register(); */
    btstack_init();
}

void btstack_exit_in_other_mode(void)
{
    btstack_exit();
    app_bt_hdl.init_ok = 0;
}

#endif


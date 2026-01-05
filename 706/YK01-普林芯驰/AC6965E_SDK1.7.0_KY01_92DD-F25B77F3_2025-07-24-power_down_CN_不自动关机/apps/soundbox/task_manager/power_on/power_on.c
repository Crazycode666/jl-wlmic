#include "system/includes.h"
#include "media/includes.h"
#include "app_config.h"
#include "tone_player.h"
#include "asm/charge.h"
#include "app_charge.h"
#include "app_main.h"
#include "ui_manage.h"
#include "vm.h"
#include "app_chargestore.h"
#include "user_cfg.h"
#include "ui/ui_api.h"
#include "app_task.h"
#include "key_event_deal.h"


#define LOG_TAG_CONST       APP_IDLE
#define LOG_TAG             "[APP_IDLE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"


static void  lcd_ui_power_on_timeout(void *p)
{
#if (TCFG_SPI_LCD_ENABLE)
    /* sys_key_event_enable(); */
    /* logo_time = timer_get_ms(); */
    /* while (timer_get_ms() - logo_time <= 2 * 1000) { //显示开机logo */
    /* os_time_dly(10); */
    /* } */

    UI_HIDE_WINDOW(ID_WINDOW_POWER_ON);
    UI_SHOW_WINDOW(ID_WINDOW_MAIN);
#if TCFG_APP_BT_EN
    app_task_switch_to(APP_BT_TASK);
#else
    app_task_switch_to(APP_MUSIC_TASK);
#endif
#endif

}

static void  lcd_ui_power_on()
{
#if (TCFG_SPI_LCD_ENABLE)
    int logo_time = 0;
    UI_SHOW_WINDOW(ID_WINDOW_POWER_ON);
    sys_timeout_add(NULL, lcd_ui_power_on_timeout, 1000);
#endif
}

int connect_7063_flag = 0;
int bt_conn_flag = 0;
int disk_flag = 0;
static read_state_flag1 = 0;
static read_state_flag2 = 2;

static cablemic_flag = 0;
static cablemic_flag1 = 2;


extern void pwm_cablemic_led();
void cableMic_dete()
{
    u32 gpio = IO_PORTA_01;//指定IO
    gpio_set_pull_down(gpio, 0);//看需求是否需要开内部下拉
    gpio_set_pull_up(gpio, 0);//看需求是否需要开内部上拉
    gpio_set_die(gpio, 1);
    gpio_set_direction(gpio, 1);
    delay(100);//设置方向寄存器后,不能立马读电压
    if(gpio_read(gpio))
    {
        printf(" ################# PA_2 %d  #################\n",gpio_read(gpio));
        cablemic_flag = 0;
    }
    else{
        printf(" ################# PA_2 %d  #################\n",gpio_read(gpio));
        cablemic_flag = 1;
    }

    if(cablemic_flag != cablemic_flag1)
    {
        cablemic_flag1 = cablemic_flag;
        if(cablemic_flag)
        {
            printf(" ################# CABLEMIC_FLAG %d  #################\n",cablemic_flag);
            gpio_set_direction(IO_PORTA_03,0);
            gpio_direction_output(IO_PORTA_03,0);
            //user_led_on(IO_PORTA_02);
            pwm_cablemic_led();
            tone_play_by_path(tone_table[IDEX_CABLE_MIC], 1);
        }
        else{
                printf(" ################# CABLEMIC_FLAG %d  #################\n",cablemic_flag);
            gpio_set_direction(IO_PORTA_03,0);
            gpio_direction_output(IO_PORTA_03,1);
            gpio_led_high_block(IO_PORTA_02);
            tone_play_by_path(tone_table[IDEX_WIRLESS_MIC], 1);
        }
    }
}

static u16 cablemic_dete_time = 0;
void cableMic_dete_add()
{
    if(!cablemic_dete_time)
    {
        cablemic_dete_time = sys_timer_add(NULL,cableMic_dete,200);
    }

}


void cableMic_dete_del()
{
    if(cablemic_dete_time)
    {
        sys_timer_del(cablemic_dete_time);
        cablemic_dete_time = 0;
    }
}


static aux_state_flag1 = 0;
static aux_state_flag2 = 2;
void aux_dete()
{
    u32 gpio = IO_PORT_DP;//指定IO
    gpio_set_pull_down(gpio, 0);//看需求是否需要开内部下拉
    gpio_set_pull_up(gpio, 0);//看需求是否需要开内部上拉
    gpio_set_die(gpio, 1);
    gpio_set_direction(gpio, 1);
    delay(100);//设置方向寄存器后,不能立马读电压
    if(gpio_read(gpio))
    {
        aux_state_flag1 = 0;
    }
    else{
        aux_state_flag1 = 1;
    }

    if(aux_state_flag1 != aux_state_flag2)
    {
        aux_state_flag2 = aux_state_flag1;
        if(aux_state_flag1)
        {
            //tone_play_by_path(tone_table[IDEX_AUX_MODE], 1);
            gpio_set_direction(IO_PORTB_06,0);
            gpio_direction_output(IO_PORTB_06,0);

        }
        else{
            gpio_set_direction(IO_PORTB_06,0);
            gpio_direction_output(IO_PORTB_06,1);
        }

    }
}

static u16 aux_dete_time = 0;
void aux_dete_add()
{
    if(!aux_dete_time)
    {
        aux_dete_time = sys_timer_add(NULL,aux_dete,500);
    }

}


void aux_dete_del()
{

    if(aux_dete_time)
    {
        sys_timer_del(aux_dete_time);
        aux_dete_time = 0;
    }
}

void poweroff_dete()
{
    printf("################  BT FLAG ################# %d\n",bt_conn_flag);
    if(connect_7063_flag || bt_conn_flag || disk_flag || cablemic_flag)
    {
        read_state_flag1 = 0;
        printf("1111111111111111111111111111111\n");
        //sys_auto_shut_down_disable();
    }

    else
    {
        read_state_flag1 = 1;
        printf("2222222222222222222222222222222222222\n");
        //sys_auto_shut_down_enable();
    }

    if(read_state_flag2 != read_state_flag1)
    {
        read_state_flag2 = read_state_flag1;
        if(connect_7063_flag || bt_conn_flag || disk_flag || cablemic_flag)
            sys_auto_shut_down_disable();
        else
            sys_auto_shut_down_enable();
    }

}


static u16 poweroff_dete_time  = 0;
void poweroff_dete_add()
{
    if(!poweroff_dete_time)
    {
        poweroff_dete_time = sys_timer_add(NULL,poweroff_dete,1000);
    }
}


void poweroff_dete_del()
{

    if(poweroff_dete_time)
    {
        sys_timer_del(poweroff_dete_time);
        poweroff_dete_time = 0;
    }
}
void connect_7063_dete()
{

    u32 gpio = IO_PORTB_04;//指定IO
    gpio_set_pull_down(gpio, 0);//看需求是否需要开内部下拉
    gpio_set_pull_up(gpio, 0);//看需求是否需要开内部上拉
    gpio_set_die(gpio, 1);
    gpio_set_direction(gpio, 1);
    delay(100);//设置方向寄存器后,不能立马读电压
    if(gpio_read(gpio))
    {
        connect_7063_flag = 1;
    }
    else{
        connect_7063_flag = 0;
    }
}

static u16 connect_7063_dete_time = 0;
void connect_7063_dete_add()
{
    if(!connect_7063_dete_time)
    {
        connect_7063_dete_time = sys_timer_add(NULL,connect_7063_dete,1000);

    }
}


void connect_7063_dete_del()
{
    if(connect_7063_dete_time)
    {
        sys_timer_del(connect_7063_dete_time);
        connect_7063_dete_time = 0;
    }
}

static int power_on_init(void)
{
    ///有些需要在开机提示完成之后再初始化的东西， 可以在这里初始化
#if (TCFG_SPI_LCD_ENABLE)
    lcd_ui_power_on();//由ui决定切换的模式
    return 0;
#endif
        gpio_set_direction(IO_PORTA_01,0);
        gpio_direction_output(IO_PORTA_01,1);

    connect_7063_dete_add();
    poweroff_dete_add();
    cableMic_dete_add();
    aux_dete_add();
#if TCFG_APP_BT_EN
    app_task_switch_to(APP_MUSIC_TASK);
#else

#if TCFG_USB_APPLE_DOCK_EN //苹果iap协议使用pc模式
    app_task_switch_to(APP_PC_TASK);
#else
    app_task_switch_to(APP_SLEEP_TASK);
    /* app_task_switch_to(APP_PC_TASK); */
    /* app_task_switch_to(APP_MUSIC_TASK); */
    /* app_task_switch_to(APP_IDLE_TASK); */
    /* app_task_switch_to(APP_LINEIN_TASK);//如果带检测，设备不在线，则不跳转 */
#endif

#endif

    return 0;
}

static int power_on_unint(void)
{

    tone_play_stop();
    UI_HIDE_CURR_WINDOW();
    return 0;
}






static int poweron_sys_event_handler(struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        break;
    case SYS_BT_EVENT:
        break;
    case SYS_DEVICE_EVENT:
        break;
    default:
        return false;
    }
    return false;
}


static void  tone_play_end_callback(void *priv, int flag)
{
    int index = (int)priv;

    if (APP_POWERON_TASK != app_get_curr_task()) {
        log_error("tone callback task out \n");
        return;
    }

    switch (index) {
    case IDEX_TONE_POWER_ON:
        power_on_init();
        break;
    }
}


void app_poweron_task()
{
    int msg[32];

    UI_SHOW_MENU(MENU_POWER_UP, 0, 0, NULL);
    os_time_dly(50);
    gpio_set_direction(IO_PORTB_06,0);
    gpio_direction_output(IO_PORTB_06,1);



    int err =  tone_play_with_callback_by_name(tone_table[IDEX_TONE_POWER_ON], 1, tone_play_end_callback, (void *)IDEX_TONE_POWER_ON);

    /* if (err) { //提示音没有,播放失败，直接init流程 */
    /* power_on_init(); */
    /* } */


    while (1) {
        app_task_get_msg(msg, ARRAY_SIZE(msg), 1);
        switch (msg[0]) {
        case APP_MSG_SYS_EVENT:
            if (poweron_sys_event_handler((struct sys_event *)(msg + 1)) == false) {
                app_default_event_deal((struct sys_event *)(&msg[1]));    //由common统一处理
            }
            break;
        default:
            break;
        }

        if (app_task_exitting()) {
            power_on_unint();
            return;
        }
    }

}


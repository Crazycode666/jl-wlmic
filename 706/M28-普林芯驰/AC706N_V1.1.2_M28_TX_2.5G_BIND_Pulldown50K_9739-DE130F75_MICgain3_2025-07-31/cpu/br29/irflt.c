#include "asm/includes.h"
#include "asm/irflt.h"
#include "timer.h"
#include "generic/gpio.h"

#define ir_log 		printf

/* TODO */
//红外定时器定义
#define IR_TIMER                    TIMER3
#define IR_IRQ_TIME_IDX             IRQ_TIME3_IDX
#define IR_TIME_REG                 JL_TIMER3

#define USE_IRFLT_OUT               1

IR_CODE  ir_code;       ///<红外遥控信息

static u8 cmp_start = 0;

enum TIMER_CLK_SOURCES {
    TMR_LSB_CLK = 1,
    TMR_RC_250K,
    TMR_RC_16M,
    TMR_LRC_CLK,
    TMR_STD_12M,
    TMR_STD_24M,
    TMR_STD_48M,
    TMR_CLK_OUT2,
    TMR_PAT_CLK,
    TMR_RTC_OSC,
    TMR_RX_CIN = 15,
};
static const u16 timer_div[] = {
    /*0000*/    1,
    /*0001*/    4,
    /*0010*/    16,
    /*0011*/    64,
    /*0100*/    2,
    /*0101*/    8,
    /*0110*/    32,
    /*0111*/    128,
    /*1000*/    256,
    /*1001*/    4 * 256,
    /*1010*/    16 * 256,
    /*1011*/    64 * 256,
    /*1100*/    2 * 256,
    /*1101*/    8 * 256,
    /*1110*/    32 * 256,
    /*1111*/    128 * 256,
};


/*----------------------------------------------------------------------------*/
/**@brief   time1红外中断服务函数
   @param   void
   @param   void
   @return  void
   @note    void timer1_ir_isr(void)
*/
/*----------------------------------------------------------------------------*/
___interrupt
void timer_ir_isr(void)
{
    IR_TIME_REG->CON |= BIT(14);
    u16 bCap1 = IR_TIME_REG->PRD;
    IR_TIME_REG->CNT = 0;
    u8 cap = bCap1 / ir_code.timer_pad;

    ir_code.boverflow = 0;

    if (cmp_start < 3) {
        return;
    }

    /* putchar('0' + (cap/10)); */
    /* putchar('0' + (cap%10)); */

    if (cap <= 1) {
        ir_code.wData >>= 1;
        ir_code.bState++;
    } else if (cap == 2) {
        ir_code.wData >>= 1;
        ir_code.wData |= 0x8000;
        ir_code.bState++;
    }

    if (ir_code.bState == 16) {
        ir_code.wUserCode = ir_code.wData;
    }
    if (ir_code.bState == 33) {
        ir_code.bState = 1;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   timer模块的初始化
/*----------------------------------------------------------------------------*/
#define TIMER_UNIT_MS           1
#define MAX_TIME_CNT 0x07ff //分频准确范围，更具实际情况调整
#define MIN_TIME_CNT 0x0030
void ir_timer_cap_init(void)
{
    u32 prd_cnt;
    u32 app_timer_clk = 24000000;
    u8 index;
    for (index = 0; index < (sizeof(timer_div) / sizeof(timer_div[0])); index++) {
        prd_cnt = TIMER_UNIT_MS * (app_timer_clk / 1000) / timer_div[index];
        if (prd_cnt > MIN_TIME_CNT && prd_cnt < MAX_TIME_CNT) {
            break;
        }
    }
    ir_code.timer_pad = prd_cnt;
    cmp_start = 0;
    request_irq(IR_IRQ_TIME_IDX, 5, timer_ir_isr, 0);
#if USE_IRFLT_OUT
    IR_TIME_REG->CON = ((TMR_STD_24M << 10) | (index << 4) | BIT(2) | BIT(1) | BIT(0));
#else
    IR_TIME_REG->CON = ((TMR_STD_24M << 10) | (index << 4) | BIT(1) | BIT(0));
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief   获取ir按键值
   @param   void
   @param   void
   @return  void
   @note    void get_irkey_value(void)
*/
/*----------------------------------------------------------------------------*/
u8 get_irflt_value(void)
{
    u8 tkey = 0xff;
    if (ir_code.bState != 32) {
        return tkey;
    }
    if ((((u8 *)&ir_code.wData)[0] ^ ((u8 *)&ir_code.wData)[1]) == 0xff) {
        tkey = (u8)ir_code.wData;
    } else {
        ir_code.bState = 0;
    }
    return tkey;
}

static u8 ir_io_level = 0;
static u8 ir_io = 0;
void ir_input_io_sel(u8 port)
{
    ir_io = port;
#if USE_IRFLT_OUT
#else
    if (IR_TIMER == 0) {
        SFR(JL_IOMC->IOMC0, 30, 2, 1);
    } else {
        SFR(JL_IOMC->IOMC1, 0 + 2 * (IR_TIMER - 1), 2, 1);
    }
#endif
    gpio_ich_sel_iutput_signal(port, INPUT_CH_SIGNAL_IRFLT, INPUT_CH_TYPE_GP_ICH);
    gpio_set_direction(port, 1);
    gpio_set_die(port, 1);
    gpio_set_pull_up(port, 1);
    gpio_set_pull_down(port, 0);
}

void ir_output_timer_sel()
{
}

static void ir_timeout(void *priv)
{
    ir_code.boverflow++;
    if (ir_code.boverflow > 56) { //56*2ms ~= 112ms
        ir_code.boverflow = 56;
        ir_code.bState = 0;
    }
    cmp_start ++;
    if (cmp_start > 3) {
        cmp_start = 3;
    }
}

void ir_timeout_set(void)
{
    sys_s_hi_timer_add(NULL, ir_timeout, 2); //2ms
}

static u8 ir_io_sus = 0;
u8 ir_io_suspend(void)
{
    if (ir_io_sus) {
        return 1;
    }
    if (ir_code.boverflow < 7) { //14ms内，红外接收有可能在忙碌
        return 1;
    }
    ir_io_level = gpio_read(ir_io);
    IR_TIME_REG->CON |= BIT(14);
    IR_TIME_REG->CON &= ~(0b11 << 0);
    ir_io_sus = 1;
    return 0;
}

u8 ir_io_resume(void)
{
    if (!ir_io_sus) {
        return 0;
    }
    ir_io_sus = 0;
    gpio_set_direction(ir_io, 1);
    gpio_set_die(ir_io, 1);
    gpio_set_pull_up(ir_io, 1);
    gpio_set_pull_down(ir_io, 0);
    delay(10);
    if ((ir_io_level) && (ir_io_level != (gpio_read(ir_io)))) {
        ir_code.boverflow = 0;
    }
    cmp_start = 0;
    IR_TIME_REG->CNT = 0;
    IR_TIME_REG->CON |= BIT(14);
    IR_TIME_REG->CON |= (0b11 << 0);
    return 0;
}


void irflt_config()
{
    JL_IR->RFLT_CON = 0;
    JL_IR->RFLT_CON |= (0b1000 << 4);	//256 div
    JL_IR->RFLT_CON |= (0b11 << 2);		//STD_24M
    JL_IR->RFLT_CON |= BIT(0);          //irflt enable
    ir_timer_cap_init();
}

void log_irflt_info()
{
    ir_log("RFLT_CON = 0x%x", JL_IR->RFLT_CON);
    ir_log("IR_TIME_REG = 0x%x", IR_TIME_REG->CON);
}


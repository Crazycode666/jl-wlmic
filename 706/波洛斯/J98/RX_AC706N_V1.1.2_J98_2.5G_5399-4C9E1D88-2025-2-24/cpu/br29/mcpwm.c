#include "asm/mcpwm.h"
#include "asm/clock.h"
#include "asm/gpio.h"

#define MCPWM_DEBUG_ENABLE  	1
#if MCPWM_DEBUG_ENABLE
#define mcpwm_debug(fmt, ...) printf("[MCPWM] "fmt, ##__VA_ARGS__)
#else
#define mcpwm_debug(...)
#endif

#define MCPWM_CLK   clk_get("mcpwm")


PWM_TIMER_REG *get_pwm_timer_reg(pwm_ch_num_type index)
{
    PWM_TIMER_REG *reg = NULL;
    switch (index) {
    case pwm_ch0:
        reg = (PWM_TIMER_REG *)(&(JL_MCPWM->TMR0_CON));
        break;
    case pwm_ch1:
        reg = (PWM_TIMER_REG *)(&(JL_MCPWM->TMR1_CON));
        break;
    default:
        break;
    }
    return reg;
}

PWM_CH_REG *get_pwm_ch_reg(pwm_ch_num_type index)
{
    PWM_CH_REG *reg = NULL;
    switch (index) {
    case pwm_ch0:
        reg = (PWM_CH_REG *)(&(JL_MCPWM->CH0_CON0));
        break;
    case pwm_ch1:
        reg = (PWM_CH_REG *)(&(JL_MCPWM->CH1_CON0));
        break;
    default:
        break;
    }
    return reg;
}

/*
 * @brief 更改MCPWM的频率
 * @parm frequency 频率
 */
void mcpwm_set_frequency(pwm_ch_num_type ch, pwm_aligned_mode_type align, u32 frequency)
{
    PWM_TIMER_REG *reg = get_pwm_timer_reg(ch);
    if (reg == NULL) {
        return;
    }

    reg->tmr_con = 0;
    reg->tmr_cnt = 0;
    reg->tmr_pr = 0;

    u32 i = 0;
    u32 mcpwm_div_clk = 0;
    u32 mcpwm_tmr_pr = 0;
    u32 mcpwm_fre_min = 0;
    u32 clk = MCPWM_CLK;
    for (i = 0; i < 16; i++) {
        mcpwm_fre_min = clk / (65536 * (1 << i));
        if ((frequency >= mcpwm_fre_min) || (i == 15)) {
            break;
        }
    }
    reg->tmr_con |= (i << 3); //div 2^i
    mcpwm_div_clk = clk / (1 << i);
    if (frequency == 0) {
        mcpwm_tmr_pr = 0;
    } else {
        if (align == pwm_center_aligned) { //中心对齐
            mcpwm_tmr_pr = mcpwm_div_clk / (frequency * 2) - 1;
        } else {
            mcpwm_tmr_pr = mcpwm_div_clk / frequency - 1;
        }
    }
    reg->tmr_pr = mcpwm_tmr_pr;
    //timer mode
    if (align == pwm_center_aligned) { //中心对齐
        reg->tmr_con |= 0b10;
    } else {
        reg->tmr_con |= 0b01;
    }
}

/*
 * @brief 设置两个引脚的占空比
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1
 * @parm duty 同时设置H和L引脚的占空比：0 ~ 10000 对应 0% ~ 100%
 */
void mcpwm_set_duty(pwm_ch_num_type pwm_ch, u16 duty)
{
    PWM_TIMER_REG *timer_reg = get_pwm_timer_reg(pwm_ch);
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(pwm_ch);
    if (pwm_reg && timer_reg) {
        pwm_reg->ch_cmph = (timer_reg->tmr_pr + 1) * duty / 10000;
        pwm_reg->ch_cmpl = pwm_reg->ch_cmph;
    }
}
/*
 * @brief 设置H引脚的占空比
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1
 * @parm h_duty H引脚的占空比：0 ~ 10000 对应 0% ~ 100%，如果没有使能引脚，则设置的占空比无效
 */
void mcpwm_set_h_duty(pwm_ch_num_type pwm_ch, u16 h_duty)
{
    PWM_TIMER_REG *timer_reg = get_pwm_timer_reg(pwm_ch);
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(pwm_ch);
    if (pwm_reg && timer_reg) {
        pwm_reg->ch_cmph = (timer_reg->tmr_pr + 1) * h_duty / 10000;
    }
}
/*
 * @brief 设置L引脚的占空比
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1
 * @parm l_duty L引脚的占空比：0 ~ 10000 对应 0% ~ 100%，如果没有使能引脚，则设置的占空比无效
 */
void mcpwm_set_l_duty(pwm_ch_num_type pwm_ch, u16 l_duty)
{
    PWM_TIMER_REG *timer_reg = get_pwm_timer_reg(pwm_ch);
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(pwm_ch);
    if (pwm_reg && timer_reg) {
        pwm_reg->ch_cmpl = (timer_reg->tmr_pr + 1) * l_duty / 10000;
    }
}

/*
 * @brief 打开或者关闭一个时基
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1，pwm_ch2
 * @parm enable 1：打开  0：关闭
 */
void mctimer_ch_open_or_close(pwm_ch_num_type pwm_ch, u8 enable)
{
    if (pwm_ch > pwm_ch_max) {
        return;
    }
    if (enable) {
        JL_MCPWM->MCPWM_CON0 |=  BIT(pwm_ch + 8); //TnEN
    } else {
        JL_MCPWM->MCPWM_CON0 &= ~BIT(pwm_ch + 8); //TnDIS
    }
}


/*
 * @brief 打开或者关闭一个通道
 * @parm pwm_ch_num 通道号：pwm_ch0，pwm_ch1，pwm_ch2
 * @parm enable 1：打开  0：关闭
 */
void mcpwm_ch_open_or_close(pwm_ch_num_type pwm_ch, u8 enable)
{
    if (pwm_ch >= pwm_ch_max) {
        return;
    }
    if (enable) {
        JL_MCPWM->MCPWM_CON0 |=  BIT(pwm_ch); //PWMnEN
    } else {
        JL_MCPWM->MCPWM_CON0 &= ~BIT(pwm_ch); //PWMnDIS
    }
}

/*
 * @brief 关闭MCPWM模块
 */
void mcpwm_open(pwm_ch_num_type pwm_ch)
{
    if (pwm_ch >= pwm_ch_max) {
        return;
    }
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(pwm_ch);
    pwm_reg->ch_con1 &= ~(0b111 << 8);
    pwm_reg->ch_con1 |= (pwm_ch << 8); //sel mctmr
    mcpwm_ch_open_or_close(pwm_ch, 1);
    mctimer_ch_open_or_close(pwm_ch, 1);
}


/*
 * @brief 关闭MCPWM模块
 */
void mcpwm_close(pwm_ch_num_type pwm_ch)
{
    mctimer_ch_open_or_close(pwm_ch, 0);
    mcpwm_ch_open_or_close(pwm_ch, 0);
}
void pwm_led_open()
{
    mcpwm_open(1);
}

void pwm_led_close()
{
    mcpwm_close(1);
}

void log_pwm_info(pwm_ch_num_type pwm_ch)
{
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(pwm_ch);
    PWM_TIMER_REG *timer_reg = get_pwm_timer_reg(pwm_ch);
    mcpwm_debug("tmr%d con0 = 0x%x", pwm_ch, timer_reg->tmr_con);
    mcpwm_debug("tmr%d pr = 0x%x", pwm_ch, timer_reg->tmr_pr);
    mcpwm_debug("pwm ch%d_con0 = 0x%x", pwm_ch, pwm_reg->ch_con0);
    mcpwm_debug("pwm ch%d_con1 = 0x%x", pwm_ch, pwm_reg->ch_con1);
    mcpwm_debug("pwm ch%d_cmph = 0x%x, pwm ch%d_cmpl = 0x%x", pwm_ch, pwm_reg->ch_cmph, pwm_ch, pwm_reg->ch_cmpl);
    mcpwm_debug("MCPWM_CON0 = 0x%x", JL_MCPWM->MCPWM_CON0);
    mcpwm_debug("mcpwm clk = %d", MCPWM_CLK);
}

void mcpwm_init(struct pwm_platform_data *arg)
{
    //set output IO
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(arg->pwm_ch_num);
    if (pwm_reg == NULL) {
        return;
    }
    //set mctimer frequency
    mcpwm_set_frequency(arg->pwm_ch_num, arg->pwm_aligned_mode, arg->frequency);

    pwm_reg->ch_con0 = 1;

    /*if (arg->h_inv_en) {            //每个周期，是否先输出低电平
        pwm_reg->ch_con0 |=  BIT(4);
    } else {
        pwm_reg->ch_con0 &= ~BIT(4);
    }
    if (arg->l_inv_en) {            //每个周期，是否先输出低电平
        pwm_reg->ch_con0 |=  BIT(5);
    } else {
        pwm_reg->ch_con0 &= ~BIT(5);
    }*/

    mcpwm_open(arg->pwm_ch_num); 	 //mcpwm enable
    //set duty
    mcpwm_set_h_duty(arg->pwm_ch_num, arg->h_duty);
    mcpwm_set_l_duty(arg->pwm_ch_num, arg->l_duty);
    //H:
    /*if (arg->h_pin < IO_MAX_NUM) {
        gpio_och_sel_output_signal(arg->h_pin, OUTPUT_CH_SIGNAL_MCPWM0_H + 2 * arg->pwm_ch_num);
        gpio_set_direction(arg->h_pin, 0); //DIR output
    }*/
    //L:
    if (arg->l_pin < IO_MAX_NUM) {
        gpio_och_sel_output_signal(arg->l_pin, OUTPUT_CH_SIGNAL_MCPWM0_L + 2 * arg->pwm_ch_num);
        gpio_set_direction(arg->l_pin, 0); //DIR output
    }
    //mcpwm_close(arg->pwm_ch_num);
    log_pwm_info(arg->pwm_ch_num);
}


///////////// for test code //////////////////
void mcpwm_test(void)
{
#define PWM_CH0_ENABLE 		0
#define PWM_CH1_ENABLE 		1

    struct pwm_platform_data pwm_p_data;

#if PWM_CH0_ENABLE
    memset((u8 *)&pwm_p_data, 0, sizeof(struct pwm_platform_data));
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned;         //边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch0;                        //通道号
    pwm_p_data.frequency = 10000;                           //10KHz
   // pwm_p_data.h_pin = IO_PORTA_01;                                  //任意IO，不需要则填-1
    pwm_p_data.h_duty = 1000;                               //占空比，选的引脚有值才有效
    pwm_p_data.l_pin = IO_PORTA_01;                         //任意IO，不需要就填-1
    pwm_p_data.l_duty = 1000;                               //占空比，选的引脚有值才有效
    //pwm_p_data.h_inv_en = 1;
    pwm_p_data.l_inv_en = 1;
    mcpwm_init(&pwm_p_data);
#endif

#if PWM_CH1_ENABLE
    memset((u8 *)&pwm_p_data, 0, sizeof(struct pwm_platform_data));
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned;         //边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch1;                        //通道号
    pwm_p_data.frequency = 10000;                            //1KHz
    pwm_p_data.h_pin = IO_PORTB_05;                         //任意IO，不需要则填-1
    pwm_p_data.h_duty = 3000;                               //占空比，选的引脚有值才有效
    pwm_p_data.l_pin = IO_PORTB_05;                         //任意IO，不需要就填-1
    pwm_p_data.l_duty = 3000;                               //占空比，选的引脚有值才有效
    pwm_p_data.l_inv_en = 1;                                //l_pin的每个周期先输出低电平，即占空比体现在低电平
    //pwm_p_data.h_inv_en = 1;
    mcpwm_init(&pwm_p_data);
#endif


    extern void clk_out(u8 gpio, enum CLK_OUT_SOURCE clk);
    clk_out1(IO_PORTC_05, LSB_CLK_OUT);
    /*extern void wdt_clear();
    while (1) {
        wdt_clear();
    }*/
}


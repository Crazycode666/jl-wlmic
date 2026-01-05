#include "system/includes.h"

#define LED_SPI                 JL_SPI1
#define LED_SPI_IO              IO_PORTB_04
#define LED_SPI_DAT_BAUD        8000000
#define LED_SPI_REST_BAUD       1000000
#define LED_SPI_CLOCK_BASE		clk_get("lsb")

static OS_SEM led_spi_sem;
static u8 led_spi_busy = 0;
static u8 led_spi_sus = 0;

___interrupt
void led_spi_isr()
{
    LED_SPI->CON &= ~BIT(13);   //关闭中断
    LED_SPI->CON |=  BIT(14);   //清pnding
    os_sem_post(&led_spi_sem);
    led_spi_busy = 0;
}

void led_spi_init(void)
{
    gpio_set_die(LED_SPI_IO, 1);
    gpio_set_direction(LED_SPI_IO, 0);
    gpio_set_pull_up(LED_SPI_IO, 0);
    gpio_set_pull_down(LED_SPI_IO, 0);
    gpio_write(LED_SPI_IO, 0);
    gpio_och_sel_output_signal(LED_SPI_IO, OUTPUT_CH_SIGNAL_SPI1_DO);
    SFR(JL_IOMC->IOMC0, 18, 1, 1);//断开硬件引脚信号
    request_irq(IRQ_SPI1_IDX, 0, led_spi_isr, 0);
    os_sem_create(&led_spi_sem, 1);

    LED_SPI->BUF = 0;
    LED_SPI->CON = 0x4021;
    LED_SPI->CON1 = 0;
}

void led_spi_rgb_to_24byte(u8 r, u8 g, u8 b, u8 *buf, int idx)
{
    buf = buf + idx * 24;
    u32 dat = ((g << 16) | (r << 8) | b);
    for (u8 i = 0; i < 24; i ++) {
        if (dat & BIT(23 - i)) {
            *(buf + i) = 0x7c;
        } else {
            *(buf + i) = 0x60;
        }
    }
}

void led_spi_rest()
{
    u8 tmp_buf[16] = {0};
    LED_SPI->BAUD = LED_SPI_CLOCK_BASE / LED_SPI_REST_BAUD - 1;
    LED_SPI->CON |= BIT(14);
    LED_SPI->ADR = (u32)tmp_buf;
    LED_SPI->CNT = 16;
    while (!(LED_SPI->CON & BIT(15)));
    LED_SPI->CON |= BIT(14);
}

void led_spi_send_rgbbuf(u8 *rgb_buf, u16 led_num) //rgb_buf的大小 至少要等于 led_num * 24
{
    if (!led_num) {
        return;
    }
    while (led_spi_sus) {
        os_time_dly(1);
    }
    led_spi_busy = 1;
    led_spi_rest();
    LED_SPI->BAUD = LED_SPI_CLOCK_BASE / LED_SPI_DAT_BAUD - 1;
    LED_SPI->CON |= BIT(14);
    LED_SPI->ADR = (u32)rgb_buf;
    LED_SPI->CNT = led_num * 24;
    while (!(LED_SPI->CON & BIT(15)));
    LED_SPI->CON |= BIT(14);
    led_spi_busy = 0;
}

void led_spi_send_rgbbuf_isr(u8 *rgb_buf, u16 led_num) //rgb_buf的大小 至少要等于 led_num * 24
{
    if (!led_num) {
        return;
    }
    while (led_spi_sus) {
        os_time_dly(1);
    }
    led_spi_busy = 1;
    os_sem_pend(&led_spi_sem, 0);
    led_spi_rest();
    LED_SPI->BAUD = LED_SPI_CLOCK_BASE / LED_SPI_DAT_BAUD - 1;
    LED_SPI->CON |= BIT(14);
    LED_SPI->ADR = (u32)rgb_buf;
    LED_SPI->CNT = led_num * 24;
    LED_SPI->CON |= BIT(13);//打开中断
}

u8 led_spi_suspend(void)
{
    if (led_spi_sus) {
        return 1;
    }
    if (led_spi_busy) {
        return 1;
    }
    gpio_set_die(LED_SPI_IO, 0);
    gpio_set_direction(LED_SPI_IO, 1);
    gpio_set_pull_up(LED_SPI_IO, 0);
    gpio_set_pull_down(LED_SPI_IO, 0);
    gpio_och_disable_output_signal(LED_SPI_IO, OUTPUT_CH_SIGNAL_SPI1_DO);
    LED_SPI->CON |=  BIT(14);
    LED_SPI->CON &= ~BIT(0);
    led_spi_sus = 1;
    return 0;
}

u8 led_spi_resume(void)
{
    if (!led_spi_sus) {
        return 0;
    }
    gpio_set_die(LED_SPI_IO, 1);
    gpio_set_direction(LED_SPI_IO, 0);
    gpio_set_pull_up(LED_SPI_IO, 0);
    gpio_set_pull_down(LED_SPI_IO, 0);
    gpio_write(LED_SPI_IO, 0);
    gpio_och_sel_output_signal(LED_SPI_IO, OUTPUT_CH_SIGNAL_SPI1_DO);
    LED_SPI->CON = 0x4021;
    led_spi_sus = 0;
    return 0;
}

static u8 spi_dat_buf[24 * 2] __attribute__((aligned(4)));
extern void wdt_clear();

void led_spi_test(void)
{
    printf("******************  led spi test  *******************\n");
    led_spi_init();
    u8 cnt = 0;
    while (1) {
        cnt ++;
        led_spi_rgb_to_24byte(cnt, 255 - cnt, 0, spi_dat_buf, 0);
        led_spi_rgb_to_24byte(0, 0, cnt, spi_dat_buf, 1);
#if 1
        led_spi_send_rgbbuf(spi_dat_buf, 2);        //等待的方式，建议用在发的数据量小的场合
#else
        led_spi_send_rgbbuf_isr(spi_dat_buf, 2);    //中断的方式，建议用在发的数据量大的场合
#endif
        os_time_dly(2);
        wdt_clear();
    }
}



///////////////////////////////////////////////////////////////////////////


int mic_energy_value = -90;
int mic_energy_level = 0;


int mic_energy_level_set(int value)
{
    if(-50 <= value && value <= -40)      //一档
        mic_energy_level = 0;
    else if(-40 < value && value <= -35)    //二档
        mic_energy_level = 1;
    else if(-35 < value && value <= -28)    //三档
        mic_energy_level = 2;
    else if(-28 < value && value <= -19)    //四档
        mic_energy_level = 3;
    else if(-19 < value && value <= -5)      //五档
        mic_energy_level = 4;
    return mic_energy_level;
}


u16 user_spi_led_mode = 4;
void user_mode_set(int mode,int cnt)
{
    switch(mode)
    {
        case 0:
            led_spi_rgb_to_24byte(0, 0, 0, spi_dat_buf, 1);
            led_spi_rgb_to_24byte(0, 0, 0, spi_dat_buf, 2);
            led_spi_rgb_to_24byte(0, 0, 0, spi_dat_buf, 3);
            led_spi_rgb_to_24byte(0, 0, 0, spi_dat_buf, 4);
            led_spi_rgb_to_24byte(0, 0, 0, spi_dat_buf, 5);
            break;
        case 1:
            led_spi_rgb_to_24byte(0, 255 - cnt, 0, spi_dat_buf, 1);
            led_spi_rgb_to_24byte(0, 255 - cnt, 0, spi_dat_buf, 2);
            led_spi_rgb_to_24byte(0, 255 - cnt, 0, spi_dat_buf, 3);
            led_spi_rgb_to_24byte(0, 255 - cnt, 0, spi_dat_buf, 4);
            led_spi_rgb_to_24byte(0, 255 - cnt, 0, spi_dat_buf, 5);
            break;
        case 2:
            led_spi_rgb_to_24byte(0, 0, 255 - cnt, spi_dat_buf, 1);
            led_spi_rgb_to_24byte(0, 0, 255 - cnt, spi_dat_buf, 2);
            led_spi_rgb_to_24byte(0, 0, 255 - cnt, spi_dat_buf, 3);
            led_spi_rgb_to_24byte(0, 0, 255 - cnt, spi_dat_buf, 4);
            led_spi_rgb_to_24byte(0, 0, 255 - cnt, spi_dat_buf, 5);
            break;
        case 3:
            led_spi_rgb_to_24byte(0, 255, 0, spi_dat_buf, 1);
            led_spi_rgb_to_24byte(0, 255, 0, spi_dat_buf, 2);
            led_spi_rgb_to_24byte(0,255, 0, spi_dat_buf, 3);
            led_spi_rgb_to_24byte(255,255, 0, spi_dat_buf, 4);
            led_spi_rgb_to_24byte(255, 0, 0, spi_dat_buf, 5);
            break;
        case 4:
            switch(mic_energy_level_set(mic_energy_value))
            {
            case 0:
                led_spi_rgb_to_24byte(0, 70, 0, spi_dat_buf, 1);
                led_spi_rgb_to_24byte(0, 0, 0, spi_dat_buf, 2);
                led_spi_rgb_to_24byte(0,0, 0, spi_dat_buf, 3);
                led_spi_rgb_to_24byte(0,0, 0, spi_dat_buf, 4);
                led_spi_rgb_to_24byte(0, 0, 0, spi_dat_buf, 5);
                break;
            case 1:
                led_spi_rgb_to_24byte(0, 70, 0, spi_dat_buf, 1);
                led_spi_rgb_to_24byte(0, 70, 0, spi_dat_buf, 2);
                led_spi_rgb_to_24byte(0,0, 0, spi_dat_buf, 3);
                led_spi_rgb_to_24byte(0,0, 0, spi_dat_buf, 4);
                led_spi_rgb_to_24byte(0, 0, 0, spi_dat_buf, 5);
                break;
            case 2:
                led_spi_rgb_to_24byte(0, 70, 0, spi_dat_buf, 1);
                led_spi_rgb_to_24byte(0, 70, 0, spi_dat_buf, 2);
                led_spi_rgb_to_24byte(0,70, 0, spi_dat_buf, 3);
                led_spi_rgb_to_24byte(0,0, 0, spi_dat_buf, 4);
                led_spi_rgb_to_24byte(0, 0, 0, spi_dat_buf, 5);
                break;
            case 3:
                led_spi_rgb_to_24byte(0, 70, 0, spi_dat_buf, 1);
                led_spi_rgb_to_24byte(0, 70, 0, spi_dat_buf, 2);
                led_spi_rgb_to_24byte(0,70, 0, spi_dat_buf, 3);
                led_spi_rgb_to_24byte(40,40, 0, spi_dat_buf, 4);
                led_spi_rgb_to_24byte(0, 0, 0, spi_dat_buf, 5);
                break;
            case 4:
                led_spi_rgb_to_24byte(0, 70, 0, spi_dat_buf, 1);
                led_spi_rgb_to_24byte(0, 70, 0, spi_dat_buf, 2);
                led_spi_rgb_to_24byte(0, 70, 0, spi_dat_buf, 3);
                led_spi_rgb_to_24byte(40,40, 0, spi_dat_buf, 4);
                led_spi_rgb_to_24byte(50, 0, 0, spi_dat_buf, 5);
                break;
            }
            break;

    }
}
static int mode = 0;

void rgb_display()
{
     static  u8 cnt = 0;
        //cnt ++;
        //#if(RGB_MODE == 1)
        /*led_spi_rgb_to_24byte(255, 0, 0, spi_dat_buf, 0);
        led_spi_rgb_to_24byte(255, 0, 0, spi_dat_buf, 1);
        led_spi_rgb_to_24byte(255, 0, 0, spi_dat_buf, 2);
        led_spi_rgb_to_24byte(255, 0, 0, spi_dat_buf, 3);
        led_spi_rgb_to_24byte(255, 0, 0, spi_dat_buf, 4);
        #elif(RGB_MODE == 2)
        led_spi_rgb_to_24byte(255 - cnt, 0, 0, spi_dat_buf, 1);
        led_spi_rgb_to_24byte(255 - cnt, 0, 0, spi_dat_buf, 2);
        led_spi_rgb_to_24byte(255 - cnt, 0, 0, spi_dat_buf, 3);
        led_spi_rgb_to_24byte(255 - cnt, 0, 0, spi_dat_buf, 4);
        //led_spi_rgb_to_24byte(255 - cnt, 0, 0, spi_dat_buf, 5);
        #elif(RGB_MODE == 3)
        led_spi_rgb_to_24byte(0, 0, 255, spi_dat_buf, 0);
        led_spi_rgb_to_24byte(0, 0, 255, spi_dat_buf, 1);
        led_spi_rgb_to_24byte(0, 0, 255, spi_dat_buf, 2);
        led_spi_rgb_to_24byte(0, 0, 255, spi_dat_buf, 3);
        led_spi_rgb_to_24byte(0, 0, 255, spi_dat_buf, 4);
        #elif(RGB_MODE == 4)
        led_spi_rgb_to_24byte(0, 0, 255, spi_dat_buf, 0);
        led_spi_rgb_to_24byte(0, 255, 0, spi_dat_buf, 1);
        led_spi_rgb_to_24byte(255, 0, 0, spi_dat_buf, 2);
        led_spi_rgb_to_24byte(0, 0, 255, spi_dat_buf, 3);
        led_spi_rgb_to_24byte(0, 255, 255, spi_dat_buf, 4);
        #endif // if
        */

       /* if(cnt == 255)
        {
            is_max_value = 1;
            if(mode < 2)
                mode++;
            else
                mode = 0;
            //cnt--;
        }
        else if(cnt == 0){
            is_max_value = 0;
            //cnt++;
        }
        if(is_max_value)
        {
            cnt--;
        }
        else{
            cnt++;
        }*/
        user_mode_set(user_spi_led_mode,0);

#if 1
        led_spi_send_rgbbuf(spi_dat_buf, 6);        //等待的方式，建议用在发的数据量小的场合
        //spi_dma_send(SPI1,spi_dat_buf,sizeof(spi_dat_buf));
#else
        led_spi_send_rgbbuf_isr(spi_dat_buf, 5);    //中断的方式，建议用在发的数据量大的场合
#endif
        //os_time_dly(1);
        //wdt_clear();
}

static u16 rgb_time = 0;
void rgb_display_add()
{
    if(!rgb_time)
    {
        rgb_time = sys_timer_add(NULL,rgb_display,20);
    }
}


void rgb_display_del()
{
    if(rgb_time)
    {
        sys_timer_del(rgb_time);
        rgb_time = 0;
    }
}


void user_rgb_display()
{
    led_spi_init();
    //spi_set_baud(SPI1,)
    rgb_display_add();
    //rgb_display();
}

//////////////////////////////////////////////////////////////////////////

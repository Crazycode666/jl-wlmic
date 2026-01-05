#include "asm/power_interface.h"
#include "app_config.h"
#include "asm/cpu.h"
#include "asm/gpio.h"

static u8 gpiousb = 0x3;
static JL_PORT_FLASH_TypeDef usb_port;

/* cpu公共流程：
 * 请勿添加板级相关的流程，例如宏定义
 * 可以重写改流程
 * 所有io保持原状，除usb io
 */
void sleep_gpio_protect(u32 gpio)
{
    if (gpio == IO_PORT_DP) {
        gpiousb &= ~BIT(0);
    } else if (gpio == IO_PORT_DM) {
        gpiousb &= ~BIT(1);
    }
}

void sleep_enter_callback_common(void *priv)
{
    usb_port.OUT  = JL_PORTUSB->OUT ;
    usb_port.DIR  = JL_PORTUSB->DIR ;
    usb_port.DIE  = JL_PORTUSB->DIE ;
    usb_port.DIEH = JL_PORTUSB->DIEH;
    usb_port.PU0  = JL_PORTUSB->PU0 ;
    usb_port.PD0  = JL_PORTUSB->PD0 ;
    usb_port.HD0  = JL_PORTUSB->HD0 ;
    usb_port.HD1  = JL_PORTUSB->HD1 ;
    usb_port.SPL  = JL_PORTUSB->SPL ;
    usb_port.CON  = JL_PORTUSB->CON ;

    if (gpiousb) {
        usb_iomode(1);

        if (gpiousb & BIT(0)) {
            gpio_set_pull_up(IO_PORT_DP, 0);
            gpio_set_pull_down(IO_PORT_DP, 0);
            gpio_set_direction(IO_PORT_DP, 1);
            gpio_set_die(IO_PORT_DP, 0);
            gpio_set_dieh(IO_PORT_DP, 0);
        }

        if (gpiousb & BIT(1)) {
            gpio_set_pull_up(IO_PORT_DM, 0);
            gpio_set_pull_down(IO_PORT_DM, 0);
            gpio_set_direction(IO_PORT_DM, 1);
            gpio_set_die(IO_PORT_DM, 0);
            gpio_set_dieh(IO_PORT_DM, 0);
        }
    }
}

void sleep_exit_callback_common(void *priv)
{
    JL_PORTUSB->OUT   = usb_port.OUT;
    JL_PORTUSB->DIR   = usb_port.DIR;
    JL_PORTUSB->DIE   = usb_port.DIE;
    JL_PORTUSB->DIEH  = usb_port.DIEH;
    JL_PORTUSB->PU0   = usb_port.PU0;
    JL_PORTUSB->PD0   = usb_port.PD0;
    JL_PORTUSB->HD0   = usb_port.HD0;
    JL_PORTUSB->HD1   = usb_port.HD1;
    JL_PORTUSB->SPL   = usb_port.SPL;
    JL_PORTUSB->CON   = usb_port.CON;

    gpiousb = 0x3;
}

static struct gpio_value soff_gpio_config = {
    .gpioa = 0xffff,
    .gpiob = 0xffff,
    .gpioc = 0xffff,
    .gpiod = 0xffff,
    .gpiop = 0x1,//
    .gpiousb = 0x3,
};

void soff_gpio_protect(u32 gpio)
{
    if ((gpio >= 0) && (gpio < IO_MAX_NUM)) {
        port_protect((u16 *)&soff_gpio_config, gpio);
    } else if (gpio == IO_PORT_DP) {
        soff_gpio_config.gpiousb &= ~BIT(0);
    } else if (gpio == IO_PORT_DM) {
        soff_gpio_config.gpiousb &= ~BIT(1);
    }
}

/* cpu公共流程：
 * 请勿添加板级相关的流程，例如宏定义
 * 可以重写改流程
 * 释放除内置flash外的所有io
 */

//maskrom 使用到的io
void mask_io_cfg()
{
    struct boot_soft_flag_t boot_soft_flag = {0};

    boot_soft_flag.flag0.boot_ctrl.flash_power_keep = 0;
    boot_soft_flag.flag0.boot_ctrl.skip_flash_reset = 0;
    boot_soft_flag.flag0.boot_ctrl.sfc_fast_boot = 0;

    boot_soft_flag.flag1.usb_io.usbdm = SOFTFLAG_HIGH_RESISTANCE;
    boot_soft_flag.flag1.usb_io.usbdp = SOFTFLAG_HIGH_RESISTANCE;

    boot_soft_flag.flag2.pa5_pa6.pa5 = SOFTFLAG_HIGH_RESISTANCE;
    boot_soft_flag.flag2.pa5_pa6.pa6 = SOFTFLAG_HIGH_RESISTANCE;

    boot_soft_flag.flag3.pp0_io.pp0 = SOFTFLAG_HIGH_RESISTANCE;
    boot_soft_flag.flag3.pp0_io.disable_uart_upgrade = 0;


    mask_softflag_config(&boot_soft_flag);
}

void board_set_soft_poweroff_common(void *priv)
{
    //flash电源
    if (GET_SFC_PORT() == 0) {
        soff_gpio_protect(SPI0_PWR_A);
        soff_gpio_protect(SPI0_CS_A);
        soff_gpio_protect(SPI0_CLK_A);
        soff_gpio_protect(SPI0_DO_D0_A);
        soff_gpio_protect(SPI0_DI_D1_A);
        if (get_sfc_bit_mode() == 4) {
            soff_gpio_protect(SPI0_WP_D2_A);
            soff_gpio_protect(SPI0_HOLD_D3_A);
        }
    } else {


    }

    if (IS_MCLR_EN()) {
        soff_gpio_protect(MCLR_PORT);
    }

    mask_io_cfg();

    gpio_dir(GPIOA,    0, 16,  soff_gpio_config.gpioa, GPIO_OR);
    gpio_set_pu(GPIOA, 0, 16, ~soff_gpio_config.gpioa, GPIO_AND);
    gpio_set_pd(GPIOA, 0, 16, ~soff_gpio_config.gpioa, GPIO_AND);
    gpio_die(GPIOA,    0, 16, ~soff_gpio_config.gpioa, GPIO_AND);
    gpio_dieh(GPIOA,   0, 16, ~soff_gpio_config.gpioa, GPIO_AND);

    gpio_dir(GPIOB,    0, 16,  soff_gpio_config.gpiob, GPIO_OR);
    gpio_set_pu(GPIOB, 0, 16, ~soff_gpio_config.gpiob, GPIO_AND);
    gpio_set_pd(GPIOB, 0, 16, ~soff_gpio_config.gpiob, GPIO_AND);
    gpio_die(GPIOB,    0, 16, ~soff_gpio_config.gpiob, GPIO_AND);
    gpio_dieh(GPIOB,   0, 16, ~soff_gpio_config.gpiob, GPIO_AND);

    gpio_dir(GPIOC,    0, 16,  soff_gpio_config.gpioc, GPIO_OR);
    gpio_set_pu(GPIOC, 0, 16, ~soff_gpio_config.gpioc, GPIO_AND);
    gpio_set_pd(GPIOC, 0, 16, ~soff_gpio_config.gpioc, GPIO_AND);
    gpio_die(GPIOC,    0, 16, ~soff_gpio_config.gpioc, GPIO_AND);
    gpio_dieh(GPIOC,   0, 16, ~soff_gpio_config.gpioc, GPIO_AND);

    gpio_dir(GPIOD,    0, 16,  soff_gpio_config.gpiod, GPIO_OR);
    gpio_set_pu(GPIOD, 0, 16, ~soff_gpio_config.gpiod, GPIO_AND);
    gpio_set_pd(GPIOD, 0, 16, ~soff_gpio_config.gpiod, GPIO_AND);
    gpio_die(GPIOD,    0, 16, ~soff_gpio_config.gpiod, GPIO_AND);
    gpio_dieh(GPIOD,   0, 16, ~soff_gpio_config.gpiod, GPIO_AND);

    gpio_dir(GPIOP,    0, 1,   soff_gpio_config.gpiop, GPIO_OR);
    gpio_set_pu(GPIOP, 0, 1,  ~soff_gpio_config.gpiop, GPIO_AND);
    gpio_set_pd(GPIOP, 0, 1,  ~soff_gpio_config.gpiop, GPIO_AND);
    gpio_die(GPIOP,    0, 1,  ~soff_gpio_config.gpiop, GPIO_AND);
    gpio_dieh(GPIOP,   0, 1,  ~soff_gpio_config.gpiop, GPIO_AND);

    if (soff_gpio_config.gpiousb) {
        usb_iomode(1);

        if (soff_gpio_config.gpiousb & BIT(0)) {
            gpio_set_pull_up(IO_PORT_DP, 0);
            gpio_set_pull_down(IO_PORT_DP, 0);
            gpio_set_direction(IO_PORT_DP, 1);
            gpio_set_die(IO_PORT_DP, 0);
            gpio_set_dieh(IO_PORT_DP, 0);
        }

        if (soff_gpio_config.gpiousb & BIT(1)) {
            gpio_set_pull_up(IO_PORT_DM, 0);
            gpio_set_pull_down(IO_PORT_DM, 0);
            gpio_set_direction(IO_PORT_DM, 1);
            gpio_set_die(IO_PORT_DM, 0);
            gpio_set_dieh(IO_PORT_DM, 0);
        }
    }
}



#include "asm/includes.h"
//#include "asm/ldo.h"
//#include "asm/cache.h"
#include "asm/wdt.h"
#include "asm/debug.h"
#include "asm/efuse.h"
#include "asm/power_interface.h"
#include "system/task.h"
#include "system/bank_switch.h"
#include "timer.h"
#include "asm/clock.h"
#include "app_config.h"
#include "gpio.h"

#include "asm/power/power_api.h"
#define LOG_TAG_CONST       SETUP
#define LOG_TAG             "[SETUP]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

//extern void dv15_dac_early_init(u8 ldo_sel, u8 pwr_sel, u32 dly_msecs);
//
extern void sys_timer_init(void);

extern void tick_timer_init(void);

extern void vPortSysSleepInit(void);

extern void exception_irq_handler(void);
int __crc16_mutex_init();

extern int __crc16_mutex_init();

#define DEBUG_SINGAL_IDLE(x)        //if (x) IO_DEBUG_1(A, 7) else IO_DEBUG_0(A, 7)
#define DEBUG_SINGAL_1S(x)          //if (x) IO_DEBUG_1(A, 6) else IO_DEBUG_0(A, 6)

#if (defined CONFIG_DEBUG_ENABLE) || (defined CONFIG_DEBUG_LITE_ENABLE)
void debug_uart_init(const struct uart_platform_data *data);
#endif

#if 0
___interrupt
void exception_irq_handler(void)
{
    ___trig;

    exception_analyze();

    log_flush();
    while (1);
}
#endif



/*
 * 此函数在cpu0上电后首先被调用,负责初始化cpu内部模块
 *
 * 此函数返回后，操作系统才开始初始化并运行
 *
 */

#if 0
static void early_putchar(char a)
{
    if (a == '\n') {
        UT2_BUF = '\r';
        __asm_csync();
        while ((UT2_CON & BIT(15)) == 0);
    }
    UT2_BUF = a;
    __asm_csync();
    while ((UT2_CON & BIT(15)) == 0);
}

void early_puts(char *s)
{
    do {
        early_putchar(*s);
    } while (*(++s));
}
#endif

void cpu_assert_debug()
{
#ifdef CONFIG_DEBUG_ENABLE
    log_flush();
    local_irq_disable();
    while (1);
#else
    p33_tx_1byte(P3_SFLAG0, 0xac);
    cpu_reset();
#endif
}

//AT(.psram_code)
static u32 timer_cnt = 0;
void timer(void *p)
{
    /* DEBUG_SINGAL_1S(1); */

    mem_stats();

    os_system_info_output();
    /* os_system_info_reset(); */

    sys_timer_dump();
    usr_timer_dump();

    /* DEBUG_SINGAL_1S(0);*/
}

extern void sputchar(char c);
extern void sput_buf(const u8 *buf, int len);
void sput_u32hex(u32 dat);
void *vmem_get_phy_adr(void *vaddr);

void test_fun()
{
    wdt_close();
    while (1);

}


__attribute__((weak))
void maskrom_init(void)
{
    return;
}

static void (*uart_db_irq_handler_callback)(u8 *packet, u32 size);
extern void uart_dev_set_irq_handler_hook(void *uart_irq_hook);
static void uart_db_irq_handler_hook(u8 *rbuf, u32 len)
{
    if (len) {
        log_info("uart_rx_data %d:", len);
        put_buf(rbuf, len);
    }

    if (uart_db_irq_handler_callback) {
        uart_db_irq_handler_callback(rbuf, len);
    }
}

void uart_db_regiest_recieve_callback(void *rx_cb)
{
    uart_db_irq_handler_callback = rx_cb;
}

static void app_test_handler(void *priv)
{
    int ret;
    int msg[32];

    printf("%s: >>>>>>", __func__);

    while (1) {
        ret = os_taskq_pend(NULL, msg, ARRAY_SIZE(msg));
        printf("ret = 0x%x", ret);
        wdt_clear();
    }
}

static void br29_system_test(void)
{
    task_create(app_test_handler, NULL, "app_core");

    os_start();

    local_irq_enable();

    while (1) {
        asm("idle");
    }
}

void app_bank_init()
{
    /*request_irq(IRQ_SYSCALL_IDX, 3, bank_syscall_entry, 0);*/
    load_common_code();
}

void boot_init_hook(void *_info)
{
    tick_timer_init();
}

void memory_init(void);
void setup_arch()
{
    q32DSP(core_num())->PMU_CON1 &= ~BIT(8); //open bpu

#if (defined TCFG_CHIP_RESET_PIN) && (TCFG_CHIP_RESET_PIN != NO_CONFIG_PORT)
    gpio_longpress_pin0_reset_config(TCFG_CHIP_RESET_PIN, TCFG_CHIP_RESET_LEVEL, TCFG_CHIP_RESET_TIME);
#endif

    //上电初始所有io、及硬件
    port_init();

    memory_init();

    wdt_init(WDT_8S);

    power_set_default_mode(TCFG_LOWPOWER_POWER_SEL);

#if TCFG_CLOCK_PLL_240MHZ
    clock_set_pll_target_frequency(240);
#endif

    clk_voltage_init(TCFG_CLOCK_MODE, DVDD_DEFAULT_VOL);

    clk_early_init(TCFG_CLOCK_SYS_SRC, TCFG_CLOCK_OSC_HZ, TCFG_CLOCK_SYS_HZ);

    /*interrupt_init();*/

#if (defined CONFIG_DEBUG_ENABLE) || (defined CONFIG_DEBUG_LITE_ENABLE)
    debug_uart_init(NULL);

#if TCFG_UART0_RX_PORT != NO_CONFIG_PORT
    JL_UART0->CON0 &= ~BIT(2); //disable Tx pending Enable
    uart_dev_set_irq_handler_hook(uart_db_irq_handler_hook);

#if TCFG_LOWPOWER_LOWPOWER_SEL
    /*需要关闭POWERDOWN,否则接收会丢数据*/
#error "need define TCFG_LOWPOWER_LOWPOWER_SEL  0 !!!!!!"
#endif
#endif

#ifdef CONFIG_DEBUG_ENABLE
    log_early_init(1024);
#endif

#endif

    mem_stats();
    log_i("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    log_i("         setup_arch %s %s", __DATE__, __TIME__);
    log_i("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    clock_dump();

    power_reset_source_dump();

    power_wakeup_reason_dump();

    request_irq(1, 2, exception_irq_handler, 0);

    debug_init();

    sys_timer_init();

    /* sys_timer_add(NULL, timer, 5 * 1000); */

    __crc16_mutex_init();
}

/*-----------------------------------------------------------*/


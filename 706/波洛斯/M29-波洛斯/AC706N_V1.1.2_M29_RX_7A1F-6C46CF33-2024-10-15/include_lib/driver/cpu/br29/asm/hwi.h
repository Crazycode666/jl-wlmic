#ifndef HWI_H
#define HWI_H


//=================================================
#define IRQ_EMUEXCPT_IDX   		0
#define IRQ_EXCEPTION_IDX  		1
#define IRQ_SYSCALL_IDX    		2
#define IRQ_TICK_TMR_IDX   		3
#define IRQ_TIME0_IDX      		4
#define IRQ_TIME1_IDX      		5
#define IRQ_TIME2_IDX      		6
#define IRQ_UART0_IDX      		7
#define IRQ_UART1_IDX      		8
#define IRQ_SPI0_IDX       		9
#define IRQ_SPI1_IDX       		10

#define IRQ_IIC_IDX        		11
#define IRQ_PORT_IDX		   	12
#define IRQ_GPADC_IDX		   	13
#define IRQ_SBC_IDX			   	14
#define IRQ_LRCT_IDX		   	15
#define IRQ_GPC_IDX			   	16
#define IRQ_RDEC0_IDX		   	17
#define IRQ_SD0_IDX        		18
#define IRQ_USB_SOF_IDX         19
#define IRQ_USB_CTRL_IDX        20 //USB_SIE
#define IRQ_FMRX_IDX		   	21
#define IRQ_TIME3_IDX      		22
#define IRQ_PWM_LED_IDX		   	23
#define IRQ_MCPWM_CHX_IDX       24
#define IRQ_MCPWM_TMR_IDX       25

#define IRQ_PMU_SOFT_IDX	  	26 //
#define IRQ_LP_TIMER0_IDX  		27
#define IRQ_LP_TIMER1_IDX  		28

#define IRQ_AUDIO_IDX      		36
#define IRQ_SYNC_IDX      		37
#define IRQ_ALINK0_IDX     		38
#define IRQ_BT_CLKN_IDX    		39
#define IRQ_BT_DBG_IDX     		40
#define IRQ_BLE_RX_IDX     		41
#define IRQ_BLE_EVENT_IDX  		42
#define IRQ_BT_TIMEBASE_IDX  	43
#define IRQ_WL_LOFC_IDX    		44
#define IRQ_BREDR_IDX      		45
#define IRQ_EQ_IDX         		46
#define IRQ_AES_IDX        		47
#define IRQ_SRC_SYNC_IDX        48
#define IRQ_FFT_IDX	       		56

#define IRQ_SOFT0_IDX      		60
#define IRQ_SOFT1_IDX      		61
#define IRQ_SOFT2_IDX      		62
#define IRQ_SOFT3_IDX      		63


//=================================================

//=================================================

void interrupt_init();


/* --------------------------------------------------------------------------*/
/**
 * @brief 中断注册函数
 *
 * @param index 中断号
 * @param priority 优先级，范围0-6可用
 * @param handler 中断服务函数
 * @param cpu_id 相应中断服务函数的CPU
 */
/* ----------------------------------------------------------------------------*/
void request_irq(u8 index, u8 priority, void (*handler)(void), u8 cpu_id);

void unrequest_irq(u8 index);

void reg_set_ip(unsigned char index, unsigned char priority, u8 cpu_id);


/* --------------------------------------------------------------------------*/
/**
 * @brief 设置不可屏蔽中断（不可屏蔽中断不区分优先级）
 *
 * @param index 中断号
 * @param cpu_id 相应中断服务函数的CPU
 */
/* ----------------------------------------------------------------------------*/
void irq_unmask_set(u8 index, u8 cpu_id);

void bit_clr_ie(unsigned char index);
void bit_set_ie(unsigned char index);
bool irq_read(u32 index);

#define irq_disable(x)  bit_clr_ie(x)
#define irq_enable(x)   bit_set_ie(x)

//---------------------------------------------//
// low power waiting
//---------------------------------------------//
__attribute__((always_inline))
static void lp_waiting(int *ptr, int pnd, int cpd, char inum)
{
    q32DSP(core_num())->IWKUP_NUM = inum;
    while (!(*ptr & pnd)) {
        asm volatile("idle");
    }
    *ptr |= cpd;
}

//---------------------------------------------//
// interrupt cli/sti
//---------------------------------------------//

static inline int int_cli(void)
{
    int msg;
    asm volatile("cli %0" : "=r"(msg) :);
    return msg;
}

static inline void int_sti(int msg)
{
    asm volatile("sti %0" :: "r"(msg));
}

#ifdef IRQ_TIME_COUNT_EN
void irq_handler_enter(int irq);

void irq_handler_exit(int irq);

void irq_handler_times_dump();
#else

#define irq_handler_enter(irq)      do { }while(0)
#define irq_handler_exit(irq)       do { }while(0)
#define irq_handler_times_dump()    do { }while(0)

#endif


#endif


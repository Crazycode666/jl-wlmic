/**
 * @file gpio.h
 * @brief
 * @author @zh-jieli.com
 * @version 1.0.0
 * @date 2018-10-11
 */

#ifndef  __GPIO_H__
#define  __GPIO_H__

#include "typedef.h"

// TODO
#define sfc_boot_port()     0


#define IO_GROUP_NUM 		16

#define IO_PORTA_00 				(IO_GROUP_NUM * 0 + 0)
#define IO_PORTA_01 				(IO_GROUP_NUM * 0 + 1)
#define IO_PORTA_02 				(IO_GROUP_NUM * 0 + 2)
#define IO_PORTA_03 				(IO_GROUP_NUM * 0 + 3)
#define IO_PORTA_04 				(IO_GROUP_NUM * 0 + 4)
#define IO_PORTA_05 				(IO_GROUP_NUM * 0 + 5)
#define IO_PORTA_06 				(IO_GROUP_NUM * 0 + 6)

#define IO_PORTB_00 				(IO_GROUP_NUM * 1 + 0)
#define IO_PORTB_01 				(IO_GROUP_NUM * 1 + 1)
#define IO_PORTB_02 				(IO_GROUP_NUM * 1 + 2)
#define IO_PORTB_03 				(IO_GROUP_NUM * 1 + 3)
#define IO_PORTB_04 				(IO_GROUP_NUM * 1 + 4)
#define IO_PORTB_05 				(IO_GROUP_NUM * 1 + 5)
#define IO_PORTB_06 				(IO_GROUP_NUM * 1 + 6)
#define IO_PORTB_07 				(IO_GROUP_NUM * 1 + 7)

#define IO_PORTC_00 				(IO_GROUP_NUM * 2 + 0)
#define IO_PORTC_01 				(IO_GROUP_NUM * 2 + 1)
#define IO_PORTC_02 				(IO_GROUP_NUM * 2 + 2)
#define IO_PORTC_03 				(IO_GROUP_NUM * 2 + 3)
#define IO_PORTC_04 				(IO_GROUP_NUM * 2 + 4)
#define IO_PORTC_05 				(IO_GROUP_NUM * 2 + 5)

#define IO_PORTD_00 				(IO_GROUP_NUM * 3 + 0)
#define IO_PORTD_01 				(IO_GROUP_NUM * 3 + 1)
#define IO_PORTD_02 				(IO_GROUP_NUM * 3 + 2)
#define IO_PORTD_03 				(IO_GROUP_NUM * 3 + 3)

#define IO_PORTP_00 				(IO_GROUP_NUM * 4 + 0)

#define IO_PORT_DP   				(IO_GROUP_NUM * 5 + 0)
#define IO_PORT_DM 	    			(IO_GROUP_NUM * 5 + 1)

#define IO_MAX_NUM 					(IO_PORT_DM + 1)

#define P33_IO_OFFSET               0
#define IO_CHGFL_DET                (IO_MAX_NUM + P33_IO_OFFSET + 0)
#define IO_VBGOK_DET                (IO_MAX_NUM + P33_IO_OFFSET + 1)
#define IO_VBTCH_DET                (IO_MAX_NUM + P33_IO_OFFSET + 2)
#define IO_LDOIN_DET                (IO_MAX_NUM + P33_IO_OFFSET + 3)
#define IO_SPDIF_RX_DET             (IO_MAX_NUM + P33_IO_OFFSET + 4)

#define IO_PORT_MAX					(IO_MAX_NUM + 1)

#define GPIOA                       (IO_GROUP_NUM * 0)
#define GPIOB                       (IO_GROUP_NUM * 1)
#define GPIOC                       (IO_GROUP_NUM * 2)
#define GPIOD                       (IO_GROUP_NUM * 3)
#define GPIOP                       (IO_GROUP_NUM * 4)
#define GPIOUSB                     (IO_GROUP_NUM * 5)
#define GPIOP33                     (IO_MAX_NUM + P33_IO_OFFSET)

enum {
    INPUT_CH0,
    INPUT_CH1,
    INPUT_CH2,
    INPUT_CH3,
};

enum {
    LOW_POWER_FREE = 0,
    LOW_POWER_KEEP = 1,
};
enum gpio_op_mode {
    GPIO_SET = 1,
    GPIO_AND,
    GPIO_OR,
    GPIO_XOR,
};
enum gpio_direction {
    GPIO_OUT = 0,
    GPIO_IN = 1,
};
struct gpio_reg {
    volatile unsigned int in;
    volatile unsigned int out;
    volatile unsigned int dir;
    volatile unsigned int die;
    volatile unsigned int dieh;
    volatile unsigned int pu;
    volatile unsigned int pd;
    volatile unsigned int hd0;
    volatile unsigned int hd;
    volatile unsigned int spl;
    volatile unsigned int con;
};

struct gpio_platform_data {
    unsigned int gpio;
};

#define GPIO_PLATFORM_DATA_BEGIN(data) \
	static const struct gpio_platform_data data = { \


#define GPIO_PLATFORM_DATA_END() \
	};



/**
 * @brief usb_iomode
 *
 * @param enable 1，使能；0，关闭
 */
void usb_iomode(u32 enable);
/**
 * @brief gpio_direction_input
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param dir 1，输入；0，输出
 *
 * @return
 */
int gpio_set_direction(u32 gpio, u32 dir);


/**
 * @brief gpio_direction_input
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param value 1，输出1,  0，输出0
 *
 * @return
 */
int gpio_set_output_value(u32 gpio, u32 dir);

/**
 * @brief gpio_dir
 *
 * @param gpio [GPIOA GPIOB GPIOC GPIOD GPIOR GPIOUSB]
 * @param start [0-15]，GPIOR[0-3]，GPIOUSB[0-1]
 * @param len  [0-15]，GPIOR[0-3]，GPIOUSB[0-1]
 * @param dat 1，输入；0，输出
 *
 * @return
 */
u32 gpio_dir(u32 gpio, u32 start, u32 len, u32 dat, enum gpio_op_mode op);

/**
 * @brief gpio_direction_output
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param value 1，输出1；0，输出0
 *
 * @return
 */
int gpio_direction_output(u32 gpio, int value);

/**
 * @brief gpio_out
 *
 * @param gpio [GPIOA GPIOB GPIOC GPIOD GPIOR GPIOUSB]
 * @param start [0-15]，GPIOR[0-3]，GPIOUSB[0-1]
 * @param len  [0-15]，GPIOR[0-3]，GPIOUSB[0-1]
 * @param dat 1，输入；0，输出
 *
 * @return
 */
u32 gpio_out(u32 gpio, u32 start, u32 len, u32 dat);

/**
 * @brief gpio_set_pull_up
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param value 1，上拉；0，不上拉
 *
 * @return
 */
int gpio_set_pull_up(u32 gpio, int value);


/**
 * @brief gpio_set_pu
 *
 * @param gpio [GPIOA GPIOB GPIOC GPIOD GPIOR GPIOUSB]
 * @param start [0-15]，GPIOR[0-3]，GPIOUSB[0-1]
 * @param len  [0-15]，GPIOR[0-3]，GPIOUSB[0-1]
 * @param dat 1，上拉；0，不上拉
 *
 * @return
 */
u32 gpio_set_pu(u32 gpio, u32 start, u32 len, u32 dat, enum gpio_op_mode op);

/**
 * @brief gpio_set_pull_down
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param value 1，下拉；0，不下拉
 *
 * @return
 */
int gpio_set_pull_down(u32 gpio, int value);

/**
 * @brief gpio_set_pd
 *
 * @param gpio [GPIOA GPIOB GPIOC GPIOD GPIOR GPIOUSB]
 * @param start [0-15]，GPIOR[0-3]，GPIOUSB[0-1]
 * @param len  [0-15]，GPIOR[0-3]，GPIOUSB[0-1]
 * @param dat 1，下拉；0，不下拉
 *
 * @return
 */
u32 gpio_set_pd(u32 gpio, u32 start, u32 len, u32 dat, enum gpio_op_mode op);

enum GPIO_HDRIVE {
    GPIO_HDRIVE_2_4_MA = 0, //2.4mA
    GPIO_HDRIVE_8_MA,    //8mA
    GPIO_HDRIVE_26_4_MA, //26.4mA
    GPIO_HDRIVE_40_MA,   //40mA
};
/**
 * @brief gpio_set_hd
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param value see enum GPIO_HDRIVE
 *
 * @return 0:ok, <0:fail
 */
int gpio_set_hd(u32 gpio, int value);

/**
 * @brief gpio_set_die
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param value 1，IO普通输入；0，IO模拟输入
 *
 * @return
 */
int gpio_set_die(u32 gpio, int value);

/**
 * @brief gpio_set_dieh
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param value 1，IO普通输入；0，IO模拟输入
 *
 * @return
 */
u32 gpio_set_dieh(u32 gpio, u32 value);

/**
 * @brief gpio_die
 *
 * @param gpio [GPIOA GPIOB GPIOC GPIOD GPIOR GPIOUSB]
 * @param start [0-15]，GPIOR[0-3]，GPIOUSB[0-1]
 * @param len  [0-15]，GPIOR[0-3]，GPIOUSB[0-1]
 * @param dat 1，IO普通输入；0，IO模拟输入
 *
 * @return
 */
u32 gpio_die(u32 gpio, u32 start, u32 len, u32 dat, enum gpio_op_mode op);

/**
 * @brief gpio_dieh
 *
 * @param gpio [GPIOA GPIOB GPIOC GPIOD GPIOR GPIOUSB]
 * @param start [0-15]，GPIOR[0-3]，GPIOUSB[0-1]
 * @param len  [0-15]，GPIOR[0-3]，GPIOUSB[0-1]
 * @param dat 1，IO普通输入；0，IO模拟输入
 *
 * @return
 */
u32 gpio_dieh(u32 gpio, u32 start, u32 len, u32 dat, enum gpio_op_mode op);

/**
 * @brief gpio_set_output_channle
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param clk 参考枚举CHx_UTx_TX，如CH0_UT0_TX
 *
 * @return
 */
u32 gpio_output_channle(u32 gpio, u32 clk);

/**
 * @brief gpio_read
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 *
 * @return
 */
int gpio_read(u32 gpio);

/**
 * @brief gpio_in
 *
 * @param gpio [GPIOA GPIOB GPIOC GPIOD GPIOR GPIOUSB]
 *
 * @return
 */
u32 gpio_in(u32 gpio);
/**
 * @brief gpio_write
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 * @param value 1，输出1；0，输出0
 *
 * @return
 */
u32 gpio_write(u32 gpio, u32 value);

/**
 * @brief gpio_wakeup0 use IN_CHNL0_SEL
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 *
 * @return
 */
u32 gpio_wakeup0(u32 gpio);

/**
 * @brief gpio_irflt_in use IN_CHNL1_SEL
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 *
 * @return
 */
u32 gpio_irflt_in(u32 gpio);

/**
 * @brief gpio_cap_mux use IN_CHNL2_SEL
 *
 * @param gpio 参考宏IO_PORTx_xx，如IO_PORTA_00
 *
 * @return
 */
u32 gpio_cap_mux(u32 gpio);


/**
 * @brief gpio_uart_rx_input
 *
 * @param gpio
 * @param ut
 * @param ch
 *
 * @return
 */
u32 gpio_uart_rx_input(u32 gpio, u32 ut, u32 ch);

/**
 * @brief
 *
 * @return
 */
u32 gpio_close_uart0(void);

/**
 * @brief
 *
 * @return
 */
u32 gpio_close_uart1(void);

/**
 * @brief
 *
 * @return
 */
u32 gpio_close_uart2(void);

/**
 * @brief gpio_set_uart0
 *
 * @param ch 0:3 选择对应IO br22
 *         |ch|tx|rx|
 *         |- |- |- |
 *         |0|PA5_TX|PA6_RX|
 *         |1|PB7_TX|PB8_RX|
 *         |2|PA7_TX|PA8_RX|
 *         |3|预留|预留|
 *         |-1|关闭对应的IO口串口功能|no|
 *
 * @return
 */
u32 gpio_set_uart0(u32 ch);
/**
 * @brief gpio_set_uart1
 *
 * @param ch 0:3 选择对应IO  br22
 *         |ch|tx|rx|
 *         |- |- |- |
 *         |0|PB5_TX|PB6_RX|
 *         |1|预留|预留|
 *         |2|PA1_TX|PA2_RX|
 *         |3|USBDP |USBDM |
 *         |-1|关闭对应的IO口串口功能|no|
 *
 * @return
 */
u32 gpio_set_uart1(u32 ch);
/**
 * @brief gpio_set_uart2
 *
 * @param ch 0:3 选择对应IO  br22
 *         |ch|tx|rx|
 *         |- |- |- |
 *         |0|PA3_TX|PA4_RX|
 *         |1|预留|预留|
 *         |2|预留|预留|
 *         |3|PA9_TX|PA10_RX|
 *         |-1|关闭对应的IO口串口功能|no|
 *
 * @return
 */
u32 gpio_set_uart2(u32 ch);

enum {
    IRFLT_LSB,
    IRFLT_RC,
    IRFLT_OSC,
    IRFLT_PLL48M,
};
enum {
    IRFLT_DIV1,
    IRFLT_DIV2,
    IRFLT_DIV4,
    IRFLT_DIV8,
    IRFLT_DIV16,
    IRFLT_DIV32,
    IRFLT_DIV64,
    IRFLT_DIV128,
    IRFLT_DIV256,
    IRFLT_DIV512,
    IRFLT_DIV1024,
    IRFLT_DIV2048,
    IRFLT_DIV4096,
    IRFLT_DIV8192,
    IRFLT_DIV16384,
    IRFLT_DIV32768,
};
/* u32 irflt_config(u32 osc, u32 div); */

/**
 * @brief gpio_irflt_to_timer
 *
 * @param t: [0-3]
 *
 * @return
 */
u32 gpio_irflt_to_timer(u32 t);


u32 get_gpio(const char *p);


//===================================================//
// BR30 Crossbar API
//===================================================//
enum PFI_TABLE {
    PFI_GP_ICH0 = ((u32)(&(JL_IMAP->FI_GP_ICH0))),
    PFI_GP_ICH1 = ((u32)(&(JL_IMAP->FI_GP_ICH1))),
    PFI_GP_ICH2 = ((u32)(&(JL_IMAP->FI_GP_ICH2))),
    PFI_GP_ICH3 = ((u32)(&(JL_IMAP->FI_GP_ICH3))),
    PFI_GP_ICH4 = ((u32)(&(JL_IMAP->FI_GP_ICH4))),
    PFI_GP_ICH5 = ((u32)(&(JL_IMAP->FI_GP_ICH5))),
    PFI_SD0_CMD = ((u32)(&(JL_IMAP->FI_SD0_CMD))),
    PFI_SD0_DA0 = ((u32)(&(JL_IMAP->FI_SD0_DA0))),
    PFI_UART1_RX = ((u32)(&(JL_IMAP->FI_UART1_RX))),
    PFI_TOTAl = ((u32)(&(JL_IMAP->FI_TOTAL))),
};

enum OUTPUT_CH_TYPE {
    OUTPUT_CH_TYPE_GP_OCH = 0,
    OUTPUT_CH_TYPE_MAX = 6,
};

enum OUTPUT_CH_SIGNAL {
    OUTPUT_CH_SIGNAL_UART0_TX,
    OUTPUT_CH_SIGNAL_UART1_RTS,
    OUTPUT_CH_SIGNAL_SPI0_DO,
    OUTPUT_CH_SIGNAL_SPI1_DO,
    OUTPUT_CH_SIGNAL_TIMER0_PWM,
    OUTPUT_CH_SIGNAL_TIMER1_PWM,
    OUTPUT_CH_SIGNAL_TIMER2_PWM,
    OUTPUT_CH_SIGNAL_TIMER3_PWM,
    OUTPUT_CH_SIGNAL_MCPWM0_H,
    OUTPUT_CH_SIGNAL_MCPWM0_L,
    OUTPUT_CH_SIGNAL_MCPWM1_H,
    OUTPUT_CH_SIGNAL_MCPWM1_L,
    OUTPUT_CH_SIGNAL_CLOCK_OUT1,
    OUTPUT_CH_SIGNAL_CLOCK_OUT2,
    OUTPUT_CH_SIGNAL_GP_ICH0,
    OUTPUT_CH_SIGNAL_GP_ICH1,
};

enum INPUT_CH_TYPE {
    INPUT_CH_TYPE_GP_ICH = 0,
    INPUT_CH_TYPE_TIME2_PWM = 6,
    INPUT_CH_TYPE_TIME3_PWM,
    INPUT_CH_TYPE_MAX = 8,
};

enum INPUT_CH_SIGNAL {
    //ICH_CON0
    INPUT_CH_SIGNAL_WLC_EXT_ACT,
    INPUT_CH_SIGNAL_IRFLT,
    INPUT_CH_SIGNAL_CAP,
    INPUT_CH_SIGNAL_CLK_MUX,
    INPUT_CH_SIGNAL_QDEC_SIN0,
    INPUT_CH_SIGNAL_QDEC_SIN1,
    INPUT_CH_SIGNAL_UART0_RX,
    INPUT_CH_SIGNAL_UART1_CTS,

    //ICH_CON1
    INPUT_CH_SIGNAL_EXT_CLK,
    INPUT_CH_SIGNAL_RESERVE0,
    INPUT_CH_SIGNAL_RESERVE1,
    INPUT_CH_SIGNAL_RESERVE2,
    INPUT_CH_SIGNAL_RESERVE3,
    INPUT_CH_SIGNAL_RESERVE4,
    INPUT_CH_SIGNAL_RESERVE5,
    INPUT_CH_SIGNAL_RESERVE6,
};


//=================================================================================//
//@brief: CrossBar 获取某IO的输出映射寄存器
//@input:
// 		gpio: 需要输出外设信号的IO口; 如IO_PORTA_00
//@return:
// 		输出映射寄存器地址; 如&(JL_OMAP->PA0_OUT)
//=================================================================================//
u32 *gpio2crossbar_outreg(u32 gpio);

//=================================================================================//
//@brief: CrossBar 获取某IO的输入映射序号
//@input:
// 		gpio: 需要输出外设信号的IO口; 如IO_PORTA_00
//@return:
// 		输出映射序号; 如PA0_IN
//=================================================================================//
u32 gpio2crossbar_inport(u32 gpio);

//=================================================================================//
//@brief: CrossBar 输出设置 API, 将指定IO口设置为某个外设的输出
//@input:
// 		gpio: 需要输出外设信号的IO口;
// 		fun_index: 需要输出到指定IO口的外设信号, 可以输出外设信号列表请查看io_omap.h文件;
// 		dir_ctl: IO口方向由外设控制使能, 常设为1;
// 		data_ctl: IO口电平状态由外设控制使能, 常设为1;
// 		low_power_mode: 低功耗状态是否保持io功能;
//@return:
// 		1)0: 执行正确;
//		2)-EINVAL: 传参出错;
//@note: 所映射的IO需要在设置IO状态为输出配置;
//@example: 将UART0的Tx信号输出到IO_PORTA_05口:
// 			gpio_direction_output(IO_PORTA_05, 1); //设置IO为输出状态
//			gpio_set_fun_output_port(IO_PORTA_05, FO_UART0_TX, 1, 1); //将UART0的Tx信号输出到IO_PORTA_05口
//=================================================================================//
int gpio_set_fun_output_port(u32 gpio, u32 fun_index, u8 dir_ctl, u8 data_ctl, u8 low_power_mode);

u32 gpio_get_ompap_low_power_mode(u32 offset);
//=================================================================================//
//@brief: CrossBar 输出设置 API, 将指定IO释放外设控制, 变为普通IO;
//@input:
// 		gpio: 需要释放外设控制IO口, 释放后变为普通IO模式;
//@return:
// 		1)0: 执行正确;
//		2)-EINVAL: 传参出错;
//@note:
//@example: 将IO_PORTA_05口被某一外设控制状态释放:
// 			gpio_disable_fun_output_port(IO_PORTA_05);
//=================================================================================//
int gpio_disable_fun_output_port(u32 gpio);

//=================================================================================//
//@brief: CrossBar 输入设置 API, 将某个外设的输入设置为从某个IO输入
//@input:
// 		gpio: 需要输入外设信号的IO口;
// 		pfun: 需要从指定IO输入的外设信号, 可以输入的外设信号列表请查看gpio.h文件enum PFI_TABLE枚举项;
// 		low_power_mode: 低功耗状态是否保持io功能;
//@return:
// 		1)0: 执行正确;
//		2)-EINVAL: 传参出错;
//@note: 所映射的IO需要在设置IO状态为输入配置;
//@example: 将UART0的Rx信号设置为IO_PORTA_05口输入:
//			gpio_set_die(IO_PORTA_05, 1); 		//数字输入使能
//			gpio_set_pull_up(IO_PORTA_05, 1);  //上拉输入使能
//			gpio_direction_input(IO_PORTA_05);  //设置IO为输入状态
//			gpio_set_fun_input_port(IO_PORTA_05, PFI_UART0_RX); //将UART0的Rx信号设置为IO_PORTA_05口输入
//=================================================================================//
int gpio_set_fun_input_port(u32 gpio, enum PFI_TABLE pfun, u32 low_power_mode);
u32 gpio_get_impap_low_power_mode(u32 offset);

//=================================================================================//
//@brief: CrossBar 输入设置 API, 将某个外设信号释放IO口控制, 变为普通IO;
//@input:
// 		pfun: 需要释放由某个IO口输入的外设信号, 外设信号列表请查看gpio.h文件enum PFI_TABLE枚举项;
//@return:  默认为0, 无出错处理;
//@note:
//@example: 将外设信号PFI_UART0_RX释放由某个IO输入:
// 			gpio_disable_fun_input_port(PFI_UART0_RX);
//=================================================================================//
int gpio_disable_fun_input_port(enum PFI_TABLE pfun);

//=================================================================================//
//@brief: Output Channel输出设置 API, 将指定IO口设置为某个外设的输出
//@input:
// 		gpio: 需要输出外设信号的IO口;
// 		signal: 将enum OUTPUT_CH_SIGNAL列表中需要输出到指定IO口的外设信号, 可以输出的外设信号列表请查看gpio.h文件的enum OUTPUT_CH_SIGNAL枚举项;
//@return:  默认为0, 出错内部触发ASSERT;
//@note: 所映射的IO需要在设置IO状态为输出配置;
//@example: 将OUTPUT_CH_SIGNAL_MC_PWM0_H的Tx信号输出到IO_PORTA_05口:
// 			gpio_direction_output(IO_PORTA_05, 1); //设置IO为输出状态
//			gpio_och_sel_output_signal(IO_PORTA_05, OUTPUT_CH_SIGNAL_MC_PWM0_H); //将OUTPUT_CH_SIGNAL_MC_PWM0_H信号输出到IO_PORTA_05口
//=================================================================================//
int gpio_och_sel_output_signal(u32 gpio, enum OUTPUT_CH_SIGNAL signal);

//=================================================================================//
//@brief: Output Channel 输出设置 API, 将指定IO释放外设控制, 变为普通IO;
//@input:
// 		gpio: 需要释放外设控制IO口, 释放后变为普通IO模式;
// 		signal: 将enum OUTPUT_CH_SIGNAL列表中需要取消输出的外设信号, 外设信号列表请查看gpio.h文件的enum OUTPUT_CH_SIGNAL枚举项;;
//@return:  默认为0, 无出错处理;
//@note:
//@example: 将OUTPUT_CH_SIGNAL_MC_PWM0_H取消输出IO_PORTA_05:
// 			gpio_och_disable_output_signal(IO_PORTA_05, OUTPUT_CH_SIGNAL_MC_PWM0_H);
//=================================================================================//
int gpio_och_disable_output_signal(u32 gpio, enum OUTPUT_CH_SIGNAL signal);

//=================================================================================//
//@brief: Input Channel 输入设置 API, 将某个外设的输入设置为从某个IO输入
//@input:
// 		gpio: 需要输入外设信号的IO口;
// 		signal: 需要从指定IO输入的外设信号, 可以输入的外设信号列表请查看gpio.h文件enum INPUT_CH_SIGNAL枚举项;
//      type: INPUT_CH 类型, 常设为INPUT_CH_TYPE_GP_ICH;
//@return:  默认为0, 出错内部触发ASSERT;
//@note: 所映射的IO需要在设置IO状态为输入配置;
//@example: 将INPUT_CH_SIGNAL_TIMER0_CIN信号设置为IO_PORTA_05口输入:
//			gpio_set_die(IO_PORTA_05, 1); 		//数字输入使能
//			gpio_set_pull_up(IO_PORTA_05, 1);  //上拉输入使能
//			gpio_direction_input(IO_PORTA_05);  //设置IO为输入状态
//			gpio_ich_sel_iutput_signal(IO_PORTA_05, INPUT_CH_SIGNAL_TIMER0_CIN, INPUT_CH_TYPE_GP_ICH); //将INPUT_CH_SIGNAL_TIMER0_CIN信号设置为IO_PORTA_05口输入
//=================================================================================//
int gpio_ich_sel_iutput_signal(u32 gpio, enum INPUT_CH_SIGNAL signal, enum INPUT_CH_TYPE type);

//=================================================================================//
//@brief: Input Channel 输入设置 API, 将某个外设信号释放IO口控制, 变为普通IO;
//@input:
// 		gpio: 需要取消输入外设信号的IO口;
// 		signal: 需要取消输入的外设信号, 外设信号列表请查看gpio.h文件enum INPUT_CH_SIGNAL枚举项;
//      type: INPUT_CH 类型, 常设为INPUT_CH_TYPE_GP_ICH;
//@return:  默认为0, 无出错处理;
//@note:
//@example: 将外设信号INPUT_CH_SIGNAL_TIMER0_CIN释放由某个IO输入:
// 			gpio_ich_disable_iutput_signal(IO_PORTA_05, INPUT_CH_SIGNAL_TIMER0_CIN, INPUT_CH_TYPE_GP_ICH);
//=================================================================================//
int gpio_ich_disable_iutput_signal(u32 gpio, enum INPUT_CH_SIGNAL signal, enum INPUT_CH_TYPE type);

u32 gpio_set_spl(u32 gpio, u32 value);
int gpio_port_wkup_ich_ios(u32 gpio);
/*index:gp_ich0~3*/
int gpio_port_wkup_ich_disable(u32 index);

void gpio_longpress_pin0_reset_config(u32 pin, u32 level, u32 time);
void gpio_longpress_pin1_reset_config(u32 pin, u32 level, u32 time);
int gpio_read_direction(u32 gpio);

#endif  /*GPIO_H*/

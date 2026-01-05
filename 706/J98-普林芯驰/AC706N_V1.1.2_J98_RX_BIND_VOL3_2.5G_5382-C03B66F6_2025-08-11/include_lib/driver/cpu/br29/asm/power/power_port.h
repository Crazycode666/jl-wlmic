/**@file  		power_port.h
* @brief
* @details
* @author
* @date     	2021-10-13
* @version    	V1.0
* @copyright  	Copyright(c)2010-2021  JIELI
 */
#ifndef __POWER_PORT_H__
#define __POWER_PORT_H__

enum {
    PORTA_GROUP = 0,
    PORTB_GROUP,
    PORTC_GROUP,
    PORTD_GROUP,
    PORTP_GROUP,
};

struct gpio_value {
    u16 gpioa;
    u16 gpiob;
    u16 gpioc;
    u16 gpiod;
    u16 gpiop;
    u16 gpiousb;
};

#define GET_SFC_PORT()		0

#define CLOSE_USB_BRIDGE()	JL_USB->CON0 = 0

#define CPU_SLEEP()							\
		JL_CLOCK->PWR_CON |= BIT(2)

#define SYS_WK_DLY(en)						\
		if(en){ 							\
			JL_CLOCK->PWR_CON &= ~BIT(3); 	\
		}else{ 								\
    		JL_CLOCK->PWR_CON |= BIT(3); 	\
		}

#define GET_MAIN_SYS_POWER_FLAG() ((JL_CLOCK->PWR_CON & BIT(12)) ? 1 : 0)
#define MAIN_SYS_POWERUP_CLEAR()  JL_CLOCK->PWR_CON |= BIT(11)
#define GET_MAIN_SYS_RESET_FLAG() ((JL_CLOCK->PWR_CON & BIT(7)) ? 1 : 0)
#define MAIN_SYS_RESET_CLEAR()    JL_CLOCK->PWR_CON |= BIT(6)
#define GET_MAIN_SYS_RST_SRC()    (JL_RST->RST_SRC)

#define LRCT_TS(x)                SFR(JL_LRCT->CON,  1,  3,  x)

#define LP_KST              JL_PMU->PMU_CON |= BIT(6)
#define LP_TMR0_WKEN()		JL_PMU->PMU_CON |= BIT(1)
#define LP_TMR1_WKEN()		JL_PMU->PMU_CON |= BIT(2)


#define READ_NORMAL_MODE                        0b0000  //CMD 1bit, ADDR 1bit clk < 30MHz
#define FAST_READ_NORMAL_MODE                   0b0001  //CMD 1bit, ADDR 1bit
#define FAST_READ_DUAL_IO_NORMAL_READ_MODE      0b0100  //CMD 1bit, ADDR 2bit
#define FAST_READ_DUAL_IO_CONTINUOUS_READ_MODE  0b0110  // no cmd addr 2bit
#define FAST_READ_DUAL_OUTPUT_MODE              0b0010  //cmd 1bit addr 1bit
#define FAST_READ_QUAD_OUTPUT_MODE              0b0011  //cmd 1bit addr 1bit
#define FAST_READ_QUAD_IO_NORMAL_READ_MODE      0b0101  //cmd 1bit addr 4bit
#define FAST_READ_QUAD_IO_CONTINUOUS_READ_MODE  0b0111  //no cmd addr 4bit

#define GET_SFC_MODE()		((JL_SFC->CON >> 26) & 0xf)

#define     _PORT(p)            JL_PORT##p
#define     _PORT_IN(p,b)       P##p##b##_IN
#define     _PORT_OUT(p,b)      JL_OMAP->P##p##b##_OUT

#define     SPI_PORT(p)         _PORT(p)
#define     SPI0_FUNC_OUT(p,b)  _PORT_OUT(p,b)
#define     SPI0_FUNC_IN(p,b)   _PORT_IN(p,b)

// | func\port |  A   |
// |-----------|------|
// | CS        | PD3  |
// | CLK       | PD0  |
// | DO(D0)    | PD1  |
// | DI(D1)    | PD2  |
// | WP(D2)    | PA5  |
// | HOLD(D3)  | PA6  |
#define     FSPG_A              0   //FSPG output bit
#define     FSPG_CS_EN          1   //FSPG CS connect enable bit
#define     FSPG_HD0            2   //FSPG HD0
#define     FSPG_HD1            3   //FSPG HD1
#define     FSPG_OE             4   //FSPG output enable bit
#define     FSPG_PD             5   //FSPG pull down enable bit

////////////////////////////////////////////////////////////////////////////////
#define     PORT_SPI0_CSA       D
#define     SPI0_CSA            3

#define     PORT_SPI0_CLKA      D
#define     SPI0_CLKA           0

#define     PORT_SPI0_DOA       D
#define     SPI0_DOA            1

#define     PORT_SPI0_DIA       D
#define     SPI0_DIA            2

#define     PORT_SPI0_D2A       A
#define     SPI0_D2A            5

#define     PORT_SPI0_D3A       A
#define     SPI0_D3A            6

#define 	IO_PORT_NULL		0xabcdef

#define		SPI0_PWR_A		IO_PORT_NULL
#define		SPI0_CS_A		IO_PORTD_03
#define 	SPI0_CLK_A		IO_PORTD_00
#define 	SPI0_DO_D0_A	IO_PORTD_01
#define 	SPI0_DI_D1_A	IO_PORTD_02
#define 	SPI0_WP_D2_A	IO_PORTA_05
#define 	SPI0_HOLD_D3_A	IO_PORTA_06


#define 	MCLR_PORT		IO_PORTC_01


void soff_gpio_protect(u32 gpio);
void board_set_soft_poweroff_common(void *priv);
void sleep_exit_callback_common(void *priv);
void sleep_enter_callback_common(void *priv);

u32 get_sfc_port(void);
u8 get_sfc_bit_mode();
u8 get_sfc1_bit_mode();
void port_init(void);
void port_protect(u16 *port_group, u32 port_num);

u8 WSIG_to_PANA(u8 wsig);
u8 PANA_to_WSIG(u8 iomap);

#endif


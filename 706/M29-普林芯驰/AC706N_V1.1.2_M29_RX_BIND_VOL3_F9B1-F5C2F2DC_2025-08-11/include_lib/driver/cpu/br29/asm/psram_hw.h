#ifndef __PSRAM_HW_H__
#define __PSRAM_HW_H__

//IO Mapping Define:
#define PSRAM_PORT_SEL(x) 	//SFR(JL_IOMC->CON0, 27, 1, x)


//PORTA IO Define:
#define PSRAM_PORTA_CS 				IO_PORTC_03
#define PSRAM_PORTA_CLK 			IO_PORTC_04
#define PSRAM_PORTA_D0 				IO_PORTC_00
#define PSRAM_PORTA_D1 				IO_PORTC_02
#define PSRAM_PORTA_D2 				IO_PORTC_01
#define PSRAM_PORTA_D3 				IO_PORTC_05

//PORTB IO Define:
#define PSRAM_PORTB_CS 				IO_PORTA_14
#define PSRAM_PORTB_CLK 			IO_PORTA_13
#define PSRAM_PORTB_D0 				IO_PORTC_08
#define PSRAM_PORTB_D1 				IO_PORTA_15
#define PSRAM_PORTB_D2 				IO_PORTC_07
#define PSRAM_PORTB_D3 				IO_PORTA_12


#endif /* #ifndef __PSRAM_HW_H__ */

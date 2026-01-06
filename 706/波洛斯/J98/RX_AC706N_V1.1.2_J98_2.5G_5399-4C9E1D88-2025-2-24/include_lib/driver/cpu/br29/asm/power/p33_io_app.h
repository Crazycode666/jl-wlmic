#ifndef __P33_IO_APP_H__
#define __P33_IO_APP_H__

#define MAX_WAKEUP_PORT     8  //最大同时支持数字io输入个数
#define MAX_WAKEUP_ANA_PORT 3   //最大同时支持模拟io输入个数

typedef enum {
    RISING_EDGE = 0,
    FALLING_EDGE,
    BOTH_EDGE,
} POWER_WKUP_EDGE;

enum {
    P3_WKUP_SRC_PCNT_OVF = 0,
    P3_WKUP_SRC_PORT_EDGE,
    P3_WKUP_SRC_ANA_EDGE,
    P3_WKUP_SRC_VDDIO_LVD = 4,
    P3_WKUP_SRC_WDT = 7,
};

typedef enum {
    PORT_FLT_NULL = 0,
    PORT_FLT_16us = 0,
    PORT_FLT_128us,
    PORT_FLT_1ms,
    PORT_FLT_4ms,
} POWER_WKUP_FLT;

typedef enum {
    APORT_FLT_NULL = 0,
    APORT_FLT_16us = 0,
    APORT_FLT_128us,
    APORT_FLT_1ms,
    APORT_FLT_16ms,
} POWER_AWKUP_FLT;

struct port_wakeup {
    u8 iomap;      			  //唤醒io
    u8 pullup_down_enable;    //上下拉是否使能
    u8 filter_enable;		  //滤波使能，所有通道共用滤波参数
    POWER_WKUP_EDGE edge;     //唤醒边沿条件
};

#define PCNT_PND_CLR()		    	p33_fast_access(P3_PCNT_CON, BIT(6), 1)

//==============================DIGITAL==================================
//ie
#define P33_SET_WKUP_P_IE(data)		P33_CON_SET(P3_WKUP_P_IE0, 0, 8, data & 0xff)
#define P33_GET_WKUP_P_IE()			P33_CON_GET(P3_WKUP_P_IE0)
#define P33_OR_WKUP_P_IE(data)		p33_fast_access(P3_WKUP_P_IE0, data & 0xff, 1)
#define P33_AND_WKUP_P_IE(data)		p33_fast_access(P3_WKUP_P_IE0, data & 0xff, 0)

#define P33_SET_WAKEUP_N_IE(data)	P33_CON_SET(P3_WKUP_N_IE0, 0, 8, data & 0xff)
#define P33_GET_WKUP_N_IE()			P33_CON_GET(P3_WKUP_N_IE0)
#define P33_OR_WKUP_N_IE(data)		p33_fast_access(P3_WKUP_N_IE0, data & 0xff, 1)
#define P33_AND_WKUP_N_IE(data)		p33_fast_access(P3_WKUP_N_IE0, data & 0xff, 0)

//pnd
#define P33_GET_WKUP_P_PND()		(P33_CON_GET(P3_WKUP_P_PND0))
#define P33_GET_WKUP_N_PND()		(P33_CON_GET(P3_WKUP_N_PND0))
#define P33_SET_WKUP_P_CPND(data)	p33_fast_access(P3_WKUP_P_CPND0, data & 0xff, 1)
#define P33_SET_WKUP_N_CPND(data)	p33_fast_access(P3_WKUP_N_CPND0, data & 0xff, 1)

//flt
#define P33_SET_WKUP_FLT_EN(data)	P33_CON_SET(P3_WKUP_FLT_EN0, 0, 8, data & 0xff)
#define P33_OR_WKUP_FLT_EN(data)	p33_fast_access(P3_WKUP_FLT_EN0, data & 0xff, 1)
#define P33_AND_WKUP_FLT_EN(data)	p33_fast_access(P3_WKUP_FLT_EN0, data & 0xff, 0)
#define P33_SET_WKUP_CLK_SEL(data)	P33_CON_SET(P3_WKUP_CLK_SEL, 0, 2, data & 0x3)

//==============================ANALOG==================================
//ie
#define P33_SET_AWKUP_P_IE(data)	P33_CON_SET(P3_AWKUP_P_IE, 0, 8, data & 0xff)
#define P33_GET_AWKUP_P_IE()		P33_CON_GET(P3_AWKUP_P_IE)
#define P33_OR_AWKUP_P_IE(data)		p33_fast_access(P3_AWKUP_P_IE, data & 0xff, 1)
#define P33_AND_AWKUP_P_IE(data)	p33_fast_access(P3_AWKUP_P_IE, data & 0xff, 0)

#define P33_SET_AWAKEUP_N_IE(data)	P33_CON_SET(P3_AWKUP_N_IE, 0, 8, data & 0xff)
#define P33_GET_AWKUP_N_IE()		P33_CON_GET(P3_AWKUP_N_IE)
#define P33_OR_AWKUP_N_IE(data)		p33_fast_access(P3_AWKUP_N_IE, data & 0xff, 1)
#define P33_AND_AWKUP_N_IE(data)	p33_fast_access(P3_AWKUP_N_IE, data & 0xff, 0)

//pnd
#define P33_GET_AWKUP_P_PND()		(P33_CON_GET(P3_AWKUP_P_PND))
#define P33_GET_AWKUP_N_PND()		(P33_CON_GET(P3_AWKUP_N_PND))
#define P33_SET_AWKUP_P_CPND(data)	p33_fast_access(P3_AWKUP_P_CPND, data & 0xff, 1)
#define P33_SET_AWKUP_N_CPND(data)	p33_fast_access(P3_AWKUP_N_CPND, data & 0xff, 1)

#define P33_SET_AWKUP_FLT_EN(data)	P33_CON_SET(P3_AWKUP_FLT_EN, 0, 8, data & 0xff)
#define P33_OR_AWKUP_FLT_EN(data)	p33_fast_access(P3_AWKUP_FLT_EN, data & 0xff, 1)
#define P33_AND_AWKUP_FLT_EN(data)	p33_fast_access(P3_AWKUP_FLT_EN, data & 0xff, 0)
#define P33_SET_AWKUP_CLK_SEL(data)	P33_CON_SET(P3_AWKUP_CLK_SEL, 0, 2, data & 0x3)

#endif

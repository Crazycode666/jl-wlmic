#ifndef __CHARGE_APP_H__
#define __CHARGE_APP_H__

/************************P3_CHG_CON0*****************************/
#define CHARGE_EN(en)           	p33_fast_access(P3_CHG_CON0, BIT(0), en)

#define CHGGO_EN(en)            	p33_fast_access(P3_CHG_CON0, BIT(1), en)

#define IS_CHARGE_EN()				((P33_CON_GET(P3_CHG_CON0) & BIT(0)) ? 1: 0 )

#define CHG_HV_MODE(mode)       	p33_fast_access(P3_CHG_CON0, BIT(2), mode)

#define CHG_TRICKLE_EN(en)          p33_fast_access(P3_CHG_CON0, BIT(3), en)

#define CHARGE_FULL_V_SEL(a)		P33_CON_SET(P3_CHG_CON0, 4, 4, a)

/************************P3_CHG_CON1*****************************/
#define CHARGE_mA_SEL(a)			P33_CON_SET(P3_CHG_CON1, 0, 8, a); \
									P33_CON_SET(P3_CHG_CON2, 0, 2, (a & 0x0300) >> 8)

/************************P3_CHG_CON2*****************************/
#define CHARGE_FULL_mA_SEL(a)		P33_CON_SET(P3_CHG_CON2, 4, 2, a)

/************************P3_CHG_CON3*****************************/
#define CHGV_VREF_SEL(a)        	P33_CON_SET(P3_CHG_CON3, 4, 2, a)

#define CHGI_TRIM_SEL(a)       	 	P33_CON_SET(P3_CHG_CON3, 0, 4, a)

/************************P3_CHG_CON4*****************************/
enum {
    CHARGE_DET_VOL_365V,
    CHARGE_DET_VOL_375V,
    CHARGE_DET_VOL_385V,
    CHARGE_DET_VOL_395V,
};
#define CHARGE_DET_VOL(a)			P33_CON_SET(P3_CHG_CON4, 1, 2, a)

#define CHARGE_DET_EN(en)			p33_fast_access(P3_CHG_CON4, BIT(0), en)

/************************P3_VPWR_CON0*****************************/
#define GET_L5V_RES_DET_S_SEL() 	(P33_CON_GET(P3_VPWR_CON0) & 0x06)

#define L5V_IO_MODE(a)              p33_fast_access(P3_VPWR_CON0, BIT(3), a)

#define IS_L5V_LOAD_EN()        	((P33_CON_GET(P3_VPWR_CON0) & BIT(0)) ? 1: 0)

#define L5V_RES_DET_S_SEL(a)		P33_CON_SET(P3_VPWR_CON0, 1, 2, a)

#define L5V_LOAD_EN(a)		    	p33_fast_access(P3_VPWR_CON0, BIT(0), a)

/************************P3_CHG_WKUP*****************************/
#define CHARGE_LEVEL_DETECT_EN(a)	p33_fast_access(P3_CHG_WKUP, BIT(0), a)

#define CHARGE_EDGE_DETECT_EN(a)	p33_fast_access(P3_CHG_WKUP, BIT(1), a)

#define CHARGE_WKUP_SOURCE_SEL(a)	P33_CON_SET(P3_CHG_WKUP, 2, 2, a)

#define CHARGE_WKUP_EN(a)			p33_fast_access(P3_CHG_WKUP, BIT(4), a)

#define CHARGE_WKUP_EDGE_SEL(a)		p33_fast_access(P3_CHG_WKUP, BIT(5), a)

#define CHARGE_WKUP_PND_CLR()		p33_fast_access(P3_CHG_WKUP, BIT(6), 1)

/************************P3_AWKUP_LEVEL*****************************/
#define CHARGE_FULL_FILTER_GET()	((P33_CON_GET(P3_AWKUP_LEVEL) & BIT(2)) ? 1: 0)

#define LVCMP_DET_FILTER_GET()      ((P33_CON_GET(P3_AWKUP_LEVEL) & BIT(1)) ? 1: 0)

#define LDO5V_DET_FILTER_GET()      ((P33_CON_GET(P3_AWKUP_LEVEL) & BIT(0)) ? 1: 0)

/************************P3_ANA_READ*****************************/
#define CHARGE_FULL_FLAG_GET()		((P33_CON_GET(P3_ANA_READ) & BIT(2)) ? 1: 0 )

#define LVCMP_DET_GET()			    ((P33_CON_GET(P3_ANA_READ) & BIT(1)) ? 1: 0 )

#define LDO5V_DET_GET()			    ((P33_CON_GET(P3_ANA_READ) & BIT(0)) ? 1: 0 )

#endif

/**@file  		power_reset.h
* @brief        复位模块接口
* @details		HW API
* @author
* @date     	2021-8-26
* @version  	V1.0
* @copyright    Copyright:(c)JIELI  2011-2020  @ , All Rights Reserved.
 */

#ifndef __POWER_RESET_H__
#define __POWER_RESET_H__

/*
 *复位原因包括两种
 1.系统复位源: p33 p11 主系统
 2.自定义复位源：唤醒、断言、异常等
 */
enum RST_REASON {
    /*主系统*/
    MSYS_P11_RST,                   //P11复位主系统
    MSYS_P33_RST,                   //P33复位主系统
    MSYS_P33_SOFF_RST,              //低功耗唤醒复位(soff legacy)，无P11系统结构
    MSYS_DVDD_POR_RST,              //DVDD上电
    MSYS_DVDD_OK_RST,               //DVDD不稳定
    MSYS_SOFT_RST,                  //主系统软件复位
    MSYS_P2M_RST,                   //低功耗唤醒复位(softoff advance && deepsleep)，有P11系统结构
    MSYS_POWER_RETURN,              //主系统未被复位

    /*P11*/
    P11_PVDD_POR_RST = 10,          //pvdd上电
    P11_P33_SOFF_RST,               //低功耗唤醒复位(soff legacy)，有P11系统结构
    P11_P33_RST,                    //p33复位P11
    P11_WDT_RST,                    //看门狗复位
    P11_SOFT_RST,                   //P11软件复位
    P11_MSYS_RST,                   //主系统复位P11
    P11_POWER_RETURN,               //P11系统未被复位

    /*P33*/
    P33_VDDIO_POR_RST = 20,         //vddio上电复位(电池/vpwr供电)
    P33_VDDIO_LVD_RST,              //vddio低压复位、上电复位(电池/vpwr供电)
    P33_VCM_RST,                    //vcm高电平短接复位
    P33_MCLR_RST,                   //mclr低电平复位
    P33_PPINR_RST,                  //数字输入长按复位
    P33_P11_RST,                    //p11复位p33
    P33_MSYS_RST,                   //主系统复位p33
    P33_SOFT_RST,                   //p33软件复位
    P33_PPINR1_RST,                 //模拟输入长按复位，根据设计可能包括charge_full、vatch、ldoint、vabt_det
    P33_WDT_RST,                    //看门狗复位
    P33_POWER_RETURN,               //p33系统未被复位。

    /*RTC*/
    R3_VDDIO_RST,
    R3_SOFT_RST,
    R3_POWER_RETURN,

    /*SUB*/
    P33_EXCEPTION_SOFT_RST,	    		//异常软件复位
    P33_ASSERT_SOFT_RST,				//断言软件复位
};

void p33_soft_reset(void);

void reset_source_value_dump(void);

bool is_reset_source(enum RST_REASON index);

void power_reset_source_dump(void);

bool is_soft_reset(void);
#endif



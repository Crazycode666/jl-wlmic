#ifndef CONFIG_BOARD_AC706N_TEST_CFG_H
#define CONFIG_BOARD_AC706N_TEST_CFG_H

#include "board_ac706n_test_global_build_cfg.h"

#ifdef CONFIG_BOARD_AC706N_TEST

#define CONFIG_SDFILE_ENABLE

//*********************************************************************************//
//                                 配置开始                                        //
//*********************************************************************************//
#define ENABLE_THIS_MOUDLE					1
#define DISABLE_THIS_MOUDLE					0

#define ENABLE								1
#define DISABLE								0

#define NO_CONFIG_PORT						(-1)

//*********************************************************************************//
//                                  app 配置                                       //
//*********************************************************************************//
#define TCFG_APP_LINEIN_EN					0
#define TCFG_APP_PC_EN					    0
#define TCFG_APP_LIVE_MIC_EN				1
#define TCFG_APP_LIVE_IIS_EN                0


//*********************************************************************************//
//                                 UART配置                                        //
//*********************************************************************************//
#define TCFG_UART0_ENABLE					ENABLE_THIS_MOUDLE                     //串口打印模块使能
#define TCFG_UART0_RX_PORT					NO_CONFIG_PORT                         //串口接收脚配置（用于打印可以选择NO_CONFIG_PORT）
#define TCFG_UART0_TX_PORT  				IO_PORT_DP                             //串口发送脚配置
#define TCFG_UART0_BAUDRATE  				2000000                                //串口波特率配置

//*********************************************************************************//
//                                 IIC配置                                        //
//*********************************************************************************//
/*软件IIC设置*/
#define TCFG_SW_I2C0_CLK_PORT               IO_PORTB_04                             //软件IIC  CLK脚选择
#define TCFG_SW_I2C0_DAT_PORT               IO_PORTB_05                             //软件IIC  DAT脚选择
#define TCFG_SW_I2C0_DELAY_CNT              75                                      //IIC延时参数，影响通讯时钟频率

/*硬件IIC端口选择*/
#define TCFG_HW_I2C0_CLK_PORT               IO_PORTC_04
#define TCFG_HW_I2C0_DAT_PORT               IO_PORTC_05
#define TCFG_HW_I2C0_CLK                    100000                                  //硬件IIC波特率

//*********************************************************************************//
//                                 硬件SPI 配置                                        //
//*********************************************************************************//
#define	TCFG_HW_SPI1_ENABLE		            DISABLE_THIS_MOUDLE
#define TCFG_HW_SPI1_PORT_CLK               IO_PORTA_07
#define TCFG_HW_SPI1_PORT_DO                IO_PORTA_08
#define TCFG_HW_SPI1_PORT_DI                NO_CONFIG_PORT
#define TCFG_HW_SPI1_BAUD		            24000000L
#define TCFG_HW_SPI1_MODE		            SPI_MODE_BIDIR_1BIT
#define TCFG_HW_SPI1_ROLE		            SPI_ROLE_MASTER

#define	TCFG_HW_SPI2_ENABLE		            DISABLE_THIS_MOUDLE
#define TCFG_HW_SPI2_PORT_CLK               NO_CONFIG_PORT
#define TCFG_HW_SPI2_PORT_DO                NO_CONFIG_PORT
#define TCFG_HW_SPI2_PORT_DI                NO_CONFIG_PORT
#define TCFG_HW_SPI2_BAUD		            2000000L
#define TCFG_HW_SPI2_MODE		            SPI_MODE_BIDIR_1BIT
#define TCFG_HW_SPI2_ROLE		            SPI_ROLE_MASTER

//*********************************************************************************//
//                                 USB 配置                                        //
//*********************************************************************************//
#define TCFG_PC_ENABLE						TCFG_APP_PC_EN//PC模块使能
#define TCFG_UDISK_ENABLE					DISABLE_THIS_MOUDLE//U盘模块使能
#define TCFG_HOST_AUDIO_ENABLE				DISABLE_THIS_MOUDLE
#define TCFG_HID_HOST_ENABLE				DISABLE_THIS_MOUDLE
#define TCFG_OTG_USB_DEV_EN                 BIT(0)//USB0 = BIT(0)  USB1 = BIT(1)
#define TCFG_PC_FOUR_MIC                    DISABLE_THIS_MOUDLE

#include "usb_std_class_def.h"

#undef  USB_DEVICE_CLASS_CONFIG
#define USB_DEVICE_CLASS_CONFIG 		    TCFG_PC_FOUR_MIC ? (MIC_CLASS | SPEAKER_CLASS) : (MASSSTORAGE_CLASS|SPEAKER_CLASS|MIC_CLASS|HID_CLASS)

//*********************************************************************************//
//                                 key 配置                                        //
//*********************************************************************************//
#define KEY_IO_NUM_MAX						6
#define KEY_AD_NUM_MAX						10
#define KEY_IR_NUM_MAX						21
#define KEY_TOUCH_NUM_MAX					6
#define KEY_RDEC_NUM_MAX                    6
#define KEY_CTMU_TOUCH_NUM_MAX				6

#define MULT_KEY_ENABLE						DISABLE 		//是否使能组合按键消息, 使能后需要配置组合按键映射表

#define TCFG_KEY_TONE_EN					DISABLE 		// 按键提示音。

#define TCFG_CHIP_RESET_PIN					IO_PORTB_02 // 长按复位
#define TCFG_CHIP_RESET_LEVEL				0 // 0-低电平复位；1-高电平复位
#define TCFG_CHIP_RESET_TIME				8 // 复位时间1 2 4 8 16 单位为秒

//*********************************************************************************//
//                                 iokey 配置                                      //
//*********************************************************************************//
#define TCFG_IOKEY_ENABLE					DISABLE_THIS_MOUDLE //是否使能IO按键

#define TCFG_IOKEY_POWER_CONNECT_WAY		ONE_PORT_TO_LOW    //按键一端接低电平一端接IO
#define TCFG_IOKEY_POWER_ONE_PORT			IO_PORTB_02        //IO按键端口

#define TCFG_IOKEY_PREV_CONNECT_WAY			ONE_PORT_TO_LOW  //按键一端接低电平一端接IO
#define TCFG_IOKEY_PREV_ONE_PORT			IO_PORTB_01

#define TCFG_IOKEY_NEXT_CONNECT_WAY 		ONE_PORT_TO_LOW  //按键一端接低电平一端接IO
#define TCFG_IOKEY_NEXT_ONE_PORT			IO_PORTB_03

#define TCFG_IOKEY_MODE_CONNECT_WAY 		DOUBLE_PORT_TO_IO
#define TCFG_IOKEY_MODE_OUT_PORT			IO_PORTB_03
#define TCFG_IOKEY_MODE_IN_PORT 			IO_PORTB_01

//*********************************************************************************//
//                                 adkey 配置                                      //
//*********************************************************************************//
#define TCFG_ADKEY_ENABLE                   ENABLE_THIS_MOUDLE //是否使能AD按键
#define TCFG_ADKEY_PORT                     IO_PORTB_02         //AD按键端口(需要注意选择的IO口是否支持AD功能)
/*AD通道选择，需要和AD按键的端口相对应:
    AD_CH_PA1    AD_CH_PA3    AD_CH_PA4    AD_CH_PA5
    AD_CH_PA9    AD_CH_PA1    AD_CH_PB1    AD_CH_PB4
    AD_CH_PB6    AD_CH_PB7    AD_CH_DP     AD_CH_DM
    AD_CH_PB2
*/
#define TCFG_ADKEY_AD_CHANNEL               AD_CH_IO_PB2
#define TCFG_ADKEY_EXTERN_UP_ENABLE         ENABLE_THIS_MOUDLE //是否使用外部上拉

#if TCFG_ADKEY_EXTERN_UP_ENABLE
#define R_UP    220                 //22K，外部上拉阻值在此自行设置
#else
#define R_UP    100                 //10K，内部上拉默认10K
#endif

//必须从小到大填电阻，没有则同VDDIO,填0x3ffL
#define TCFG_ADKEY_AD0      (0)                                 //0R
#define TCFG_ADKEY_AD1      (0x3ffL * 30   / (30   + R_UP))     //3k
#define TCFG_ADKEY_AD2      (0x3ffL * 62   / (62   + R_UP))     //6.2k
#define TCFG_ADKEY_AD3      (0x3ffL * 91   / (91   + R_UP))     //9.1k
#define TCFG_ADKEY_AD4      (0x3ffL * 150  / (150  + R_UP))     //15k
#define TCFG_ADKEY_AD5      (0x3ffL * 240  / (240  + R_UP))     //24k
#define TCFG_ADKEY_AD6      (0x3ffL * 330  / (330  + R_UP))     //33k
#define TCFG_ADKEY_AD7      (0x3ffL * 510  / (510  + R_UP))     //51k
#define TCFG_ADKEY_AD8      (0x3ffL * 1000 / (1000 + R_UP))     //100k
#define TCFG_ADKEY_AD9      (0x3ffL * 2200 / (2200 + R_UP))     //220k
#define TCFG_ADKEY_VDDIO    (0x3ffL)

#define TCFG_ADKEY_VOLTAGE0 ((TCFG_ADKEY_AD0 + TCFG_ADKEY_AD1) / 2)
#define TCFG_ADKEY_VOLTAGE1 ((TCFG_ADKEY_AD1 + TCFG_ADKEY_AD2) / 2)
#define TCFG_ADKEY_VOLTAGE2 ((TCFG_ADKEY_AD2 + TCFG_ADKEY_AD3) / 2)
#define TCFG_ADKEY_VOLTAGE3 ((TCFG_ADKEY_AD3 + TCFG_ADKEY_AD4) / 2)
#define TCFG_ADKEY_VOLTAGE4 ((TCFG_ADKEY_AD4 + TCFG_ADKEY_AD5) / 2)
#define TCFG_ADKEY_VOLTAGE5 ((TCFG_ADKEY_AD5 + TCFG_ADKEY_AD6) / 2)
#define TCFG_ADKEY_VOLTAGE6 ((TCFG_ADKEY_AD6 + TCFG_ADKEY_AD7) / 2)
#define TCFG_ADKEY_VOLTAGE7 ((TCFG_ADKEY_AD7 + TCFG_ADKEY_AD8) / 2)
#define TCFG_ADKEY_VOLTAGE8 ((TCFG_ADKEY_AD8 + TCFG_ADKEY_AD9) / 2)
#define TCFG_ADKEY_VOLTAGE9 ((TCFG_ADKEY_AD9 + TCFG_ADKEY_VDDIO) / 2)

#define TCFG_ADKEY_VALUE0                   0
#define TCFG_ADKEY_VALUE1                   1
#define TCFG_ADKEY_VALUE2                   2
#define TCFG_ADKEY_VALUE3                   3
#define TCFG_ADKEY_VALUE4                   4
#define TCFG_ADKEY_VALUE5                   5
#define TCFG_ADKEY_VALUE6                   6
#define TCFG_ADKEY_VALUE7                   7
#define TCFG_ADKEY_VALUE8                   8
#define TCFG_ADKEY_VALUE9                   9

//*********************************************************************************//
//                              tocuh key 配置                                     //
//*********************************************************************************//
#define TCFG_TOUCH_KEY_ENABLE               DISABLE_THIS_MOUDLE             //是否使能plcnt触摸按键
//key0配置
#define TCFG_TOUCH_KEY0_PRESS_DELTA	   	    100//变化阈值，当触摸产生的变化量达到该阈值，则判断被按下，每个按键可能不一样，可先在驱动里加到打印，再反估阈值
#define TCFG_TOUCH_KEY0_PORT 				IO_PORTB_06 //触摸按键key0 IO配置
#define TCFG_TOUCH_KEY0_VALUE 				0x12 		//触摸按键key0 按键值
//key1配置
#define TCFG_TOUCH_KEY1_PRESS_DELTA	   	    100//变化阈值，当触摸产生的变化量达到该阈值，则判断被按下，每个按键可能不一样，可先在驱动里加到打印，再反估阈值
#define TCFG_TOUCH_KEY1_PORT 				IO_PORTB_07 //触摸按键key1 IO配置
#define TCFG_TOUCH_KEY1_VALUE 				0x34        //触摸按键key1 按键值

//*********************************************************************************//
//                                 Audio配置                                       //
//*********************************************************************************//

/*PA*/
#define TCFG_AUDIO_DAC_PA_PORT				NO_CONFIG_PORT

//>>>>>Audio_DAC配置
#define TCFG_AUDIO_DAC_ENABLE				ENABLE_THIS_MOUDLE

#define TCFG_SUPPORT_MIC_CAPLESS            DISABLE_THIS_MOUDLE
#define TCFG_MIC_CAPLESS_ENABLE				DISABLE_THIS_MOUDLE
#define TCFG_MIC1_CAPLESS_ENABLE            DISABLE_THIS_MOUDLE

#define TCFG_AUDIO_DAC_CONNECT_MODE         DAC_OUTPUT_LR
#define TCFG_AUDIO_DAC_MODE                 DAC_MODE_SINGLE
#define TCFG_DAC_PERFORMANCE_MODE           DAC_MODE_LOW_POWER
#define TCFG_DAC_POWER_LEVEL                AUDIO_VCM_CAP_LEVEL2
#define TCFG_VCM_CAP_EN                     ENABLE

/*
 *系统音量类型选择
 *软件数字音量是指纯软件对声音进行运算后得到的
 *硬件数字音量是指dac内部数字模块对声音进行运算后输出
 *选择软件数字音量或者软件数字音量会固定模拟音量，
 *固定的模拟音量值由 MAX_ANA_VOL 决定
 */
#define VOL_TYPE_DIGITAL		0	//软件数字音量
#define VOL_TYPE_ANALOG			1	//硬件模拟音量
#define VOL_TYPE_AD				2	//联合音量(模拟数字混合调节)
#define VOL_TYPE_DIGITAL_HW		3  	//硬件数字音量
#define VOL_TYPE_DIGGROUP		4	//独立通道软件数字音量
#define SYS_VOL_TYPE            VOL_TYPE_DIGGROUP
//每个解码通道都开启数字音量管理,音量类型为VOL_TYPE_DIGGROUP时要使能
#define SYS_DIGVOL_GROUP_EN     DISABLE

#if  (SYS_VOL_TYPE == VOL_TYPE_DIGGROUP)
#undef SYS_DIGVOL_GROUP_EN
#define SYS_DIGVOL_GROUP_EN     ENABLE
#endif
/*
 *通话的时候使用数字音量
 *0：通话使用和SYS_VOL_TYPE一样的音量调节类型
 *1：通话使用数字音量调节，更加平滑
 */
#define TCFG_CALL_USE_DIGITAL_VOLUME		0

/*
 *PC模式左右声道音量独立设置功能
 */
#define PC_VOL_INDEPENDENT_EN                  0
//>>>>>Audio_ADC配置
#define TCFG_AUDIO_ADC_ENABLE				ENABLE_THIS_MOUDLE
/*MIC LDO电流档位设置：0:0.625ua    1:1.25ua    2:1.875ua    3:2.5ua*/
#define TCFG_AUDIO_ADC_LD0_SEL				2

#define TCFG_AUDIO_NOISE_GATE				DISABLE_THIS_MOUDLE

/*MIC模式配置:单端隔直电容模式/差分隔直电容模式/单端省电容模式*/
#define TCFG_AUDIO_MIC_MODE                 AUDIO_MIC_CAP_DIFF_MODE

#define TCFG_USE_OTHER_MIC_BIAS             DISABLE_THIS_MOUDLE

// 选择哪个mic 作为省电容mic来预校准, 必须先把要设置的mic的TCFG_AUDIO_MIC_MODE设成AUDIO_MIC_CAPLESS_MODE
#define TCFG_CAPLESS_MIC_CHANNEL            AUDIO_ADC_MIC_0

/*
 *>>MIC电源管理:根据具体方案，选择对应的mic供电方式
 *(1)如果是多种方式混合，则将对应的供电方式或起来即可，比如(MIC_PWR_FROM_GPIO | MIC_PWR_FROM_MIC_BIAS)
 *(2)如果使用固定电源供电(比如dacvdd)，则配置成DISABLE_THIS_MOUDLE
 */
#define MIC_PWR_FROM_GPIO       (1UL << 0)  //使用普通IO输出供电
#define MIC_PWR_FROM_MIC_BIAS   (1UL << 1)  //使用内部mic_ldo供电(有上拉电阻可配)
#define MIC_PWR_FROM_MIC_LDO    (1UL << 2)  //使用内部mic_ldo供电
//配置MIC电源
#define TCFG_AUDIO_MIC_PWR_CTL              MIC_PWR_FROM_MIC_BIAS

//使用普通IO输出供电:不用的port配置成NO_CONFIG_PORT
#if (TCFG_AUDIO_MIC_PWR_CTL & MIC_PWR_FROM_GPIO)
#define TCFG_AUDIO_MIC_PWR_PORT             IO_PORTC_02
#endif/*MIC_PWR_FROM_GPIO*/

//使用内部mic_ldo供电(有上拉电阻可配)
#if (TCFG_AUDIO_MIC_PWR_CTL & MIC_PWR_FROM_MIC_BIAS)
#define TCFG_AUDIO_MIC_BIAS_EN             ENABLE_THIS_MOUDLE
#endif/*MIC_PWR_FROM_MIC_BIAS*/

//使用内部mic_ldo供电(Port:PA0)
#if (TCFG_AUDIO_MIC_PWR_CTL & MIC_PWR_FROM_MIC_LDO)
#define TCFG_AUDIO_MIC_LDO_EN               ENABLE_THIS_MOUDLE
#endif/*MIC_PWR_FROM_MIC_LDO*/
/*>>MIC电源管理配置结束*/

//第三方清晰语音开发使能
#define TCFG_CVP_DEVELOP_ENABLE             DISABLE_THIS_MOUDLE

/*通话降噪模式配置*/
#define CVP_ANS_MODE	0	/*传统经典降噪*/
#define CVP_DNS_MODE	1	/*神经网络降噪*/
#define TCFG_AUDIO_CVP_NS_MODE				CVP_ANS_MODE

/*
 * ENC(双mic降噪)配置
 * 双mic降噪包括DMS_NORMAL和DMS_FLEXIBLE，在使能TCFG_AUDIO_DUAL_MIC_ENABLE
 * 的前提下，根据具体需求，选择对应的DMS模式
 */
/*ENC(双mic降噪)使能*/
#define TCFG_AUDIO_DUAL_MIC_ENABLE			DISABLE_THIS_MOUDLE

/*DMS模式选择*/
#define DMS_NORMAL		1	//普通双mic降噪(mic距离固定)
#define DMS_FLEXIBLE	2	//适配mic距离不固定且距离比较远的情况，比如头戴式话务耳机
#define TCFG_AUDIO_DMS_SEL					DMS_NORMAL

/*ENC双mic配置主mic副mic对应的mic port*/
#define DMS_MASTER_MIC0		0 //mic0是主mic
#define DMS_MASTER_MIC1		1 //mic1是主mic
#define TCFG_AUDIO_DMS_MIC_MANAGE			DMS_MASTER_MIC0

/*通话CVP 测试使能，配合量产设备测试MIC频响和算法性能*/
#define TCFG_AUDIO_CVP_DUT_ENABLE			DISABLE_THIS_MOUDLE

#define TCFG_AUDIO_ADC_MIC_CHA				AUDIO_ADC_MIC_1 //ac706只有一个adc通道

// AUTOMUTE
#define AUDIO_OUTPUT_AUTOMUTE   			DISABLE_THIS_MOUDLE



/*Audio数据导出配置:支持BT_SPP\SD\UART载体导出*/
#define AUDIO_DATA_EXPORT_USE_SD	1
#define AUDIO_DATA_EXPORT_USE_SPP 	2
#define AUDIO_DATA_EXPORT_USE_UART 	3
#define TCFG_AUDIO_DATA_EXPORT_ENABLE		DISABLE_THIS_MOUDLE

/*通话清晰语音处理数据导出*/
// #define AUDIO_PCM_DEBUG

/*通话参数在线调试*/
#define TCFG_AEC_TOOL_ONLINE_ENABLE         DISABLE_THIS_MOUDLE

/*多mic写卡功能，用于调试*/
#define TCFG_MULTIPLE_MIC_REC_ENABLE        DISABLE_THIS_MOUDLE

/*麦克风 、频响测试*/
#define TCFG_AUDIO_MIC_DUT_ENABLE           DISABLE_THIS_MOUDLE

//AudioEffects代码链接管理
#define AUDIO_EFFECTS_DRC_AT_RAM			0
#define AUDIO_EFFECTS_REVERB_AT_RAM			0
#define AUDIO_EFFECTS_ECHO_AT_RAM			0
#define AUDIO_EFFECTS_AFC_AT_RAM			0
#define AUDIO_EFFECTS_EQ_AT_RAM				0
#define AUDIO_EFFECTS_NOISEGATE_AT_RAM		0
#define AUDIO_EFFECTS_MIC_EFFECT_AT_RAM		0
#define AUDIO_EFFECTS_MIC_STREAM_AT_RAM		0
#define AUDIO_EFFECTS_DEBUG_AT_RAM			0
#define AUDIO_EFFECTS_DYNAMIC_EQ_AT_RAM     0
#define AUDIO_EFFECTS_VBASS_AT_RAM          0
#define AUDIO_STREAM_AT_RAM                 0
//*********************************************************************************//
//                                  充电舱配置                                     //
//   充电舱/蓝牙测试盒/ANC测试三者为同级关系,开启任一功能都会初始化PP0通信接口     //
//*********************************************************************************//
#define TCFG_CHARGESTORE_ENABLE				DISABLE_THIS_MOUDLE         //是否支持智能充电舱
#define TCFG_TEST_BOX_ENABLE			    DISABLE_THIS_MOUDLE         //是否支持蓝牙测试盒
#define TCFG_ANC_BOX_ENABLE			        DISABLE_THIS_MOUDLE         //是否支持ANC测试盒
#define TCFG_CHARGESTORE_PORT				IO_PORTP_00                 //通讯的IO口

//*********************************************************************************//
//                                  充电参数配置                                   //
//*********************************************************************************//
//是否支持芯片内置充电
#define TCFG_CHARGE_ENABLE					ENABLE_THIS_MOUDLE
//是否支持开机充电
#define TCFG_CHARGE_POWERON_ENABLE			DISABLE
//是否支持拔出充电自动开机功能
#define TCFG_CHARGE_OFF_POWERON_NE			ENABLE
//充电截止电压
#define TCFG_CHARGE_FULL_V					CHARGE_FULL_V_4199
//充电截止电流
#define TCFG_CHARGE_FULL_MA					CHARGE_FC_IS_CC_DIV_10
//恒流充电电流
#define TCFG_CHARGE_MA						(200)//0~480mA
//涓流充电电流
#define TCFG_CHARGE_TRICKLE_MA              (20) //0~48mA

#if (TCFG_PC_ENABLE && TCFG_CHARGE_ENABLE && !TCFG_CHARGE_POWERON_ENABLE)
#define TCFG_USB_PORT_CHARGE                ENABLE
#endif

//*********************************************************************************//
//                                  LED 配置                                       //
//*********************************************************************************//
#define TCFG_PWMLED_ENABLE					ENABLE_THIS_MOUDLE			//是否支持PMW LED推灯模块
#define TCFG_PWMLED_IOMODE					LED_ONE_IO_MODE				//LED模式，单IO还是两个IO推灯
#define TCFG_PWMLED_PIN						IO_PORTB_04					//LED使用的IO口


//*********************************************************************************//
//                                  UI 配置                                        //
//*********************************************************************************//
#define TCFG_UI_ENABLE 						DISABLE_THIS_MOUDLE//UI总开关

/* 选择UI风格 */
#define CONFIG_UI_STYLE                     STYLE_JL_LED7

/* 选择屏幕类型 */
#define TCFG_UI_LED7_ENABLE 			 	DISABLE_THIS_MOUDLE//ENABLE_THIS_MOUDLE 	//UI使用LED7显示需要 ENABLE

/* 仅led7屏时有效：led7跑ram 不屏蔽中断(需要占据2k附近ram) */
#define TCFG_LED7_RUN_RAM 					DISABLE_THIS_MOUDLE 	//led7跑ram 不屏蔽中断(需要占据2k附近ram)

//*********************************************************************************//
//                                  时钟配置                                       //
//*********************************************************************************//
#define TCFG_CLOCK_SYS_SRC					SYS_CLOCK_INPUT_PLL_BT_OSC   //系统时钟源选择
#define TCFG_CLOCK_SYS_HZ					96000000                     //系统时钟设置
#define TCFG_CLOCK_OSC_HZ					24000000                     //外界晶振频率设置
#define TCFG_CLOCK_MODE                     CLOCK_MODE_ADAPTIVE

// 240Mhz: 160 120 96 80 48...
// 192Mhz: 192 128 96 64 48...
#define TCFG_CLOCK_PLL_240MHZ				0	// PLL跑240Mhz

//*********************************************************************************//
//                                  低功耗配置                                     //
//*********************************************************************************//
#define TCFG_LOWPOWER_POWER_SEL				PWR_LDO15          //电源模式设置，可选DCDC和LDO
#define TCFG_LOWPOWER_BTOSC_DISABLE			0                   //低功耗模式下BTOSC是否保持
#define TCFG_LOWPOWER_LOWPOWER_SEL			0                   //芯片是否进入powerdown
#define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_34V
#define TCFG_LOWPOWER_VDDIOW_LEVEL			VDDIOW_VOL_28V     //弱VDDIO等级配置
#define TCFG_LOWPOWER_OSC_TYPE              OSC_TYPE_LRC
#define TCFG_LOWPOWER_RAM_SIZE              0

//*********************************************************************************//
//                                  EQ配置                                         //
//*********************************************************************************//
#define TCFG_EQ_ENABLE                      0     //支持EQ功能,EQ总使能
// #if TCFG_EQ_ENABLE
#define TCFG_BT_MUSIC_EQ_ENABLE             0     //支持蓝牙音乐EQ
#define TCFG_AEC_DCCS_EQ_ENABLE           	0     // AEC DCCS
#define TCFG_AEC_UL_EQ_ENABLE           	0     // AEC UL
#define TCFG_BROADCAST_MODE_EQ_ENABLE       0     //支持广播模式EQ
#define TCFG_LINEIN_MODE_EQ_ENABLE          0     //支持linein近端EQ
#define TCFG_PC_MODE_EQ_ENABLE              0     //支持pc模式EQ
#define TCFG_AUDIO_OUT_EQ_ENABLE			0 	  //高低音EQ
#define TCFG_IIS_IN_MODE_EQ_ENABLE          0     //支持IIS输入模式EQ


#define EQ_SECTION_MAX                      10	  //EQ段数
#define TCFG_DYNAMIC_EQ_ENABLE              0     //动态EQ使能，接在EQ后，需输入32bit位宽数据

/*省电容mic通过eq模块实现去直流滤波*/
#if (TCFG_SUPPORT_MIC_CAPLESS && (TCFG_MIC_CAPLESS_ENABLE || TCFG_MIC1_CAPLESS_ENABLE))
#if ((TCFG_EQ_ENABLE == 0) || (TCFG_AEC_DCCS_EQ_ENABLE == 0))
#error "MicCapless enable,Please enable TCFG_EQ_ENABLE and TCFG_AEC_DCCS_EQ_ENABLE"
#endif
#endif

#define TCFG_DRC_ENABLE						0	  //DRC 总使能
#define TCFG_AUDIO_MDRC_ENABLE              0     //多带DRC使能  0:关闭多带DRC，  1：使能多带DRC  2：使能多带DRC 并且 多带DRC后再做一次全带的drc


#define TCFG_BT_MUSIC_DRC_ENABLE            0     //支持蓝牙音乐DRC
#define TCFG_BROADCAST_MODE_DRC_ENABLE      0     //支持广播模式DRC
#define TCFG_LINEIN_MODE_DRC_ENABLE         0     //支持LINEIN模式DRC
#define TCFG_PC_MODE_DRC_ENABLE             0     //支持PC模式DRC
#define TCFG_AUDIO_OUT_DRC_ENABLE			0 	  //高低音EQ后的DRC
#define TCFG_IIS_IN_MODE_DRC_ENABLE         0     //支持IIS输入模式DRC


#define LINEIN_MODE_SOLE_EQ_EN              0  //linein模式是否需要独立的音效

//*********************************************************************************//
//                          新音箱配置工具 && 调音工具                             //
//*********************************************************************************//
#define TCFG_CFG_TOOL_ENABLE				DISABLE		  	//是否支持音箱在线配置工具
#define TCFG_EFFECT_TOOL_ENABLE				DISABLE		  	//是否支持在线音效调试,使能该项还需使能EQ总使能TCFG_EQ_ENABL,
#define TCFG_NULL_COMM						0				//不支持通信
#define TCFG_UART_COMM						1				//串口通信
#define TCFG_USB_COMM						2				//USB通信
#define TCFG_SPP_COMM						3				//SPP通信
#if (TCFG_CFG_TOOL_ENABLE || TCFG_EFFECT_TOOL_ENABLE)
#define TCFG_COMM_TYPE						TCFG_USB_COMM	//通信方式选择
#else
#define TCFG_COMM_TYPE						TCFG_NULL_COMM
#endif
#define TCFG_ONLINE_TX_PORT					IO_PORT_DP      //UART模式调试TX口选择
#define TCFG_ONLINE_RX_PORT					IO_PORT_DM      //UART模式调试RX口选择
#define TCFG_ONLINE_ENABLE                  (TCFG_EFFECT_TOOL_ENABLE)    //是否支持音效在线调试功能

/***********************************非用户配置区***********************************/
#if (TCFG_CFG_TOOL_ENABLE || TCFG_ONLINE_ENABLE)
#if TCFG_COMM_TYPE == TCFG_UART_COMM
#undef TCFG_UDISK_ENABLE
#define TCFG_UDISK_ENABLE 					0
#undef TCFG_SD0_PORTS
#define TCFG_SD0_PORTS 					   'B'
#endif
#endif

#include "usb_std_class_def.h"
#if (TCFG_CFG_TOOL_ENABLE || TCFG_EFFECT_TOOL_ENABLE)
#if (TCFG_COMM_TYPE == TCFG_USB_COMM)
#define TCFG_USB_CDC_BACKGROUND_RUN         ENABLE
#endif
#if (TCFG_COMM_TYPE == TCFG_UART_COMM)
#define TCFG_USB_CDC_BACKGROUND_RUN         DISABLE
#endif
#endif
/**********************************************************************************/

//*********************************************************************************//
//                               声卡功能相关配置                                  //
//*********************************************************************************//
#define SOUNDCARD_ENABLE                    DISABLE
#define MULTI_AUDIO_UPLOAD_TO_UAC_ENABLE    DISABLE //非声卡方案的多音频UAC传输配置

//目前只支持uac数据上行
#if MULTI_AUDIO_UPLOAD_TO_UAC_ENABLE
#undef  USB_DEVICE_CLASS_CONFIG
#define USB_DEVICE_CLASS_CONFIG 		    TCFG_PC_FOUR_MIC ? (MIC_CLASS) : (MASSSTORAGE_CLASS|MIC_CLASS|HID_CLASS)
#endif

//*********************************************************************************//
//                                  混响配置                                   //
//*********************************************************************************//
#define TCFG_MIC_EFFECT_ENABLE       	DISABLE_THIS_MOUDLE
#define TCFG_MIC_EFFECT_START_DIR    	DISABLE//开机直接启动混响

//广播mic 音效选择
#define WIRELESS_MIC_EFFECT_ENABLE       	ENABLE_THIS_MOUDLE
#define WIRELESS_MIC_TX_EFFECT_ENABLE   ENABLE //主机发送前做mic音效
#define WIRELESS_MIC_RX_EFFECT_ENABLE    DISABLE //从机收到数据做mic音效

#define WIRELESS_MIC_EFFECT_ONLY_DVOL       ENABLE //mic音效只跑数字音量节点

//混响效果配置
#define MIC_EFFECT_REVERB		1 //混响
#define MIC_EFFECT_ECHO         2 //回声
#define MIC_EFFECT_REVERB_ECHO  3 //混响+回声
#define MIC_EFFECT_MEGAPHONE    4 //扩音器/大声公
#define MIC_EFFECT_REVERB_ECHO_ADVANCE  0xb//混响+回声+混响尾部多带drc处理
#define TCFG_MIC_EFFECT_SEL          	MIC_EFFECT_REVERB_ECHO

//混响降噪使能配置，目前的版本需要根据采样率更改中断点数
#define MIC_EFFECT_LLNS         0 //ENABLE

//MIC变声模块使能配置
#define TCFG_MIC_VOICE_CHANGER_ENABLE 	DISABLE
#define TCFG_MIC_AUTOTUNE_ENABLE        DISABLE//电音模块使能
#define TCFG_MIC_BASS_AND_TREBLE_ENABLE     DISABLE//混响高低音
#define TCFG_MIC_GAIN_ENABLE            DISABLE//gain模块

/***********************************非用户配置区***********************************/
#if TCFG_MIC_EFFECT_ENABLE || (WIRELESS_MIC_EFFECT_ENABLE && !WIRELESS_MIC_EFFECT_ONLY_DVOL)
#undef EQ_SECTION_MAX
#define EQ_SECTION_MAX 10
#ifdef TCFG_AEC_ENABLE
#undef TCFG_AEC_ENABLE
#define TCFG_AEC_ENABLE 0
#endif//TCFG_AEC_ENABLE

#if !TCFG_EQ_ENABLE
#undef TCFG_EQ_ENABLE
#define TCFG_EQ_ENABLE 1//混响必开eq
#endif
#if !TCFG_DRC_ENABLE
#undef TCFG_DRC_ENABLE
#define TCFG_DRC_ENABLE 1//混响必开drc
#endif
#endif
/**********************************************************************************/
#define TCFG_REVERB_SAMPLERATE_DEFUAL (44100)
#define MIC_EFFECT_SAMPLERATE			(44100L)

#if TCFG_MIC_EFFECT_ENABLE
#undef MIC_SamplingFrequency
#define     MIC_SamplingFrequency         1
#undef MIC_AUDIO_RATE
#define     MIC_AUDIO_RATE              MIC_EFFECT_SAMPLERATE
#undef  SPK_AUDIO_RATE
#define SPK_AUDIO_RATE                  TCFG_REVERB_SAMPLERATE_DEFUAL
#endif


/*********配置MIC_EFFECT_CONFIG宏定义即可********************************/
#define TCFG_USB_MIC_ECHO_ENABLE           DISABLE //不能与TCFG_MIC_EFFECT_ENABLE同时打开
#define TCFG_USB_MIC_DATA_FROM_MICEFFECT   DISABLE //要确保开usbmic前已经开启混响

//*********************************************************************************//
//                                  系统配置                                         //
//*********************************************************************************//
#define TCFG_AUTO_SHUT_DOWN_TIME		          0   //没有蓝牙连接自动关机时间
#define TCFG_SYS_LVD_EN						      1   //电量检测使能
#define TCFG_POWER_ON_NEED_KEY				      1	  //是否需要按按键开机配置
#define TWFG_APP_POWERON_IGNORE_DEV         	  4000//上电忽略挂载设备，0时不忽略，非0则n毫秒忽略

//*********************************************************************************//
//                                  蓝牙配置                                       //
//*********************************************************************************//
#define TCFG_USER_BLE_ENABLE                	  0   //BLE功能使能
#define TCFG_USER_BT_CLASSIC_ENABLE         	  0   //经典蓝牙功能使能
#define TCFG_BT_SUPPORT_AAC                 	  0   //AAC格式支持
#define TCFG_BT_SNIFF_ENABLE                	  0   //bt sniff 功能使能
#define TCFG_BT_RF_USE_EXT_PA_ENABLE	          0   //是否使用外置PA

#define USER_SUPPORT_PROFILE_SPP    0
#define USER_SUPPORT_PROFILE_HFP    0
#define USER_SUPPORT_PROFILE_A2DP   0
#define USER_SUPPORT_PROFILE_AVCTP  0
#define USER_SUPPORT_PROFILE_HID    0
#define USER_SUPPORT_PROFILE_PNP    0
#define USER_SUPPORT_PROFILE_PBAP   0
#define USER_SUPPORT_PROFILE_MAP    0

#define TCFG_BD_NUM						    1   //连接设备个数配置
#define TCFG_AUTO_STOP_PAGE_SCAN_TIME       0 //配置一拖二第一台连接后自动关闭PAGE SCAN的时间(单位分钟)
#define TCFG_USER_ESCO_SLAVE_MUTE           0   //对箱通话slave静音

#define BT_INBAND_RINGTONE                  0   //是否播放手机自带来电铃声
#define BT_PHONE_NUMBER                     0   //是否播放来电报号
#define BT_SUPPORT_DISPLAY_BAT              0   //是否使能电量检测
#define BT_SUPPORT_MUSIC_VOL_SYNC           0   //是否使能音量同步

#define TCFG_BLUETOOTH_BACK_MODE			0	//后台模式

/*spp数据导出配置*/
#if ((TCFG_AUDIO_DATA_EXPORT_ENABLE == AUDIO_DATA_EXPORT_USE_SPP) || TCFG_AUDIO_MIC_DUT_ENABLE)
#undef TCFG_USER_BLE_ENABLE
#undef TCFG_BD_NUM
#undef USER_SUPPORT_PROFILE_SPP
#undef USER_SUPPORT_PROFILE_A2DP
#define TCFG_USER_BLE_ENABLE        0//spp数据导出，关闭ble
#define TCFG_BD_NUM					1//连接设备个数配置
#define USER_SUPPORT_PROFILE_SPP	1
#define USER_SUPPORT_PROFILE_A2DP   0
#define APP_ONLINE_DEBUG            1//通过spp导出数据
#else
#define APP_ONLINE_DEBUG            0//在线APP调试,发布默认不开
#endif/*TCFG_AUDIO_DATA_EXPORT_ENABLE*/

/*以下功能需要打开SPP和在线调试功能
 *1、回音消除参数在线调试
 *2、通话CVP_DUT 产测模式
 *3、ANC工具蓝牙spp调试
 */
#if (TCFG_AEC_TOOL_ONLINE_ENABLE || TCFG_AUDIO_CVP_DUT_ENABLE)
#undef USER_SUPPORT_PROFILE_SPP
#undef APP_ONLINE_DEBUG
#define USER_SUPPORT_PROFILE_SPP	1
#define APP_ONLINE_DEBUG            1
#endif/*TCFG_AEC_TOOL_ONLINE_ENABLE*/

#if (APP_ONLINE_DEBUG && !USER_SUPPORT_PROFILE_SPP)
#error "NEED ENABLE USER_SUPPORT_PROFILE_SPP!!!"
#endif

#if ((TCFG_CFG_TOOL_ENABLE || TCFG_EFFECT_TOOL_ENABLE) && (TCFG_COMM_TYPE == TCFG_SPP_COMM))
#if ((!USER_SUPPORT_PROFILE_SPP) || (!APP_ONLINE_DEBUG))
#error "Please enable USER_SUPPORT_PROFILE_SPP and APP_ONLINE_DEBUG ! !"
#endif
#endif

#define LE_BIG_TX_ROLE			   BIT(0)
#define LE_BIG_RX_ROLE			   BIT(1)
#define LE_AUDIO_TEST_EN		   0//LE_BIG_TX_ROLE

#define TCFG_BROADCAST_ENABLE       ENABLE_THIS_MOUDLE
/* le audio BIG mode control */
#if TCFG_BROADCAST_ENABLE
#define LEA_BIG_CTRLER_TX_EN        1
#define LEA_BIG_CTRLER_RX_EN        0
#define BROADCAST_DATA_SYNC_EN      1   //广播状态同步
#define BROADCAST_TX_LOCAL_DEC_EN   0   //广播音频发送端是否解码发送的音频
#define BROADCAST_RECEIVER_CLOSE_EDR_CONN   0   //广播从机关闭经典蓝牙连接
#define BROADCAST_TX_CHANNEL_SEPARATION     0   //发送端音频声道分离使能，要求发送端为双声道，接收端为单声道并且码率为发送端一半
#define BROADCAST_SOURCE_TRIGGER_BY_TX_ALIGN 1 //adc的启动由首次tx_alagn触发，需要使用同步处理。
#define BROADCAST_SOURCE_IRQ_SYNC_TX_ALIGN   1 //adc的中断节奏与tx_align节奏对其。不需要同步处理,需要使能 BROADCAST_SOURCE_TRIGGER_BY_TX_ALIGN  tx端加eq处理，需要关掉此功能。

#define BROADCAST_ENTER_PAIR_BEFORE_RUN     2   //广播音频之前先进行配对，0-不配对，1-广播配对，2-连接配对
#if BROADCAST_DATA_SYNC_EN
#define BROADCAST_CUSTOM_DATA_LEN     2   //自定义数据大小，不要修改
#else
#define BROADCAST_CUSTOM_DATA_LEN     0   //自定义数据大小，不要修改
#endif

#else
#define LEA_BIG_CTRLER_TX_EN        0
#define LEA_BIG_CTRLER_RX_EN        0
#define BROADCAST_DATA_SYNC_EN      0
#define BROADCAST_TX_LOCAL_DEC_EN   0
#define BROADCAST_RECEIVER_CLOSE_EDR_CONN   0
#define BROADCAST_TX_CHANNEL_SEPARATION     0
#define BROADCAST_SOURCE_TRIGGER_BY_TX_ALIGN 0
#define BROADCAST_SOURCE_IRQ_SYNC_TX_ALIGN  0
#define BROADCAST_ENTER_PAIR_BEFORE_RUN     0
#define BROADCAST_CUSTOM_DATA_LEN     0
#endif

#define TCFG_CONNECTED_ENABLE       DISABLE_THIS_MOUDLE
/* le audio CIG mode control */
#if TCFG_CONNECTED_ENABLE
#define LEA_CIG_CENTRAL_EN          0
#define LEA_CIG_PERIPHERAL_EN       1
#define CONNECTED_KEY_EVENT_SYNC    0
#define LEA_CIG_CONNECTION_NUM      1   //配置连接设备数，最大配置为2
#define CONNECTED_TX_LOCAL_DEC_EN   0   //CIG音频发送端是否解码发送的音频
#define CONNECTED_TX_CHANNEL_SEPARATION 0   //发送端音频声道分离使能，要求发送端为双声道，接收端为单声道并且码率为发送端一半
#else
#define LEA_CIG_CENTRAL_EN          0
#define LEA_CIG_PERIPHERAL_EN       0
#define CONNECTED_KEY_EVENT_SYNC    0
#define LEA_CIG_CONNECTION_NUM      0
#define CONNECTED_TX_LOCAL_DEC_EN   0
#define CONNECTED_TX_CHANNEL_SEPARATION 0
#endif

//*********************************************************************************//
//                           编解码格式配置(CodecFormats Config)                   //
//*********************************************************************************//
//解码格式使能配置(Decode Format Config)
#define TCFG_DEC_MP3_ENABLE                 DISABLE
#define TCFG_DEC_WTGV2_ENABLE				DISABLE
#define TCFG_DEC_PCM_ENABLE					DISABLE

//编码格式使能配置(Encode Format Config)
#define TCFG_ENC_MP3_ENABLE                 DISABLE
#define TCFG_ENC_JLA_ENABLE				    ENABLE
#define TCFG_ENC_JLA_LL_ENABLE		   		ENABLE

//JL_CC_CODED_EN和JL_MPR_HARD_CMB_EN只需开启其中一个
//JL_CC_CODED_EN暂不支持
//JL_LOW_TX_POWER_EN使能后最高发射功率会被压低
#define JL_CC_CODED_EN						0
#define JL_MPR_HARD_CMB_EN					0
#define JL_LOW_TX_POWER_EN                  0

//JLA 编解码参数配置
#if (TCFG_ENC_JLA_ENABLE || TCFG_DEC_JLA_ENABLE)
#define JLA_CODING_SAMPLERATE  48000 	//JLA 编码的采样率 广播暂不支持44100;
#define JLA_CODING_FRAME_LEN   25 	    //一帧编码的pcm时间长度,只支持25，50，100,分别对应2.5ms,5ms,10ms的数据长度;
#define JLA_CODING_CHANNEL     1  		//JLA 的通道数 其他参数不变，实际单声道的码率是双声道码率的 2 倍;
#if JL_CC_CODED_EN
#define JLA_CODING_BIT_RATE    160000
#else
#define JLA_CODING_BIT_RATE    84000
#endif
#endif/*TCFG_ENC_JLA_ENABLE*/

#if TCFG_ENC_JLA_LL_ENABLE

#define JLA_LL_ORDER1  0 //jla_ll 一阶编码
#define JLA_LL_ORDER2  1 //jla_ll 二阶编码

#define JLA_LL_CODING_ORDER_TYPE    JLA_LL_ORDER1 //JLA_LL 编码阶数类型配置

#define JLA_LL_CODING_SAMPLERATE   	32000 	//JLA_LL 编码的采样率;
#define JLA_LL_CODING_FRAME_LEN   	25 	    //一帧编码的pcm时间长度,只支持25，50，100,分别对应2.5ms,5ms,10ms的数据长度;
#define JLA_LL_CODING_CHANNEL     	1  		//JLA_LL 的通道数 ;
#define JLA_LL_CODING_BIT_RATE    	(JLA_LL_CODING_SAMPLERATE * JLA_LL_CODING_CHANNEL * 16 / 4)  //JLA_LL的码率;

#define JLA_LL_CODEC_INPUT_POINT  	(JLA_LL_CODING_SAMPLERATE * JLA_LL_CODING_FRAME_LEN * JLA_LL_CODING_CHANNEL / 10 / 1000)
#if (JLA_LL_CODING_ORDER_TYPE == JLA_LL_ORDER1)
//编码压缩比配置。0 ~ JLA_LL_CODEC_INPUT_POINT,  0 :压缩率最高,
//长度需要减少N个byte ,则((JLA_LL_CODEC_INPUT_POINT - 8 * 2 * (N >> 1) - (N & 1) * 4) )
#define JLA_LL_CODEC_CR_CONFIG      ((JLA_LL_CODEC_INPUT_POINT - 8 * 2 * (0 >> 1) - (0 & 1) * 4) )
#else
//长度需要减少N个byte ,则(JLA_LL_CODEC_INPUT_POINT - 8 * N)
#define JLA_LL_CODEC_CR_CONFIG  	(JLA_LL_CODEC_INPUT_POINT - 8 * (1 + BROADCAST_CUSTOM_DATA_LEN))

#endif//JLA_LL_CODING_ORDER_TYPE == JLA_LL_ORDER1

#endif

#define LIVE_AUDIO_CODING_JLA		    0
#define LIVE_AUDIO_CODING_JLA_LL	   	1  //低延时编码(JLA low latency)

#define WIERLESS_TRANS_CODING_TYPE 	    LIVE_AUDIO_CODING_JLA_LL  //无线音频传输格式设置

#if (WIERLESS_TRANS_CODING_TYPE == LIVE_AUDIO_CODING_JLA)
#undef TCFG_ENC_JLA_LL_ENABLE
#define TCFG_ENC_JLA_LL_ENABLE        DISABLE

#define BROADCAST_CODING_SAMPLE_RATE  JLA_CODING_SAMPLERATE
#define BROADCAST_CODING_BIT_RATE     JLA_CODING_BIT_RATE
#define BROADCAST_CODING_FRAME_LEN    JLA_CODING_FRAME_LEN
#define BROADCAST_CODING_CHANNEL      JLA_CODING_CHANNEL
#else
#undef TCFG_ENC_JLA_ENABLE
#define TCFG_ENC_JLA_ENABLE           DISABLE

#define BROADCAST_CODING_SAMPLE_RATE  JLA_LL_CODING_SAMPLERATE
#define BROADCAST_CODING_BIT_RATE     JLA_LL_CODING_BIT_RATE
#define BROADCAST_CODING_FRAME_LEN    JLA_LL_CODING_FRAME_LEN
#define BROADCAST_CODING_CHANNEL      JLA_LL_CODING_CHANNEL
#endif /*WIERLESS_TRANS_CODING_TYPE == LIVE_AUDIO_CODING_JLA*/



/*实时音频低延时使能，BIG和CIG两种模式均可使用*/
#define TCFG_LIVE_AUDIO_LOW_LATENCY_EN      1

#define TCFG_IIS_CAPTURE_SAMPLE_PERIOD      1000//us

// JLA编解码代码放RAM使能
#define TCFG_JLA_CODEC_AT_RAM_EN            1

#if TCFG_LIVE_AUDIO_LOW_LATENCY_EN
#if (TCFG_ENC_JLA_ENABLE && (JLA_CODING_FRAME_LEN > 50))
#error "Low latency live audio need use JLA_CODING_FRAME_LEN 50! You can comment out this code if you don't want the lowest latency!"
#endif
#endif

//*********************************************************************************//
//                                  IIS 配置                                       //
//*********************************************************************************//

//混响同时开启iis输入输出时，会对数据进行拆包处理，建议IIS中断点数512，混响每次处理的数据点数256
#define TCFG_AUDIO_INPUT_IIS                DISABLE_THIS_MOUDLE
#define TCFG_AUDIO_OUTPUT_IIS               DISABLE_THIS_MOUDLE
//是否输出iiS的时候同时输出DAC
#define TCFG_AUDIO_OUTPUT_DAC_IIS           DISABLE_THIS_MOUDLE

#define TCFG_IIS_MODE                       (0)   //  0:master  1:slave
#define TCFG_IIS_SR                         44100
#define TCFG_IIS_PORT                       ALINK0_PORTA

// OUTPUT WAY
#if TCFG_AUDIO_OUTPUT_IIS
#define AUDIO_OUT_WAY_TYPE 				    AUDIO_WAY_TYPE_IIS
#else
#define AUDIO_OUT_WAY_TYPE 					AUDIO_WAY_TYPE_DAC
#endif

#if TCFG_AUDIO_OUTPUT_DAC_IIS
#define AUDIO_OUT_WAY_TYPE 				    AUDIO_WAY_TYPE_DAC_IIS
#endif

//使能IIS MIC流程(esco通话流程中使用)
#define TCFG_ESCO_IIS_MIC_ENABLE            DISABLE

//iis采样率为48k，需要将PLL改为240M
#if  (TCFG_IIS_SR == 48000)
#ifdef TCFG_CLOCK_PLL_240MHZ
#undef TCFG_CLOCK_PLL_240MHZ
#define TCFG_CLOCK_PLL_240MHZ               1
#endif
#endif

//*********************************************************************************//
//                                  linein配置                                     //
//*********************************************************************************//
#define TCFG_LINEIN_ENABLE					TCFG_APP_LINEIN_EN		// linein使能
#define TCFG_LINEIN_LR_CH					AUDIO_ADC_LINE1
#define TCFG_LINEIN_DETECT_ENABLE           DISABLE
#define TCFG_LINEIN_CHECK_PORT				IO_PORTA_03				// linein检测IO
#define TCFG_LINEIN_PORT_UP_ENABLE        	1						// 检测IO上拉使能
#define TCFG_LINEIN_PORT_DOWN_ENABLE       	0						// 检测IO下拉使能
#define TCFG_LINEIN_AD_CHANNEL             	NO_CONFIG_PORT			// 检测IO是否使用AD检测
#define TCFG_LINEIN_VOLTAGE                	0						// AD检测时的阀值

#define TCFG_LINEIN_ENERGY_DETECT           DISABLE
#if TCFG_LINEIN_ENERGY_DETECT
#undef  TCFG_LINEIN_DETECT_ENABLE
#define TCFG_LINEIN_DETECT_ENABLE           DISABLE

#undef  SYS_DIGVOL_GROUP_EN
#define SYS_DIGVOL_GROUP_EN                 ENABLE
#endif

/*Linein输入方式配置*/
#define LINEIN_INPUT_WAY_ANALOG      0	// 模拟通路
#define LINEIN_INPUT_WAY_ADC         1	// 数字通路
#define LINEIN_INPUT_WAY_DAC         2  // 数字通路（DAC）
#define TCFG_LINEIN_INPUT_WAY               LINEIN_INPUT_WAY_ADC	//AC706N仅支持数字通路（ADC通路)

#define TCFG_LINEIN_MULTIPLEX_WITH_FM		DISABLE 				// linein 脚与 FM 脚复用
#define TCFG_LINEIN_MULTIPLEX_WITH_SD		DISABLE 				// linein 检测与 SD cmd 复用
#define TCFG_LINEIN_SD_PORT		            0// 0:sd0 1:sd1     //选择复用的sd

//*********************************************************************************//
//                                  fat 文件系统配置                                       //
//*********************************************************************************//
#define CONFIG_FATFS_ENABLE					DISABLE
#define TCFG_LFN_EN                         0

//*********************************************************************************//
//                         音效配置(AudioEffects Config)
//*********************************************************************************//
/* 音效配置-人声消除(AudioEffects-VocalRemove)*/
#define AUDIO_VOCAL_REMOVE_EN       0

/*音效配置-等响度(AudioEffects-EqualLoudness)*/
//注1：等响度 开启后，需要固定模拟音量,调节软件数字音量
//注2：等响度使用EQ实现，同个数据流中，若打开等响度，请开EQ总使能，关闭其他EQ,例如蓝牙模式EQ
#define AUDIO_EQUALLOUDNESS_CONFIG  0	//等响度, 不支持四声道

/*音效配置-环绕声(AudioEffects-Surround)*/
#define AUDIO_SURROUND_CONFIG     	0	//3D环绕

/*音效配置-虚拟低音(AudioEffects-VBass)*/
#define AUDIO_VBASS_CONFIG        	0	//虚拟低音,虚拟低音不支持四声道
#define AUDIO_VBASS_32BIT_OUT_EN    0   //虚拟低音32bit位宽输出使能 0:16bit 1:32bit，开启32bit后不支持环绕声

#define MUSIC_EXT_EQ_AFTER_DRC              0     //drc后的eq
#define TCFG_PHASER_GAIN_AND_CH_SWAP_ENABLE 0     //gain和smix模块

#define AUDIO_SPECTRUM_CONFIG     	0  //频响能量值获取接口
#define AUDIO_MIDI_CTRL_CONFIG    	0  //midi电子琴接口使能

#define BASS_AND_TREBLE_ENABLE            0//旋钮调eq调节接口使能(高音、低音eq/drc使能, 使能后，相关参数有效果文件配置)
#if BASS_AND_TREBLE_ENABLE
#if TCFG_AUDIO_OUT_EQ_ENABLE == 0
#undef TCFG_AUDIO_OUT_EQ_ENABLE
#define TCFG_AUDIO_OUT_EQ_ENABLE 1
#endif
#if TCFG_AUDIO_OUT_DRC_ENABLE == 0
#undef TCFG_AUDIO_OUT_DRC_ENABLE
#define TCFG_AUDIO_OUT_DRC_ENABLE 1
#endif
#if TCFG_MIC_BASS_AND_TREBLE_ENABLE == 0
#undef TCFG_MIC_BASS_AND_TREBLE_ENABLE
#define TCFG_MIC_BASS_AND_TREBLE_ENABLE 1
#endif

#endif


//*********************************************************************************//
//                                 无线2.X声道配置
//                                 2.1  情景一：1个全频音箱(从机做LR) + 一个低音炮(主机做低音)
//                                 2.1  情景二：两个全频音箱(从机1做L,从机2做R) + 一个低音炮(主机做低音)
//*********************************************************************************//
#define WIRELESS_SOUND_TRACK_2_P_X_ENABLE   0//无线2.1声道使能
#define SOUND_TRACK_2_P_X_CH_CONFIG        WIRELESS_SOUND_TRACK_2_P_X_ENABLE //2.X声道总使能

#if SOUND_TRACK_2_P_X_CH_CONFIG

#define BIT(n)                       (1UL << (n))
#define TWO_POINT_ONE_CH             BIT(0)  //2.1声道
#define TWO_POINT_TWO_CH             BIT(1)  //2.2声道
#define TWO_POINT_ZERO_CH            BIT(2)  //2.0声道
#define WIRELESS_SCENE_ONE           BIT(3)  //无线2.1声道，场景1
#define WIRELESS_SCENE_TWO           BIT(4)  //无线2.1声道，场景2
#define TWO_POINT_X_CH               WIRELESS_SCENE_TWO// WIRELESS_SCENE_ONE //选配xx

/*无线2.x声道下相应模块使能*/
#if (TWO_POINT_X_CH == WIRELESS_SCENE_ONE) || (TWO_POINT_X_CH == WIRELESS_SCENE_TWO)
#define TCFG_EQ_DIVIDE_ENABLE               1
#undef TCFG_PHASER_GAIN_AND_CH_SWAP_ENABLE
#define TCFG_PHASER_GAIN_AND_CH_SWAP_ENABLE 1
#undef MUSIC_EXT_EQ_AFTER_DRC
#define MUSIC_EXT_EQ_AFTER_DRC              1
#define MUSIC_EXT_EQ2_AFTER_DRC             1
#if (TWO_POINT_X_CH == WIRELESS_SCENE_TWO)
#undef  AUDIO_SURROUND_CONFIG
#define AUDIO_SURROUND_CONFIG     	        1
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)
#define AUDIO_SURROUND_LEFT_OR_RIGHT     	        0  //立体声时、配置环绕声声道，0:左左   1:右右
#endif/*(TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)*/
#endif/*(TWO_POINT_X_CH == WIRELESS_SCENE_TWO)*/
#endif/*(TWO_POINT_X_CH == WIRELESS_SCENE_ONE) || (TWO_POINT_X_CH == WIRELESS_SCENE_TWO)*/


#endif/*SOUND_TRACK_2_P_X_CH_CONFIG*/

#if WIRELESS_SOUND_TRACK_2_P_X_ENABLE || TCFG_DYNAMIC_EQ_ENABLE
#undef  EQ_SECTION_MAX
#define EQ_SECTION_MAX                      20	  //EQ段数
#if TCFG_DYNAMIC_EQ_ENABLE
#define TCFG_BROADCAST_MODE_LAST_DRC_ENABLE 0     //广播音箱动态eq后添加个drc限幅处理，默认关闭(跑不过来)
#endif/*TCFG_DYNAMIC_EQ_ENABLE*/
#endif/*WIRELESS_SOUND_TRACK_2_P_X_ENABLE || TCFG_DYNAMIC_EQ_ENABLE*/



//*********************************************************************************//
//                                 1.1声道配置
//                       左声道输出全频，右声道输出低音
//*********************************************************************************//
#define  SOUND_TRACK_1_P_1_CH_CONFIG  0


#if SOUND_TRACK_1_P_1_CH_CONFIG
#undef TCFG_PHASER_GAIN_AND_CH_SWAP_ENABLE
#define TCFG_PHASER_GAIN_AND_CH_SWAP_ENABLE  1
#undef TCFG_EQ_SPILT_ENABLE
#define TCFG_EQ_SPILT_ENABLE  1
#undef TCFG_DRC_SPILT_ENABLE
#define TCFG_DRC_SPILT_ENABLE  1
#endif
//*********************************************************************************//
//                                 配置结束                                        //
//*********************************************************************************//

#endif //CONFIG_BOARD_AC706N_TEST
#endif //CONFIG_BOARD_AC706N_TEST_CFG_H

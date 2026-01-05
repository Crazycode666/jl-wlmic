#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/*
 * 系统打印总开关
 */


#ifdef CONFIG_RELEASE_ENABLE
#define LIB_DEBUG    0
#else
#define LIB_DEBUG    1
#endif

#define CONFIG_DEBUG_LIB(x)         (x & LIB_DEBUG)

#define CONFIG_DEBUG_ENABLE

#ifndef CONFIG_DEBUG_ENABLE
//#define CONFIG_DEBUG_LITE_ENABLE  //轻量级打印开关, 默认关闭
#endif

//*********************************************************************************//
//						APP应用默认配置	                                           //
//注意！！！注意！！！注意！！！         	                                       //
//1、以下配置各个app应用方向的默认宏定义	                                       //
//2、禁止直接修改这里的配置														   //
//3、应用方向的配置如果与以下默认配置不一致， 必须在自己板卡中进行重定义           //
//4、应用方向的配置,在这里只定义大的功能方向， 细节宏定义请在板卡中定义            //
//*********************************************************************************//

// #define TCFG_MIXER_EXT_ENABLE											0

#define TCFG_PC_BACKMODE_ENABLE											0

#define TCFG_ENC_WRITE_FILE_ENABLE		                                1

#define TCFG_MEDIA_LIB_USE_MALLOC										1

#define TCFG_AEC_ENABLE													1

#define TCFG_DEV_UPDATE_IF_NOFILE_ENABLE								0

#define TCFG_DEV_MANAGER_ENABLE											1

#define SD_BAUD_RATE_CHANGE_WHEN_SCAN                                   (12000000L)

#define BLUETOOTH_TOGGLE                                                1

#define TCFG_NEED_DEC_ENABLE                                            0

//*********************************************************************************//
//						APP应用默认配置请在此之前定义	                           //
//						APP应用默认配置请在此之前定义	                           //
//						APP应用默认配置请在此之前定义	                           //
//*********************************************************************************//
#include "asm/clock_define.h"
#include "board_config.h"
#include "btcontroller_mode.h"
#include "user_cfg_id.h"

#define STYLE_JL_WTACH              (1)//彩屏demo
#define STYLE_JL_SOUNDBOX           (2)//点阵屏demo
#define STYLE_JL_CHARGE             (3)//点阵屏充电仓
#define STYLE_JL_LED7               (4)//led7
#define STYLE_UI_SIMPLE             (5)//没有ui框架

#if (!BLUETOOTH_TOGGLE)
#undef TCFG_APP_BT_EN
#define TCFG_APP_BT_EN                  0

#undef TCFG_USER_BLE_ENABLE
#define TCFG_USER_BLE_ENABLE            0

#undef TCFG_BROADCAST_ENABLE
#define TCFG_BROADCAST_ENABLE           0

#undef TCFG_CONNECTED_ENABLE
#define TCFG_CONNECTED_ENABLE           0

#undef TCFG_USER_BT_CLASSIC_ENABLE
#define TCFG_USER_BT_CLASSIC_ENABLE     0
#endif

#ifdef CONFIG_SDFILE_ENABLE

#define SDFILE_DEV				"sdfile"
#define SDFILE_MOUNT_PATH     	"mnt/sdfile"

#if (USE_SDFILE_NEW)
#define SDFILE_APP_ROOT_PATH       	SDFILE_MOUNT_PATH"/app/"  //app分区
#define SDFILE_RES_ROOT_PATH       	SDFILE_MOUNT_PATH"/res/"  //资源文件分区
#else
#define SDFILE_RES_ROOT_PATH       	SDFILE_MOUNT_PATH"/C/"
#endif

#endif

#define RTC_CLK_RES_SEL             CLK_SEL_32K     //rtc时钟源选择:CLK_SEL_32K/CLK_SEL_12M/CLK_SEL_24M

//*********************************************************************************//
//                                 测试模式配置                                    //
//*********************************************************************************//
#if (CONFIG_BT_MODE == BT_NORMAL)
//enable dut mode,need disable sleep(TCFG_LOWPOWER_LOWPOWER_SEL = 0)
#define TCFG_NORMAL_SET_DUT_MODE                  0
#if TCFG_NORMAL_SET_DUT_MODE
#undef  TCFG_LOWPOWER_LOWPOWER_SEL
#define TCFG_LOWPOWER_LOWPOWER_SEL                0
#endif

#else

#undef  TCFG_BD_NUM
#define TCFG_BD_NUM						          1

#undef  TCFG_USER_BLE_ENABLE
#define TCFG_USER_BLE_ENABLE                      1     //BLE功能使能

#undef  TCFG_AUTO_SHUT_DOWN_TIME
#define TCFG_AUTO_SHUT_DOWN_TIME		          0

#undef  TCFG_SYS_LVD_EN
#define TCFG_SYS_LVD_EN						      0

#undef  TCFG_LOWPOWER_LOWPOWER_SEL
#define TCFG_LOWPOWER_LOWPOWER_SEL                0

#undef TCFG_AUDIO_DAC_LDO_VOLT
#define TCFG_AUDIO_DAC_LDO_VOLT			    DUT_AUDIO_DAC_LDO_VOLT

#undef TCFG_LOWPOWER_POWER_SEL
#define TCFG_LOWPOWER_POWER_SEL				PWR_LDO15

#undef  TCFG_PWMLED_ENABLE
#define TCFG_PWMLED_ENABLE					DISABLE_THIS_MOUDLE

#undef  TCFG_ADKEY_ENABLE
#define TCFG_ADKEY_ENABLE                   DISABLE_THIS_MOUDLE

#undef  TCFG_IOKEY_ENABLE
#define TCFG_IOKEY_ENABLE					DISABLE_THIS_MOUDLE

#undef TCFG_TEST_BOX_ENABLE
#define TCFG_TEST_BOX_ENABLE			    0

#undef TCFG_AUTO_SHUT_DOWN_TIME
#define TCFG_AUTO_SHUT_DOWN_TIME		          0

#undef TCFG_POWER_ON_NEED_KEY
#define TCFG_POWER_ON_NEED_KEY				      0

#undef TCFG_SD0_ENABLE
#define TCFG_SD0_ENABLE				0

#undef TCFG_SD1_ENABLE
#define TCFG_SD1_ENABLE				0

#undef TCFG_APP_PC_EN
#define TCFG_APP_PC_EN					    0

#undef TCFG_PC_ENABLE
#define TCFG_PC_ENABLE                     0

#undef TCFG_UDISK_ENABLE
#define TCFG_UDISK_ENABLE				0

/* #undef TCFG_UART0_ENABLE
#define TCFG_UART0_ENABLE					DISABLE_THIS_MOUDLE */

#endif //(CONFIG_BT_MODE != BT_NORMAL)

/////要确保 上面 undef 后在include usb
#include "usb_common_def.h"

#if (CONFIG_BT_MODE != BT_NORMAL)
////bqb 如果测试3M tx buf 最好加大一点
#undef  CONFIG_BT_TX_BUFF_SIZE
#define CONFIG_BT_TX_BUFF_SIZE  (6 * 1024)
#endif

//*********************************************************************************//
//                                 电源切换配置                                    //
//*********************************************************************************//

#define PHONE_CALL_USE_LDO15	CONFIG_PHONE_CALL_USE_LDO15


#ifdef CONFIG_FPGA_ENABLE

#undef TCFG_CLOCK_OSC_HZ
#define TCFG_CLOCK_OSC_HZ		12000000

#endif

/* 关闭没使用到的模块 */
#define TCFG_DEC_SBC_CLOSE          1
#define TCFG_DEC_MSBC_CLOSE         1
#define TCFG_DEC_SBC_HWACCEL_CLOSE  1
#define TCFG_DEC_CVSD_CLOSE         1
#define TCFG_DEC_PCM_CLOSE          1

//*********************************************************************************//
//                                 升级配置                                        //
//*********************************************************************************//
#if (defined(CONFIG_CPU_BR30) || defined(CONFIG_CPU_BR25) || defined(CONFIG_CPU_BR23) || defined(CONFIG_CPU_BR28))
//升级LED显示使能
#define UPDATE_LED_REMIND
//升级提示音使能
#define UPDATE_VOICE_REMIND
#endif

#if (defined(CONFIG_CPU_BR23) || defined(CONFIG_CPU_BR25))
//升级IO保持使能
//#define DEV_UPDATE_SUPPORT_JUMP           //目前只有br23\br25支持
#endif

#if (defined(CONFIG_CPU_BR23) || defined(CONFIG_CPU_BR25) || defined(CONFIG_CPU_BR28))
#define USER_UART_UPDATE_ENABLE           0//用于客户开发上位机或者多MCU串口升级方案

#define UART_UPDATE_SLAVE	0
#define UART_UPDATE_MASTER	1

//配置串口升级的角色
#define UART_UPDATE_ROLE	UART_UPDATE_SLAVE

#if USER_UART_UPDATE_ENABLE
#undef TCFG_CHARGESTORE_ENABLE
#undef TCFG_TEST_BOX_ENABLE
#define TCFG_CHARGESTORE_ENABLE				DISABLE_THIS_MOUDLE       //用户串口升级也使用了UART1
#endif

#endif  //USER_UART_UPDATE_ENABLE

//*********************************************************************************//
//                                 录音配置                                        //
//*********************************************************************************//
//录音文件夹名称定义，可以通过修改此处修改录音文件夹名称
#define REC_FOLDER_NAME				"JL_REC"

//*********************************************************************************//
//                                 无线麦配置                                        //
//*********************************************************************************//

//产测模式
#define  PRODUCT_TEST_MODE_ENABLE   0
//产测特殊名字
#define  PRODUTC_TEST_NAME          "product_fast_test"

//*********************************************************************************//
//                                256Kconfig
//*********************************************************************************//

#ifdef CONFIG_256K_FLASH
#define CONFIG_SOUNDBOX_FLASH_256K
#endif

#include "macro_default.h"
#include "build_error.h"

#endif


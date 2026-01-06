#include "key_event_deal.h"
#include "key_driver.h"
#include "app_config.h"
#include "board_config.h"
#include "app_task.h"

#ifdef CONFIG_BOARD_AC706N_2T1_SIMPLEX
/***********************************************************
 *				linein 模式的 iokey table
 ***********************************************************/
#if TCFG_APP_LINEIN_EN
const u16 linein_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
    [0] = {
        KEY_MUSIC_PP,			KEY_POWEROFF,			KEY_POWEROFF_HOLD,	KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [1] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [2] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [3] = {
        KEY_CHANGE_MODE,		KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [4] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [5] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
};
#endif

/***********************************************************
 *				pc 模式的 iokey table
 ***********************************************************/
#if TCFG_APP_PC_EN
const u16 pc_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
    [0] = {
        KEY_MUSIC_PP,			KEY_POWEROFF,			KEY_POWEROFF_HOLD,	KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [1] = {
        KEY_VOL_UP,				KEY_VOL_UP,				KEY_VOL_UP,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [2] = {
        KEY_VOL_DOWN,			KEY_VOL_DOWN,			KEY_VOL_DOWN,		KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [3] = {
        KEY_CHANGE_MODE,		KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [4] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [5] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
};
#endif

/***********************************************************
 *				idle 模式的 iokey table
 ***********************************************************/
const u16 idle_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
    [0] = {
        KEY_NULL,			    KEY_POWER_ON,			KEY_POWER_ON_HOLD,	KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [1] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,           KEY_NULL
    },
    [2] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,           KEY_NULL
    },
    [3] = {
        KEY_NULL,		        KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [4] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [5] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
};

/***********************************************************
 *				broadcast mic 模式的 adkey table
***********************************************************/
#if TCFG_APP_LIVE_MIC_EN
const u16 live_mic_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
    [0] = {
        KEY_NULL,			    KEY_POWEROFF,			KEY_POWEROFF_HOLD,	KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [1] = {
        KEY_VOL_UP,			    KEY_VOL_UP,		        KEY_VOL_UP,		    KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [2] = {
        KEY_VOL_DOWN,			KEY_VOL_DOWN,		    KEY_VOL_DOWN,		KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [3] = {
        KEY_MUSIC_PP,			KEY_NULL,		        KEY_NULL,		    KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [4] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [5] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
};
#endif

/***********************************************************
 *				broadcast iis 模式的 adkey table
***********************************************************/
#if TCFG_APP_LIVE_IIS_EN
const u16 live_iis_key_io_table[KEY_IO_NUM_MAX][KEY_EVENT_MAX] = {
    //单击             //长按          //hold         //抬起            //双击                //三击
    [0] = {
        KEY_NULL,			    KEY_POWEROFF,			KEY_POWEROFF_HOLD,	KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [1] = {
        KEY_VOL_UP,			    KEY_VOL_UP,		        KEY_VOL_UP,		    KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [2] = {
        KEY_VOL_DOWN,			KEY_VOL_DOWN,		    KEY_VOL_DOWN,		KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [3] = {
        KEY_MUSIC_PP,			KEY_NULL,		        KEY_NULL,		    KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [4] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
    [5] = {
        KEY_NULL,				KEY_NULL,				KEY_NULL,			KEY_NULL,	KEY_NULL,			KEY_NULL
    },
};
#endif
#endif

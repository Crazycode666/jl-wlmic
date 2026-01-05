#include "system/includes.h"
#include "app_config.h"


/*任务列表, 注意:stack_size设置为32*n*/
const struct task_info task_info_table[] = {
    {"app_core",            1,     0,   512,  768  },
    {"sys_event",           6,     0,   256,   0    },
    {"btctrler",            4,     0,   512,   384  },
    {"systimer",		    6,	   0,   128,   0    },
    {"dev_mg",           	3,     0,   512,   512  },
    {"update",				1,	   0,   512,   0	},

#if (TCFG_USER_BLE_ENABLE)
    {"btstack",             3,     0,   768,   256  },
#else
    {"btstack",             3,     0,   256,   256  },
#endif

#if (WIRELESS_MIC_EFFECT_ENABLE && WIRELESS_MIC_RX_EFFECT_ENABLE)
    {"audio_dec",           6,     1,   640,   128  },
    {"wireless_mic_effect", 4,     1,   256 + 256,   512 },
#else
    {"audio_dec",           6,     1,   512,   128  },
#endif

#if(USER_UART_UPDATE_ENABLE)
    {"uart_update",	        1,	   0,   256,   128	},
#endif

#if(TCFG_CHARGE_BOX_ENABLE)
    {"chgbox_n",            6,     0,   512,   128  },
#endif

#if TCFG_KEY_TONE_EN
    {"key_tone",            5,     0,   256,   32},
#endif

#if(TCFG_HOST_AUDIO_ENABLE)
    {"uac_play",            6,     0,   768,   0    },
    {"uac_record",          6,     0,   768,   32   },
#endif

#if ADAPTER_UART_DEMO
    {"uart_task",           2,  0,   256,       48   },
#endif

#if (TCFG_UART_1T2_EN)
    {"uart_1t2",	 		2,     0,	256,   64   },
#if (UART_1T2_ROLE == UART_1T2_ROLE_MASTER)
    {"uart_1t2_s",	 		2,     0,	256,   0  },
#endif
#endif

    /* {"btencry",             1,     0,   512,   128  }, */
    /* {"mic_stream",          5,     0,   768,   128  }, */
    /* {"usb_msd",             1,     0,   512,   128  }, */
    /* {"usb_audio",           5,     0,   256,   256  }, */
    /* {"plnk_dbg",            5,     0,   256,   256  }, */
    /* {"adc_linein",          2,     0,   768,   128  }, */
    /* {"enc_write",           1,     0,   768,   0 	}, */
    /* {"ui",                  3,     0,   320,   128  }, */
    /* {"audio_enc",           2,     0,   256,   128  }, */
    /* {"aec",				   2,	   0,   768,   0	}, */
    /* {"aec_dbg",			   3,	   0,   128,   128	}, */
    /* {"demo_task1",          2,     1,   768,   32 }, */
    {0, 0},
};



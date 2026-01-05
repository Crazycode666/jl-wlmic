#include "system/includes.h"
#include "app_config.h"


/*任务列表, 注意:stack_size设置为32*n*/
const struct task_info task_info_table[] = {
    {"app_core",            1,     0,   512,  768  },
    {"sys_event",           6,     0,   256,   0    },
    {"btctrler",            4,     0,   256 + 32,   384  },
    {"btstack",             3,     0,   256,   256  },
    {"update",				1,	   0,   512,   0	},
    {"systimer",		    6,	   0,   128,   0    },
    {"dev_mg",           	3,     0,   512,   512  },

#if WIRELESS_MIC_EFFECT_ENABLE
    {"wireless_mic_effect", 4,     1,   256 + 256,   512 },
#endif

#if ((defined TCFG_LIVE_AUDIO_LOW_LATENCY_EN) && TCFG_LIVE_AUDIO_LOW_LATENCY_EN)
    {"audio_enc",           7,     1,   384,   128  },
#else
    {"audio_enc",           2,     0,   384,   128  },
#endif

#if(USER_UART_UPDATE_ENABLE)
    {"uart_update",	        1,	   0,   256,   128	},
#endif

#if (TCFG_UI_ENABLE)
    {"ui",                  3,     0,   320,   128  },
#endif

#if(TCFG_CHARGE_BOX_ENABLE)
    {"chgbox_n",            6,     0,   512,   128  },
#endif

#if (AI_APP_PROTOCOL)
    {"app_proto",           2,     0,   512,   64   },
    {"dw_update",           2,     0,   256,   128  },
#endif

#if TCFG_KEY_TONE_EN
    {"key_tone",            5,     0,   256,   32},
#endif

#if TCFG_MIXER_CYCLIC_TASK_EN
    {"mix_out",             5,     0,   256,   0},
#endif

    {"mic_stream",          5,     0,   768,   128  },
    {"audio_vad",           1,     1,   512,   128 },
#if(TCFG_HOST_AUDIO_ENABLE)
    {"uac_play",            6,     0,   768,   0    },
    {"uac_record",          6,     0,   768,   32   },
#endif

#if (TCFG_BROADCAST_ENABLE || TCFG_CONNECTED_ENABLE)
#if (WIRELESS_MIC_EFFECT_ENABLE && WIRELESS_MIC_TX_EFFECT_ENABLE)
    {"bcsync",              4,     0,   256 + 256,   64 },
#else
    {"bcsync",              5,     0,   256,   64 },
#endif//WIRELESS_MIC_EFFECT_ENABLE && WIRELESS_MIC_TX_EFFECT_ENABLE
#endif

    /* {"mic_stream_agc",      2,     1,   768,   32 }, */
    /* {"audio_dec",           3,     1,   768 + 32,   128  }, */
    /* {"usb_msd",           	1,     0,   512,   128  }, */
    /* {"usb_audio",           5,     0,   256,   256  }, */
    /* {"plnk_dbg",            5,     0,   256,   256  }, */
    /* {"adc_linein",          2,     0,   768,   128  }, */
    /* {"enc_write",           1,     0,   768,   0 	}, */
    /* {"btencry",             1,     0,   512,   128  }, */
    /* {"demo_task1",          2,     1,   768,   32 }, */
    /* {"aec",					2,	   0,   768,   0	}, */
    /* {"aec_dbg",				3,	   0,   128,   128	}, */
    {0, 0},
};



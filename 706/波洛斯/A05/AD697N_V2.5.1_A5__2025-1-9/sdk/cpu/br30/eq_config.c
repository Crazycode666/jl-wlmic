#include "system/includes.h"
#include "media/includes.h"
#include "app_config.h"
#include "app_online_cfg.h"
#include "online_db/online_db_deal.h"



#include "config/config_interface.h"

#include "cfg_tool.h"
#include "audio_effects.h"
#define LOG_TAG     "[EFFECTS]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#include "debug.h"

#ifdef CONFIG_EQ_APP_SEG_ENABLE
#pragma const_seg(	".eq_app_codec_const")
#pragma code_seg (	".eq_app_codec_code" )
#endif
const u8 audio_eq_sdk_name[16] 		= "AC897N";//AD697N
const u8 audio_eq_ver[4] 			= {0, 7, 1, 2};

#define EQ_FILE_NAME 			SDFILE_RES_ROOT_PATH"eq_cfg_hw.bin"


#if TCFG_EQ_FILE_SWITCH_EN
const u8 *eq_file_switch_list[] = {
    (u8 *)SDFILE_RES_ROOT_PATH"eq_cfg_hw.bin",
    (u8 *)SDFILE_RES_ROOT_PATH"eq_cfg_hw1.bin",
    (u8 *)SDFILE_RES_ROOT_PATH"eq_cfg_hw2.bin",
};
#endif




#if (TCFG_EQ_ENABLE != 0)


__attribute__((weak)) u8 get_eq_mode_max(void)
{
    return EQ_MODE_MAX;
}

/*
 *下行的宽频和窄频段数需一致，上行的宽频和窄频段数需要一致
 *表的每一项顺序不可修改
 * */
eq_tool_cfg eq_tool_tab[] = {
    {call_eq_mode, (u8 *)"通话宽频下行EQ", 0x3001, TCFG_CALL_DL_EQ_SECTION, 1, {EQ_ONLINE_CMD_CALL_EQ_SEG, 0}},
    {call_narrow_eq_mode,	(u8 *)"通话窄频下行EQ", 0x3002, TCFG_CALL_DL_EQ_SECTION, 1, {EQ_ONLINE_CMD_CALL_EQ_SEG, 0}},
    {aec_eq_mode, (u8 *)"通话宽频上行EQ", 0x3003, TCFG_CALL_UL_EQ_SECTION,   1, {EQ_ONLINE_CMD_AEC_EQ_SEG,  0}},
    {aec_narrow_eq_mode,	(u8 *)"通话窄频上行EQ", 0x3004, TCFG_CALL_UL_EQ_SECTION,   1, {EQ_ONLINE_CMD_AEC_EQ_SEG,  0}},
    {song_eq_mode, (u8 *)"音乐音效", 	0x3000, EQ_SECTION_MAX, 6, {EQ_ONLINE_CMD_SONG_EQ_SEG, EQ_ONLINE_CMD_SONG_DRC, ONLINE_CMD_MUSIC_VIRTUAL_BASS, ONLINE_CMD_MUSIC_DYNAMIC_EQ_PRO_DET, ONLINE_CMD_MUSIC_MBLIMITER, ONLINE_CMD_MUSIC_POST_EQ}},

};

__attribute__((weak))
s16 get_ci_tx_size()
{
    return 0x30;
}

int eq_init(void)
{
    eq_adjust_parm parm = {0};
#if TCFG_EQ_ONLINE_ENABLE
    parm.online_en = 1;
#endif


#if TCFG_USE_EQ_FILE || EQ_FILE_CP_TO_CUSTOM
    parm.file_en = 1;
#endif

#if TCFG_USER_TWS_ENABLE
    parm.tws = 1;
#endif

#if APP_ONLINE_DEBUG
    parm.app = 1;
#endif
    parm.mode_num = 5;// 一共有多少个模式
    parm.eq_tool_tab = eq_tool_tab;

    EQ_CFG *eq_cfg = eq_cfg_open(&parm);
    if (eq_cfg) {
        u8 ret = -1;

        audio_effects_ultra_init();//默认参数初始化

        if (parm.file_en) {
            ret = eq_file_get_cfg(eq_cfg, (u8 *)EQ_FILE_NAME);//获取文件内配置参数
            if (ret) { //获取文件内配置参数失败
                for (int i = 0; i < parm.mode_num; i++) {
                    log_debug("eq_cfg->seg_num[%d] %d\n", i, eq_cfg->seg_num[i]);
                }
            }
        }

        if (config_audio_eq_online_en) {
            eq_cfg->priv = eq_cfg;
            eq_cfg->send_cmd = ci_send_cmd;
#if APP_ONLINE_DEBUG
            if (eq_cfg->app) {
                app_online_db_register_handle(DB_PKT_TYPE_EQ, eq_app_online_parse);
            }
#endif
        }

#if EQ_FILE_CP_TO_CUSTOM
        cp_eq_file_seg_to_custom_tab();
#endif
    }

    int cpu_section = 0;
    //单声道或者立体声申请的cpu eq mem
    if (EQ_SECTION_MAX > 10) {
        u8 add_num = 0;
        if (hw_crossover_type0) {
            add_num = 4;//drc分频器使用eq硬件加速时、3段4阶使用最大eq段数24段(每段4个eq, 6声道)
        }
        cpu_section = EQ_SECTION_MAX - (int)&EQ_PRIV_SECTION_NUM + add_num;
    }
    audio_eq_init(cpu_section);
    return 0;
}
__initcall(eq_init);


#if TCFG_EQ_FILE_SWITCH_EN
//根据eq_file_switch_list切换到指定的eq文件
void eq_file_set_by_index(u8 index)
{
    if (index >= ARRAY_SIZE(eq_file_switch_list)) {
        printf("err, max index %d\n", ARRAY_SIZE(eq_file_switch_list));
        return;
    }
    EQ_CFG *eq_cfg = get_eq_cfg_hdl();
    if (!eq_cfg) {
        return;
    }
    int ret = eq_file_get_cfg(eq_cfg, eq_file_switch_list[index]);
    printf("eq_file_switch : %d, ret : %d", index, ret);
}

//根据eq_file_switch_list成员个数顺序切换eq文件
void eq_file_switch()
{
    static u8 index = 0;
    index++;
    if (index >= ARRAY_SIZE(eq_file_switch_list)) {
        index = 0;
    }
    eq_file_set_by_index(index);
    audio_effects_ultra_update(song_eq_mode);//更新音乐音效
}

#endif
/*
 *通话下行eq系数表
 * */
#if TCFG_EQ_ENABLE && TCFG_PHONE_EQ_ENABLE
#if EQ_FILE_CP_TO_CUSTOM
struct eq_seg_info phone_eq_tab_normal[] = {
#else
const struct eq_seg_info phone_eq_tab_normal[] = {
#endif
    {0, EQ_IIR_TYPE_HIGH_PASS, 200,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 300,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 400,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
#if TCFG_CALL_DL_EQ_SECTION > 3
    {3, EQ_IIR_TYPE_BAND_PASS, 400,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 400,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 400,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 400,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 400,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 400,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 400,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
#endif
};
#endif

/*
 *通话上行eq系数表
 * */
#if EQ_FILE_CP_TO_CUSTOM
struct eq_seg_info ul_eq_tab_normal[] = {
#else
const struct eq_seg_info ul_eq_tab_normal[] = {
#endif
    {0, EQ_IIR_TYPE_HIGH_PASS, 200,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 300,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 400,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
#if TCFG_CALL_UL_EQ_SECTION > 3
    {3, EQ_IIR_TYPE_BAND_PASS, 400,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 400,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 400,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 400,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 400,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 400,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 400,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
#endif
};




const struct eq_seg_info eq_tab_normal[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,    0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,  0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,  0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000, 0 * (1 << 20), (int)(0.7f * (1 << 24))},
};

const struct eq_seg_info eq_tab_rock[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    -2 * (1 << 20), (int)(0.7f * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    2 * (1 << 20), (int)(0.7f * (1 << 24))},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,    4 * (1 << 20), (int)(0.7f * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   -2 * (1 << 20), (int)(0.7f * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  -2 * (1 << 20), (int)(0.7f * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   4 * (1 << 20), (int)(0.7f * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  4 * (1 << 20), (int)(0.7f * (1 << 24))},
};

const struct eq_seg_info eq_tab_pop[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,     3 * (1 << 20), (int)(0.7f * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     1 * (1 << 20), (int)(0.7f * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   -2 * (1 << 20), (int)(0.7f * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   -4 * (1 << 20), (int)(0.7f * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  -4 * (1 << 20), (int)(0.7f * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  -2 * (1 << 20), (int)(0.7f * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   1 * (1 << 20), (int)(0.7f * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  2 * (1 << 20), (int)(0.7f * (1 << 24))},
};

const struct eq_seg_info eq_tab_classic[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,     0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     8 * (1 << 20), (int)(0.7f * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    8 * (1 << 20), (int)(0.7f * (1 << 24))},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,    4 * (1 << 20), (int)(0.7f * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,    0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   2 * (1 << 20), (int)(0.7f * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  2 * (1 << 20), (int)(0.7f * (1 << 24))},
};

const struct eq_seg_info eq_tab_country[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    -2 * (1 << 20), (int)(0.7f * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,    2 * (1 << 20), (int)(0.7f * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,    2 * (1 << 20), (int)(0.7f * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   4 * (1 << 20), (int)(0.7f * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  4 * (1 << 20), (int)(0.7f * (1 << 24))},
};

const struct eq_seg_info eq_tab_jazz[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,     0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,     0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,    0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,    4 * (1 << 20), (int)(0.7f * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,    4 * (1 << 20), (int)(0.7f * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,   4 * (1 << 20), (int)(0.7f * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,   2 * (1 << 20), (int)(0.7f * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,   3 * (1 << 20), (int)(0.7f * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000,  4 * (1 << 20), (int)(0.7f * (1 << 24))},
};
struct eq_seg_info eq_tab_custom[] = {
    {0, EQ_IIR_TYPE_BAND_PASS, 31,    0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {1, EQ_IIR_TYPE_BAND_PASS, 62,    0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {2, EQ_IIR_TYPE_BAND_PASS, 125,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {3, EQ_IIR_TYPE_BAND_PASS, 250,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {4, EQ_IIR_TYPE_BAND_PASS, 500,   0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {5, EQ_IIR_TYPE_BAND_PASS, 1000,  0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {6, EQ_IIR_TYPE_BAND_PASS, 2000,  0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {7, EQ_IIR_TYPE_BAND_PASS, 4000,  0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {8, EQ_IIR_TYPE_BAND_PASS, 8000,  0 * (1 << 20), (int)(0.7f * (1 << 24))},
    {9, EQ_IIR_TYPE_BAND_PASS, 16000, 0 * (1 << 20), (int)(0.7f * (1 << 24))},
};

// 默认系数表,用户可修改
const struct eq_seg_info *eq_type_tab[EQ_MODE_MAX] = {
    eq_tab_normal, eq_tab_rock, eq_tab_pop, eq_tab_classic, eq_tab_jazz, eq_tab_country, eq_tab_custom
};
// 默认系数表，每个表对应的总增益,用户可修改
float global_gain_tab[EQ_MODE_MAX] = {0, 0, 0, 0, 0, 0, 0};
//默认配置表段数
static const u8 tab_section_num[] = {
    ARRAY_SIZE(eq_tab_normal), ARRAY_SIZE(eq_tab_rock),
    ARRAY_SIZE(eq_tab_pop), ARRAY_SIZE(eq_tab_classic),
    ARRAY_SIZE(eq_tab_jazz), ARRAY_SIZE(eq_tab_country),
    ARRAY_SIZE(eq_tab_custom)
};


/*
 *mode:枚举型EQ_MODE
 *return 返回对应系数表的段数
 * */
u8 eq_get_table_nsection(EQ_MODE mode)
{
    if (mode >= ARRAY_SIZE(eq_type_tab)) {
        log_e("mode error %d\n", mode);
        return 0;
    }
    if (mode >= ARRAY_SIZE(tab_section_num)) {
        log_e("mode error %d\n", mode);
        return 0;
    }

    return tab_section_num[mode];
}

u8 eq_get_table_nsection_inside(EQ_MODE mode)
{
    if (mode >= ARRAY_SIZE(eq_type_tab)) {
        log_e("mode error %d\n", mode);
        return 0;
    }
    if (mode >= ARRAY_SIZE(tab_section_num)) {
        log_e("mode error %d\n", mode);
        return 0;
    }

    return tab_section_num[mode];
}
static u8 eq_mode = 0;
//eq效果表切换
int eq_mode_sw(void)
{
    eq_mode++;
    if (eq_mode >= ARRAY_SIZE(eq_type_tab)) {
        eq_mode = 0;
    }
    struct eq_seg_info *seg = (struct eq_seg_info *)eq_type_tab[eq_mode];

    u8 nsection = ARRAY_SIZE(eq_tab_normal);
    if (nsection > mSECTION_MAX) {
        log_e("ERROR nsection:%d > mSECTION_MAX:%d ", nsection, mSECTION_MAX);
        return -1;//
    }
    //覆盖music eq的系数buf
    music_eq_tab.seg_num = nsection;
    music_eq_tab.global_gain = global_gain_tab[eq_mode];
    memcpy(music_eq_tab.seg, seg, sizeof(struct eq_seg_info)*nsection);

    cur_eq_set_global_gain(AEID_MUSIC_EQ, global_gain_tab[eq_mode]);
    for (int i = 0; i < nsection; i++) {
        cur_eq_set_update(AEID_MUSIC_EQ, &seg[i], nsection);
    }

    return 0;
}
//指定设置某个eq效果表
int eq_mode_set(EQ_MODE mode)
{

    if (mode >= ARRAY_SIZE(eq_type_tab)) {
        log_e("mode err %d\n", mode);
        return -1;//
    }

    eq_mode = mode;
    struct eq_seg_info *seg = (struct eq_seg_info *)eq_type_tab[eq_mode];
    u8 nsection = ARRAY_SIZE(eq_tab_normal);
    if (nsection > mSECTION_MAX) {
        log_e("ERROR nsection:%d > mSECTION_MAX:%d ", nsection, mSECTION_MAX);
        return -1;//
    }
    //覆盖music eq的系数buf
    music_eq_tab.seg_num = nsection;
    music_eq_tab.global_gain = global_gain_tab[eq_mode];
    memcpy(music_eq_tab.seg, seg, sizeof(struct eq_seg_info)*nsection);

    cur_eq_set_global_gain(AEID_MUSIC_EQ, global_gain_tab[eq_mode]);
    for (int i = 0; i < nsection; i++) {
        cur_eq_set_update(AEID_MUSIC_EQ, &seg[i], nsection);
    }

    return 0;
}
//返回某个eq效果模式标号
EQ_MODE eq_mode_get_cur(void)
{
    return eq_mode;
}

/*----------------------------------------------------------------------------*/
/**@brief   设置custom系数表的某一段系数
  @param   seg->index：第几段(从0开始)
  @param   seg->iir_type:滤波器类型(EQ_IIR_TYPE_HIGH_PASS, EQ_IIR_TYPE_LOW_PASS, EQ_IIR_TYPE_BAND_PASS, EQ_IIR_TYPE_HIGH_SHELF,EQ_IIR_TYPE_LOW_SHELF)
  @param   seg->freq:中心截止频率(20~22kHz)
  @param   seg->gain:总增益(-18~18)*(1<<20)
  @param   seg->q : q值（0.3~30）*(1<<24)
  @return
  @note    外部使用
  */
/*----------------------------------------------------------------------------*/
int eq_mode_set_custom_seg(struct eq_seg_info *seg)
{
    struct eq_seg_info *tar_seg = eq_tab_custom;
    u8 index = seg->index;
    if (index > ARRAY_SIZE(eq_tab_custom)) {
        log_e("index %d > max_nsection %d", index, ARRAY_SIZE(eq_tab_custom));
        return -1;
    }
    memcpy(&tar_seg[index], seg, sizeof(struct eq_seg_info));
    return 0;
}
/*----------------------------------------------------------------------------*/
/**@brief    获取某个模式eq表内，某一段eq的信息
   @param
   @param
   @return  返回eq信息
   @note
*/
/*----------------------------------------------------------------------------*/
struct eq_seg_info *eq_mode_get_seg(EQ_MODE mode, u8 index)
{
    if (mode >= ARRAY_SIZE(eq_type_tab)) {
        log_e("mode error %d\n", mode);
        return NULL;
    }
    if (index >= eq_get_table_nsection_inside(mode)) {
        log_e("index error %d\n", index);
        return NULL;
    }

    struct eq_seg_info *seg = (struct eq_seg_info *)eq_type_tab[mode];
    return &seg[index];
}

/*----------------------------------------------------------------------------*/
/**@brief   获取custom系数表的增益、频率
  @param   index:哪一段
  @param   freq:中心截止频率
  @param   gain:增益
  @return
  @note    外部使用
  */
/*----------------------------------------------------------------------------*/
int eq_mode_set_custom_info(u16 index, int freq, int gain)
{
    struct eq_seg_info *seg = eq_mode_get_seg(EQ_MODE_CUSTOM, index);//获取某段eq系数
    if (!seg) {
        return -1;
    }
    seg->freq = freq;//修改freq gain
    seg->gain = gain * (1 << 20);
    eq_mode_set_custom_seg(seg);//重设系数

    eq_mode_set(EQ_MODE_CUSTOM);//设置更新系数
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief   设置用custom系数表一段eq的增益
  @param   index:哪一段
  @param   gain:增益
  @return
  @note    外部使用
  */
/*----------------------------------------------------------------------------*/
int eq_mode_set_custom_param(u16 index, int gain)
{
    struct eq_seg_info *seg = eq_mode_get_seg(EQ_MODE_CUSTOM, index);//获取某段eq系数
    if (!seg) {
        return -1;
    }
    seg->gain = gain * (1 << 20);
    eq_mode_set_custom_seg(seg);//重设系数

    eq_mode_set(EQ_MODE_CUSTOM);//设置更新系数
    return 0;
}


s8 eq_mode_get_gain(EQ_MODE mode, u16 index)
{
    struct eq_seg_info *seg = eq_mode_get_seg(mode, index);
    if (!seg) {
        return 0;
    }
    return seg->gain >> 20;
}
/*----------------------------------------------------------------------------*/
/**@brief   获取某eq系数表一段eq的中心截止频率
  @param   mode:EQ_MODE_NORMAL, EQ_MODE_ROCK,EQ_MODE_POP,EQ_MODE_CLASSIC,EQ_MODE_JAZZ,EQ_MODE_COUNTRY, EQ_MODE_CUSTOM
  @param   index:哪一段
  @return  中心截止频率
  @note    外部使用
  */
/*----------------------------------------------------------------------------*/
int eq_mode_get_freq(EQ_MODE mode, u16 index)
{
    struct eq_seg_info *seg = eq_mode_get_seg(mode, index);
    if (!seg) {
        return 0;
    }
    return seg->freq;
}
/*----------------------------------------------------------------------------*/
/**@brief   设置用custom系数表一段eq的增益
  @param   index:哪一段
  @param   gain:增益
  @return
  @note    外部使用
  */
/*----------------------------------------------------------------------------*/
void set_global_gain(EQ_MODE mode, float global_gain)
{
    global_gain_tab[mode] = global_gain;
    cur_eq_set_global_gain(AEID_MUSIC_EQ, global_gain);
}

#endif

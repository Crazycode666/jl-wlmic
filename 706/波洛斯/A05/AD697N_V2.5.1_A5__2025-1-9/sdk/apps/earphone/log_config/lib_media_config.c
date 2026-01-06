/*********************************************************************************************
    *   Filename        : lib_driver_config.c

    *   Description     : Optimized Code & RAM (编译优化配置)

    *   Author          : Bingquan

    *   Email           : caibingquan@zh-jieli.com

    *   Last modifiled  : 2019-03-18 14:58

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
*********************************************************************************************/
#include "app_config.h"
#include "system/includes.h"
#include "media/includes.h"

#ifndef TCFG_AUDIO_AAC_RAM_MALLOC_ENABLE
#define TCFG_AUDIO_AAC_RAM_MALLOC_ENABLE	1
#endif/*TCFG_AUDIO_AAC_RAM_MALLOC_ENABLE*/

#if CONFIG_MEDIA_LIB_USE_MALLOC
const int config_mp3_dec_use_malloc     = 1;
const int config_mp3pick_dec_use_malloc = 1;
const int config_wma_dec_use_malloc     = 1;
const int config_wmapick_dec_use_malloc = 1;
const int config_m4a_dec_use_malloc     = 1;
const int config_m4apick_dec_use_malloc = 1;
const int config_wav_dec_use_malloc     = 1;
const int config_alac_dec_use_malloc    = 1;
const int config_dts_dec_use_malloc     = 1;
const int config_amr_dec_use_malloc     = 1;
const int config_flac_dec_use_malloc    = 1;
const int config_ape_dec_use_malloc     = 1;
const int config_aac_dec_use_malloc     = TCFG_AUDIO_AAC_RAM_MALLOC_ENABLE;
const int config_aptx_dec_use_malloc    = 1;
const int config_midi_dec_use_malloc    = 1;
#else
const int config_mp3_dec_use_malloc     = 0;
const int config_mp3pick_dec_use_malloc = 0;
const int config_wma_dec_use_malloc     = 0;
const int config_wmapick_dec_use_malloc = 0;
const int config_m4a_dec_use_malloc     = 0;
const int config_m4apick_dec_use_malloc = 0;
const int config_wav_dec_use_malloc     = 0;
const int config_alac_dec_use_malloc    = 0;
const int config_dts_dec_use_malloc     = 0;
const int config_amr_dec_use_malloc     = 0;
const int config_flac_dec_use_malloc    = 0;
const int config_ape_dec_use_malloc     = 0;
const int config_aac_dec_use_malloc     = 0;
const int config_aptx_dec_use_malloc    = 0;
const int config_midi_dec_use_malloc    = 0;
#endif

const int config_audio_dac_noisefloor_optimize_enable = BIT(1);


const int config_audio_eq_en = 0
#if TCFG_EQ_ENABLE
                               | EQ_EN         //eq使能
#if TCFG_EQ_ONLINE_ENABLE
                               | EQ_ONLINE_EN //在线调试使能
#endif/*TCFG_AUDIO_OUT_EQ_ENABLE*/
#if TCFG_USE_EQ_FILE
                               | EQ_FILE_EN   //使用eq_cfg_hw.bin文件效果
#if TCFG_EQ_FILE_SWITCH_EN
                               | EQ_FILE_SWITCH_EN   //使能eq_cfg_hw.bin多文件切换，对应旧版config_audio_eq_file_sw_en
#endif
#endif/*TCFG_USE_EQ_FILE*/
#if TCFG_AUDIO_OUT_EQ_ENABLE
                               | EQ_HIGH_BASS_EN //高低音接口使能
                               | EQ_HIGH_BASS_FADE_EN  //高低音接口数据更新使用淡入淡出,配合config_audio_eq_fade_step步进使用
#endif/*TCFG_AUDIO_OUT_EQ_ENABLE*/
#if (RCSP_ADV_EN)&&(JL_EARPHONE_APP_EN)&&(TCFG_DRC_ENABLE == 0)
                               /* |EQ_FILTER_COEFF_LIMITER_ZERO_EN */
#endif
#ifndef CONFIG_SOUNDBOX_FLASH_256K
                               | EQ_HW_UPDATE_COEFF_ONLY_EN
                               | EQ_HW_LR_ALONE
#endif/* CONFIG_SOUNDBOX_FLASH_256K */
#if defined(EQ_CORE_V1) && TCFG_DRC_ENABLE
                               | EQ_SUPPORT_MULIT_CHANNEL_EN //eq是否支持多声道（3~8） 打开:支持  否则：仅支持1~2声道*/
                               | EQ_HW_CROSSOVER_TYPE0_EN //硬件分频器用序列进序列出
                               //|EQ_HW_CROSSOVER_TYPE1_EN  //硬件分频器使用块出方式，会增加mem(该方式仅支持单声道处理)
#endif/*(EQ_CORE_V1) && TCFG_DRC_ENABLE*/
#if defined(TCFG_EQ_DIVIDE_ENABLE) && TCFG_EQ_DIVIDE_ENABLE
                               | EQ_LR_DIVIDE_EN
#endif/*TCFG_EQ_DIVIDE_ENABLE*/

#if defined(EQ_LITE_CODE) && EQ_LITE_CODE
                               | EQ_LITE_VER_EN //不支持异步，不支持默认效果切换接口，仅支持文件解析
#if TCFG_DRC_ENABLE
                               | EQ_SUPPORT_32BIT_SYNC_EN //32bit同步方式eq使能,eq_lite_en后，该宏需使能
#endif/*TCFG_DRC_ENABLE*/
#endif/* EQ_LITE_CODE */
                               | EQ_FILTER_COEFF_FADE_EN//默认系数切换更新时使用淡入淡出

#if defined(EQ_CORE_V1)
                               | EQ_SUPPORT_OLD_VER_EN //AC700N,或上BIT(1)支持版本0.7.1.0,0.7.1.1
#endif/*EQ_CORE_V1*/
#endif/* TCFG_EQ_ENABLE */
                               | 0; //end
const int AUDIO_EQ_CLEAR_MEM_BY_MUTE_TIME_MS = config_audio_dac_noisefloor_optimize_enable ? (10) : 0; //连续多长时间静音就清除EQ MEM
const int AUDIO_EQ_CLEAR_MEM_BY_MUTE_LIMIT = 0; //静音判断阀值
#if TCFG_DRC_ENABLE
const int config_audio_drc_en = 1
#if TCFG_AUDIO_MDRC_ENABLE
                                | BIT(5)//bit5使能wdrc
#endif
                                ;
#else
const int config_audio_drc_en = 0;
#endif



const int WAV_MAX_BITRATEV = (48 * 2 * 32); // wav最大支持比特率，单位kbps


// 解码一次输出点数,建议范围32到900,例如128代表128对点
// 超过128时，解码需要使用malloc，如config_wav_dec_use_malloc=1
const int WAV_DECODER_PCM_POINTS = 128;

// output超过128时，如果不使用malloc，需要增大对应buf
// 可以看打印中解码器需要的大小，一般输出每增加1长度增加4个字节
int wav_mem_ext[(1336  + 3) / 4] SEC(.wav_mem); //超过128要增加这个数组的大小


const int MP3_OUTPUT_LEN = 1;
const int MP3_TGF_TWS_EN = 1;  //tws解码使能
const int MP3_TGF_POSPLAY_EN = 1;//顶点播放、获取ms级别时间 接口使能
const int MP3_TGF_AB_EN = 1;   //AB点功能使能
const int MP3_TGF_FASTMO = 0;  //快速解码模式使能（默认关闭，之前给一个sdk单独加的，配置是否解高频，接双声道等）
const int MP3_SEARCH_MAX = 200; //本地解码设成200， 网络解码可以设成3

#ifdef CONFIG_ANC_30C_ENABLE
const char config_audio_30c_en = 1;
#else
const char config_audio_30c_en = 0;
#endif

#if AUDIO_SURROUND_CONFIG
const int const_surround_en = BIT(2) | 1;//或上BIT(2)使能新的环绕音效
#else
const int const_surround_en = 0;
#endif

#if AUDIO_VBASS_CONFIG
const int const_vbass_en = 1;
#else
const int const_vbass_en = 0;
#endif

//wts解码支持采样率可选择，可以同时打开也可以单独打开
const  int  silk_fsN_enable = 1;   //支持8-12k采样率
const  int  silk_fsW_enable = 1;  //支持16-24k采样率
const  int  WTGV2_STACK2BUF = 0;  //等于1时解码buf会加大760，栈会减小

const int const_eq_debug = 0;

const int vc_pitchshift_fastmode_flag        = 1; //变声快速模式使能
const int  vc_pitchshift_downmode_flag = 0;  //变声下采样处理使能
const int  VC_KINDO_TVM = 1;       //含义为EFFECT_VOICECHANGE_KIN0是否另一种算法 : 0表示跟原来一样，1表示用另一种

#if TCFG_LOWEST_POWER_EN
const int LPC_JUST_FADE = 1; //播歌PLC仅淡入淡出配置, 0 - 补包运算(Ram 3660bytes, Code 1268bytes)，1 - 仅淡出淡入(Ram 688bytes, Code 500bytes)
#else
const int LPC_JUST_FADE = 0; //播歌PLC仅淡入淡出配置, 0 - 补包运算(Ram 3660bytes, Code 1268bytes)，1 - 仅淡出淡入(Ram 688bytes, Code 500bytes)
#endif
const int APLC_MOV_STAKLEN = 1024; //影响plc申请的buf大小跟速度，这个值越大，申请的buf越多，速度也越快。 增加的buf大小是  APLC_MOV_STAKLEN *类型(16bit是 sizeof(short), 32bit 是sizeof(int))
const char config_audio_mixer_ch_highlight_enable = 0; //混音器声音突出功能使能

const int audio_dec_app_mix_en = 1;
const int audio_tws_auto_channel = 1;

const unsigned char config_audio_dac_underrun_protect = 1;
#ifdef SBC_CUSTOM_DECODER_BUF_SIZE
const short config_sbc_decoder_buf_size = 512;
#endif


/*第二版串口写卡工具，支持写不定长度数据*/
u8 const_audio_pcm_debug_v2_en = 0;

/**
 * @brief Log (Verbose/Info/Debug/Warn/Error)
 */
/*-----------------------------------------------------------*/
const char log_tag_const_v_EQ_CFG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_EQ_CFG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_EQ_CFG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_EQ_CFG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_EQ_CFG AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_EQ_CFG_TOOL AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_i_EQ_CFG_TOOL AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_d_EQ_CFG_TOOL AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(FALSE);
const char log_tag_const_w_EQ_CFG_TOOL AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_EQ_CFG_TOOL AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_EQ_APPLY AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_i_EQ_APPLY AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_EQ_APPLY AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_w_EQ_APPLY AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_EQ_APPLY AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_i_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_w_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_APP_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_i_APP_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_APP_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_w_APP_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_APP_DRC AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);


const char log_tag_const_v_EQ AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_i_EQ AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_EQ AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_w_EQ AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_EQ AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);

const char log_tag_const_v_AUDIO_DECODER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_c_AUDIO_DECODER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_i_AUDIO_DECODER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_AUDIO_DECODER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);
const char log_tag_const_e_AUDIO_DECODER AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);


const char log_tag_const_v_EFFECTS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_i_EFFECTS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_d_EFFECTS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_w_EFFECTS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(0);
const char log_tag_const_e_EFFECTS AT(.LOG_TAG_CONST) = CONFIG_DEBUG_LIB(TRUE);



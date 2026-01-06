/*
 ****************************************************************
 *							AUDIO DMS(DualMic System)
 * File  : audio_aec_dms.c
 * By    :
 * Notes : AEC回音消除 + 双mic降噪(ENC)
 *
 ****************************************************************
 */
#include "aec_user.h"
#include "system/includes.h"
#include "media/includes.h"
#include "application/eq_config.h"
#include "application/audio_pitch.h"
#include "circular_buf.h"
#include "audio_aec_online.h"
#include "audio_dms_tool.h"
#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#endif/*TCFG_USER_TWS_ENABLE*/
#include "amplitude_statistic.h"
#include "audio_gain_process.h"
#include "audio_config.h"
#include "app_main.h"
#include "effects_adj.h"
#if TCFG_AUDIO_CVP_SYNC
#include "audio_cvp_sync.h"
#endif/*TCFG_AUDIO_CVP_SYNC*/
#include "cvp/cvp_common.h"

#if !defined(TCFG_CVP_DEVELOP_ENABLE) || (TCFG_CVP_DEVELOP_ENABLE == 0)

//*********************************************************************************//
//                          DMS_GLOBAL_V200配置                                    //
//*********************************************************************************//
/*
 *DMS版本配置
 *DMS_GLOBAL_V100:第一版双麦算法
 *DMS_GLOBAL_V200:第二版双麦算法，更少的ram和mips
 */
#if (TCFG_AUDIO_DMS_GLOBAL_VERSION == DMS_GLOBAL_V200)
const u8 CONST_DMS_GLOBAL_VERSION = DMS_GLOBAL_V200;
#else
const u8 CONST_DMS_GLOBAL_VERSION = DMS_GLOBAL_V100;
#endif
#if (TCFG_AUDIO_CVP_NS_MODE == CVP_ANS_MODE) && (TCFG_AUDIO_DMS_GLOBAL_VERSION == DMS_GLOBAL_V200)
/*双麦耳机使用ans时,需要配置使用DMS_GLOBAL_V100*/
#error "CVP_ANS_MODE and DMS_GLOBAL_V200 connot exits together"
#endif

/*
 * 非线性压制模式选择
 * JLSP_NLP_MODE1: 模式1为单独的NLP回声抑制，回声压制会偏过，该模式下NLP模块可以单独开启
 * JLSP_NLP_MODE2: 模式2下回声信号会先经过AEC线性压制，然后进行NLP非线性压制
 *                 此模式NLP不能单独打开需要同时打开AEC,使用AEC模块压制不够时，建议开启该模式
 */
const u8 CONST_JLSP_NLP_MODE = JLSP_NLP_MODE1;

/*
 * 风噪降噪模式选择
 * JLSP_WD_MODE1: 模式1为常规的降风噪模式，风噪残余会偏大些
 * JLSP_WD_MODE2: 模式2为神经网络降风噪，风噪抑制会比较干净，但是会需要多消耗31kb的flash
 */
const u8 CONST_JLSP_WD_MODE = JLSP_WD_MODE1;

//**************************DMS_GLOBAL_V200配置 end*******************************//

#if TCFG_AUDIO_DUAL_MIC_ENABLE
#include "commproc_dms.h"

#define LOG_TAG_CONST       AEC_USER
#define LOG_TAG             "[AEC_USER]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"
#include "audio_aec_debug.c"

/*AAC内存overlay的时候，AEC用户空间内存也overlay，提高利用率*/
#if TCFG_AUDIO_AAC_RAM_MALLOC_ENABLE
#define AEC_USER_MALLOC_ENABLE	1
#else
#define AEC_USER_MALLOC_ENABLE	1
#endif/*TCFG_AUDIO_AAC_RAM_MALLOC_ENABLE*/

/*AEC_TOGGLE:AEC模块使能开关，Disable则数据完全不经过处理，AEC模块不占用资源*/
#define AEC_TOGGLE				1	/*回音消除模块开关*/
/*mic去直流滤波eq*/
#define AEC_DCCS_EN				TCFG_AEC_DCCS_EQ_ENABLE
/*mic普通eq*/
#define AEC_UL_EQ_EN			TCFG_AEC_UL_EQ_ENABLE

extern const int  VC_KINDO_TVM;
extern struct adc_platform_data adc_data;
extern struct audio_dac_hdl dac_hdl;

#ifdef CONFIG_FPGA_ENABLE
const u8 CONST_AEC_ENABLE = 1;
#else
const u8 CONST_AEC_ENABLE = 1;
#endif/*CONFIG_FPGA_ENABLE*/

/*
 * 串口写卡数据导出
 * 0 : 关闭数据导出
 * 1 : 导出 mic0 mic1 far
 * 2 : 导出 mic0 mic1 out
 * 3 : 导出 mic0 far  out
 */
#ifdef AUDIO_PCM_DEBUG
const u8 CONST_AEC_EXPORT = 2;/*1:far 2:out*/
#else
const u8 CONST_AEC_EXPORT = 0;
#endif

//*********************************************************************************//
//                                预处理配置(Pre-process Config)               	   //
//*********************************************************************************//
/*预增益配置*/
#define CVP_PRE_GAIN_ENABLE				0	//算法处理前预加数字增益放大

/*响度指示器*/
#define CVP_LOUDNESS_TRACE_ENABLE		0	//跟踪获取当前mic输入幅值

//*********************************************************************************//
//                                调试配置(Debug Config)                           //
//*********************************************************************************//
/*使能即可跟踪通话过程的内存情况*/
#define CVP_MEM_TRACE_ENABLE	0

/*
 *延时估计使能
 *点烟器需要做延时估计
 *其他的暂时不需要做
 */
const u8 CONST_AEC_DLY_EST = 0;

#if TCFG_AEC_SIMPLEX
const u8 CONST_AEC_SIMPLEX = 1;
#else
const u8 CONST_AEC_SIMPLEX = 0;
#endif/*TCFG_AEC_SIMPLEX*/

/*
 *ANS版本配置
 *DMS_V100:传统降噪
 *DMS_V200:AI降噪，需要更多的ram和mips
 **/
#if (TCFG_AUDIO_CVP_NS_MODE == CVP_ANS_MODE)
const u8 CONST_DMS_VERSION = DMS_V100;
#else
const u8 CONST_DMS_VERSION = DMS_V200;
#endif/*TCFG_AUDIO_CVP_NS_MODE*/

//*********************************************************************************//
//                                DNS配置                                          //
//*********************************************************************************//
/* SNR估计,可以实现场景适配 */
const u8 CONST_DNS_SNR_EST = 0;//注意：双麦没有该功能

/* DNS后处理 */
const u8 CONST_DNS_POST_ENHANCE = 0;

/*
 *风噪自适应GainFloor使能控制
 *0:关闭, 1:开启
 */
const u8 CONST_DMS_WNC = 0;

//*******************************DNS配置 end**************************************//

//*********************************************************************************//
//                        麦克风异常检测配置                                       //
//*********************************************************************************//
/*麦克风异常检测，双mic切单mic*/
const u8 CONST_DMS_MALFUNCTION_DETECT = 0;

/*
 * 是否设置通话mic默认状态
 * 需要重回定义audio_dms_get_malfunc_state(void)
 */
const u8 CONST_DMS_SET_MFDT_STATE = 1;

//*************************麦克风异常检测配置 end**********************************//

/*
 * 风噪下ENC是否保留人声
 *0:加强对风噪的压制，1:保留更多的人声
 */
const u8 CONST_DMS_WINDREPLACE = 1;

/*
 * 副麦回音消除使能控制
 * 0 : 不对副mic做AEC
 * 1 : 对副mic做AEC(需要打开AEC模块)
 */
const u8 CONST_DMS_REF_MIC_AEC_ENABLE = 0;

/*数据输出开头丢掉的数据包数*/
#define AEC_OUT_DUMP_PACKET		15
/*数据输出开头丢掉的数据包数*/
#define AEC_IN_DUMP_PACKET		1

/*复用lmp rx buf(一般通话的时候复用)
 *rx_buf概率产生碎片，导致alloc失败，因此默认配0
 */
#define MALLOC_MULTIPLEX_EN		0
extern void *lmp_malloc(int);
extern void lmp_free(void *);
void *zalloc_mux(int size)
{
#if MALLOC_MULTIPLEX_EN
    void *p = NULL;
    do {
        p = lmp_malloc(size);
        if (p) {
            break;
        }
        printf("aec_malloc wait...\n");
        os_time_dly(2);
    } while (1);
    if (p) {
        memset(p, 0, size);
    }
    printf("[malloc_mux]p = 0x%x,size = %d\n", p, size);
    return p;
#else
    return zalloc(size);
#endif
}

void free_mux(void *p)
{
#if MALLOC_MULTIPLEX_EN
    printf("[free_mux]p = 0x%x\n", p);
    lmp_free(p);
#else
    free(p);
#endif
}

struct audio_aec_hdl {
    u8 start;				//aec模块状态
    u8 inbuf_clear_cnt;		//aec输入数据丢掉
    u8 output_fade_in;		//aec输出淡入使能
    u8 output_fade_in_gain;	//aec输出淡入增益
    u8 EnableBit;			//aec使能模块
    u8 input_clear;			//清0输入数据标志
    u16 dump_packet;		//前面如果有杂音，丢掉几包
    u8 output_buf[1000];	//aec数据输出缓存
    cbuffer_t output_cbuf;
#if AEC_UL_EQ_EN
    struct audio_eq *ul_eq;
#endif/*AEC_UL_EQ_EN*/
#if AEC_DCCS_EN
    struct audio_eq *dccs_eq;
#endif/*AEC_DCCS_EN*/
    struct dms_attr attr;	//aec模块参数属性
#if AEC_PITCHSHIFTER_CONFIG
    s_pitch_hdl *pitch ; //变声
    u8 pitch_mode_L; //记录变声模式
    u8 pitch_mode_R; //记录变声模式
#endif/* AEC_PITCHSHIFTER_CONFIG */
};
#if AEC_USER_MALLOC_ENABLE
struct audio_aec_hdl *aec_hdl = NULL;
#else
struct audio_aec_hdl aec_handle AT(.aec_mux);
struct audio_aec_hdl *aec_hdl = &aec_handle;
#endif/*AEC_USER_MALLOC_ENABLE*/

#if AEC_DCCS_EN
//一段高通滤波器 可调中心截止频率、带宽
// 默认   /*freq:100*/   /*quality:0.7f*/
//
struct eq_seg_info dccs_eq_tab_8k[] = {
    {0, EQ_IIR_TYPE_HIGH_PASS, 100,   0 << 20, (int)(0.7f * (1 << 24))},
};
struct eq_seg_info dccs_eq_tab_16k[] = {
    {0, EQ_IIR_TYPE_HIGH_PASS, 100,   0 << 20, (int)(0.7f * (1 << 24))},
};
static  int dccs_tab[5];
int aec_dccs_eq_filter(void *eq, int sr, struct audio_eq_filter_info *info)
{
    if (!sr) {
        sr = 16000;
    }
    u8 nsection = ARRAY_SIZE(dccs_eq_tab_8k);
    local_irq_disable();
    if (sr == 16000) {
        for (int i = 0; i < nsection; i++) {
            eq_seg_design((struct eq_seg_info *)&dccs_eq_tab_16k[i], sr, &dccs_tab[5 * i]);
        }
    } else {
        for (int i = 0; i < nsection; i++) {
            eq_seg_design((struct eq_seg_info *)&dccs_eq_tab_8k[i], sr, &dccs_tab[5 * i]);
        }
    }
    local_irq_enable();

    info->L_coeff = (int *)dccs_tab;
    info->R_coeff = (int *)dccs_tab;
    info->L_gain = 0;
    info->R_gain = 0;
    info->nsection = nsection;
    return 0;
}
#endif/*AEC_DCCS_EN*/


/*
*********************************************************************
*                  Audio AEC Process_Probe
* Description: AEC模块数据前处理回调(预处理)
* Arguments  : mic0 mic0数据地址
*			   mic1	mic1数据地址
*			   mic2	mic2数据地址
*			   ref	参考数据地址
*			   len	数据长度(Unit:byte)
* Return	 : 0 成功 其他 失败
* Note(s)    : 在源数据经过CVP模块前，可以增加自定义处理
*********************************************************************
*/
static LOUDNESS_M_STRUCT mic_loudness;
AT(.audio_cvp.text.cache.L2.cvp)
static int audio_aec_probe(short *mic0, short *mic1, short *mic2, short *ref, u16 len)
{
#if TCFG_AUDIO_MIC_ARRAY_TRIM_ENABLE
    if (app_var.audio_mic_array_trim_en) {
        if (app_var.audio_mic_array_diff_cmp > 1.0f) {
            float mic0_gain = 1.0 / app_var.audio_mic_array_diff_cmp;
            GainProcess_16Bit(mic0, mic0, mic0_gain, 1, 1, 1, len >> 1);
        } else {
            float mic1_gain = app_var.audio_mic_array_diff_cmp;
            GainProcess_16Bit(mic1, mic1, mic1_gain, 1, 1, 1, len >> 1);
        }
    }
#endif

#if CVP_PRE_GAIN_ENABLE
    GainProcess_16Bit(mic0, mic0, 12.f, 1, 1, 1, len >> 1);
    GainProcess_16Bit(mic1, mic1, 12.f, 1, 1, 1, len >> 1);
#endif/*CVP_PRE_GAIN_ENABLE*/

#if CVP_LOUDNESS_TRACE_ENABLE
    loudness_meter_short(&mic_loudness, mic0, len >> 1);
#endif/*CVP_LOUDNESS_TRACE_ENABLE*/

#if AEC_DCCS_EN
    if (aec_hdl->dccs_eq) {
        audio_eq_run(aec_hdl->dccs_eq, mic0, len);
    }
#endif/*AEC_DCCS_EN*/
    return 0;
}

/*
*********************************************************************
*                  Audio AEC Process_Post
* Description: AEC模块数据后处理回调
* Arguments  : data 数据地址
*			   len	数据长度
* Return	 : 0 成功 其他 失败
* Note(s)    : 在数据处理完毕，可以增加自定义后处理
*********************************************************************
*/
AT(.audio_cvp.text.cache.L2.cvp)
static int audio_aec_post(s16 *data, u16 len)
{
#if AEC_UL_EQ_EN
    if (aec_hdl->ul_eq) {
        audio_eq_run(aec_hdl->ul_eq, data, len);
    }
#endif/*AEC_UL_EQ_EN*/
#if AEC_PITCHSHIFTER_CONFIG
    if (aec_hdl && aec_hdl->pitch) {
        pitch_run(aec_hdl->pitch, data, data, len, 1);
    }
#endif/* AEC_PITCHSHIFTER_CONFIG */
    return 0;
}

static int audio_aec_update(u8 EnableBit)
{
    y_printf("aec_update:%d,%d", aec_hdl->attr.wideband, EnableBit);
    if (CONST_DMS_VERSION == DMS_V200) {
        clk_set("sys", CONFIG_BT_CALL_DNS_HZ);
        return 0;
    }
    if (aec_hdl->attr.wideband) {
        if ((EnableBit & AEC_EN) && (EnableBit & NLP_EN)) {
            clk_set("sys", BT_CALL_16k_ADVANCE_HZ);
        } else {
            clk_set("sys", BT_CALL_16k_HZ);
        }
    } else {
        if ((EnableBit & AEC_EN) && (EnableBit & NLP_EN)) {
            clk_set("sys", BT_CALL_ADVANCE_HZ);
        } else {
            clk_set("sys", BT_CALL_HZ);
        }
    }
    return 0;
}

/*跟踪系统内存使用情况:physics memory size xxxx bytes*/
static void sys_memory_trace(void)
{
    static int cnt = 0;
    if (cnt++ > 200) {
        cnt = 0;
        mem_stats();
    }
}

/*延时估计信息输出*/
static int last_DlyEst = -10000;
#if 0
void aec_estimate_dump(int DlyEst)
{
    if (DlyEst != last_DlyEst) {
        printf("DlyEst:%d\n", DlyEst);
        last_DlyEst = DlyEst;
    }
}
#endif

extern void esco_enc_resume(void);
int audio_aec_sync_buffer_set(s16 *data, int len)
{
    u16 wlen = cbuf_write(&aec_hdl->output_cbuf, data, len);
    /* printf("wlen:%d-%d\n",len,aec_hdl->output_cbuf.data_len); */
    esco_enc_resume();
    return wlen;
}
/*
*********************************************************************
*                  Audio AEC Output Handle
* Description: AEC模块数据输出回调
* Arguments  : data 输出数据地址
*			   len	输出数据长度
* Return	 : 数据输出消耗长度
* Note(s)    : None.
*********************************************************************
*/
AT(.audio_cvp.text.cache.L2.cvp)
static int audio_aec_output(s16 *data, u16 len)
{
#if TCFG_KWS_VOICE_RECOGNITION_ENABLE
    //Voice Recognition get mic data here
    extern void kws_aec_data_output(void *priv, s16 * data, int len);
    kws_aec_data_output(NULL, data, len);

#endif/*TCFG_KWS_VOICE_RECOGNITION_ENABLE*/

#if CVP_MEM_TRACE_ENABLE
    sys_memory_trace();
#endif/*CVP_MEM_TRACE_ENABLE*/

    if (aec_hdl->dump_packet) {
        aec_hdl->dump_packet--;
        memset(data, 0, len);
    } else  {
        if (aec_hdl->output_fade_in) {
            s32 tmp_data;
            //printf("fade:%d\n",aec_hdl->output_fade_in_gain);
            for (int i = 0; i < len / 2; i++) {
                tmp_data = data[i];
                data[i] = tmp_data * aec_hdl->output_fade_in_gain >> 7;
            }
            aec_hdl->output_fade_in_gain += 12;
            if (aec_hdl->output_fade_in_gain >= 128) {
                aec_hdl->output_fade_in = 0;
            }
        }
    }

#if TCFG_AUDIO_CVP_SYNC
    audio_cvp_sync_run(data, len);
    return len;
#endif/*TCFG_AUDIO_CVP_SYNC*/

    u16 wlen = cbuf_write(&aec_hdl->output_cbuf, data, len);
    //printf("wlen:%d-%d\n",len,aec_hdl.output_cbuf.data_len);
    esco_enc_resume();
#if 1
    static u32 aec_output_max = 0;
    if (aec_output_max < aec_hdl->output_cbuf.data_len) {
        aec_output_max = aec_hdl->output_cbuf.data_len;
        y_printf("o_max:%d", aec_output_max);
    }
#endif
    if (wlen != len) {
        putchar('f');
    }
    return wlen;
}

/*
*********************************************************************
*                  Audio AEC Output Read
* Description: 读取aec模块的输出数据
* Arguments  : buf  读取数据存放地址
*			   len	读取数据长度
* Return	 : 数据读取长度
* Note(s)    : None.
*********************************************************************
*/
int audio_aec_output_read(s16 *buf, u16 len)
{
    //printf("rlen:%d-%d\n",len,aec_hdl.output_cbuf.data_len);
    local_irq_disable();
    if (!aec_hdl || !aec_hdl->start) {
        printf("audio_aec close now");
        local_irq_enable();
        return -EINVAL;
    }
    u16 rlen = cbuf_read(&aec_hdl->output_cbuf, buf, len);
    if (rlen == 0) {
        //putchar('N');
    }
    local_irq_enable();
    return rlen;
}

/*
*********************************************************************
*                  Audio AEC Parameters
* Description: AEC模块配置参数
* Arguments  : p	参数指针
* Return	 : None.
* Note(s)    : 读取配置文件成功，则使用配置文件的参数配置，否则使用默
*			   认参数配置
*********************************************************************
*/
static void audio_aec_param_init(struct dms_attr *p)
{
    int ret = 0;
    AEC_DMS_CONFIG cfg;
#if TCFG_AEC_TOOL_ONLINE_ENABLE
    ret = aec_cfg_online_update_fill(&cfg, sizeof(AEC_DMS_CONFIG));
#endif/*TCFG_AEC_TOOL_ONLINE_ENABLE*/
    if (ret == 0) {
#if (TCFG_AUDIO_CVP_NS_MODE == CVP_ANS_MODE)
        ret = syscfg_read(CFG_DMS_ID, &cfg, sizeof(AEC_DMS_CONFIG));
#else
        ret = syscfg_read(CFG_DMS_DNS_ID, &cfg, sizeof(AEC_DMS_CONFIG));
#endif/*TCFG_AUDIO_CVP_NS_MODE*/
    }
    log_info("CVP_DMS_NS_MODE = %d\n", TCFG_AUDIO_CVP_NS_MODE);
    if (ret == sizeof(AEC_DMS_CONFIG)) {
        log_info("read dms_param ok\n");
        p->EnableBit = cfg.enable_module;
        p->ul_eq_en = cfg.ul_eq_en;
        p->AGC_NDT_fade_in_step = cfg.ndt_fade_in;
        p->AGC_NDT_fade_out_step = cfg.ndt_fade_out;
        p->AGC_DT_fade_in_step = cfg.dt_fade_in;
        p->AGC_DT_fade_out_step = cfg.dt_fade_out;
        p->AGC_NDT_max_gain = cfg.ndt_max_gain;
        p->AGC_NDT_min_gain = cfg.ndt_min_gain;
        p->AGC_NDT_speech_thr = cfg.ndt_speech_thr;
        p->AGC_DT_max_gain = cfg.dt_max_gain;
        p->AGC_DT_min_gain = cfg.dt_min_gain;
        p->AGC_DT_speech_thr = cfg.dt_speech_thr;
        p->AGC_echo_present_thr = cfg.echo_present_thr;

        p->aec_process_maxfrequency = cfg.aec_process_maxfrequency;
        p->aec_process_minfrequency = cfg.aec_process_minfrequency;
        p->af_length = cfg.af_length;

        p->nlp_process_maxfrequency = cfg.nlp_process_maxfrequency;
        p->nlp_process_minfrequency = cfg.nlp_process_minfrequency;
        p->overdrive = cfg.overdrive;

        p->aggressfactor = cfg.aggressfactor;
        p->minsuppress = cfg.minsuppress;
        p->init_noise_lvl = cfg.init_noise_lvl;
        p->DNS_highGain = 2.0f;
        p->DNS_rbRate = 0.5f;

        p->wn_detect_time = 0.32f;
        p->wn_detect_time_ratio_thr = 0.8f;
        p->wn_detect_thr = 0.5f;
        p->wn_minsuppress = 0.5f;

        p->enc_process_maxfreq = cfg.enc_process_maxfreq;
        p->enc_process_minfreq = cfg.enc_process_minfreq;
        p->sir_maxfreq = cfg.sir_maxfreq;
        p->mic_distance = cfg.mic_distance;
        p->target_signal_degradation = cfg.target_signal_degradation;
        p->enc_aggressfactor = cfg.enc_aggressfactor;
        p->enc_minsuppress = cfg.enc_minsuppress;

        p->global_minsuppress = cfg.global_minsuppress;
    } else {
        p->EnableBit = NLP_EN | ANS_EN | ENC_EN | AGC_EN;
        p->ul_eq_en = 1;
        p->AGC_NDT_fade_in_step = 1.3f;
        p->AGC_NDT_fade_out_step = 0.9f;
        p->AGC_DT_fade_in_step = 1.3f;
        p->AGC_DT_fade_out_step = 0.9f;
        p->AGC_NDT_max_gain = 12.f;
        p->AGC_NDT_min_gain = 0.f;
        p->AGC_NDT_speech_thr = -50.f;
        p->AGC_DT_max_gain = 12.f;
        p->AGC_DT_min_gain = 0.f;
        p->AGC_DT_speech_thr = -40.f;
        p->AGC_echo_present_thr = -70.f;

        p->aec_process_maxfrequency = 8000;
        p->aec_process_minfrequency = 0;
        p->af_length = 128;

        p->nlp_process_maxfrequency = 8000;
        p->nlp_process_minfrequency = 0;
        p->overdrive = 1;

#if (TCFG_AUDIO_CVP_NS_MODE == CVP_ANS_MODE)
        p->aggressfactor = 1.25f;
        p->minsuppress = 0.04f;
#else/*TCFG_AUDIO_CVP_NS_MODE == CVP_DNS_MODE*/
        p->aggressfactor = 1.0f;
        p->minsuppress = 0.1f;
        p->DNS_highGain = 2.0f;
        p->DNS_rbRate = 0.5f;
#endif/*TCFG_AUDIO_CVP_NS_MODE*/
        p->init_noise_lvl = -75.f;

        p->wn_detect_time = 0.32f;
        p->wn_detect_time_ratio_thr = 0.8f;
        p->wn_detect_thr = 0.5f;
        p->wn_minsuppress = 0.5f;

        p->enc_process_maxfreq = 8000;
        p->enc_process_minfreq = 0;
        p->sir_maxfreq = 3000;
        p->mic_distance = 0.015f;
        p->target_signal_degradation = 1;
        p->enc_aggressfactor = 4.f;
        p->enc_minsuppress = 0.09f;

        p->global_minsuppress = 0.04f;
        log_error("read dms_param default\n");
    }

    if (CONST_DMS_MALFUNCTION_DETECT) {
        /*检测时间*/
        p->detect_time = 1.0f;            // in second
        /*0~-90 dB 两个mic能量差异持续大于此阈值超过检测时间则会检测为故障*/
        p->detect_eng_diff_thr = 6.f;     //  dB
        /*0~-90 dB 当处于故障状态时，正常的mic能量大于此阈值才会检测能量差异，避免安静环境下误判切回正常状态*/
        p->detect_eng_lowerbound = -55.f; // 0~-90 dB start detect when mic energy lower than this
        p->MalfuncDet_MaxFrequency = 8000;  //检测频率上限
        p->MalfuncDet_MinFrequency = 400;   //检测频率下限
        p->OnlyDetect = 0;// 0 -> 故障切换到单mic模式， 1-> 只检测不切换
    }

    p->AGC_echo_hold = 0;
    p->AGC_echo_look_ahead = 0;
    p->dly_est = CONST_AEC_DLY_EST;
    //aec_param_dump(p);
}

/*
 * 开mic异常检测，设置默认使用的mic
 * 返回参数 0 : 使用双麦
 * 返回参数 -1 : 使用副麦
 * 返回参数 1 : 使用主麦
 */
int audio_dms_get_malfunc_state(void)
{
    int malfunc_state = 0;
    int ret = syscfg_read(CFG_DMS_MALFUNC_STATE_ID, &malfunc_state, sizeof(int));
    if (ret != sizeof(int)) {
        return 0;
    }
    printf("%s : %d", __func__, malfunc_state);
    return malfunc_state;;
}

static int audio_cvp_advanced_options(void *aec, void *nlp, void *ns, void *enc, void *agc, void *wn, void *mfdt)
{
    printf("%s:%d", __func__, __LINE__);

#if (TCFG_AUDIO_DMS_SEL == DMS_NORMAL)
    /*tws双麦*/
    if (CONST_DMS_GLOBAL_VERSION == DMS_GLOBAL_V200) {
        JLSP_dms_wind_cfg *wn_cfg = (JLSP_dms_wind_cfg *)wn;
        wn_cfg->wn_msc_th = 0.6f;
        wn_cfg->ms_th = 80.0f;//110.0f,

        wn_cfg->wn_inty1 = 100;
        wn_cfg->wn_inty2 = 30;
        wn_cfg->wn_gain1 = 1.0f;
        wn_cfg->wn_gain2 = 1.1f;

        wn_cfg->t1_bot = 0;
        wn_cfg->t1_top = 0;
        wn_cfg->t2_bot = 0;
        wn_cfg->t2_top = 0;
        wn_cfg->offset = 0;

        wn_cfg->t1_bot_cnt_limit = 3;
        wn_cfg->t1_top_cnt_limit = 10;
        wn_cfg->t2_bot_cnt_limit = 3;
        wn_cfg->t2_top_cnt_limit = 10;
    } else {

    }
#else
    /*话务耳机*/
#endif
    return 0;
}

/*
*********************************************************************
*                  Audio AEC Open
* Description: 初始化AEC模块
* Arguments  : sr 			采样率(8000/16000)
*			   enablebit	使能模块(AEC/NLP/AGC/ANS...)
*			   out_hdl		自定义回调函数，NULL则用默认的回调
* Return	 : 0 成功 其他 失败
* Note(s)    : 该接口是对audio_aec_init的扩展，支持自定义使能模块以及
*			   数据输出回调函数
*********************************************************************
*/
int audio_aec_open(u16 sample_rate, s16 enablebit, int (*out_hdl)(s16 *data, u16 len))
{
    struct dms_attr *aec_param;
    printf("audio_dms_open\n");
    mem_stats();
#if TCFG_AUDIO_CVP_SYNC
    audio_cvp_sync_open(sample_rate);
#endif/*TCFG_AUDIO_CVP_SYNC*/
#if AEC_USER_MALLOC_ENABLE
    aec_hdl = zalloc(sizeof(struct audio_aec_hdl));
    if (aec_hdl == NULL) {
        log_error("aec_hdl malloc failed");
        return -ENOMEM;
    }
#endif/*AEC_USER_MALLOC_ENABLE*/

    extern u32 aec_addr[], aec_begin[], aec_size[];
    memcpy(aec_addr, aec_begin, (u32)aec_size) ;

#if CVP_LOUDNESS_TRACE_ENABLE
    loudness_meter_init(&mic_loudness, sample_rate, 50, 0);
#endif/*CVP_LOUDNESS_TRACE_ENABLE*/

    cbuf_init(&aec_hdl->output_cbuf, aec_hdl->output_buf, sizeof(aec_hdl->output_buf));

    aec_hdl->dump_packet = AEC_OUT_DUMP_PACKET;
    aec_hdl->inbuf_clear_cnt = AEC_IN_DUMP_PACKET;
    aec_hdl->output_fade_in = 1;
    aec_hdl->output_fade_in_gain = 0;
    aec_param = &aec_hdl->attr;
    aec_param->cvp_advanced_options = audio_cvp_advanced_options;
    aec_param->aec_probe = audio_aec_probe;
    aec_param->aec_post = audio_aec_post;
#if TCFG_AEC_TOOL_ONLINE_ENABLE
    aec_param->aec_update = audio_aec_update;
#endif/*TCFG_AEC_TOOL_ONLINE_ENABLE*/
    aec_param->output_handle = audio_aec_output;

    audio_aec_param_init(aec_param);

    if (enablebit >= 0) {
        aec_param->EnableBit = enablebit;
    }
    if (out_hdl) {
        aec_param->output_handle = out_hdl;
    }

#if TCFG_AEC_SIMPLEX
    aec_param->wn_en = 1;
    aec_param->EnableBit = AEC_MODE_SIMPLEX;
    if (sr == 8000) {
        aec_param->SimplexTail = aec_param->SimplexTail / 2;
    }
#else
    aec_param->wn_en = 0;
#endif/*TCFG_AEC_SIMPLEX*/

    /*根据清晰语音处理模块配置，配置相应的系统时钟*/
    int aec_clock = BT_CALL_16k_ADVANCE_HZ;
    if (sample_rate == 16000) { //WideBand宽带
        aec_param->wideband = 1;
        aec_param->hw_delay_offset = 60;
        if ((aec_param->EnableBit & AEC_EN) && (aec_param->EnableBit & NLP_EN)) {
            aec_clock =  BT_CALL_16k_ADVANCE_HZ;
        } else {
            aec_clock = BT_CALL_16k_HZ;
        }
    } else {//NarrowBand窄带
        aec_param->wideband = 0;
        aec_param->hw_delay_offset = 55;
        if ((aec_param->EnableBit & AEC_EN) && (aec_param->EnableBit & NLP_EN)) {
            aec_clock = BT_CALL_ADVANCE_HZ;
        } else {
            aec_clock = BT_CALL_HZ;
        }
    }

    //DNS
    if (CONST_DMS_VERSION == DMS_V200) {
        if ((aec_param->EnableBit & AEC_EN) && (aec_param->EnableBit & NLP_EN)) {
            aec_clock = CONFIG_BT_CALL_DNS_ADVANCE_HZ;
        } else {
            aec_clock = CONFIG_BT_CALL_DNS_HZ;
        }
    }

    printf("aec_clock:%d,EnableBit:%x\n", aec_clock, aec_param->EnableBit);
    clk_set("sys", aec_clock);

#if AEC_UL_EQ_EN
    if (aec_param->ul_eq_en) {
        struct audio_eq_param ul_eq_param = {0};
        ul_eq_param.sr = sample_rate;
        ul_eq_param.channels = 1;
        ul_eq_param.online_en = 1;
        ul_eq_param.mode_en = 0;
        ul_eq_param.remain_en = 0;
        ul_eq_param.max_nsection = ul_eq_tab.seg_num;
        ul_eq_param.nsection = ul_eq_tab.seg_num;
        ul_eq_param.cb = eq_get_filter_info;
        ul_eq_param.eq_name = AEID_UL_EQ;
        ul_eq_param.seg = ul_eq_tab.seg;
        ul_eq_param.global_gain = ul_eq_tab.global_gain;
        aec_hdl->ul_eq = audio_dec_eq_open(&ul_eq_param);

    }
#endif

#if AEC_DCCS_EN
    if (adc_data.mic_capless) {
        struct audio_eq_param dccs_eq_param = {0};
        dccs_eq_param.sr = sample_rate;
        dccs_eq_param.channels = 1;
        dccs_eq_param.online_en = 0;
        dccs_eq_param.mode_en = 0;
        dccs_eq_param.remain_en = 0;

        dccs_eq_param.cb = eq_get_filter_info;
        dccs_eq_param.eq_name = AEID_DCC_EQ;
        dccs_eq_param.global_gain  = 0 ;
        if (sample_rate == 16000) {
            dccs_eq_param.seg = dccs_eq_tab_16k;
            dccs_eq_param.max_nsection = ARRAY_SIZE(dccs_eq_tab_16k);
            dccs_eq_param.nsection = ARRAY_SIZE(dccs_eq_tab_16k);
        } else {
            dccs_eq_param.seg = dccs_eq_tab_8k;
            dccs_eq_param.max_nsection = ARRAY_SIZE(dccs_eq_tab_8k);
            dccs_eq_param.nsection = ARRAY_SIZE(dccs_eq_tab_8k);
        }
        aec_hdl->dccs_eq = audio_dec_eq_open(&dccs_eq_param);

    }
#endif

#if AEC_PITCHSHIFTER_CONFIG
    PITCH_SHIFT_PARM sparm = {0};
    sparm.sr = sample_rate;
    sparm.shiftv =  100;
    sparm.effect_v =  EFFECT_PITCH_SHIFT;
    sparm.formant_shift = 100;
    aec_hdl->pitch = open_pitch(&sparm);
    pause_pitch(aec_hdl->pitch, 1);
    aec_hdl->pitch_mode_L = 0;
    aec_hdl->pitch_mode_R = 0;

#endif/* AEC_PITCHSHIFTER_CONFIG */
    //aec_param_dump(aec_param);
    aec_hdl->EnableBit = aec_param->EnableBit;
    aec_param->aptfilt_only = 0;
    aec_param->output_sel = DMS_OUTPUT_SEL_DEFAULT;
#if TCFG_KWS_VOICE_RECOGNITION_ENABLE
    extern u8 kws_get_state(void);
    if (kws_get_state()) {
        aec_param->EnableBit = AEC_EN;
        aec_param->aptfilt_only = 1;
        printf("kws open,aec_enablebit=%x", aec_param->EnableBit);
        //临时关闭aec, 对比测试
        //aec_param->toggle = 0;
    }
#endif/*TCFG_KWS_VOICE_RECOGNITION_ENABLE*/

    if (CONST_DMS_WNC) {
        aec_param->EnableBit |= WNC_EN;
    }

    if (CONST_DMS_MALFUNCTION_DETECT) {
        aec_param->EnableBit |= ENC_EN | MFDT_EN;
    }

    if (CONST_AEC_DLY_EST) {
        last_DlyEst = -10000;
        aec_param->EnableBit = 0;/*延时估计的时候不跑实际算法，资源可能不够*/
    }

#if AEC_TOGGLE
    int ret = aec_open(aec_param);
    if (ret == -EPERM) {
        printf("Chip not support ENC(dms+aec) mode!!");//该芯片不支持双mic ENC功能
    }
    ASSERT(ret != -EPERM, "Chip not support dms+aec mode!!");
#endif
    aec_hdl->start = 1;
    mem_stats();
    printf("audio_dms_open succ\n");
    return 0;
}

/*
*********************************************************************
*                  Audio AEC Init
* Description: 初始化AEC模块
* Arguments  : sample_rate 采样率(8000/16000)
* Return	 : 0 成功 其他 失败
* Note(s)    : None.
*********************************************************************
*/
int audio_aec_init(u16 sample_rate)
{
    return audio_aec_open(sample_rate, -1, NULL);
}

/*
*********************************************************************
*                  Audio AEC Reboot
* Description: AEC模块复位接口
* Arguments  : reduce 复位/恢复标志
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_aec_reboot(u8 reduce)
{
    if (aec_hdl) {
        printf("audio_aec_dms_reboot:%x,%x,start:%d", aec_hdl->EnableBit, aec_hdl->attr.EnableBit, aec_hdl->start);
        if (aec_hdl->start) {
            if (reduce) {
                aec_hdl->attr.EnableBit = AEC_EN;
                aec_hdl->attr.aptfilt_only = 1;
                aec_dms_reboot(aec_hdl->attr.EnableBit);
            } else {
                if (aec_hdl->EnableBit != aec_hdl->attr.EnableBit) {
                    aec_hdl->attr.aptfilt_only = 0;
                    aec_dms_reboot(aec_hdl->EnableBit);
                }
            }
        }
    } else {
        printf("audio_aec close now\n");
    }
}

/*
*********************************************************************
*                  Audio AEC Output Select
* Description: 输出选择
* Arguments  : sel = DMS_OUTPUT_SEL_DEFAULT 默认输出算法处理结果
*				   = DMS_OUTPUT_SEL_MASTER	输出主mic(通话mic)的原始数据
*				   = DMS_OUTPUT_SEL_SLAVE	输出副mic(降噪mic)的原始数据
*			   agc 输出数据要不要经过agc自动增益控制模块
* Return	 : None.
* Note(s)    : 可以通过选择不同的输出，来测试mic的频响和ENC指标
*********************************************************************
*/
void audio_aec_output_sel(CVP_OUTPUT_ENUM sel, u8 agc)
{
    if (aec_hdl)	{
        printf("dms_output_sel:%d\n", sel);
        if (agc) {
            aec_hdl->attr.EnableBit |= AGC_EN;
        } else {
            aec_hdl->attr.EnableBit &= ~AGC_EN;
        }
        aec_hdl->attr.output_sel = sel;
    }
}

/*
*********************************************************************
*                  Audio AEC Close
* Description: 关闭AEC模块
* Arguments  : None.
* Return	 : None.
* Note(s)    : None.
*********************************************************************
*/
void audio_aec_close(void)
{
    printf("audio_aec_close:%x", (u32)aec_hdl);
#if TCFG_AUDIO_CVP_SYNC
    audio_cvp_sync_close();
#endif/*TCFG_AUDIO_CVP_SYNC*/
    if (aec_hdl) {
        aec_hdl->start = 0;
#if AEC_TOGGLE
        if (CONST_DMS_MALFUNCTION_DETECT && CONST_DMS_SET_MFDT_STATE) {
            int malfunc_state = cvp_dms_get_malfunc_state();
            int ret = syscfg_write(CFG_DMS_MALFUNC_STATE_ID, &malfunc_state, sizeof(int));
            if (ret != sizeof(int)) {
                printf("vm read  err !!!");
            }
            printf("cvp_dms_get_malfunc_state:%d", malfunc_state);
        }

        aec_close();
#endif
#if AEC_DCCS_EN
        if (aec_hdl->dccs_eq) {
            audio_dec_eq_close(aec_hdl->dccs_eq);
            aec_hdl->dccs_eq = NULL;
        }
#endif/*AEC_DCCS_EN*/
#if AEC_UL_EQ_EN
        if (aec_hdl->ul_eq) {
            audio_dec_eq_close(aec_hdl->ul_eq);
            aec_hdl->ul_eq = NULL;
        }
#endif/*AEC_UL_EQ_EN*/

#if AEC_PITCHSHIFTER_CONFIG
        if (aec_hdl && aec_hdl->pitch) {
            close_pitch(aec_hdl->pitch);
            aec_hdl->pitch = NULL;
        }
#endif/* AEC_PITCHSHIFTER_CONFIG */
        local_irq_disable();
#if AEC_USER_MALLOC_ENABLE
        free(aec_hdl);
#endif/*AEC_USER_MALLOC_ENABLE*/
        aec_hdl = NULL;
        local_irq_enable();
    }
}

/*
*********************************************************************
*                  Audio AEC Status
* Description: AEC模块当前状态
* Arguments  : None.
* Return	 : 0 关闭 其他 打开
* Note(s)    : None.
*********************************************************************
*/
u8 audio_aec_status(void)
{
    if (aec_hdl) {
        return aec_hdl->start;
    }
    return 0;
}

/*
*********************************************************************
*                  Audio AEC Input
* Description: AEC源数据输入
* Arguments  : buf	输入源数据地址
*			   len	输入源数据长度
* Return	 : None.
* Note(s)    : 输入一帧数据，唤醒一次运行任务处理数据，默认帧长256点
*********************************************************************
*/
void audio_aec_inbuf(s16 *buf, u16 len)
{
    if (aec_hdl && aec_hdl->start) {
        if (aec_hdl->input_clear) {
            memset(buf, 0, len);
        }
#if AEC_TOGGLE
        if (aec_hdl->inbuf_clear_cnt) {
            aec_hdl->inbuf_clear_cnt--;
            memset(buf, 0, len);
        }
        int ret = aec_in_data(buf, len);
        if (ret == -1) {
        } else if (ret == -2) {
            log_error("aec inbuf full\n");
        }
#else
        audio_aec_output(buf, len);
#endif/*AEC_TOGGLE*/
    }
}

/*
*********************************************************************
*                  Audio AEC Input Reference
* Description: AEC源参考数据输入
* Arguments  : buf	输入源数据地址
*			   len	输入源数据长度
* Return	 : None.
* Note(s)    : 双mic ENC的参考mic数据输入,单mic的无须调用该接口
*********************************************************************
*/
void audio_aec_inbuf_ref(s16 *buf, u16 len)
{
    if (aec_hdl && aec_hdl->start) {
        aec_in_data_ref(buf, len);
    }
}

/*
*********************************************************************
*                  Audio AEC Reference
* Description: AEC模块参考数据输入
* Arguments  : buf	输入参考数据地址
*			   len	输入参考数据长度
* Return	 : None.
* Note(s)    : 声卡设备是DAC，默认不用外部提供参考数据
*********************************************************************
*/
void audio_aec_refbuf(s16 *buf, u16 len)
{
    if (aec_hdl && aec_hdl->start) {
#if AEC_TOGGLE
        aec_ref_data(buf, len);
#endif/*AEC_TOGGLE*/
    }
}

/*获取风噪的检测结果，1：有风噪，0：无风噪*/
int audio_cvp_dms_wnc_state(void)
{
    int state = 0;
    if (aec_hdl) {
        state = cvp_dms_get_wind_detect_state();
        printf("wnc state : %d", state);
    } else {
        state = -1;
    }
    return state;
}

/*获取单双麦切换状态
 * 0: 正常双麦 ;
 * 1: 副麦坏了，触发故障
 * -1: 主麦坏了，触发故障
 */
int audio_cvp_dms_malfunc_state()
{
    int state = 0;
    if (aec_hdl) {
        state = cvp_dms_get_malfunc_state();
        printf("malfunc state : %d", state);
    } else {
        state = -2;
        printf("cvp malfunc disable !!!");
    }
    return state;
}

/*
 * 获取mic的能量值，开了MFDT_EN才能用
 * mic: 0 获取主麦能量
 * mic：1 获取副麦能量
 * return：返回能量值，[0~90.3],返回-1表示错误
 */
float audio_cvp_dms_mic_energy(u8 mic)
{
    float mic_db = 0;
    if (aec_hdl) {
        mic_db = cvp_dms_get_mic_energy(mic);
        printf("malfunc mic[%d] energy : %d", mic, (int)mic_db);
    } else {
        mic_db = -1;
        printf("cvp malfunc disable !!!");
    }
    return mic_db;
}

/**
 * 以下为用户层扩展接口
 */
//pbg profile use it,don't delete
void aec_input_clear_enable(u8 enable)
{
    if (aec_hdl) {
        aec_hdl->input_clear = enable;
        log_info("aec_input_clear_enable= %d\n", enable);
    }
}

#if AEC_PITCHSHIFTER_CONFIG

/*----------------------------------------------------------------------------*/
/**@brief    变声参数直接更新
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
static void set_pitch_para(u32 shiftv, u32 sr, u8 effect, u32 formant_shift)
{
    if (aec_hdl && aec_hdl->pitch) {
        PITCH_SHIFT_PARM parm = {0};

        parm.sr = sr;
        parm.shiftv = shiftv;
        parm.effect_v = effect;
        parm.formant_shift = formant_shift;

        //printf("\n\n\nshiftv[%d],sr[%d],effect[%d],formant_shift[%d] \n\n", parm.shiftv, parm.sr, parm.effect_v, parm.formant_shift);
        update_picth_parm(aec_hdl->pitch, &parm);
    }
}

/*----------------------------------------------------------------------------*/
/**@brief    变声效果设置
   @param  sound:原声SOUND_ORIGINAL,男声SOUND_UNCLE, 女声SOUND_GODDESS
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_aec_pitch_change(u32 sound)
{
    if (aec_hdl && aec_hdl->pitch) {
        switch (sound) {
        case SOUND_ORIGINAL:
            pause_pitch(aec_hdl->pitch, 1);
            log_info("pitch origin\n");
            break;
        case SOUND_UNCLE:
            set_pitch_para(136, aec_hdl->pitch->parm.sr, EFFECT_PITCH_SHIFT, 0);
            pause_pitch(aec_hdl->pitch, 0);
            log_info("pitch uncle\n");
            break;
        case SOUND_GODDESS:
            if (VC_KINDO_TVM) {
                set_pitch_para(56, aec_hdl->pitch->parm.sr, EFFECT_VOICECHANGE_KIN0, 150);
            } else {
                set_pitch_para(46, aec_hdl->pitch->parm.sr, EFFECT_VOICECHANGE_KIN2, 80);
            }

            /* set_pitch_para(66, aec_hdl->pitch->parm.sr, EFFECT_VOICECHANGE_KIN0, 150); */
            pause_pitch(aec_hdl->pitch, 0);
            log_info("pitch goddess\n");
            break;
        default:
            break;
        }
    }
}

struct _esco_eff {
    u16 eff_L;
    u16 eff_R;
};
struct _esco_eff esco_eff;
#define TWS_FUNC_ID_ESCO_EFF \
	((int)(('E' + 'S' + 'C' + 'O') << (2 * 8)) | \
	 (int)(('E' + 'F' + 'F') << (1 * 8)) | \
	 (int)(('S' + 'Y' + 'N' + 'C') << (0 * 8)))


/*----------------------------------------------------------------------------*/
/**@brief   变声控制
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void audio_aec_pitch_change_ctrl()
{
    if (!aec_hdl) {
        return;
    }
#if TCFG_USER_TWS_ENABLE
    int state = tws_api_get_tws_state();
    if (state & TWS_STA_SIBLING_CONNECTED) {// tws链接时，左耳触发男声， 右耳触发女声
        enum audio_channel channel = tws_api_get_local_channel() == 'L' ? AUDIO_CH_L : AUDIO_CH_R;
        if (channel == AUDIO_CH_L) {
            log_info("aec_hdl->pitch_mode_L %d\n", aec_hdl->pitch_mode_L);
            if (aec_hdl->pitch_mode_L != SOUND_UNCLE) {
                aec_hdl->pitch_mode_L = SOUND_UNCLE;
                aec_hdl->pitch_mode_R = SOUND_UNCLE;
            } else {
                aec_hdl->pitch_mode_L = SOUND_ORIGINAL;
                aec_hdl->pitch_mode_R = SOUND_ORIGINAL;
            }
        } else if (channel == AUDIO_CH_R) {
            log_info("aec_hdl->pitch_mode_R %d\n", aec_hdl->pitch_mode_R);
            if (aec_hdl->pitch_mode_R != SOUND_GODDESS) {
                aec_hdl->pitch_mode_R = SOUND_GODDESS;
                aec_hdl->pitch_mode_L = SOUND_GODDESS;
            } else {
                aec_hdl->pitch_mode_R = SOUND_ORIGINAL;
                aec_hdl->pitch_mode_L = SOUND_ORIGINAL;
            }
        }
        esco_eff.eff_L = aec_hdl->pitch_mode_L;
        esco_eff.eff_R = aec_hdl->pitch_mode_R;
        tws_api_send_data_to_sibling((u8 *)&esco_eff, sizeof(struct _esco_eff), TWS_FUNC_ID_ESCO_EFF);//发送左右耳的变声模式
    } else
#endif/*TCFG_USER_TWS_ENABLE*/
    {
        //tws未连接时，变声直接切换，男声->女声->原声
        if (aec_hdl->pitch_mode_L != SOUND_UNCLE) {
            aec_hdl->pitch_mode_L = SOUND_UNCLE;
        } else if (aec_hdl->pitch_mode_L == SOUND_UNCLE) {
            aec_hdl->pitch_mode_L = SOUND_GODDESS;
        } else {
            aec_hdl->pitch_mode_L = SOUND_ORIGINAL;
        }
        audio_aec_pitch_change(aec_hdl->pitch_mode_L);
    }
}


#if TCFG_USER_TWS_ENABLE
static void tws_esco_pitch_align(void *data, u16 len, bool rx)
{
    memcpy(&esco_eff, data, sizeof(struct _esco_eff));
    int state = tws_api_get_tws_state();
    enum audio_channel channel;
    if (state & TWS_STA_SIBLING_CONNECTED) {
        channel = tws_api_get_local_channel() == 'L' ? AUDIO_CH_L : AUDIO_CH_R;
        if (channel == AUDIO_CH_L) {//左耳设置变声
            log_info("channel %d============= eff_L %d\n", channel, esco_eff.eff_L);
            audio_aec_pitch_change(esco_eff.eff_L);
            if (aec_hdl) {
                aec_hdl->pitch_mode_L =  esco_eff.eff_L;
            }
        } else {//右耳设置变声
            log_info("channel %d============= eff_R %d\n", channel, esco_eff.eff_R);
            audio_aec_pitch_change(esco_eff.eff_R);
            if (aec_hdl) {
                aec_hdl->pitch_mode_R =  esco_eff.eff_R;
            }
        }
    }
}

REGISTER_TWS_FUNC_STUB(esco_align_eff) = {
    .func_id = TWS_FUNC_ID_ESCO_EFF,
    .func    = tws_esco_pitch_align,
};

#endif/*TCFG_USER_TWS_ENABLE*/

#endif/* AEC_PITCHSHIFTER_CONFIG */


#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE == 1*/
#endif /*TCFG_CVP_DEVELOP_ENABLE*/

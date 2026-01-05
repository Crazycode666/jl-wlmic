#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "asm/audio_src.h"
#include "asm/audio_adc.h"
#include "audio_enc.h"
#include "app_main.h"
#include "app_task.h"
#include "encode/encode_write.h"
#include "clock_cfg.h"
#include "audio_config.h"
#include "dev_manager.h"
#include "audio_track.h"

#define LADC_MIC_BUF_NUM        2
#define LADC_MIC_CH_NUM         1

#if TCFG_MIC_EFFECT_ENABLE
#if (RECORDER_MIX_EN)
#define LADC_MIC_IRQ_POINTS     160
#else
#if (TCFG_MIC_EFFECT_SEL == MIC_EFFECT_REVERB)
#define LADC_MIC_IRQ_POINTS    REVERB_LADC_IRQ_POINTS
#else
#define LADC_MIC_IRQ_POINTS     ((MIC_EFFECT_SAMPLERATE/1000)*4)
#endif
#endif/*RECORDER_MIX_EN*/
#elif (WIRELESS_MIC_EFFECT_ENABLE && MIC_EFFECT_LLNS) /*无线mic 低延时*/
#define LADC_MIC_IRQ_POINTS     ((JLA_CODING_SAMPLERATE/1000)*5)
#else
#define LADC_MIC_IRQ_POINTS     256
#endif

#define LADC_MIC_BUFS_SIZE      (LADC_MIC_CH_NUM * LADC_MIC_BUF_NUM * LADC_MIC_IRQ_POINTS)

struct ladc_mic_demo {
    struct audio_adc_output_hdl adc_output;
    struct adc_mic_ch mic_ch;
    s16 adc_buf[LADC_MIC_BUFS_SIZE];    //align 4Bytes
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)
    s16 tmp_buf[LADC_MIC_IRQ_POINTS * 2];
#endif
    int cut_timer;
};
struct ladc_mic_demo *ladc_mic = NULL;

extern struct audio_adc_hdl adc_hdl;
extern bool app_task_msg_post(int msg, int argc, ...);
extern void audio_linein_set_irq_point(u16 point_unit);

static void adc_mic_demo_output(void *priv, s16 *data, int len)
{
    struct audio_adc_hdl *hdl = priv;
    //putchar('o');
    if (ladc_mic == NULL) {
        return;
    }
    //printf("mic:%x,len:%d,ch:%d",data,len,hdl->channel);
    putchar('.');
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)
    //mono->dual
    for (u16 i = 0; i < len / 2; i++) {
        ladc_mic->tmp_buf[2 * i] = data[i];
        ladc_mic->tmp_buf[2 * i + 1] = data[i];
    }
    int wlen = app_audio_output_write(ladc_mic->tmp_buf, len * hdl->channel * 2);
#else
    int wlen = app_audio_output_write(data, len * hdl->channel);
    if (wlen != len) {
        //printf("wlen:%d-%d",wlen,len);
    }
#endif
}

static u8 mic_demo_idle_query()
{
    return (ladc_mic ? 0 : 1);
}
REGISTER_LP_TARGET(mic_demo_lp_target) = {
    .name = "mic_demo",
    .is_idle = mic_demo_idle_query,
};

void audio_adc_mic_demo(u16 sr)
{
    r_printf("audio_adc_mic_open:%d\n", sr);
    ladc_mic = zalloc(sizeof(struct ladc_mic_demo));
    if (ladc_mic) {
        audio_adc_mic_open(&ladc_mic->mic_ch, TCFG_AUDIO_ADC_MIC_CHA, &adc_hdl);
        audio_adc_mic_set_sample_rate(&ladc_mic->mic_ch, sr);
        audio_adc_mic_set_gain(&ladc_mic->mic_ch, 20);
        audio_adc_mic_set_buffs(&ladc_mic->mic_ch, ladc_mic->adc_buf, LADC_MIC_IRQ_POINTS * 2, LADC_MIC_BUF_NUM);
        ladc_mic->adc_output.handler = adc_mic_demo_output;
        ladc_mic->adc_output.priv = &adc_hdl;
        audio_adc_add_output_handler(&adc_hdl, &ladc_mic->adc_output);
        audio_adc_mic_start(&ladc_mic->mic_ch);

        app_audio_output_samplerate_set(sr);
        app_audio_output_start();
    }
}

int audio_adc_mic_init(u16 sr)
{
    //printf("ladc_mic_open:%d\n",sr);
    u8 ladc_mic_gain = 5;
    ASSERT(ladc_mic == NULL);
    ladc_mic = zalloc(sizeof(struct ladc_mic_demo));
    if (ladc_mic) {
        audio_adc_mic_open(&ladc_mic->mic_ch, TCFG_AUDIO_ADC_MIC_CHA, &adc_hdl);
        audio_adc_mic_set_sample_rate(&ladc_mic->mic_ch, sr);
        audio_adc_mic_set_gain(&ladc_mic->mic_ch, ladc_mic_gain);
        audio_adc_mic_set_buffs(&ladc_mic->mic_ch, ladc_mic->adc_buf, LADC_MIC_IRQ_POINTS * 2, LADC_MIC_BUF_NUM);
        audio_adc_mic_start(&ladc_mic->mic_ch);
        return 0;
    } else {
        return -1;
    }
}

void audio_adc_mic_exit(void)
{
    //printf("ladc_mic_close\n");
    if (ladc_mic) {
        audio_adc_mic_close(&ladc_mic->mic_ch);
        free(ladc_mic);
        ladc_mic = NULL;
    }
}

#define ADC_BUF_NUM        2
#define ADC_CH_NUM         2
#define ADC_IRQ_POINTS     256
#define ADC_BUFS_SIZE      (ADC_BUF_NUM *ADC_CH_NUM* ADC_IRQ_POINTS)

#define ADC_STORE_PCM_SIZE	(ADC_BUFS_SIZE * 4)

#if (SOUNDCARD_ENABLE || TCFG_MIC_EFFECT_ENABLE)
#define ADC_LINEIN_IRQ_POINTS         256
#else
#define ADC_LINEIN_IRQ_POINTS         64
#endif

#define ADC_LINEIN_BUFS_SIZE          (ADC_BUF_NUM *ADC_CH_NUM* ADC_LINEIN_IRQ_POINTS)
#define ADC_LINEIN_STORE_PCM_SIZE     (ADC_LINEIN_BUFS_SIZE * 6)
//#define ADC_STORE_PCM_MAX	(ADC_STORE_PCM_SIZE	* 80 / 100)

struct mic_sample_hdl {
    OS_SEM sem;
    struct adc_mic_ch mic_ch;
    struct audio_adc_output_hdl sample_output;
    s16 adc_buf[ADC_BUFS_SIZE];
    /* s16 *store_pcm_buf[ADC_STORE_PCM_SIZE]; */
    s16 *store_pcm_buf;
    cbuffer_t cbuf;
    void (*resume)(void);
    u8 channel_num;
    volatile u8 wait_resume;
    u16 sample_rate;

    spinlock_t lock;
};


void mic_sample_set_resume_handler(void *priv, void (*resume)(void))
{
    struct mic_sample_hdl *mic = (struct mic_sample_hdl *)priv;

    if (mic) {
        mic->resume = resume;
    }
}

static void mic_sample_output_handler(void *priv, s16 *data, int len)
{
    struct mic_sample_hdl *mic = (struct mic_sample_hdl *)priv;

    int wlen = cbuf_write(&mic->cbuf, data, len * mic->channel_num);
    /* os_sem_post(&linein->sem); */
    if (wlen != len * mic->channel_num) {
        putchar('W');
        //r_printf(">> wlen = %d, cbuf_len = %d", wlen, cbuf_get_data_size(&linein->cbuf));
        cbuf_clear(&mic->cbuf);
        cbuf_write(&mic->cbuf, data, len * mic->channel_num);

        if (mic->resume) {
            mic->resume();
        }
        return;
    }
    if (mic->resume) {
        local_irq_disable();
        if (mic->wait_resume) {
            mic->wait_resume = 0;
            mic->resume();
        }
        local_irq_enable();
    }
}


int mic_sample_read(void *hdl, void *data, int len)
{
    struct mic_sample_hdl *mic = (struct mic_sample_hdl *)hdl;
    u32 tlen = LADC_MIC_IRQ_POINTS  * 2;
    if (tlen > len) {
        tlen = len;
    }
    int rlen = cbuf_read(&mic->cbuf, data, tlen);
    local_irq_disable();
    if (rlen < tlen) {
        mic->wait_resume = 1;
    }
    local_irq_enable();

    return rlen;
}

int mic_sample_size(void *hdl)
{
    struct mic_sample_hdl *mic = (struct linein_sample_hdl *)hdl;
    return cbuf_get_data_size(&mic->cbuf);
}

int mic_sample_total(void *hdl)
{
    struct mic_sample_hdl *mic = (struct mic_sample_hdl *)hdl;
    return mic->cbuf.total_len;
}

static void mic_sample_resume(void *priv)
{
    struct mic_sample_hdl *mic = priv;
    if (mic->resume) {
        mic->resume();
    }
}

void *mic_sample_open(struct mic_sample_params *mic_params)
{
    struct mic_sample_hdl *mic = NULL;
    struct mic_sample_params *params = NULL;

    if (!mic_params) {
        return NULL;
    } else {
        params = zalloc(sizeof(struct mic_sample_params));
        if (!params) {
            printf("mic sample params zalloc error");
            return NULL;
        }
        memcpy(params, mic_params, sizeof(struct mic_sample_params));
    }
    mic =  zalloc(sizeof(struct mic_sample_hdl));
    if (!mic) {
        return NULL;
    }
    if (!params->output_priv) {
        memset(mic, 0x0, sizeof(struct mic_sample_hdl));
        mic->store_pcm_buf = malloc(LADC_MIC_IRQ_POINTS  * 2 * 2 * 6);
        if (!mic->store_pcm_buf) {
            return NULL;
        }
        cbuf_init(&mic->cbuf, mic->store_pcm_buf, LADC_MIC_IRQ_POINTS  * 2 * 2 * 6);
    } else {
        /* mic->effect = wireless_mic_effect_init(); */

    }
    mic->sample_rate = params->sample_rate;
    spin_lock_init(&mic->lock);
    os_sem_create(&mic->sem, 0);
    if (audio_mic_open(&mic->mic_ch, mic->sample_rate, params->gain) == 0) {
        if (params->output_priv) {
            mic->sample_output.handler = params->output_handler;
            mic->sample_output.priv = params->output_priv;
            printf("**************get output handler*********************");
        } else {
            mic->sample_output.handler = mic_sample_output_handler;
            mic->sample_output.priv = mic;
        }
        extern u8 get_audio_mic_ch_num(void);
        mic->channel_num = get_audio_mic_ch_num();
        printf("-------------mic->channel_num:%d ----------\n", mic->channel_num);
        audio_mic_add_output(&mic->sample_output);
        audio_mic_start(&mic->mic_ch);
    }
    free(params);

    return mic;
}

void mic_sample_close(void *hdl)
{
    struct mic_sample_hdl *mic = (struct mic_sample_hdl *)hdl;

    if (!mic) {
        return;
    }
    audio_mic_close(&mic->mic_ch, &mic->sample_output);
    free(mic->store_pcm_buf);
    free(mic);
}
/******************************************************/
#define LADC_LINEIN_BUF_NUM        2
#define LADC_LINEIN_CH_NUM         2
#define LADC_LINEIN_IRQ_POINTS     32//256
#define LADC_LINEIN_BUFS_SIZE      (LADC_LINEIN_CH_NUM * LADC_LINEIN_BUF_NUM * LADC_LINEIN_IRQ_POINTS)
struct audio_adc_var {
    struct audio_adc_output_hdl adc_output;
    struct adc_linein_ch ch;
    s16 adc_buf[LADC_LINEIN_BUFS_SIZE];    //align 4Bytes
};
struct audio_adc_var *ladc_linein = NULL;

static void adc_linein_demo_output(void *priv, s16 *data, int len)
{
    struct audio_adc_hdl *hdl = priv;
    putchar('.');
    //printf("linein:%x,len:%d,ch:%d",data,len,hdl->channel);
    int wlen = app_audio_output_write(data, len * hdl->channel);
    if (wlen != len) {
        //printf("wlen:%d-%d",wlen,len);
    }
}

#if 0
/*
 **************************************************************
 * Audio ADC 多通道使用demo
 * 数据结构：LINL LINR MIC LINL LINR MIC ...
 *
 **************************************************************
 */
#define LADC_BUF_NUM        2
#define LADC_CH_NUM         3
#if TCFG_REVERB_SAMPLERATE_DEFUALT >= 32000
#define LADC_IRQ_POINTS     48
#else
#define LADC_IRQ_POINTS     32
#endif
#define LADC_BUFS_SIZE      (LADC_CH_NUM * LADC_BUF_NUM * LADC_IRQ_POINTS)
#define  LADC_2_DAC_ENABLE	0

/*调试使用，推mic数据/linein数据/mic&line混合数据到dac*/
#define LADC_MIC_2_DAC		BIT(0)
#define LADC_LIN_2_DAC		BIT(1)
#define LADC_2_DAC			(LADC_MIC_2_DAC | LADC_LIN_2_DAC)

typedef struct {
    struct audio_adc_output_hdl output;
    struct adc_linein_ch linein_ch;
    struct adc_mic_ch mic_ch;
    s16 adc_buf[LADC_BUFS_SIZE];    //align 4Bytes
    s16 temp_buf[LADC_IRQ_POINTS * 2];
    u8 mic_gain;
    u8 ladc_gain;
    u8 mic_en: 2;
    u8 ladc_en: 2;
    u8 ladc_ch: 3;
    cbuffer_t *mic_pcm_cbuf;
    void (*mic_pcm_resume)(void *priv);
    void *mic_pcm_resume_priv;
    cbuffer_t *ladc_pcm_cbuf;
    void (*ladc_pcm_resume)(void *priv);
    void *ladc_pcm_resume_priv;
} audio_adc_t;
static audio_adc_t *ladc_var = NULL;

static void audio_adc_output_demo(void *priv, s16 *data, int len)
{
    struct audio_adc_hdl *hdl = priv;
    int wlen = 0;
    u16 i;
    /* putchar('.'); */
    if (ladc_var == NULL) {
        return;
    }
    /* printf("linein:%x,len:%d,ch:%d",data,len,hdl->channel); */
    if (ladc_var->ladc_ch == 2) {
        if (ladc_var->mic_en) {
            if (ladc_var->mic_pcm_cbuf) {
                for (i = 0; i < len / 2; i++) {
                    ladc_var->temp_buf[i] = data[i * 3 + 2];
                }
                wlen = cbuf_write(ladc_var->mic_pcm_cbuf, ladc_var->temp_buf, len);
                if (wlen != len) {
                    putchar('#');
                }
                if (ladc_var->mic_pcm_resume) {
                    ladc_var->mic_pcm_resume(ladc_var->mic_pcm_resume_priv);
                }
            }
        }
        if (ladc_var->ladc_en) {
            if (ladc_var->ladc_pcm_cbuf) {
                for (i = 1; i < len / 2; i++) {
                    data[i * 2] = data[i * 3];
                    data[i * 2 + 1] = data[i * 3 + 1];

                }
                wlen = cbuf_write(ladc_var->ladc_pcm_cbuf, data, len * 2);
                if (wlen != len * 2) {
                    putchar('e');
                }
                if (ladc_var->ladc_pcm_resume) {
                    ladc_var->ladc_pcm_resume(ladc_var->ladc_pcm_resume_priv);
                }
            }
        }
    } else {
        if (ladc_var->mic_en) {
            if (ladc_var->mic_pcm_cbuf) {
                for (i = 0; i < len / 2; i++) {
                    ladc_var->temp_buf[i] = data[i * 2 + 1];
                }
                wlen = cbuf_write(ladc_var->mic_pcm_cbuf, ladc_var->temp_buf, len);
                if (wlen != len) {
                    putchar('E');
                }
                if (ladc_var->mic_pcm_resume) {
                    ladc_var->mic_pcm_resume(ladc_var->mic_pcm_resume_priv);
                }
            }
        }
        if (ladc_var->ladc_en) {
            if (ladc_var->ladc_pcm_cbuf) {
                for (i = 1; i < len / 2; i++) {
                    data[i] = data[i * 2];
                }
                wlen = cbuf_write(ladc_var->ladc_pcm_cbuf, data, len);
                if (wlen != len) {
                    putchar('e');
                }
                if (ladc_var->ladc_pcm_resume) {
                    ladc_var->ladc_pcm_resume(ladc_var->ladc_pcm_resume_priv);
                }
            }
        }
    }
}



/* void audio_adc_open_demo(void) */
int audio_three_adc_open(void)
{
    u16 ladc_sr;
#if (defined(TCFG_REVERB_SAMPLERATE_DEFUALT))
    ladc_sr = TCFG_REVERB_SAMPLERATE_DEFUALT;
#else
    ladc_sr = 16000;
#endif
    u8 mic_gain = 5;
    u8 linein_gain = 3;
    u8 i, temp;
    r_printf("audio_adc_open_demo,sr:%d,mic_gain:%d,linein_gain:%d\n", ladc_sr, mic_gain, linein_gain);
    if (ladc_var) {
        r_printf("ladc already open \n");
        return 0;
    }
    ladc_var = zalloc(sizeof(audio_adc_t));
    if (ladc_var) {
        temp = (0x3F & TCFG_LINEIN_LR_CH);
        printf("linein ch [%d]\n\n\n", temp);
        ladc_var->ladc_ch = 0;
        for (i = 0; i < 8; i++) {
            if ((temp >> i)&BIT(0)) {
                ladc_var->ladc_ch++;
            }
        }
        printf("linein ch [%d]\n\n\n", ladc_var->ladc_ch);
        if ((ladc_var->ladc_ch > 2) || (ladc_var->ladc_ch == 0)) {
            printf(" err ladc ch \n\n");
            free(ladc_var);
            return -1;
        }
        audio_adc_mic_open(&ladc_var->mic_ch, TCFG_AUDIO_ADC_MIC_CHA, &adc_hdl);
        audio_adc_mic_set_sample_rate(&ladc_var->mic_ch, ladc_sr);
        audio_adc_mic_set_gain(&ladc_var->mic_ch, mic_gain);
        /* audio_adc_linein_open(&ladc_var->linein_ch, AUDIO_ADC_LINE0_LR, &adc_hdl); */
        audio_adc_linein_open(&ladc_var->linein_ch, TCFG_LINEIN_LR_CH << 2, &adc_hdl);

        audio_adc_linein_set_sample_rate(&ladc_var->linein_ch, ladc_sr);
        audio_adc_linein_set_gain(&ladc_var->linein_ch, linein_gain);

        printf("adc_buf_size:%d", sizeof(ladc_var->adc_buf));
        /* audio_adc_set_buffs(&ladc_var->linein_ch, ladc_var->adc_buf, LADC_CH_NUM * LADC_IRQ_POINTS * 2, LADC_BUF_NUM); */
        audio_adc_set_buffs(&ladc_var->linein_ch, ladc_var->adc_buf, (ladc_var->ladc_ch + 1) * LADC_IRQ_POINTS * 2, LADC_BUF_NUM);
        ladc_var->output.handler = audio_adc_output_demo;
        ladc_var->output.priv = &adc_hdl;
        audio_adc_add_output_handler(&adc_hdl, &ladc_var->output);
        audio_adc_start(&ladc_var->linein_ch, &ladc_var->mic_ch);

#if LADC_2_DAC_ENABLE
        app_audio_output_samplerate_set(ladc_sr);
        app_audio_output_start();
#endif/*LADC_2_DAC_ENABLE*/
        return 0;
    } else {
        return -1;
    }
}

void audio_three_adc_close()
{
    if (ladc_var) {
        if (ladc_var->mic_en == 0) {
            ladc_var->mic_pcm_cbuf = NULL;
        }
        if (ladc_var->ladc_en == 0) {
            ladc_var->ladc_pcm_cbuf = NULL;
        }

        if (ladc_var->mic_en || ladc_var->ladc_en) {
            return;
        }
        audio_adc_close(&ladc_var->linein_ch, &ladc_var->mic_ch);
        audio_adc_del_output_handler(&adc_hdl, &ladc_var->output);
        free(ladc_var);
        ladc_var = NULL;
    }
}
void three_adc_mic_enable(u8 mark)
{
    if (ladc_var) {
        ladc_var->mic_en = mark ? 1 : 0;
    }
}

void three_adc_ladc_enable(u8 mark)
{
    if (ladc_var) {
        ladc_var->ladc_en = mark ? 1 : 0;
    }
}

void set_mic_cbuf_hdl(cbuffer_t *mic_cbuf)
{
    if (ladc_var && mic_cbuf) {
        ladc_var->mic_pcm_cbuf = mic_cbuf;
    }
}
void set_mic_resume_hdl(void (*resume)(void *priv), void *priv)
{
    if (ladc_var) {
        ladc_var->mic_pcm_resume = resume;
        ladc_var->mic_pcm_resume_priv = priv;
    }
}
void set_ladc_cbuf_hdl(cbuffer_t *ladc_cbuf)
{
    if (ladc_var && ladc_cbuf) {
        ladc_var->ladc_pcm_cbuf = ladc_cbuf;
    }
}
void set_ladc_resume_hdl(void (*resume)(void *priv), void *priv)
{
    if (ladc_var) {
        ladc_var->ladc_pcm_resume = resume;
        ladc_var->ladc_pcm_resume_priv = priv;
    }
}
void three_adc_mic_set_gain(u8 level)
{
    if (ladc_var) {
        audio_adc_mic_set_gain(&ladc_var->mic_ch, level);
    }
}
#endif
/***********************************************************************************************************/

struct linein_sample_hdl {
    OS_SEM sem;
    struct adc_linein_ch linein_ch;
    struct audio_adc_output_hdl sample_output;
    s16 adc_buf[ADC_BUFS_SIZE];
    /* s16 *store_pcm_buf[ADC_STORE_PCM_SIZE]; */
    s16 *store_pcm_buf;
    cbuffer_t cbuf;
    void (*resume)(void);
    u8 channel_num;
    volatile u8 wait_resume;
    u16 output_fade_in_gain;
    u8 output_fade_in;
    u16 sample_rate;
    void *audio_track;
    u8 source;

    spinlock_t lock;
};

/* struct linein_sample_hdl g_linein_sample_hdl sec(.linein_pcm_mem); */
/* static s16 linein_store_pcm_buf[ADC_STORE_PCM_SIZE] sec(.linein_pcm_mem); */

void linein_sample_set_resume_handler(void *priv, void (*resume)(void))
{
    struct linein_sample_hdl *linein = (struct linein_sample_hdl *)priv;

    if (linein) {
        linein->resume = resume;
    }
}

void fm_inside_output_handler(void *priv, s16 *data, int len)
{
    struct linein_sample_hdl *linein = (struct linein_sample_hdl *)priv;

    spin_lock(&linein->lock);
    if (linein->audio_track) {
        audio_local_sample_track_in_period(linein->audio_track, (len >> 1) / linein->channel_num);
    }
    spin_unlock(&linein->lock);

    int wlen = cbuf_write(&linein->cbuf, data, len);
    os_sem_post(&linein->sem);
    if (wlen != len) {
        putchar('W');
    }
    if (linein->resume) {
        local_irq_disable();
        if (linein->wait_resume) {
            linein->wait_resume = 0;
            linein->resume();
        }
        local_irq_enable();
    }
}

static void linein_sample_output_handler(void *priv, s16 *data, int len)
{
    struct linein_sample_hdl *linein = (struct linein_sample_hdl *)priv;


    spin_lock(&linein->lock);
    if (linein->audio_track) {
        audio_local_sample_track_in_period(linein->audio_track, (len >> 1));
    }
    spin_unlock(&linein->lock);

    if (linein->output_fade_in) {
        s32 tmp_data;
        //printf("fade:%d\n",aec_hdl->output_fade_in_gain);
        for (int i = 0; i < len * linein->channel_num / 2; i++) {
            tmp_data = data[i];
            data[i] = tmp_data * linein->output_fade_in_gain >> 7;

        } //end of for

        linein->output_fade_in_gain += 2 ;
        if (linein->output_fade_in_gain >= 128) {
            linein->output_fade_in = 0;
        }
    }


    int wlen = cbuf_write(&linein->cbuf, data, len * linein->channel_num);

    /* os_sem_post(&linein->sem); */
    if (wlen != len * linein->channel_num) {
        putchar('W');
        //r_printf(">> wlen = %d, cbuf_len = %d", wlen, cbuf_get_data_size(&linein->cbuf));
        cbuf_clear(&linein->cbuf);
        cbuf_write(&linein->cbuf, data, len * linein->channel_num);

        if (linein->resume) {
            linein->resume();
        }
        return;
    }
    if (linein->resume) {
        local_irq_disable();
        if (linein->wait_resume) {
            linein->wait_resume = 0;
            linein->resume();
        }
        local_irq_enable();
    }
}

AT(.fm_data_code)
int linein_stream_sample_rate(void *hdl)
{
    struct linein_sample_hdl *linein = (struct linein_sample_hdl *)hdl;
    void *audio_track;

    if (linein->audio_track) {
        int sr =  audio_local_sample_track_rate(linein->audio_track);
        if ((sr < (linein->sample_rate + 200)) && (sr > linein->sample_rate - 200)) {
            return sr;
        }
        printf("linein audio_track reset \n");
        spin_lock(&linein->lock);
        audio_track = linein->audio_track;
        linein->audio_track = NULL;
        audio_local_sample_track_close(audio_track);
        linein->audio_track = audio_local_sample_track_open(linein->channel_num, linein->sample_rate, 1000);
        spin_unlock(&linein->lock);
    }

    return linein->sample_rate;
}

AT(.fm_data_code)
int linein_sample_read(void *hdl, void *data, int len)
{
    struct linein_sample_hdl *linein = (struct linein_sample_hdl *)hdl;
#if 0
// no wait
    u8 count = 0;
__retry:
    if (cbuf_get_data_size(&linein->cbuf) < len) {
        local_irq_disable();
        os_sem_set(&linein->sem, 0);
        local_irq_enable();
        os_sem_pend(&linein->sem, 2);
        if (cbuf_get_data_size(&linein->cbuf) < len) {
            if (++count > 4) {
                return 0;
            }
            goto __retry;
        }

    }
#endif
    u32 tlen = ADC_LINEIN_IRQ_POINTS * 2;
    if (tlen > len) {
        tlen = len;
    }
    int rlen = cbuf_read(&linein->cbuf, data, tlen);
    local_irq_disable();
    if (rlen < tlen) {
        linein->wait_resume = 1;
    }
    local_irq_enable();

    return rlen;
}

AT(.fm_data_code)
int linein_sample_size(void *hdl)
{
    struct linein_sample_hdl *linein = (struct linein_sample_hdl *)hdl;
    return cbuf_get_data_size(&linein->cbuf);
}

AT(.fm_data_code)
int linein_sample_total(void *hdl)
{
    struct linein_sample_hdl *linein = (struct linein_sample_hdl *)hdl;
    return linein->cbuf.total_len;
}

static void linein_sample_resume(void *priv)
{
    struct linein_sample_hdl *linein = priv;
    if (linein->resume) {
        linein->resume();
    }
}

void *linein_sample_open(u8 source, u16 sample_rate)
{
    struct linein_sample_hdl *linein = NULL;

    linein =  zalloc(sizeof(struct linein_sample_hdl));
    if (!linein) {
        return NULL;
    }

    memset(linein, 0x0, sizeof(struct linein_sample_hdl));
    linein->store_pcm_buf = malloc(ADC_LINEIN_STORE_PCM_SIZE);
    if (!linein->store_pcm_buf) {
        return NULL;
    }
    spin_lock_init(&linein->lock);
    linein->output_fade_in_gain = 0;

    linein->output_fade_in = 1;
    if (source != 0xff) {
    }
    linein->source = source;
    cbuf_init(&linein->cbuf, linein->store_pcm_buf, ADC_LINEIN_STORE_PCM_SIZE);
    os_sem_create(&linein->sem, 0);
    if (source == 0xff) {
        return linein;
    }
    audio_linein_set_irq_point(ADC_LINEIN_IRQ_POINTS);
    if (source == TCFG_LINEIN_LR_CH) {
        if (audio_linein_open(&linein->linein_ch, sample_rate, 3) == 0) {
            linein->sample_output.handler = linein_sample_output_handler;
            linein->sample_output.priv = linein;
            linein->channel_num = get_audio_linein_ch_num();
            printf("-------------linein->channel_num:%d ----------\n", linein->channel_num);
            audio_linein_add_output(&linein->sample_output);
            audio_linein_start(&linein->linein_ch);
        }
    } else {
        if (audio_linein_fm_open(&linein->linein_ch, sample_rate, 3) == 0) {
            linein->sample_output.handler = linein_sample_output_handler;
            linein->sample_output.priv = linein;
            linein->channel_num = get_audio_linein_fm_ch_num();
            printf("-------------linein_fm->channel_num:%d ----------\n", linein->channel_num);
            audio_linein_fm_add_output(&linein->sample_output);
            audio_linein_fm_start(&linein->linein_ch);
        }
    }

    linein->sample_rate = sample_rate;
    linein->audio_track = audio_local_sample_track_open(linein->channel_num, sample_rate, 1000);


    return linein;
}

void linein_sample_close(void *hdl)
{
    struct linein_sample_hdl *linein = (struct linein_sample_hdl *)hdl;
    void *audio_track;

    if (!linein) {
        return;
    }
    if (linein->source == TCFG_LINEIN_LR_CH) {
        audio_linein_close(&linein->linein_ch, &linein->sample_output);
    } else {
        audio_linein_fm_close(&linein->linein_ch, &linein->sample_output);
    }
    audio_linein_set_irq_point(0);
    if (linein->audio_track) {
        audio_track = linein->audio_track;
        linein->audio_track = NULL;
        audio_local_sample_track_close(audio_track);
    }

    free(linein->store_pcm_buf);
    free(linein);
}








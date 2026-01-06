#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "audio_config.h"
#include "asm/audio_adc.h"
#include "sound_device_driver.h"
#include "audio_enc/audio_enc.h"
#include "audio_splicing.h"
#include "audio_path.h"

#if (TCFG_BROADCAST_ENABLE || TCFG_CONNECTED_ENABLE)

#define __ADC_IRQ_RUN AT(.adc_irq.text.cache.L2)

#if ((defined TCFG_LIVE_AUDIO_LOW_LATENCY_EN) && TCFG_LIVE_AUDIO_LOW_LATENCY_EN)
extern void audio_adc_set_irq_point_unit(u16 point_unit);
#if BROADCAST_SOURCE_IRQ_SYNC_TX_ALIGN
#if JL_LOW_LATENCY_EN
#define ADC_CAPTURE_LOW_LATENCY     2500 //us
#else
#define ADC_CAPTURE_LOW_LATENCY     5000 //us
#endif
#else
#define ADC_CAPTURE_LOW_LATENCY     1000 //us
#endif
#else
#define ADC_CAPTURE_LOW_LATENCY     0
#endif

/* #define TCFG_SOUND_PCM_DELAY_TEST   1 */

//-----------------------------------------------------------------------------
// linein
//-----------------------------------------------------------------------------

struct live_adc_capture_context {
    struct audio_adc_output_hdl adc_output; //adc 输出
    struct adc_linein_ch aux_ch;      	    //aux 通道
    struct adc_mic_ch mic_ch;               //mic 通道
    int sample_rate;   					    //采样率
    u8 gain;								//增益
    u8 input_nch;						    //打开的输入通道数
    u8 nch;                                 //输出pcm数据声道数
    u8 start;								//aux 状态
    u8 on_off;
    s16 *sample_buffer;                     //单双声道转换缓存
    void *output_path;                      //数据接口私有参数
    int (*write_frame)(void *path, struct audio_frame *frame);//输出接口
    void *clock;
    u32(*capture_time)(void *clock, u8 type);
};

__ADC_IRQ_RUN
static int adc_capture_write_frame(struct live_adc_capture_context *adc, void *data, int len)
{
    if (!adc->write_frame) {
        return len;
    }

    struct audio_frame frame = {
        .mode = AUDIO_FRAME_DMA_STREAM,
        .nch = adc->nch,
        .sample_rate = adc->sample_rate,
        .len = len,
        .data = data,
        .timestamp = adc->capture_time ? adc->capture_time(adc->clock, 0) : 0,
    };

    return adc->write_frame(adc->output_path, &frame);
}

static void aux_capture_data_handler(void *priv, s16 *data, int len)
{
    struct live_adc_capture_context *aux = (struct live_adc_capture_context *)priv;

    if (!aux || !aux->start) {
        return ;
    }
    if (!aux->on_off) {
        memset(data, 0, len * aux->input_nch);
    }
    int wlen = 0;
    if (aux->input_nch >= 2) {
        /*输出两个linein的数据,默认输出第一个和第二个采样通道的数据*/
        if (aux->nch >= 2) {
            wlen = adc_capture_write_frame(aux, data, len * 2);
            if (wlen < len * 2) {
                putchar('L');
            }
        } else { //硬件配置双声道,需要输出单声道,这种情况最好硬件也配成单声道避免资源浪费.
            pcm_dual_to_single(aux->sample_buffer, data, len * 2);
            wlen = adc_capture_write_frame(aux, aux->sample_buffer, len);
            if (wlen < len) {
                putchar('L');
            }
        }

    } else {
        if (aux->nch >= 2) { //硬件配置单声道,需要输出双声道数据,
            pcm_single_to_dual(aux->sample_buffer, data, len);
            wlen = adc_capture_write_frame(aux, aux->sample_buffer, len * 2);
            if (wlen < len * 2) {
                putchar('L');
            }
        } else {
            wlen = adc_capture_write_frame(aux, data, len);
            if (wlen < len) {
                putchar('L');
            }

        }
    }
}

void live_aux_capture_play_pause(void *capture, u8 on_off)
{
    struct live_adc_capture_context *ctx = (struct live_adc_capture_context *)capture;

    ctx->on_off = on_off;
}

void *live_aux_capture_open(struct audio_path *path)
{
    struct live_adc_capture_context *aux = zalloc(sizeof(struct live_adc_capture_context));
    ASSERT(aux);
    aux->sample_rate = path->fmt.sample_rate;
    aux->gain = 3;//params->gain;
    aux->nch = path->fmt.channel;
    aux->output_path = path->output.path;
    aux->write_frame = path->output.write_frame;
    aux->clock = path->time.reference_clock;
    aux->capture_time = path->time.request;

#if ADC_CAPTURE_LOW_LATENCY
    u16 adc_latency = aux->sample_rate * ADC_CAPTURE_LOW_LATENCY / 1000000;
    audio_adc_set_irq_point_unit(adc_latency);
#endif
    if (audio_linein_open(&aux->aux_ch, aux->sample_rate, aux->gain) == 0) {
        aux->adc_output.handler = aux_capture_data_handler;
        aux->adc_output.priv = aux;
        aux->input_nch = get_audio_linein_ch_num();
        //默认adc中断点数为256
        u16 adc_irq_points = 256;
        //需要声道转换时才申请转换缓存
        if ((aux->input_nch > aux->nch) || (aux->input_nch < aux->nch)) {
#if ADC_CAPTURE_LOW_LATENCY
            adc_irq_points = adc_latency;
#endif
            aux->sample_buffer = zalloc(adc_irq_points * 2 * aux->nch);
            ASSERT(aux->sample_buffer);
        }
        printf("-------------linein->channel_num:%d  output_num:%d----------\n", aux->input_nch, aux->nch);
        audio_linein_add_output(&aux->adc_output);
        audio_linein_start(&aux->aux_ch);
        aux->start = 1;
    }
    aux->on_off = 1;
    printf("live aux capture start succ\n");
    return aux;

}

void live_aux_capture_close(void *capture)
{
    struct live_adc_capture_context *aux = (struct live_adc_capture_context *)capture;
    printf("live aux capture close\n");
    if (!aux) {
        return;
    }
    aux->start = 0;
    audio_linein_close(&aux->aux_ch, &aux->adc_output);
#if ADC_CAPTURE_LOW_LATENCY
    audio_adc_set_irq_point_unit(0);
#endif
    local_irq_disable();
    free(aux);
    aux = NULL;
    local_irq_enable();
}

//-----------------------------------------------------------------------------
// mic
//-----------------------------------------------------------------------------

#if (defined TCFG_SOUND_PCM_DELAY_TEST && TCFG_SOUND_PCM_DELAY_TEST)
static int test_count = 0;
static u16 sin_data_offset = 0;
const s16 sin_48k[48] = {
    0, 2139, 4240, 6270, 8192, 9974, 11585, 12998,
    14189, 15137, 15826, 16244, 16384, 16244, 15826, 15137,
    14189, 12998, 11585, 9974, 8192, 6270, 4240, 2139,
    0, -2139, -4240, -6270, -8192, -9974, -11585, -12998,
    -14189, -15137, -15826, -16244, -16384, -16244, -15826, -15137,
    -14189, -12998, -11585, -9974, -8192, -6270, -4240, -2139
};
static int get_sine_data(s16 *data, u16 points, u8 ch)
{
    while (points--) {
        if (sin_data_offset >= ARRAY_SIZE(sin_48k)) {
            sin_data_offset = 0;
        }
        *data++ = sin_48k[sin_data_offset];
        if (ch == 2) {
            *data++ = sin_48k[sin_data_offset];
        }
        sin_data_offset++;
    }
    return 0;
}
#endif

__ADC_IRQ_RUN
static void mic_capture_data_handler(void *priv, s16 *data, int len)
{
    struct live_adc_capture_context *mic = (struct live_adc_capture_context *)priv;

    if (!mic || !mic->start) {
        return ;
    }

#if (defined TCFG_SOUND_PCM_DELAY_TEST && TCFG_SOUND_PCM_DELAY_TEST)
    get_sine_data(data, len / 2, 1);
#endif

    if (!mic->on_off) {
        memset(data, 0, len * mic->input_nch);
    }

    int wlen = 0;
    if (mic->nch >= 2) { //硬件配置单声道,需要输出双声道数据,
        pcm_single_to_dual(mic->sample_buffer, data, len);
        wlen = adc_capture_write_frame(mic, mic->sample_buffer, len * 2);
        if (wlen < len * 2) {
            putchar('L');
        }
    } else {
        wlen = adc_capture_write_frame(mic, data, len);
        if (wlen < len) {
            putchar('L');
        }
    }
}

void live_mic_capture_play_pause(void *capture, u8 on_off)
{
    struct live_adc_capture_context *ctx = (struct live_adc_capture_context *)capture;

    ctx->on_off = on_off;
}

void *live_mic_capture_open(struct audio_path *path)
{
    struct live_adc_capture_context *mic = zalloc(sizeof(struct live_adc_capture_context));
    ASSERT(mic);
    mic->sample_rate = path->fmt.sample_rate;
    mic->gain = 2;
    mic->nch = path->fmt.channel;
    mic->output_path = path->output.path;
    mic->write_frame = path->output.write_frame;
    mic->clock = path->time.reference_clock;
    mic->capture_time = path->time.request;

#if ADC_CAPTURE_LOW_LATENCY
    u16 adc_latency = mic->sample_rate * ADC_CAPTURE_LOW_LATENCY / 1000000;
    audio_adc_set_irq_point_unit(adc_latency);
#endif
    if (audio_mic_open(&mic->mic_ch, mic->sample_rate, mic->gain) == 0) {
        mic->adc_output.handler = mic_capture_data_handler;
        mic->adc_output.priv = mic;
        mic->input_nch = get_audio_mic_ch_num();
        //默认adc中断点数为256
        u16 adc_irq_points = 256;
        //需要声道转换时才申请转换缓存
        if ((mic->input_nch > mic->nch) || (mic->input_nch < mic->nch)) {
#if ADC_CAPTURE_LOW_LATENCY
            adc_irq_points = adc_latency;
#endif
            mic->sample_buffer = zalloc(adc_irq_points * 2 * mic->nch);
            ASSERT(mic->sample_buffer);
        }
        g_printf("-------------mic->channel_num:%d  output_num:%d----------\n", mic->input_nch, mic->nch);
        audio_mic_add_output(&mic->adc_output);

#if BROADCAST_SOURCE_TRIGGER_BY_TX_ALIGN
        audio_mic_digital_enable(0);
#endif
        audio_mic_start(&mic->mic_ch);
        mic->start = 1;
    }
    mic->on_off = 1;
    printf("live mic capture start succ\n");
    return mic;
}

void live_mic_capture_close(void *capture)
{
    struct live_adc_capture_context *mic = (struct live_adc_capture_context *)capture;
    printf("live mic capture close\n");
    if (!mic) {
        return;
    }
    mic->start = 0;
    audio_mic_close(&mic->mic_ch, &mic->adc_output);
#if ADC_CAPTURE_LOW_LATENCY
    audio_adc_set_irq_point_unit(0);
#endif
    local_irq_disable();
    free(mic);
    mic = NULL;
    local_irq_enable();
}

#endif


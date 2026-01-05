/*************************************************************************************************/
/*!
*  \file      live_audio_effect.c
*
*  \brief     这里主要存放实时音频下的效果处理
*
*  Copyright (c) 2011-2022 ZhuHai Jieli Technology Co.,Ltd.
*
*/
/*************************************************************************************************/
#include "live_audio.h"
#include "live_audio_effect.h"
#include "wireless_mic_effect.h"

struct live_audio_effect_context {
    u8 nch;
    u8 remain;
    int sample_rate;
    int (*write_frame)(void *path, struct audio_frame *frame);
    void *opath;
    void (*underrun_wakeup)(void *stream);
    void *underrun_wakeup_data;

#if (WIRELESS_MIC_EFFECT_ENABLE && WIRELESS_MIC_TX_EFFECT_ENABLE)
    struct live_mic_effect *mic_effect;
#endif
    //TODO
};

#if (WIRELESS_MIC_EFFECT_ENABLE && WIRELESS_MIC_TX_EFFECT_ENABLE)
static int live_audio_effect_output(void *priv, void *data, int len)
{
    struct live_audio_effect_context *ctx = (struct live_audio_effect_context *)priv;
    int wlen = 0;
    struct audio_frame frame = {
        .data = data,
        .len = len,
        .nch = ctx->nch,
        .sample_rate = ctx->sample_rate,
    };

    if (ctx->write_frame) {
        wlen = ctx->write_frame(ctx->opath, &frame);
    }
    return wlen;
}
#endif

void *live_audio_effect_open(struct live_audio_effect_params *params)
{
    struct live_audio_effect_context *ctx = (struct live_audio_effect_context *)zalloc(sizeof(struct live_audio_effect_context));

    if (!ctx) {
        return NULL;
    }
#if (WIRELESS_MIC_EFFECT_ENABLE && WIRELESS_MIC_TX_EFFECT_ENABLE)
    struct live_mic_effect_param  param = {
        .effect_config = LIVE_MIC_EFFECT_CONFIG,
        .sample_rate = params->sample_rate,
        .ch_num = params->nch,
        .output_priv = ctx,
        .output = live_audio_effect_output,
    };
#endif

    ctx->nch = params->nch;
    ctx->sample_rate = params->sample_rate;
    ctx->opath = params->opath;
    ctx->write_frame = params->write_frame;
    ctx->underrun_wakeup_data = params->underrun_wakeup_data;
    ctx->underrun_wakeup = params->underrun_wakeup;
#if (WIRELESS_MIC_EFFECT_ENABLE && WIRELESS_MIC_TX_EFFECT_ENABLE)
    ctx->mic_effect = live_mic_effect_init(&param);
#endif

    return ctx;
}

void live_audio_effect_close(void *effect)
{
    struct live_audio_effect_context *ctx = (struct live_audio_effect_context *)effect;
#if (WIRELESS_MIC_EFFECT_ENABLE && WIRELESS_MIC_TX_EFFECT_ENABLE)
    if (ctx->mic_effect) {
        live_mic_effect_close(ctx->mic_effect);
    }
#endif

    if (ctx) {
        free(ctx);
    }
}

int live_audio_effect_write_frame(void *effect, struct audio_frame *frame)
{
    struct live_audio_effect_context *ctx = (struct live_audio_effect_context *)effect;
    int wlen = 0;

#if (WIRELESS_MIC_EFFECT_ENABLE && WIRELESS_MIC_TX_EFFECT_ENABLE)
    wlen = live_mic_effect_input(ctx->mic_effect, frame->data, frame->len, 0);
#else
    if (!ctx->remain) {
        /*
         * 这里提供两种方式：
         * 一种是直接run，一种是通过audio_stream结构传递信息
         */
        /*
         * 直接运算
         */
        //live_audio_effect_run(frame->data, frame->len);


        /*
         * audio_stream 方式
         * struct audio_data_frame stream_frame = {
         *      .data = frame->data,
         *      .len = frame->len,
         *      .channel = frame->nch,
         *      .sample_rate = frame->sample_rate,
         * };
         * audio_stream_run(&ctx->entry, &stream_frame);
         *
         **/
    }
    if (ctx->write_frame) {
        wlen = ctx->write_frame(ctx->opath, frame);
        frame->offset += wlen;
        ctx->remain = frame->offset == frame->len ? 0 : 1;
    }
#endif

    return wlen;
}

void live_audio_effect_wakeup(void *effect)
{
    struct live_audio_effect_context *ctx = (struct live_audio_effect_context *)effect;

    if (ctx->underrun_wakeup) {
        ctx->underrun_wakeup(ctx->underrun_wakeup_data);
    }
}


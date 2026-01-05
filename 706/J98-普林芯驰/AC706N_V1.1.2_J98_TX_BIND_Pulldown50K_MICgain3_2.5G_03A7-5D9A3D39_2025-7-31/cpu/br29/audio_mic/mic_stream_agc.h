#ifndef __MIC_STREAM_AGC_H__
#define __MIC_STREAM_AGC_H__

#include "app_config.h"
#include "generic/typedef.h"
#include "media/includes.h"
#include "audio_agc.h"

struct __stream_agc {
    struct __stream_entry  *stream;

    OS_MUTEX mutex; /*!< 混合器互斥量 */
    OS_SEM                  w_sem;
    OS_SEM                  sem;
    volatile u8             status;
    volatile u8             r_busy;
    volatile u8             w_busy;
    volatile u8             fill_flag;
    int                     data_len;
    u8                      *data;


    u16 skip_counter;
    u8 *in_buf;
    u8 *out_buf;
    int frame_buf_size;
    u8 *frame_buf;
    cbuffer_t in_cbuf;
    cbuffer_t out_cbuf;
    void *agc_hdl;

};

struct __stream_agc *adapter_stream_agc_open(agc_param_t *agc_param);
void adapter_stream_agc_close(struct __stream_agc **p);

#endif//__MIC_STREAM_AGC_H__




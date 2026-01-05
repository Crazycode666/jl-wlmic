/*************************************************************************************************/
/*!
*  \file      pcm_capture_buffer.c
*
*  \brief
*
*  Copyright (c) 2011-2023 ZhuHai Jieli Technology Co.,Ltd.
*
*/
/*************************************************************************************************/
#include "pcm_capture_buffer.h"

struct live_audio_capture_buffer {
    u8 remain;
    void *audio_buffer;
    void *wakeup_data;
    void (*wakeup)(void *);
    void *path;
    int (*write_frame)(void *path, struct audio_frame *frame);
};

void *live_audio_pcm_capture_buf_open(int size,
                                      void *wakeup_data,
                                      void (*wakeup)(void *),
                                      void *path,
                                      int (*write_frame)(void *path, struct audio_frame *frame))
{
    struct live_audio_capture_buffer *capture_buf = (struct live_audio_capture_buffer *)zalloc(sizeof(struct live_audio_capture_buffer));

    struct live_audio_buffer_params buf_params = {
        .overrun_wakeup_data = capture_buf,
        .overrun_wakeup = live_audio_pcm_capture_buf_wakeup,
        .size = size,
    };

    capture_buf->audio_buffer = live_audio_buffer_init(&buf_params);
    capture_buf->wakeup_data = wakeup_data;
    capture_buf->wakeup = wakeup;
    capture_buf->path = path;
    capture_buf->write_frame = write_frame;

    return capture_buf;
}

void live_audio_pcm_capture_buf_close(void *buffer)
{
    struct live_audio_capture_buffer *capture_buf = (struct live_audio_capture_buffer *)buffer;

    if (!capture_buf) {
        return;
    }
    if (capture_buf->audio_buffer) {
        live_audio_buffer_close(capture_buf->audio_buffer);
    }

    free(capture_buf);
}

void live_audio_pcm_capture_buf_wakeup(void *buffer)
{
    struct live_audio_capture_buffer *capture_buf = (struct live_audio_capture_buffer *)buffer;

    if (capture_buf->wakeup) {
        capture_buf->wakeup(capture_buf->wakeup_data);
    }
}

int live_audio_pcm_capture_write_frame(void *buffer, struct audio_frame *frame)
{
    struct live_audio_capture_buffer *capture_buf = (struct live_audio_capture_buffer *)buffer;
    int wlen = 0;

    if (!capture_buf->remain) {
        wlen = live_audio_buffer_push_frame(capture_buf->audio_buffer, frame);
        if (frame->offset + wlen < frame->len) {
            return wlen;
        }
    }

    if (capture_buf->write_frame) {
        wlen = capture_buf->write_frame(capture_buf->path, frame);
        frame->offset += wlen;
        capture_buf->remain = frame->offset == frame->len ? 0 : 1;
    }

    return wlen;
}

int live_audio_pcm_capture_buf_read(void *buffer, void *data, int len)
{
    struct live_audio_capture_buffer *capture_buf = (struct live_audio_capture_buffer *)buffer;

    if (capture_buf->audio_buffer) {
        return live_audio_buffer_read(capture_buf->audio_buffer, data, len);
    }
    return 0;
}

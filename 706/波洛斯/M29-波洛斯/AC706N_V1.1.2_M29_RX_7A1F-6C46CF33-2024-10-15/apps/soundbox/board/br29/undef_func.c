#include "system/includes.h"

void *get_repair_api(void)
{
    return NULL;
}

__attribute__((weak))
int audio_wireless_sync_stop(void *c)
{
    return 0;
}

__attribute__((weak))
int audio_wireless_sync_sound_reset(void *c, int time)
{
    return 0;
}

__attribute__((weak))
int audio_wireless_sync_set_tws_time(void *c, int time)
{
    return 0;
}

__attribute__((weak))
int audio_wireless_sync_suspend(void *c)
{
    return 0;
}

__attribute__((weak))
int audio_wireless_sync_drop_samples(void *c, int samples)
{
    return 0;
}

__attribute__((weak))
int audio_wireless_sync_with_stream(void *c, struct remote_stream_info *info)
{
    return 0;
}

__attribute__((weak))
void clr_wdt(void)
{

}

__attribute__((weak))
void adc_rtc_ch_select(u32 ch)
{

}

__attribute__((weak))
void adc_rtc_pd(u32 value)
{

}

__attribute__((weak))
void adc_rtc_detect_en(u32 on)
{

}

__attribute__((weak))
void gpio_longpress_pin1_reset_config(u32 pin, u32 level, u32 time)
{

}

__attribute__((weak))
int audio_adc_mic3_open(struct adc_mic_ch *mic, int ch, struct audio_adc_hdl *adc)
{
    return 0;
}

__attribute__((weak))
int audio_adc_mic3_set_gain(struct adc_mic_ch *mic, int gain)
{
    return 0;
}

__attribute__((weak))
int audio_adc_linein1_open(struct adc_linein_ch *linein, int ch, struct audio_adc_hdl *adc)
{
    return 0;
}

__attribute__((weak))
int audio_adc_linein2_open(struct adc_linein_ch *linein, int ch, struct audio_adc_hdl *adc)
{
    return 0;
}

__attribute__((weak))
int audio_adc_linein3_open(struct adc_linein_ch *linein, int ch, struct audio_adc_hdl *adc)
{
    return 0;
}

__attribute__((weak))
int audio_adc_linein1_set_gain(struct adc_linein_ch *linein, int gain)
{
    return 0;
}

__attribute__((weak))
int audio_adc_linein2_set_gain(struct adc_linein_ch *linein, int gain)
{
    return 0;
}

__attribute__((weak))
int audio_adc_linein3_set_gain(struct adc_linein_ch *linein, int gain)
{
    return 0;
}

__attribute__((weak))
u8 get_charge_online_flag(void)
{
    return 0;
}

__attribute__((weak))
void mic_trim_run(void)
{

}

__attribute__((weak))
u32 clock_get_pll_target_frequency()
{
    return 0;
}

__attribute__((weak))
int alink_get_tx_pns(void *hw_alink)
{
    return 0;
}

__attribute__((weak))
u32 bt_tws_master_slot_clk(void)
{
    return 0;
}


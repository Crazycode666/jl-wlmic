#ifndef _AUDIO_COMMON_H_
#define _AUDIO_COMMON_H_

#include "generic/typedef.h"
#include "system/includes.h"

/************************************
             dac时钟
************************************/
#define	AUDIO_COMMON_CLK_DIG_SINGLE			(0)
#define	AUDIO_COMMON_CLK_DIF_XOSC			(1)

#define	AUDIO_COMMON_VCM_CAP_LEVEL1		(0)	// VCM-cap, VCM = 1.2v
#define	AUDIO_COMMON_VCM_CAP_LEVEL2		(1) // VCM-cap, VCM = 1.3v
#define	AUDIO_COMMON_VCM_CAP_LEVEL3		(2)	// VCM-cap, VCM = 1.4v
#define	AUDIO_COMMON_VCM_CAP_LEVEL4		(3)	// VCM-cap, VCM = 1.5v
#define	AUDIO_COMMON_VCM_CAP_LEVEL5		(4)	// VCM-cap, VCM = 1.6v
#define	AUDIO_COMMON_DACLDO_CAPLESS_LEVEL1	(5)	// VCM-capless, DACLDO = 2.4v
#define	AUDIO_COMMON_DACLDO_CAPLESS_LEVEL2	(6)	// VCM-capless, DACLDO = 2.6v
#define	AUDIO_COMMON_DACLDO_CAPLESS_LEVEL3	(7)	// VCM-capless, DACLDO = 2.8v
#define	AUDIO_COMMON_DACLDO_CAPLESS_LEVEL4	(8)	// VCM-capless, DACLDO = 3.0v

typedef struct {
    u8 vbg_trim_value;
    u8 power_level;
    u8 vcm_cap_en;
} audio_common_power_open_t;

void audio_common_init(void *adc_data, void *dac_data);
void audio_common_dac_init(void *dac_data);
void audio_common_adc_init(void *adc_data);
void *audio_common_get_dac_data(void);
void *audio_common_get_adc_data(void);
void audio_common_clk_irq_init();

void audio_common_clock_open(u8 clk_mode);
void audio_common_clock_close(void);
void audio_common_power_open(audio_common_power_open_t *param);
void audio_common_power_close(void);

int audio_adc_digital_status_add_check(int add);
int audio_adc_analog_status_add_check(u8 ch_index, int add);
int audio_dac_digital_status_add_check(int add);
int audio_dac_analog_status_add_check(int add);

#endif // _AUDIO_COMMON_H_


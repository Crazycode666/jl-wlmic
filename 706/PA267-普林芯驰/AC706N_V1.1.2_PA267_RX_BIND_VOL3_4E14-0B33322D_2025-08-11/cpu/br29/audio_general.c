/*
 ****************************************************************
 *							AUDIO General APIs
 * Brief : 实现一些依赖CPU的通用接口，适配所有case
 * Notes :
 ****************************************************************
 */
#include "system/includes.h"
#include "media/includes.h"
#include "audio_config.h"

void audio_adda_dump(void) //打印所有的dac,adc寄存器
{
    printf("JL_WL_AUD CON0:%x", JL_WL_AUD->CON0);
    printf("AUD_CON:%x", JL_AUDIO->AUD_CON);
    printf("DAC_CON:%x", JL_AUDIO->DAC_CON);
    printf("ADC_CON:%x", JL_AUDIO->ADC_CON);
    printf("ADC_CON1:%x", JL_AUDIO->ADC_CON1);
    printf("DAC_TM0:%x", JL_AUDIO->DAC_TM0);
    printf("DAA_CON 0:%x 	1:%x,	2:%x,	3:%x    7:%x\n", JL_ADDA->DAA_CON0, JL_ADDA->DAA_CON1, JL_ADDA->DAA_CON2, JL_ADDA->DAA_CON3, JL_ADDA->DAA_CON7);
    printf("ADA_CON 0:%x	1:%x	2:%x	3:%x	4:%x\n", JL_ADDA->ADA_CON0, JL_ADDA->ADA_CON1, JL_ADDA->ADA_CON2, JL_ADDA->ADA_CON3, JL_ADDA->ADA_CON4);
}

void audio_gain_dump(void)
{
    int dac_again_max = 7;
    u8 dac_again_l = JL_ADDA->DAA_CON1 & 0xF;
    u8 dac_again_r = (JL_ADDA->DAA_CON1 >> 4) & 0xF;
    int dac_dgain_max = 16384;
    u32 dac_dgain_l = JL_AUDIO->DAC_VL0 & 0xFFFF;
    u32 dac_dgain_r = (JL_AUDIO->DAC_VL0 >> 16) & 0xFFFF;

    u8 mic0_0_6 = (JL_ADDA->ADA_CON0) & BIT(31);
    u8 mic0_gain = (JL_ADDA->ADA_CON1 >> 24) & 0x7;

    u8 dac_dcc = (JL_AUDIO->DAC_CON1 >> 24) & 0xf;
    u8 adc_dcc = JL_AUDIO->ADC_CON1 & 0xf;

    printf("L Mute:%d ,R Mute:%d\n", (JL_ADDA->DAA_CON2 >> 15) & 0x1, (JL_ADDA->DAA_CON2 >> 20) & 0x1); //PA MUTE
    printf("MIC_G:%d,MIC_6dB_EN:%d,DAC_AG_MAX:%d,DAC_AG:%d,%d,DAC_DG_MAX:%d,DAC_DG:%d,%d\n", mic0_gain, mic0_0_6, dac_again_max, dac_again_l, dac_again_r, dac_dgain_max, dac_dgain_l, dac_dgain_r);
    printf("DAC_DCC:%d ,ADC_DCC:%d\n", dac_dcc, adc_dcc);

}

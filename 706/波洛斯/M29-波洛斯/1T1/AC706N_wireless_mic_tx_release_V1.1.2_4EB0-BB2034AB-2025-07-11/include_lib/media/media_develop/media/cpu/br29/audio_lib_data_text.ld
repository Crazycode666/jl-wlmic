

        media_data_code_start = .;
        . = ALIGN(4);
	  	*(.media.aec.text)
        . = ALIGN(4);


 AudioEffects_data_code_begin = .;
#if AUDIO_EFFECTS_REVERB_AT_RAM
		*(.platereverb_code)
		*(.platereverb_const)
#endif/*AUDIO_EFFECTS_REVERB_AT_RAM*/

#if AUDIO_EFFECTS_AFC_AT_RAM
	    *(.howlings_phf_code)
		*(.howlings_phf_const)
		/*AFC_Lib_陷波*/
		*(.notchhowling_code)
		*(.notchhowling_const)
		*(.notchhowling_sparse_code)
		*(.notchhowling_sparse_const)
        . = ALIGN(4);
#endif/*AUDIO_EFFECTS_AFC_AT_RAM*/

#if AUDIO_EFFECTS_DRC_AT_RAM
		*(.audio_drc_code)
		*(.audio_drc_const)
		*(.drc_const)
		*(.drc_code)
	    *(.sw_drc_code)
		*(.sw_drc_const)
        . = ALIGN(4);
#endif/*AUDIO_EFFECTS_DRC_AT_RAM*/

#if AUDIO_EFFECTS_EQ_AT_RAM
		*(.audio_eq_code)
		*(.audio_eq_const)
        . = ALIGN(4);
#endif/*AUDIO_EFFECTS_EQ_AT_RAM*/

#if AUDIO_EFFECTS_NOISEGATE_AT_RAM
		*(.audio_noisegate_code)
		*(.audio_noisegate_const)
		/*Lib_NoiseGate*/
		*(.noisegate_code)
		*(.noisegate_const)
		*(.noisegate_sparse_code)
		*(.noisegate_sparse_const)
#endif/*AUDIO_EFFECTS_NOISEGATE_AT_RAM*/

#if AUDIO_EFFECTS_MIC_EFFECT_AT_RAM
		*(.audio_mic_effect_const)
		*(.audio_mic_effect_code)
#endif/*AUDIO_EFFECTS_MIC_EFFECT_AT_RAM*/

#if AUDIO_EFFECTS_MIC_STREAM_AT_RAM
		*(.audio_mic_stream_const)
		*(.audio_mic_stream_code)
#endif/*AUDIO_EFFECTS_MIC_STREAM_AT_RAM*/

#if AUDIO_EFFECTS_DEBUG_AT_RAM
		*(.audio_effect_adj_const)
		*(.audio_effect_adj_code)
        . = ALIGN(4);
#endif/*AUDIO_EFFECTS_DEBUG_AT_RAM*/

#if AUDIO_EFFECTS_DYNAMIC_EQ_AT_RAM
		*(.dynamic_eq_code)
		*(.dynamic_eq_const)
	    *(.dynamic_eq_detect_code)
		*(.dynamic_eq_detect_const)
        *(.dynamic_eq_sparse_code)
		*(.dynamic_eq_sparse_const)
	    *(.dynamic_eq_detect_sparse_code)
		*(.dynamic_eq_detect_sparse_const)
	    . = ALIGN(4);
#endif/*AUDIO_EFFECTS_DYNAMIC_EQ_AT_RAM*/
#if AUDIO_EFFECTS_VBASS_AT_RAM 
		*(.audio_vbass_code)
		*(.audio_vbass_const)
		*(.vbss_code)
		*(.vbss_const)
	    *(.vbss_sparse_code)
        *(.vbss_sparse_const)
	    . = ALIGN(4);
#endif/*AUDIO_EFFECTS_VBASS_AT_RAM*/

#if AUDIO_STREAM_AT_RAM 
		*(.stream_code)
		*(.stream_const)
#endif/*AUDIO_STREAM_AT_RAM*/

        AudioEffects_data_code_end = .;
        AudioEffects_data_code_size = AudioEffects_data_code_end - AudioEffects_data_code_begin;

#if (defined(TCFG_JLA_CODEC_AT_RAM_EN) && (TCFG_JLA_CODEC_AT_RAM_EN) && (WIERLESS_TRANS_CODING_TYPE == LIVE_AUDIO_CODING_JLA))
	    *(.jla_encoder_const)
	    *(.jla_encoder_code)
	    *(.jla_codec_e_code)
	    *(.jla_codec_c_const)
	    *(.jla_codec_c_code)
	    *(.jla_codec_ari_c_const)
	    *(.jla_codec_ari_c_code)
	    *(.jla_codec_t441k_code)
#endif

#if (WIERLESS_TRANS_CODING_TYPE == LIVE_AUDIO_CODING_JLA_LL)
	    *(.jla_ll_codec.text.cache.L2.encode)
	    *(.jla_ll_codec.text.cache.L2.const)
	    *(.jla_ll_codec.text.cache.L2.run)
        *(.jla_ll_const)
        *(.jla_ll_code)
#endif
		. = ALIGN(4);

        media_data_code_end = .;




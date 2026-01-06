#include "audio_effects.h"
#include "config/config_interface.h"
#include "application/eq_config.h"
#include "cfg_tool.h"
#include "generic/typedef.h"
#if TCFG_USER_TWS_ENABLE
#include "bt_tws.h"
#endif/*TCFG_USER_TWS_ENABLE*/


#define LOG_TAG_CONST EFFECTS
#define LOG_TAG     "[EFFECTS]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_DEBUG_ENABLE
#include "debug.h"

void tws_online_effects_send(EQ_CFG *eq_cfg, void *packet, u16 size);
void tws_online_effects_timer_check(void *priv);
static u32 check_cnt = 0;

extern const u8 audio_eq_ver[4];
struct file_head {
    unsigned short id;    //eq效果文件存储的标识
    unsigned short len;   //当前标识结构长度
};

void dyeq_prot_update_in_app_core(void *p)
{
    audio_effects_ultra_update_base(AEID_MUSIC_DY_EQ_PRO_DET);
}
//在线调音解析
s32 effects_online_update_new(void *priv, void *_packet, u16 size, u8 tws_tx)
{
    int res = 0;
    if (config_audio_eq_online_en) {
        EQ_ONLINE_PACKET *packet = _packet;
        EQ_CFG *eq_cfg = priv;
        struct eq_seg_info seg = {0};
        switch (packet->cmd) {
        case EQ_ONLINE_CMD_SONG_EQ_SEG:
        case EQ_ONLINE_CMD_SONG_EQ_V1_SEG:
            memcpy(&seg, &packet->data[1], sizeof(EQ_ONLINE_PARAMETER_SEG));
            if (seg.index == (u16)EQ_GLOBAL_GAIN_CMD) {
                float global_gain;
                memcpy(&global_gain, &seg.iir_type, sizeof(float));
                music_eq_tab.global_gain = global_gain;
                cur_eq_set_global_gain(AEID_MUSIC_EQ, global_gain);
                break;
            } else if (seg.index == (u16)EQ_BYPASS_CMD) {
                int bypass;
                memcpy(&bypass, &seg.iir_type, sizeof(int));
                music_eq_tab.is_bypass = bypass;
                cur_eq_set_bypass(AEID_MUSIC_EQ, bypass);
                break;
            } else {
                if (seg.index >= music_eq_tab.seg_num) {
                    log_error("index:%d max_index:%d cmd:%d", seg.index, music_eq_tab.seg_num, packet->cmd);
                    break;
                }
                memcpy(&music_eq_tab.seg[seg.index], &seg, sizeof(struct eq_seg_info));
                log_debug("eq idx:%d, iir:%d, freq:%d, gain:%d, gain x10:%d, q:%d , q x10:%d\n", seg.index, seg.iir_type, seg.freq, seg.gain, (10 * seg.gain) >> 20, seg.q, (10 * seg.q) >> 24);
                cur_eq_set_update(AEID_MUSIC_EQ, &seg, music_eq_tab.seg_num);
            }
            break;

        case EQ_ONLINE_CMD_CALL_EQ_SEG:
        case EQ_ONLINE_CMD_CALL_EQ_V1_SEG:
#if TCFG_PHONE_EQ_ENABLE
            memcpy(&seg, &packet->data[1], sizeof(EQ_ONLINE_PARAMETER_SEG));
            if (seg.index == (u16)EQ_GLOBAL_GAIN_CMD) {
                float global_gain;
                memcpy(&global_gain, &seg.iir_type, sizeof(float));
                dl_eq_tab.global_gain = global_gain;
                cur_eq_set_global_gain(AEID_DL_EQ, global_gain);
                break;
            } else if (seg.index == (u16)EQ_BYPASS_CMD) {
                int bypass;
                memcpy(&bypass, &seg.iir_type, sizeof(int));
                dl_eq_tab.is_bypass = bypass;
                cur_eq_set_bypass(AEID_DL_EQ, bypass);
                break;
            } else {
                if (seg.index >= dl_eq_tab.seg_num) {
                    log_error("index:%d max_index:%d cmd:%d", seg.index, dl_eq_tab.seg_num, packet->cmd);
                    break;
                }
                memcpy(&dl_eq_tab.seg[seg.index], &seg, sizeof(struct eq_seg_info));
                log_debug("eq idx:%d, iir:%d, freq:%d, gain:%d, gain x10:%d, q:%d , q x10:%d\n", seg.index, seg.iir_type, seg.freq, seg.gain, (10 * seg.gain) >> 20, seg.q, (10 * seg.q) >> 24);
                cur_eq_set_update(AEID_DL_EQ, &seg, dl_eq_tab.seg_num);
            }
#endif
            break;

        case EQ_ONLINE_CMD_AEC_EQ_SEG:
        case EQ_ONLINE_CMD_AEC_EQ_V1_SEG:
#if TCFG_AEC_UL_EQ_ENABLE
            memcpy(&seg, &packet->data[1], sizeof(EQ_ONLINE_PARAMETER_SEG));
            if (seg.index == (u16)EQ_GLOBAL_GAIN_CMD) {
                float global_gain;
                memcpy(&global_gain, &seg.iir_type, sizeof(float));
                ul_eq_tab.global_gain = global_gain;
                cur_eq_set_global_gain(AEID_UL_EQ, global_gain);
                break;
            } else if (seg.index == (u16)EQ_BYPASS_CMD) {
                int bypass;
                memcpy(&bypass, &seg.iir_type, sizeof(int));
                ul_eq_tab.is_bypass = bypass;
                cur_eq_set_bypass(AEID_UL_EQ, bypass);
                break;
            } else {
                if (seg.index >= ul_eq_tab.seg_num) {
                    log_error("index:%d max_index:%d cmd:%d", seg.index, ul_eq_tab.seg_num, packet->cmd);
                    break;
                }

                memcpy(&ul_eq_tab.seg[seg.index], &seg, sizeof(struct eq_seg_info));
                log_debug("eq idx:%d, iir:%d, freq:%d, gain:%d, gain x10:%d, q:%d , q x10:%d\n", seg.index, seg.iir_type, seg.freq, seg.gain, (10 * seg.gain) >> 20, seg.q, (10 * seg.q) >> 24);
                cur_eq_set_update(AEID_UL_EQ, &seg, ul_eq_tab.seg_num);
            }
#endif
            break;
        case ONLINE_CMD_MUSIC_POST_EQ:
#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  != EFFECTS_BASE)
            memcpy(&seg, &packet->data[1], sizeof(EQ_ONLINE_PARAMETER_SEG));
            if (seg.index == (u16)EQ_GLOBAL_GAIN_CMD) {
                float global_gain;
                memcpy(&global_gain, &seg.iir_type, sizeof(float));
                post_eq_tab.global_gain = global_gain;
                cur_eq_set_global_gain(AEID_MUSIC_POST_EQ, global_gain);
                break;
            } else if (seg.index == (u16)EQ_BYPASS_CMD) {
                int bypass;
                memcpy(&bypass, &seg.iir_type, sizeof(int));
                post_eq_tab.is_bypass = bypass;
                cur_eq_set_bypass(AEID_MUSIC_POST_EQ, bypass);
                break;
            } else {
                if (seg.index >= post_eq_tab.seg_num) {
                    log_error("index:%d max_index:%d cmd:%d", seg.index, post_eq_tab.seg_num, packet->cmd);
                    break;
                }

                memcpy(&post_eq_tab.seg[seg.index], &seg, sizeof(struct eq_seg_info));
                log_debug("eq idx:%d, iir:%d, freq:%d, gain:%d, gain x10:%d, q:%d , q x10:%d\n", seg.index, seg.iir_type, seg.freq, seg.gain, (10 * seg.gain) >> 20, seg.q, (10 * seg.q) >> 24);
                cur_eq_set_update(AEID_MUSIC_POST_EQ, &seg, post_eq_tab.seg_num);
            }
#endif

            break;
        case EQ_ONLINE_CMD_SONG_DRC:

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_BASE)&& TCFG_BT_MUSIC_DRC_ENABLE
            struct audio_drc *shdl = get_cur_drc_hdl_by_name(AEID_MUSIC_DRC);
            memcpy(&music_drc_param, &packet->data[1], sizeof(struct drc_ch_org));
            if (shdl) {
                audio_drc_update_parm(shdl, (struct drc_ch *)&packet->data[1]);
            }
#endif
            break;
        case EQ_ONLINE_CMD_SONG_WDRC:

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_BASE)&& TCFG_BT_MUSIC_DRC_ENABLE
            struct audio_drc *drc = get_cur_drc_hdl_by_name(AEID_MUSIC_DRC);
            memcpy(&music_drc_param, &packet->data[1], sizeof(struct drc_ch));
            if (drc) {
                audio_drc_update_parm(drc, (struct drc_ch *)&packet->data[1]);
            }
#endif
            break;
        case ONLINE_CMD_MUSIC_VIRTUAL_BASS:

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  != EFFECTS_BASE)
            memcpy(&virtual_bass_param, &packet->data[1], sizeof(virtual_bass_param_tool_set));
            music_virtual_bass_update(AEID_MUSIC_VIRTUAL_BASS, (virtual_bass_param_tool_set *)&packet->data[1]);
#endif
            break;
        case ONLINE_CMD_MUSIC_DYNAMIC_EQ_PRO_DET:

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_ULTRA) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_ULTRA)
            struct dynamic_eq_pro *dy_eq = get_cur_dy_eq_pro_hdl_by_name(AEID_MUSIC_DY_EQ_PRO_DET);
            memcpy(&dyeq_pro_param, &packet->data[1], sizeof(dynamic_eq_pro_param_tool_set));
            dynamic_eq_pro_ext_det_printf(&dyeq_pro_param);
            if (dy_eq) {
                if (cpu_in_irq() || cpu_irq_disabled()) {
                    sys_timeout_add(NULL, dyeq_prot_update_in_app_core, 10);
                } else {
                    music_dynamic_eq_pro_det_update(AEID_MUSIC_DY_EQ_PRO_DET, (dynamic_eq_pro_param_tool_set *)&packet->data[1]);
                }
            }
#endif
            break;
        case ONLINE_CMD_MUSIC_MBLIMITER:
#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  != EFFECTS_BASE)
            memcpy(&mblimiter_param, &packet->data[1], sizeof(mblimiter_param));
            music_multiband_limiter_update(AEID_MUSIC_MBLIMITER, (struct multiband_limiter_param_tool_set *)&packet->data[1]);
#endif
            break;
        default:
            res = -EINVAL;
            break;
        }
        /* mem_stats(); */
#if TCFG_USER_TWS_ENABLE
        if (packet->cmd >= EQ_ONLINE_CMD_SONG_EQ_SEG) {
            if (tws_tx) {
                tws_online_effects_send(eq_cfg, _packet, size);//在线调音参数转发
            }
        }

        if (tws_tx) {
            int state = tws_api_get_tws_state();
            if (state & TWS_STA_SIBLING_CONNECTED) {//tws 连接上时，同步一次参数
                if (!eq_cfg->tws_tmr) {
                    check_cnt = 0;
                    eq_cfg->tws_tmr = sys_timer_add(NULL, tws_online_effects_timer_check, 500);
                }
            } else {
                if (eq_cfg->tws_tmr) {
                    check_cnt = 0;
                    sys_timer_del(eq_cfg->tws_tmr);
                    eq_cfg->tws_tmr = 0;
                }
            }
        }
#endif
    }

    return res;
}

#if TCFG_USE_EQ_FILE || EQ_FILE_CP_TO_CUSTOM
//获取文件内配置参
void copy_effects_bin_param_to_module(EQ_CFG *eq_cfg, u32 id, void *data, int len)
{
    switch (id) {
    case EQ_ONLINE_CMD_SONG_EQ_SEG:
    case EQ_ONLINE_CMD_SONG_EQ_V1_SEG:
        if (len <= sizeof(music_eq_tab)) {
            memcpy(&music_eq_tab, data, len);
            eq_printf(data, id);
        } else {
            eq_cfg->eq_file_section_err = 1;
            log_error("music_eq tab len error %d %d\n", len, sizeof(music_eq_tab));
        }
        break;
    case EQ_ONLINE_CMD_CALL_EQ_SEG:
    case EQ_ONLINE_CMD_CALL_EQ_V1_SEG:
#if TCFG_PHONE_EQ_ENABLE
        if (len <= sizeof(dl_eq_tab)) {
            memcpy(&dl_eq_tab, data, len);
            eq_printf(data, id);
        } else {
            eq_cfg->eq_file_section_err = 1;
            log_error("dl_eq tab len error %d %d\n", len, sizeof(dl_eq_tab));
        }
#endif
        break;
    case EQ_ONLINE_CMD_AEC_EQ_SEG:
    case EQ_ONLINE_CMD_AEC_EQ_V1_SEG:
#if TCFG_AEC_UL_EQ_ENABLE
        if (len <= sizeof(ul_eq_tab)) {
            memcpy(&ul_eq_tab, data, len);
            eq_printf(data, id);
        } else {
            eq_cfg->eq_file_section_err = 1;
            log_error("dl_eq tab len error %d %d\n", len, sizeof(ul_eq_tab));
        }
#endif
        break;
    case ONLINE_CMD_MUSIC_POST_EQ:
#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  != EFFECTS_BASE)
        if (len <= sizeof(post_eq_tab)) {
            memcpy(&post_eq_tab, data, len);
            eq_printf(data, id);
        } else {
            eq_cfg->eq_file_section_err = 1;
            log_error("post_eq tab len error %d %d\n", len, sizeof(post_eq_tab));
        }
#endif
        break;
    case EQ_ONLINE_CMD_SONG_DRC:
#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_BASE)&& TCFG_BT_MUSIC_DRC_ENABLE
        if (len <= sizeof(music_drc_param)) {
            memcpy(&music_drc_param, data, len);
        } else {
            log_error("music drc len error %d %d\n", len, sizeof(music_drc_param));
        }
#endif
        break;
    case EQ_ONLINE_CMD_SONG_WDRC:

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_BASE)&& TCFG_BT_MUSIC_DRC_ENABLE
        if (len <= sizeof(music_drc_param)) {
            memcpy(&music_drc_param, data, len);
        } else {
            log_error("music wdrc len error %d %d\n", len, sizeof(music_drc_param));
        }
#endif
        break;
    case ONLINE_CMD_MUSIC_VIRTUAL_BASS:
#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  != EFFECTS_BASE)
        if (len <= sizeof(virtual_bass_param)) {
            memcpy(&virtual_bass_param, data, len);
            vbass_printf(&virtual_bass_param);
        } else {
            log_error("virtual bass len error %d %d\n", len, sizeof(virtual_bass_param));
        }
#endif
        break;
    case ONLINE_CMD_MUSIC_DYNAMIC_EQ_PRO_DET:
#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_ULTRA) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_ULTRA)
        if (len <= sizeof(dyeq_pro_param)) {
            memcpy(&dyeq_pro_param, data, len);
            dynamic_eq_pro_ext_det_printf(&dyeq_pro_param);
        } else {
            log_error("dyeq pro det len error %d %d\n", len, sizeof(dyeq_pro_param));
        }
#endif
        break;

    case ONLINE_CMD_MUSIC_MBLIMITER:
#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  != EFFECTS_BASE)
        if (len <= sizeof(mblimiter_param)) {
            memcpy(&mblimiter_param, data, len);
            log_debug("is_bypass %d", mblimiter_param.common_param.is_bypass);
            log_debug("way_num %d", mblimiter_param.common_param.way_num);
            log_debug("order %d", mblimiter_param.common_param.order);
            log_debug("freq %d", mblimiter_param.common_param.low_freq);
            for (int i = 0; i < 3; i++) {
                log_debug("======================= %d\n", i);
                log_debug("%d\n", mblimiter_param.parm[i].is_bypass);
                log_debug("attack_time %d\n", mblimiter_param.parm[i].parm.attack_time);
                log_debug("release_time %d\n", mblimiter_param.parm[i].parm.release_time);
                log_debug("threshold %d\n", mblimiter_param.parm[i].parm.threshold);
                log_debug("hold_time %d\n", mblimiter_param.parm[i].parm.detect_time);
                log_debug("detect_time %d\n", mblimiter_param.parm[i].parm.hold_time);
                log_debug("input_gain %d\n", mblimiter_param.parm[i].parm.input_gain);
                log_debug("output_gain %d\n", mblimiter_param.parm[i].parm.output_gain);
                log_debug("precision%d\n", mblimiter_param.parm[i].parm.precision);
            }
        } else {
            log_error("mblimiter len error %d %d\n", len, sizeof(mblimiter_param));
        }
#endif
        break;
    default:
        log_debug("effects_id :%x null\n", id);
        break;
    }
}
#endif

#if TCFG_USE_EQ_FILE || EQ_FILE_CP_TO_CUSTOM
s32 eq_file_get_cfg(EQ_CFG *eq_cfg, u8 *path)
{
    int ret = 0;
    u16 crc16 = 0;
    FILE *file = NULL;
    u8 *file_data = NULL;
    u8 *head_buf = NULL;

    if (eq_cfg == NULL) {
        return  -EINVAL;
    }
    eq_tool_cfg *eq_tool_tab = eq_cfg->eq_tool_tab;
    log_debug(" %s\n", path);
    file = fopen((const char *)path, "r");
    if (file == NULL) {
        log_error("eq file open err\n");
        return  -ENOENT;
    }

    u8 fmt = 0;
    if (1 != fread(file, &fmt, 1)) {
        ret = -EIO;
        goto err_exit;
    }

    // eq ver
    u8 ver[4] = {0};
    if (4 != fread(file, ver, 4)) {
        ret = -EIO;
        goto err_exit;
    }

    if (memcmp(ver, audio_eq_ver, sizeof(audio_eq_ver))) {
        log_error("==========Eeffects ver err ===========, Check Check Check\n");
        put_buf((unsigned char *)ver, 4);
        fseek(file, 0, SEEK_SET);
        eq_cfg->eq_file_ver_err = 1;
        ret = -EIO;
        goto err_exit;
    }

    unsigned short mode_seq = 0;
    unsigned short mode_len = 0;
    u8 mode = 0;
    u8 mode_cnt = 0;

__next_mode:
    if (sizeof(unsigned short) != fread(file, &mode_seq, sizeof(unsigned short))) {
        ret = 0;
        goto err_exit;
    }
    if (sizeof(unsigned short) != fread(file, &mode_len, sizeof(unsigned short))) {
        ret = 0;
        goto err_exit;
    }
    int mode_start = fpos(file);
    int i = 0;
    for (i = 0; i < eq_cfg->mode_num; i++) {
        log_debug("check modes[%d] %x %x\n", eq_tool_tab[i].mode_index, eq_tool_tab[i].mode_seq, mode_seq);
        if (eq_tool_tab[i].mode_seq == mode_seq) { //识别当前读到模式的序号
            mode = eq_tool_tab[i].mode_index;
            break;
        }
    }
    if (i == eq_cfg->mode_num) {
        if (mode_seq == 0xFFFF) {
            ret = 0;
        } else {
            ret = -EIO;
        }

        goto err_exit;
    }

    while (1) {
        //read crc16
        if (sizeof(unsigned short) != fread(file, &crc16, sizeof(unsigned short))) {
            ret = 0;
            break;
        }
        int pos = fpos(file);
        //read id len
        struct file_head *file_h;
        int head_size = sizeof(struct file_head);
        head_buf = malloc(head_size);
        if (sizeof(struct file_head) != fread(file, head_buf, sizeof(struct file_head))) {
            ret = -EIO;
            break;
        }
        file_h = (struct file_head *)head_buf;

        if ((file_h->id >= EQ_ONLINE_CMD_SONG_EQ_SEG) && (file_h->id < EQ_ONLINE_CMD_MAX)) {
            fseek(file, pos, SEEK_SET);
            int data_size = file_h->len + sizeof(struct file_head);
            file_data = malloc(data_size);
            if (file_data == NULL) {
                ret = -ENOMEM;
                break;
            }
            if (data_size != fread(file, file_data, data_size)) {
                ret = -EIO;
                break;
            }
            if (crc16 == CRC16(file_data, data_size)) {
                //获取文件内配置参
                copy_effects_bin_param_to_module(eq_cfg, file_h->id, (void *)&file_data[4], data_size - sizeof(struct file_head));
            } else {
                log_error("======effects cfg_info crc16 err=====\n");
                ret = -ENOEXEC;
                goto err_exit;
            }

            free(head_buf);
            head_buf = NULL;

            free(file_data);
            file_data = NULL;
        }

        int mode_end = fpos(file);
        if ((mode_end - mode_start) == mode_len) {
            goto __next_mode;
        }
    }

err_exit:
    if (head_buf) {
        free(head_buf);
        head_buf = NULL;
    }

    if (file_data) {
        free(file_data);
        file_data = NULL;
    }
    fclose(file);
    if (ret == 0) {
        log_debug("==========cfg_info ok ===========\n");
    } else {
        log_error("==========cfg_info err=========== \n");
    }
    return ret;
}
#endif

#if TCFG_USER_TWS_ENABLE

#define TWS_FUNC_ID_EFFECTS_SYNC \
	((int)(('A' + '2' + 'D' + 'P') << (2 * 8)) | \
	 (int)(('E' + 'F' + 'F' + 'E'+'C'+'T'+'S') << (1 * 8)) | \
	 (int)(('S' + 'Y' + 'N' + 'C') << (0 * 8)))

void tws_online_effects_send(EQ_CFG *eq_cfg, void *packet, u16 size)
{
    if (config_audio_eq_online_en) {
        if (!eq_cfg) {
            return ;
        }
        int state = tws_api_get_tws_state();
        if (state & TWS_STA_SIBLING_CONNECTED) {
            tws_api_send_data_to_sibling((u8 *)packet, size, TWS_FUNC_ID_EFFECTS_SYNC);
        }
    }
}
/*
 *在线调试效果同步回调
 * */
static void tws_online_effects_align(void *data, u16 len, bool rx)
{
    if (config_audio_eq_online_en) {
        if (rx) {
            EQ_CFG *eq_cfg = get_eq_cfg_hdl();
            effects_online_update_new(eq_cfg, data, len, 0);
        }
    }
}

REGISTER_TWS_FUNC_STUB(effects_adj_align) = {
    .func_id = TWS_FUNC_ID_EFFECTS_SYNC,
    .func    = tws_online_effects_align,
};

//tws音效参数对齐检测
#define TWS_FUNC_ID_EFFECTS_SYNC_CHECK \
	((int)(('A' + '2' + 'D' + 'P') << (2 * 8)) | \
	 (int)(('E' + 'F' + 'F' + 'E'+'C'+'T'+'S') << (1 * 8)) | \
	 (int)(('C' + 'H' + 'E' + 'C'+ 'K') << (0 * 8)))




void tws_online_effects_send_check(void *packet, u16 size)
{
    if (config_audio_eq_online_en) {
        int state = tws_api_get_tws_state();
        if (state & TWS_STA_SIBLING_CONNECTED) {
            tws_api_send_data_to_sibling((u8 *)packet, size, TWS_FUNC_ID_EFFECTS_SYNC);
        }
    }
}

struct eff_packet {
    u32 id;
    u32 len;
    u8 data[0];
};
struct eff_packet_check {
    u32 id;
    u32 len;
    u8 *data;
};
static struct eff_packet_check tab[] = {

    {AEID_MUSIC_EQ, sizeof(music_eq_tab), (u8 *) &music_eq_tab},

#if TCFG_AEC_UL_EQ_ENABLE
    {AEID_UL_EQ, sizeof(ul_eq_tab), (u8 *) &ul_eq_tab},
#endif

#if TCFG_PHONE_EQ_ENABLE
    {AEID_DL_EQ, sizeof(dl_eq_tab), (u8 *) &dl_eq_tab},
#endif

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  != EFFECTS_BASE)
    {AEID_MUSIC_POST_EQ, sizeof(post_eq_tab), (u8 *) &post_eq_tab},
    {AEID_MUSIC_MBLIMITER, sizeof(mblimiter_param), (u8 *) &mblimiter_param},
#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_ULTRA) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_ULTRA)
    {AEID_MUSIC_DY_EQ_PRO_DET, sizeof(dyeq_pro_param), (u8 *) &dyeq_pro_param},
#endif
    {AEID_MUSIC_VIRTUAL_BASS, sizeof(virtual_bass_param), (u8 *) &virtual_bass_param},
#endif

#if defined(TCFG_MUSIC_EFFECTS_EN)&& defined(EFFECTS_BASE) && (TCFG_MUSIC_EFFECTS_EN  == EFFECTS_BASE)&& TCFG_BT_MUSIC_DRC_ENABLE
    {AEID_MUSIC_DRC, sizeof(struct drc_ch_org), (u8 *) &music_drc_param},
#endif
};

void tws_online_effects_timer_check(void *priv)
{
    if (config_audio_eq_online_en) {
        if (check_cnt < ARRAY_SIZE(tab)) {
            int i = check_cnt;
            int len = 8 + tab[i].len;
            int *packet = malloc(len);
            if (packet) {
                packet[0] = tab[i].id;
                packet[1] = tab[i].len;
                memcpy(&packet[2], tab[i].data, tab[i].len);
                int state = tws_api_get_tws_state();
                if (state & TWS_STA_SIBLING_CONNECTED) {
                    tws_api_send_data_to_sibling((u8 *)packet, len, TWS_FUNC_ID_EFFECTS_SYNC_CHECK);
                }
                free(packet);
            }
            check_cnt++;
        }
    }
}

/*
 *在线调试效果同步回调
 * */
static void tws_online_effects_align_check(void *data, u16 len, bool rx)
{
    if (config_audio_eq_online_en) {
        if (rx) {
            struct eff_packet *packet = malloc(len);
            if (!packet) {
                return;
            }
            memcpy(packet, data, len);
            //printf("==============twx rx online param check========\n");
            for (int i = 0; i < ARRAY_SIZE(tab); i++) {
                if ((packet->id == tab[i].id) && memcmp(packet->data, tab[i].data, packet->len)) {
                    memcpy(tab[i].data, packet->data, packet->len);
                    audio_effects_ultra_update_base(tab[i].id);
                    break;
                }
            }
            free(packet);
        }
    }
}

REGISTER_TWS_FUNC_STUB(effects_align_check) = {
    .func_id = TWS_FUNC_ID_EFFECTS_SYNC_CHECK,
    .func    = tws_online_effects_align_check,
};
#endif /* TCFG_USER_TWS_ENABLE */

static void ci_send_packet_new(EQ_CFG *eq_cfg, u32 id, u8 *packet, int size)
{
    if (config_audio_eq_online_en) {
        ASSERT(eq_cfg);
        if (eq_cfg->send_cmd) {
            eq_cfg->send_cmd(eq_cfg->priv, id, packet, size);
        }
    }
}

//eq类型表,与eq_tool_tab使用的eq类型关联
u16 eq_cmd_tool_tab[] = {EQ_ONLINE_CMD_CALL_EQ_SEG, EQ_ONLINE_CMD_CALL_EQ_SEG, EQ_ONLINE_CMD_AEC_EQ_SEG, EQ_ONLINE_CMD_AEC_EQ_SEG, EQ_ONLINE_CMD_SONG_EQ_SEG, ONLINE_CMD_MUSIC_POST_EQ};
int eq_tool_get_eq_group_count(EQ_ONLINE_PACKET *packet)
{
    if (config_audio_eq_online_en) {
        EQ_CFG *eq_cfg = get_eq_cfg_hdl();
        u32 eq_group_num = ARRAY_SIZE(eq_cmd_tool_tab);
        ci_send_packet_new(eq_cfg, EQ_CONFIG_ID, (u8 *)&eq_group_num, sizeof(u32));
    }

    return 0;
}
/*----------------------------------------------------------------------------*/
/**@brief    工具获取小机本地支持的模式的id
   @param    *_packet:收到的数据包
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
int eq_tool_get_eq_group_range(EQ_ONLINE_PACKET *packet)
{
    if (config_audio_eq_online_en) {
        EQ_CFG *eq_cfg = get_eq_cfg_hdl();
        eq_tool_cfg *eq_tool_tab = eq_cfg->eq_tool_tab;
        struct cmd {
            int id;
            int offset;
            int count;
        };
        struct cmd cmd;
        memcpy(&cmd, packet, sizeof(struct cmd));

        u16 g_id[32];
        u16 groups_cnt[32];
        log_debug("cmd.offset:%d  cmd.count:%d \n", cmd.offset, cmd.count);
        for (int i = 0; i < cmd.count; i++) {
            groups_cnt[i] = eq_cmd_tool_tab[i];
            log_debug("groups_cnt[%d] %x", i, groups_cnt[i]);
        }
        memcpy(g_id, &groups_cnt[cmd.offset], cmd.count * sizeof(u16));
        ci_send_packet_new(eq_cfg, EQ_CONFIG_ID, (u8 *)&g_id[cmd.offset], cmd.count * sizeof(u16));
    }
    return 0;
}

#if EQ_FILE_CP_TO_CUSTOM
void cp_eq_file_seg_to_custom_tab()
{
    u8 nsection = music_eq_tab.seg_num;
    struct eq_seg_info *seg = (struct eq_seg_info *)eq_type_tab[EQ_MODE_CUSTOM];
    if (nsection > eq_get_table_nsection(EQ_MODE_CUSTOM)) {
        log_error("music nsection:%d > custom nsection:%d\n", nsection, eq_get_table_nsection(EQ_MODE_CUSTOM));
        return ;
    }
    global_gain_tab[EQ_MODE_CUSTOM] = music_eq_tab.global_gain;
    memcpy(seg, music_eq_tab.seg, sizeof(struct eq_seg_info)*nsection);
}
#endif

static u8 reply_to_tool = 0;
static u8 reply_sq = 0;
/*
 *新调音回调
 * */
static void eq_online_callback_new_protocol(uint8_t *packet, u32 size)
{
    u8 *ptr = packet;
    reply_sq = ptr[1];
    reply_to_tool =  0;
    u8 *new_packet = (void *)&packet[2];
    EQ_CFG *eq_cfg = get_eq_cfg_hdl();
    if (eq_cfg && !eq_cfg->app) { //spp调音时，过滤串口调音
        eq_online_callback(new_packet, size);
    }
}

//新调音注册
REGISTER_DETECT_TARGET(eq_adj_target) = {
    .id         = EQ_CONFIG_ID,
    .tool_message_deal   = eq_online_callback_new_protocol,
};
/*
 *spp调音回调
 * */
static void eq_online_callback_spp(uint8_t *packet, u32 size)
{
    ASSERT(((int)packet & 0x3) == 0, "buf %x size %d\n", (int)packet, size);
    eq_online_callback(packet, size);
}

/*手机app 在线调时，数据解析的回调*/
int eq_app_online_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size)
{
    if (config_audio_eq_online_en) {
        EQ_CFG *eq_cfg = get_eq_cfg_hdl();
        ASSERT(eq_cfg);
        if (eq_cfg->online_en) {
            eq_cfg->parse_seq = ext_data[1];
            eq_online_callback_spp(packet, size);
        } else {
            log_debug("EQ_ONLINE,not enable!\n");
        }
    }
    return 0;
}

/*
 *在线调试，应答接口
 * */
int ci_send_cmd(void *priv, u32 id, u8 *packet, int size)
{
    EQ_CFG *eq_cfg = (EQ_CFG *)priv;
    ASSERT(eq_cfg);
    if (eq_cfg->app) {
#if defined(APP_ONLINE_DEBUG) && APP_ONLINE_DEBUG
        if (EQ_CONFIG_ID == id) {
            app_online_db_ack(eq_cfg->parse_seq, packet, size);
        }
#endif
    } else {
#if TCFG_ONLINE_ENABLE
        if (get_uart_protocol()) {
            all_assemble_package_send_to_pc(reply_to_tool, reply_sq, packet, size);
        } else {
            ci_send_packet(id, packet, size);
        }
#endif
    }
    return 0;
}

//EQ在线调试不进power down
static u8 eq_online_idle_query(void)
{
    if (config_audio_eq_online_en) {
        EQ_CFG *eq_cfg = get_eq_cfg_hdl();
        if (!eq_cfg) {
            return 1;
        }
        if (eq_cfg->online_en) {
            return 0;
        }
    }
    return 1;
}

REGISTER_LP_TARGET(eq_online_lp_target) = {
    .name = "eq_online",
    .is_idle = eq_online_idle_query,
};


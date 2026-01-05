
# make 编译并下载
# make VERBOSE=1 显示编译详细过程
# make clean 清除编译临时文件
#
# 注意： Linux 下编译方式：
#     1. 从 http://pkgman.jieliapp.com/doc/all 处找到下载链接
#     2. 下载后，解压到 /opt/jieli 目录下，保证
#       /opt/jieli/common/bin/clang 存在（注意目录层次）
#     3. 确认 ulimit -n 的结果足够大（建议大于8096），否则链接可能会因为打开文件太多而失败
#       可以通过 ulimit -n 8096 来设置一个较大的值
#

# 工具路径设置
ifeq ($(OS), Windows_NT)
# Windows 下工具链位置
TOOL_DIR := C:/JL/pi32/bin
CC    := clang.exe
CXX   := clang.exe
LD    := pi32v2-lto-wrapper.exe
AR    := llvm-ar.exe
MKDIR := mkdir_win -p
RM    := rm -rf

SYS_LIB_DIR := C:/JL/pi32/pi32v2-lib/r5
SYS_INC_DIR := C:/JL/pi32/pi32v2-include
EXT_CFLAGS  := # Windows 下不需要 -D__SHELL__
export PATH:=$(TOOL_DIR);$(PATH)

## 后处理脚本
FIXBAT          := tools\utils\fixbat.exe # 用于处理 utf8->gbk 编码问题
POST_SCRIPT     := cpu/br29/tools/download.bat
RUN_POST_SCRIPT := cpu\br29\tools\download.bat
else
# Linux 下工具链位置
TOOL_DIR := /opt/jieli/pi32v2/bin
CC    := clang
CXX   := clang
LD    := lto-wrapper
AR    := lto-ar
MKDIR := mkdir -p
RM    := rm -rf
export OBJDUMP := $(TOOL_DIR)/objdump
export OBJCOPY := $(TOOL_DIR)/objcopy
export OBJSIZEDUMP := $(TOOL_DIR)/objsizedump

SYS_LIB_DIR := $(TOOL_DIR)/../lib
SYS_INC_DIR := $(TOOL_DIR)/../include
EXT_CFLAGS  := -D__SHELL__ # Linux 下需要这个保证正确处理 download.c
export PATH:=$(TOOL_DIR):$(PATH)

## 后处理脚本
FIXBAT          := touch # Linux下不需要处理 bat 编码问题
POST_SCRIPT     := cpu/br29/tools/download.sh
RUN_POST_SCRIPT := bash $(POST_SCRIPT)
endif

CC  := $(TOOL_DIR)/$(CC)
CXX := $(TOOL_DIR)/$(CXX)
LD  := $(TOOL_DIR)/$(LD)
AR  := $(TOOL_DIR)/$(AR)
# 输出文件设置
OUT_ELF   := cpu/br29/tools/sdk.elf
OBJ_FILE  := $(OUT_ELF).objs.txt
# 编译路径设置
BUILD_DIR := objs

# 编译参数设置
CFLAGS := \
	-target pi32v2 \
	-mcpu=r5 \
	-integrated-as \
	-flto \
	-Wuninitialized \
	-Wno-invalid-noreturn \
	-fno-common \
	-integrated-as \
	-Oz \
	-g \
	-flto \
	-fallow-pointer-null \
	-fprefer-gnu-section \
	-Wno-shift-negative-value \
	-Wundef \
	-Wframe-larger-than=256 \
	-Wincompatible-pointer-types \
	-Wreturn-type \
	-Wimplicit-function-declaration \
	-fms-extensions \
	-Werror=return-type \
	-Werror=undef \
	-Werror=implicit-function-declaration \
	-w \


# C++额外的编译参数
CXXFLAGS :=


# 宏定义
DEFINES := \
	-DSUPPORT_MS_EXTENSIONS \
	-DCONFIG_RELEASE_ENABLE \
	-DCONFIG_CPU_BR29 \
	-DCONFIG_LZ4_DATA_CODE_ENABLE \
	-DCONFIG_NEW_BREDR_ENABLE \
	-DCONFIG_NEW_MODEM_ENABLE \
	-DCONFIG_NEW_TWS_FORWARD_ENABLE \
	-DCONFIG_UCOS_ENABLE \
	-DCONFIG_EQ_SUPPORT_ASYNC \
	-DEQ_CORE_V1 \
	-DCONFIG_AAC_CODEC_FFT_USE_MUTEX \
	-DCONFIG_DNS_ENABLE \
	-DCONFIG_DMS_MALLOC \
	-DCONFIG_MMU_ENABLE \
	-DCONFIG_OS_IN_MASKROM \
	-DCONFIG_SBC_CODEC_HW \
	-DCONFIG_MSBC_CODEC_HW \
	-DCONFIG_AEC_M=256 \
	-DCONFIG_AUDIO_ONCHIP \
	-DCONFIG_MEDIA_DEVELOP_ENABLE \
	-DCONFIG_MIXER_CYCLIC \
	-DCONFIG_SOUND_PLATFORM_ENABLE=1 \
	-DCONFIG_SUPPORT_EX_TWS_ADJUST \
	-D__GCC_PI32V2__ \
	-DCONFIG_NEW_ECC_ENABLE \
	-DCONFIG_MAC_ADDR_GET_USED_SFC \
	-DCONFIG_256K_FLASH \
	-DSUPPORT_BLUETOOTH_PROFILE_RELEASE \
	-DCONFIG_SOUNDBOX \
	-DCONFIG_OS_AFFINITY_ENABLE \
	-DEVENT_HANDLER_NUM_CONFIG=2 \
	-DEVENT_TOUCH_ENABLE_CONFIG=0 \
	-DEVENT_POOL_SIZE_CONFIG=256 \
	-DCONFIG_EVENT_KEY_MAP_ENABLE=0 \
	-DTIMER_POOL_NUM_CONFIG=5 \
	-DAPP_ASYNC_POOL_NUM_CONFIG=0 \
	-DVFS_ENABLE=1 \
	-DUSE_SDFILE_NEW=1 \
	-DSDFILE_STORAGE=1 \
	-DVFS_FILE_POOL_NUM_CONFIG=1 \
	-DFS_VERSION=0x020001 \
	-DFATFS_VERSION=0x020101 \
	-DSDFILE_VERSION=0x020000 \
	-DVM_MAX_PAGE_ALIGN_SIZE_CONFIG=64*1024 \
	-DVM_MAX_SECTOR_ALIGN_SIZE_CONFIG=64*1024 \
	-DVM_ITEM_MAX_NUM=256 \
	-DCONFIG_TWS_ENABLE \
	-DCONFIG_SOUNDBOX_CASE_ENABLE \
	-DCONFIG_NEW_CFG_TOOL_ENABLE \
	-DCONFIG_WIRELESS_MIC_ENABLE \
	-DCONFIG_LITE_AEC_ENABLE=1 \
	-DAUDIO_REC_POOL_NUM=1 \
	-DAUDIO_DEC_POOL_NUM=3 \
	-DCONFIG_LMP_CONN_SUSPEND_ENABLE \
	-DCONFIG_BTCTRLER_TASK_DEL_ENABLE \
	-DCONFIG_LINK_DISTURB_SCAN_ENABLE=0 \
	-DCONFIG_CODE_BANK_ENABLE \
	-DAEC_MALLOC_ENABLE=1 \
	-DCONFIG_EFFECT_CORE_V2_ENABLE \
	-DCONFIG_ITEM_FORMAT_VM \


DEFINES += $(EXT_CFLAGS) # 额外的一些定义

# 头文件搜索路径
INCLUDES := \
	-Iinclude_lib \
	-Iinclude_lib/driver \
	-Iinclude_lib/driver/device \
	-Iinclude_lib/driver/cpu/br29 \
	-Iinclude_lib/system \
	-Iinclude_lib/system/generic \
	-Iinclude_lib/system/device \
	-Iinclude_lib/system/fs \
	-Iinclude_lib/system/ui \
	-Iinclude_lib/btctrler \
	-Iinclude_lib/btctrler/port/br29 \
	-Iinclude_lib/update \
	-Iinclude_lib/agreement \
	-Iinclude_lib/btstack/third_party/common \
	-Iinclude_lib/btstack/third_party/rcsp \
	-Iinclude_lib/media/media_develop \
	-Iinclude_lib/media/media_develop/media \
	-Iinclude_lib/media/media_develop/media/cpu/br29 \
	-Iinclude_lib/media/media_develop/media/cpu/br29/asm \
	-Iapps/soundbox/include \
	-Iapps/soundbox/include/task_manager \
	-Iapps/soundbox/include/task_manager/bt \
	-Iapps/soundbox/include/task_manager/live_iis \
	-Iapps/soundbox/include/task_manager/live_mic \
	-Iapps/soundbox/include/task_manager/linein \
	-Iapps/soundbox/include/user_api \
	-Iapps/soundbox/include/wireless \
	-Iapps/common \
	-Iapps/common/device \
	-Iapps/common/audio \
	-Iapps/common/audio/live_audio \
	-Iapps/common/audio/stream \
	-Iapps/common/power_manage \
	-Iapps/common/charge_box \
	-Iapps/common/third_party_profile/common \
	-Iapps/common/third_party_profile/interface \
	-Iapps/common/third_party_profile/jieli \
	-Iapps/common/third_party_profile/jieli/trans_data_demo \
	-Iapps/common/third_party_profile/jieli/online_db \
	-Iapps/common/dev_manager \
	-Iapps/common/file_operate \
	-Iapps/common/music \
	-Iapps/common/include \
	-Iapps/common/config/include \
	-Iapps/soundbox/board/br29 \
	-Icpu/br29 \
	-Icpu/br29/audio_common \
	-Icpu/br29/audio_dec \
	-Icpu/br29/audio_enc \
	-Icpu/br29/audio_effect \
	-Icpu/br29/audio_mic \
	-Iinclude_lib/btstack \
	-Iinclude_lib/media \
	-Iapps/common/device/usb \
	-Iapps/common/device/usb/device \
	-Iapps/common/device/usb/host \
	-Iapps/soundbox/include/ui/color_led \
	-Iapps/common/uart_1t2 \
	-Iapps/common/uart_1t2/master \
	-Iapps/common/uart_1t2/slave \
	-Iinclude_lib/btstack/third_party/broadcast \
	-Iapps/common/wireless_dev_manager \
	-Iapps/soundbox/include/broadcast \
	-Iapps/soundbox/include/connected \
	-Iinclude_lib/btstack/third_party/le_audio \
	-Iapps/soundbox/wireless/parameters \
	-Iinclude_lib/system/ui/ui/cpu/br29 \
	-Iinclude_lib/system/math/cpu/br29 \
	-Icpu/br29/ui_driver \
	-Icpu/br29/audio_way \
	-Icpu/br29/audio_demo \
	-Icpu/br29/audio_online_dut \
	-Iinclude_lib/media/cvp \
	-I$(SYS_INC_DIR) \


# 需要编译的 .c 文件
c_SRC_FILES := \
	apps/common/audio/amplitude_statistic.c \
	apps/common/audio/audio_digital_vol.c \
	apps/common/audio/audio_export_demo.c \
	apps/common/audio/audio_utils.c \
	apps/common/audio/decode/audio_key_tone.c \
	apps/common/audio/decode/decode.c \
	apps/common/audio/demo/audio_mic_enc_dec_demo.c \
	apps/common/audio/live_audio/capture_synchronize.c \
	apps/common/audio/live_audio/live_audio_buffer.c \
	apps/common/audio/live_audio/live_audio_capture.c \
	apps/common/audio/live_audio/live_audio_effect.c \
	apps/common/audio/live_audio/live_audio_mixer.c \
	apps/common/audio/live_audio/live_audio_player.c \
	apps/common/audio/live_audio/live_stream_encoder.c \
	apps/common/audio/live_audio/pcm_capture_buffer.c \
	apps/common/audio/sine_make.c \
	apps/common/audio/stream/stream_entry.c \
	apps/common/audio/stream/stream_llns.c \
	apps/common/audio/stream/stream_src.c \
	apps/common/audio/stream/stream_sync.c \
	apps/common/audio/uartPcmSender.c \
	apps/common/bt_common/bt_test_api.c \
	apps/common/charge_box/chargeIc_manage.c \
	apps/common/charge_box/chgbox_box.c \
	apps/common/charge_box/chgbox_ctrl.c \
	apps/common/charge_box/chgbox_det.c \
	apps/common/charge_box/chgbox_handshake.c \
	apps/common/charge_box/chgbox_ui.c \
	apps/common/charge_box/chgbox_ui_drv_pwmled.c \
	apps/common/charge_box/chgbox_ui_drv_timer.c \
	apps/common/charge_box/chgbox_wireless.c \
	apps/common/config/app_config.c \
	apps/common/config/bt_profile_config.c \
	apps/common/config/ci_transport_uart.c \
	apps/common/config/new_cfg_tool.c \
	apps/common/debug/debug.c \
	apps/common/debug/debug_lite.c \
	apps/common/dev_manager/dev_manager.c \
	apps/common/dev_manager/dev_reg.c \
	apps/common/dev_manager/dev_update.c \
	apps/common/device/key/adkey.c \
	apps/common/device/key/ctmu_touch_key.c \
	apps/common/device/key/iokey.c \
	apps/common/device/key/key_driver.c \
	apps/common/device/key/touch_key.c \
	apps/common/device/norflash/norflash.c \
	apps/common/device/usb/device/cdc.c \
	apps/common/device/usb/device/custom_hid.c \
	apps/common/device/usb/device/descriptor.c \
	apps/common/device/usb/device/hid.c \
	apps/common/device/usb/device/msd.c \
	apps/common/device/usb/device/msd_upgrade.c \
	apps/common/device/usb/device/rcsp_hid_inter.c \
	apps/common/device/usb/device/task_pc.c \
	apps/common/device/usb/device/uac1.c \
	apps/common/device/usb/device/uac_stream.c \
	apps/common/device/usb/device/usb_device.c \
	apps/common/device/usb/device/user_setup.c \
	apps/common/device/usb/host/adb.c \
	apps/common/device/usb/host/aoa.c \
	apps/common/device/usb/host/audio.c \
	apps/common/device/usb/host/audio_demo.c \
	apps/common/device/usb/host/hid.c \
	apps/common/device/usb/host/usb_bulk_transfer.c \
	apps/common/device/usb/host/usb_ctrl_transfer.c \
	apps/common/device/usb/host/usb_host.c \
	apps/common/device/usb/host/usb_storage.c \
	apps/common/device/usb/usb_config.c \
	apps/common/device/usb/usb_host_config.c \
	apps/common/temp_trim/dtemp_pll_trim.c \
	apps/common/uart_1t2/master/uart_1t2_master_send.c \
	apps/common/uart_1t2/master/uart_1t2_serial_send.c \
	apps/common/uart_1t2/slave/uart_1t2_slave_send.c \
	apps/common/uart_1t2/uart_1t2.c \
	apps/common/ui/led7/led7_ui_api.c \
	apps/common/update/norflash_update.c \
	apps/common/update/testbox_update.c \
	apps/common/update/uart_update.c \
	apps/common/update/uart_update_master.c \
	apps/common/update/update.c \
	apps/common/wireless_dev_manager/wireless_dev_manager.c \
	apps/soundbox/aec/br29/audio_aec.c \
	apps/soundbox/aec/br29/audio_aec_demo.c \
	apps/soundbox/aec/br29/audio_aec_dms.c \
	apps/soundbox/aec/br29/audio_aec_online.c \
	apps/soundbox/app_main.c \
	apps/soundbox/board/br29/board_ac706n_1TN/board_ac706n_1TN.c \
	apps/soundbox/board/br29/board_ac706n_1TN/key_table/adkey_table.c \
	apps/soundbox/board/br29/board_ac706n_1TN/key_table/iokey_table.c \
	apps/soundbox/board/br29/board_ac706n_1TN/key_table/touch_key_table.c \
	apps/soundbox/board/br29/board_ac706n_2T1_simplex/board_ac706n_2T1_simplex.c \
	apps/soundbox/board/br29/board_ac706n_2T1_simplex/key_table/adkey_table.c \
	apps/soundbox/board/br29/board_ac706n_2T1_simplex/key_table/iokey_table.c \
	apps/soundbox/board/br29/board_ac706n_2T1_simplex/key_table/touch_key_table.c \
	apps/soundbox/board/br29/board_ac706n_test/board_ac706n_test.c \
	apps/soundbox/board/br29/board_ac706n_test/key_table/adkey_table.c \
	apps/soundbox/board/br29/board_ac706n_test/key_table/iokey_table.c \
	apps/soundbox/board/br29/board_ac706n_test/key_table/touch_key_table.c \
	apps/soundbox/board/br29/undef_func.c \
	apps/soundbox/broadcast/app_broadcast.c \
	apps/soundbox/broadcast/broadcast.c \
	apps/soundbox/common/dev_status.c \
	apps/soundbox/common/init.c \
	apps/soundbox/common/task_table.c \
	apps/soundbox/common/tone_table.c \
	apps/soundbox/common/user_cfg_new.c \
	apps/soundbox/connected/app_connected.c \
	apps/soundbox/connected/connected.c \
	apps/soundbox/connected/connected_sync_call.c \
	apps/soundbox/font/fontinit.c \
	apps/soundbox/log_config/app_config.c \
	apps/soundbox/log_config/lib_btctrler_config.c \
	apps/soundbox/log_config/lib_btstack_config.c \
	apps/soundbox/log_config/lib_driver_config.c \
	apps/soundbox/log_config/lib_media_config.c \
	apps/soundbox/log_config/lib_system_config.c \
	apps/soundbox/log_config/lib_update_config.c \
	apps/soundbox/power_manage/app_charge.c \
	apps/soundbox/power_manage/app_chargestore.c \
	apps/soundbox/power_manage/app_power_manage.c \
	apps/soundbox/task_manager/app_common.c \
	apps/soundbox/task_manager/app_task_switch.c \
	apps/soundbox/task_manager/bt/bt_other_func.c \
	apps/soundbox/task_manager/idle/idle.c \
	apps/soundbox/task_manager/linein/linein.c \
	apps/soundbox/task_manager/linein/linein_api.c \
	apps/soundbox/task_manager/linein/linein_dev.c \
	apps/soundbox/task_manager/live_sound/live_iis.c \
	apps/soundbox/task_manager/live_sound/live_iis_task.c \
	apps/soundbox/task_manager/live_sound/live_mic.c \
	apps/soundbox/task_manager/live_sound/live_mic_task.c \
	apps/soundbox/task_manager/pc/pc.c \
	apps/soundbox/task_manager/power_off/power_off.c \
	apps/soundbox/task_manager/power_on/power_on.c \
	apps/soundbox/task_manager/task_key.c \
	apps/soundbox/third_party_profile/ancs_client_demo/ancs_client_demo.c \
	apps/soundbox/third_party_profile/app_protocol_deal.c \
	apps/soundbox/third_party_profile/trans_data_demo/trans_data_demo.c \
	apps/soundbox/ui/color_led/color_led_app.c \
	apps/soundbox/ui/color_led/color_led_table.c \
	apps/soundbox/ui/color_led/driver/color_led.c \
	apps/soundbox/ui/color_led/driver/color_led_driver.c \
	apps/soundbox/ui/led/pwm_led_api.c \
	apps/soundbox/ui/led/pwm_led_para_table.c \
	apps/soundbox/ui/led7/ui_common.c \
	apps/soundbox/ui/led7/ui_idle.c \
	apps/soundbox/ui/led7/ui_linein.c \
	apps/soundbox/ui/led7/ui_pc.c \
	apps/soundbox/user_api/adapter_uart_demo.c \
	apps/soundbox/version.c \
	apps/soundbox/wireless/audio/audio_mode.c \
	apps/soundbox/wireless/ble/big.c \
	apps/soundbox/wireless/ble/cig.c \
	apps/soundbox/wireless/parameters/big_params.c \
	apps/soundbox/wireless/parameters/cig_params.c \
	apps/soundbox/wireless/test/wireless_trans_auto_test.c \
	cpu/br29/adc_api.c \
	cpu/br29/aec_tool.c \
	cpu/br29/audio_codec_clock.c \
	cpu/br29/audio_common/app_audio.c \
	cpu/br29/audio_common/sound_device_driver.c \
	cpu/br29/audio_common/sound_iis_capture.c \
	cpu/br29/audio_common/sound_pcm_demo.c \
	cpu/br29/audio_dec/audio_dec.c \
	cpu/br29/audio_dec/audio_dec_iis.c \
	cpu/br29/audio_dec/audio_dec_linein.c \
	cpu/br29/audio_dec/audio_dec_mic.c \
	cpu/br29/audio_dec/audio_dec_pc.c \
	cpu/br29/audio_dec/audio_dec_record.c \
	cpu/br29/audio_dec/audio_dec_tone.c \
	cpu/br29/audio_dec/audio_spectrum.c \
	cpu/br29/audio_dec/audio_usb_mic.c \
	cpu/br29/audio_dec/audio_usb_mix_mic.c \
	cpu/br29/audio_dec/lfwordana_enc_api.c \
	cpu/br29/audio_dec/live_stream_player.c \
	cpu/br29/audio_dec/tone_player.c \
	cpu/br29/audio_demo/audio_adc_demo.c \
	cpu/br29/audio_demo/audio_dac_demo.c \
	cpu/br29/audio_effect/audio_autotune_demo.c \
	cpu/br29/audio_effect/audio_ch_swap_demo.c \
	cpu/br29/audio_effect/audio_dynamic_eq_demo.c \
	cpu/br29/audio_effect/audio_eff_default_parm.c \
	cpu/br29/audio_effect/audio_eq_drc_demo.c \
	cpu/br29/audio_effect/audio_gain_process_demo.c \
	cpu/br29/audio_effect/audio_noise_gate_demo.c \
	cpu/br29/audio_effect/audio_sound_track_2_p_x.c \
	cpu/br29/audio_effect/audio_surround_demo.c \
	cpu/br29/audio_effect/audio_vbass_demo.c \
	cpu/br29/audio_effect/audio_voice_changer_demo.c \
	cpu/br29/audio_effect/effects_adj.c \
	cpu/br29/audio_effect/eq_config.c \
	cpu/br29/audio_enc/audio_adc_demo.c \
	cpu/br29/audio_enc/audio_enc.c \
	cpu/br29/audio_enc/audio_enc_file.c \
	cpu/br29/audio_enc/audio_enc_recoder.c \
	cpu/br29/audio_enc/audio_mic_codec.c \
	cpu/br29/audio_enc/audio_recorder_mix.c \
	cpu/br29/audio_enc/audio_sbc_codec.c \
	cpu/br29/audio_general.c \
	cpu/br29/audio_link.c \
	cpu/br29/audio_mic/effect_linein.c \
	cpu/br29/audio_mic/effect_parm.c \
	cpu/br29/audio_mic/effect_reg.c \
	cpu/br29/audio_mic/loud_speaker.c \
	cpu/br29/audio_mic/mic_effect.c \
	cpu/br29/audio_mic/mic_stream.c \
	cpu/br29/audio_mic/mic_stream_demo.c \
	cpu/br29/audio_mic/simpleAGC.c \
	cpu/br29/audio_mic/wireless_mic_effect.c \
	cpu/br29/audio_sound_dac.c \
	cpu/br29/audio_way/audio_app_stream.c \
	cpu/br29/audio_way/audio_way.c \
	cpu/br29/audio_way/audio_way_dac.c \
	cpu/br29/charge.c \
	cpu/br29/chargestore.c \
	cpu/br29/clock_manager.c \
	cpu/br29/hw_fft.c \
	cpu/br29/iic_hw.c \
	cpu/br29/iic_soft.c \
	cpu/br29/irflt.c \
	cpu/br29/led_spi.c \
	cpu/br29/lp_touch_key.c \
	cpu/br29/lp_touch_key_tool.c \
	cpu/br29/lua_port_api.c \
	cpu/br29/mcpwm.c \
	cpu/br29/overlay_code.c \
	cpu/br29/plcnt.c \
	cpu/br29/power/power_app.c \
	cpu/br29/power/power_check.c \
	cpu/br29/power/power_port.c \
	cpu/br29/power/power_trim.c \
	cpu/br29/psram.c \
	cpu/br29/pwm_led.c \
	cpu/br29/rdec.c \
	cpu/br29/setup.c \
	cpu/br29/sound_iis.c \
	cpu/br29/spi.c \
	cpu/br29/uart_dev.c \
	cpu/br29/ui_driver/LED_1888/LED1888.c \
	cpu/br29/ui_driver/lcd_seg/lcd_seg3x9_driver.c \
	cpu/br29/ui_driver/led7/led7_driver.c \
	cpu/br29/ui_driver/led7_timer.c \
	cpu/br29/ui_driver/ui_common.c \


# 需要编译的 .S 文件
S_SRC_FILES := \
	apps/soundbox/sdk_version.z.S \


# 需要编译的 .s 文件
s_SRC_FILES :=


# 需要编译的 .cpp 文件
cpp_SRC_FILES :=


# 需要编译的 .cc 文件
cc_SRC_FILES :=


# 需要编译的 .cxx 文件
cxx_SRC_FILES :=


# 链接参数
LFLAGS := \
	--plugin-opt=-pi32v2-always-use-itblock=false \
	--plugin-opt=-enable-ipra=true \
	--plugin-opt=-pi32v2-merge-max-offset=4096 \
	--plugin-opt=mcpu=r5 \
	--plugin-opt=-global-merge-on-const \
	--plugin-opt=-inline-threshold=5 \
	--plugin-opt=-inline-max-allocated-size=32 \
	--plugin-opt=-inline-normal-into-special-section=true \
	--plugin-opt=-dont-used-symbol-list=malloc,free,sprintf,printf,puts,putchar \
	--plugin-opt=save-temps \
	--plugin-opt=-pi32v2-enable-rep-memop \
	--plugin-opt=-warn-stack-size=256 \
	--sort-common \
	--dont-complain-call-overflow \
	--plugin-opt=-used-symbol-file=cpu/br29/sdk_used_list.used \
	--gc-sections \
	--start-group \
	cpu/br29/liba/cpu.a \
	cpu/br29/liba/system.a \
	cpu/br29/liba/btstack.a \
	cpu/br29/liba/btctrler.a \
	cpu/br29/liba/media.a \
	cpu/br29/liba/media_app.a \
	cpu/br29/liba/update.a \
	cpu/br29/liba/jla_codec_lib.a \
	cpu/br29/liba/lib_jla_ll_codec.a \
	cpu/br29/liba/wtg_dec_lib.a \
	cpu/br29/liba/rcsp_stack.a \
	cpu/br29/liba/tme_stack.a \
	cpu/br29/liba/aec.a \
	cpu/br29/liba/libepmotion.a \
	cpu/br29/liba/crypto_toolbox_Osize.a \
	cpu/br29/liba/libFFT_pi32v2_OnChip.a \
	cpu/br29/liba/libAptFilt_pi32v2_OnChip.a \
	cpu/br29/liba/libEchoSuppress_pi32v2_OnChip.a \
	cpu/br29/liba/libNoiseSuppress_pi32v2_OnChip.a \
	cpu/br29/liba/libSplittingFilter_pi32v2_OnChip.a \
	cpu/br29/liba/agreement.a \
	cpu/br29/liba/SpectrumShow.a \
	cpu/br29/liba/drc.a \
	cpu/br29/liba/DynamicEQ.a \
	cpu/br29/liba/lib_voiceChanger.a \
	cpu/br29/liba/VirtualBass.a \
	cpu/br29/liba/lib_sur_cal.a \
	cpu/br29/liba/loudness.a \
	cpu/br29/liba/howling.a \
	cpu/br29/liba/noisegate.a \
	cpu/br29/liba/lib_howlings_phf.a \
	cpu/br29/liba/crossover_coff.a \
	cpu/br29/liba/lib_reverb_cal.a \
	cpu/br29/liba/lib_plateReverb_adv.a \
	cpu/br29/liba/lib_pitch_speed.a \
	cpu/br29/liba/libllns.a \
	cpu/br29/liba/libjlsp.a \
	--end-group \
	-Tcpu/br29/sdk.ld \
	-M=cpu/br29/tools/sdk.map \
	--plugin-opt=mcpu=r5 \


LIBPATHS := \
	-L$(SYS_LIB_DIR) \


LIBS := \
	$(SYS_LIB_DIR)/libm.a \
	$(SYS_LIB_DIR)/libc.a \
	$(SYS_LIB_DIR)/libm.a \
	$(SYS_LIB_DIR)/libcompiler-rt.a \



c_OBJS    := $(c_SRC_FILES:%.c=%.c.o)
S_OBJS    := $(S_SRC_FILES:%.S=%.S.o)
s_OBJS    := $(s_SRC_FILES:%.s=%.s.o)
cpp_OBJS  := $(cpp_SRC_FILES:%.cpp=%.cpp.o)
cxx_OBJS  := $(cxx_SRC_FILES:%.cxx=%.cxx.o)
cc_OBJS   := $(cc_SRC_FILES:%.cc=%.cc.o)

OBJS      := $(c_OBJS) $(S_OBJS) $(s_OBJS) $(cpp_OBJS) $(cxx_OBJS) $(cc_OBJS)
DEP_FILES := $(OBJS:%.o=%.d)


OBJS      := $(addprefix $(BUILD_DIR)/, $(OBJS))
DEP_FILES := $(addprefix $(BUILD_DIR)/, $(DEP_FILES))


VERBOSE ?= 0
ifeq ($(VERBOSE), 1)
QUITE :=
else
QUITE := @
endif

# 一些旧的 make 不支持 file 函数，需要 make 的时候指定 LINK_AT=0 make
LINK_AT ?= 1

# 表示下面的不是一个文件的名字，无论是否存在 all, clean, pre_build 这样的文件
# 还是要执行命令
# see: https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean pre_build

# 不要使用 make 预设置的规则
# see: https://www.gnu.org/software/make/manual/html_node/Suffix-Rules.html
.SUFFIXES:

all: pre_build $(OUT_ELF)
	$(info +POST-BUILD)
	$(QUITE) $(RUN_POST_SCRIPT) sdk

pre_build:
	$(info +PRE-BUILD)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P cpu/br29/sdk_used_list.c -o cpu/br29/sdk_used_list.used
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P cpu/br29/sdk_ld.c -o cpu/br29/sdk.ld
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P cpu/br29/tools/download.c -o $(POST_SCRIPT)
	$(QUITE) $(FIXBAT) $(POST_SCRIPT)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P cpu/br29/tools/isd_config_rule.c -o cpu/br29/tools/isd_config.ini

clean:
	$(QUITE) $(RM) $(OUT_ELF)
	$(QUITE) $(RM) $(BUILD_DIR)



ifeq ($(LINK_AT), 1)
$(OUT_ELF): $(OBJS)
	$(info +LINK $@)
	$(shell $(MKDIR) $(@D))
	$(file >$(OBJ_FILE), $(OBJS))
	$(QUITE) $(LD) -o $(OUT_ELF) @$(OBJ_FILE) $(LFLAGS) $(LIBPATHS) $(LIBS)
else
$(OUT_ELF): $(OBJS)
	$(info +LINK $@)
	$(shell $(MKDIR) $(@D))
	$(QUITE) $(LD) -o $(OUT_ELF) $(OBJS) $(LFLAGS) $(LIBPATHS) $(LIBS)
endif


$(BUILD_DIR)/%.c.o : %.c
	$(info +CC $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.S.o : %.S
	$(info +AS $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.s.o : %.s
	$(info +AS $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.cpp.o : %.cpp
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.cxx.o : %.cxx
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.cc.o : %.cc
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

-include $(DEP_FILES)

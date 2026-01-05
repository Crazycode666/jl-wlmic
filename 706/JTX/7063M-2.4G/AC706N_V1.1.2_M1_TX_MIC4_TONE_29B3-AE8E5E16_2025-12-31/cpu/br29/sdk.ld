









  request_irq = ABSOLUTE(0xfe50);
  p33_buf = ABSOLUTE(0xfe54);
  p33_xor_1byte = ABSOLUTE(0xfe58);
  p33_and_1byte = ABSOLUTE(0xfe5c);
  p33_or_1byte = ABSOLUTE(0xfe60);
  p33_tx_1byte = ABSOLUTE(0xfe64);
  p33_rx_1byte = ABSOLUTE(0xfe68);
  P33_CON_SET = ABSOLUTE(0xfe6c);
  memcpy = ABSOLUTE(0xfe70);
  memmove = ABSOLUTE(0xfe74);
  memcmp = ABSOLUTE(0xfe78);
  memset = ABSOLUTE(0xfe7c);
  strcmp = ABSOLUTE(0xfe80);
  strlen = ABSOLUTE(0xfe84);
  ic_pll_init = ABSOLUTE(0xfe88);
  sfc_suspend = ABSOLUTE(0xfe8c);
  sfc_resume = ABSOLUTE(0xfe90);
  sfc_drop_cache = ABSOLUTE(0xfe94);
  IcuInitial = ABSOLUTE(0xfe98);
  IcuFlushinvAll = ABSOLUTE(0xfe9c);
  chip_crc16 = ABSOLUTE(0xfea0);
  CrcDecode = ABSOLUTE(0xfea4);
  mask_init = ABSOLUTE(0xfea8);
  wdt_clear = ABSOLUTE(0xfeac);
  rom_nvram_set_boot_state = ABSOLUTE(0xfeb0);
  chip_reset = ABSOLUTE(0xfeb4);
  flash_poweron = ABSOLUTE(0xfeb8);
  flash_poweroff = ABSOLUTE(0xfebc);
  uart_set_dma_dir = ABSOLUTE(0xfec0);
  doe = ABSOLUTE(0xfec4);
  LZ4_setStreamDecode = ABSOLUTE(0xfec8);
  LZ4_decompress_safe_continue = ABSOLUTE(0xfecc);
  LZ4_decompress_generic = ABSOLUTE(0xfed0);
  LZ4_decompress_safe_usingDict = ABSOLUTE(0xfed4);
  dec_isd_cfg_ini = ABSOLUTE(0xfed8);
  os_init = ABSOLUTE(0xfedc);
  os_current_task_prio = ABSOLUTE(0xfee0);
  os_current_task_stask_size = ABSOLUTE(0xfee4);
  os_time_get = ABSOLUTE(0xfee8);
  os_task_create_static = ABSOLUTE(0xfeec);
  os_task_coreaffinityset = ABSOLUTE(0xfef0);
  os_task_create_affinity_core = ABSOLUTE(0xfef4);
  rom_os_task_create = ABSOLUTE(0xfef8);
  os_time_dly = ABSOLUTE(0xfefc);
  os_task_del_req = ABSOLUTE(0xff00);
  os_task_del_res = ABSOLUTE(0xff04);
  os_task_del = ABSOLUTE(0xff08);
  os_taskq_post = ABSOLUTE(0xff0c);
  os_taskq_accept = ABSOLUTE(0xff10);
  __os_taskq_pend = ABSOLUTE(0xff14);
  os_sem_create = ABSOLUTE(0xff18);
  os_sem_del = ABSOLUTE(0xff1c);
  os_sem_pend = ABSOLUTE(0xff20);
  os_sem_post = ABSOLUTE(0xff24);
  os_sem_set = ABSOLUTE(0xff28);
  os_sem_query = ABSOLUTE(0xff2c);
  os_sem_valid = ABSOLUTE(0xff30);
  os_mutex_create = ABSOLUTE(0xff34);
  os_mutex_del = ABSOLUTE(0xff38);
  os_mutex_pend = ABSOLUTE(0xff3c);
  os_mutex_post = ABSOLUTE(0xff40);
  os_mutex_valid = ABSOLUTE(0xff44);
  os_q_create = ABSOLUTE(0xff48);
  os_q_del = ABSOLUTE(0xff4c);
  os_q_flush = ABSOLUTE(0xff50);
  os_taskq_flush = ABSOLUTE(0xff54);
  os_q_pend = ABSOLUTE(0xff58);
  os_q_recv = ABSOLUTE(0xff5c);
  os_q_peek = ABSOLUTE(0xff60);
  os_q_post = ABSOLUTE(0xff64);
  os_q_post_to_back = ABSOLUTE(0xff68);
  os_q_post_to_front = ABSOLUTE(0xff6c);
  os_q_query = ABSOLUTE(0xff70);
  os_q_valid = ABSOLUTE(0xff74);
  rom_os_task_get_handle = ABSOLUTE(0xff78);
  OSInit = ABSOLUTE(0xff7c);
  OSSchedLock = ABSOLUTE(0xff80);
  OSSchedUnlock = ABSOLUTE(0xff84);
  os_time_tick = ABSOLUTE(0xff88);
  OS_SchedRoundRobin = ABSOLUTE(0xff8c);
  OS_EventTaskRdy = ABSOLUTE(0xff90);
  OS_EventTaskWait = ABSOLUTE(0xff94);
  OS_EventTO = ABSOLUTE(0xff98);
  OS_Sched = ABSOLUTE(0xff9c);
  OSUpdateIdleTime = ABSOLUTE(0xffa0);
  prvGetExpectedIdleTime = ABSOLUTE(0xffa4);
  os_taskq_post_msg = ABSOLUTE(0xffa8);
  os_taskq_post_event = ABSOLUTE(0xffac);
  os_taskq_post_type = ABSOLUTE(0xffb0);
  task_queue_post_event = ABSOLUTE(0xffb4);
  task_queue_post_msg = ABSOLUTE(0xffb8);
  task_queue_post = ABSOLUTE(0xffbc);
  OSTaskQFlush = ABSOLUTE(0xffc0);
  __OSTaskQFlushMsg = ABSOLUTE(0xffc4);
  os_taskq_pend = ABSOLUTE(0xffc8);
  os_current_task_rom = ABSOLUTE(0xffcc);
  OSTimeDly = ABSOLUTE(0xffd0);
  OSTimeDlyResume = ABSOLUTE(0xffd4);
  OSTimeGet = ABSOLUTE(0xffd8);
  OSTimeSet = ABSOLUTE(0xffdc);
  os_var_info_get = ABSOLUTE(0xffe0);
  OSTaskCreate = ABSOLUTE(0xffe4);
  os_taskq_del_type = ABSOLUTE(0xffe8);
  STFT_Win_FixHalf_M512_D160 = ABSOLUTE(0x3a50);
  STFT_Win_FixHalf_M256_D160 = ABSOLUTE(0x3e50);
  STFT_Win_FixHalf_M256_D80 = ABSOLUTE(0x4050);
  STFT_Win_FixHalf_M128_D80 = ABSOLUTE(0x4250);
  WOLAAnalysisFilter_Float_M64_m4_r1 = ABSOLUTE(0x250);
  WOLASynthesisFilter_Float_M64_m4_r1 = ABSOLUTE(0x650);
  WOLAAnalysisFilter_Float_M128_m4_r1 = ABSOLUTE(0xa50);
  WOLASynthesisFilter_Float_M128_m4_r1 = ABSOLUTE(0x1250);
  WOLAAnalysisFilter_Float_M256_m4_r1 = ABSOLUTE(0x1a50);
  WOLASynthesisFilter_Float_M256_m4_r1 = ABSOLUTE(0x2a50);
  get_cvsd_codec_ops = ABSOLUTE(0xffec);
  get_mp3_ops = ABSOLUTE(0xfff0);
  mpegaudio_synth_full = ABSOLUTE(0xfff4);
  mp3_decode = ABSOLUTE(0xfff8);
  mpeg_decode_header = ABSOLUTE(0xfffc);
  pre_decode = ABSOLUTE(0x55c0);
  music_decode = ABSOLUTE(0x55d0);
  mp3_decoder_ops = ABSOLUTE(0x4364);
  D_windowtab = ABSOLUTE(0x63f0);
  efuse_page0 = ABSOLUTE(0x121d30);
  efuse_page1 = ABSOLUTE(0x121d34);
  _IRQ_MEM_ADDR = ABSOLUTE(0x121f00);
  _RAM_LIMIT_L = ABSOLUTE(0x100000);
  _MASK_USED_MEM_END = ABSOLUTE(0x100930);
  _MASK_EXPORT_MEM_BEGIN = ABSOLUTE(0x121cd0);
  _MASK_EXPORT_MEM_SIZE = ABSOLUTE(0x224);
  mdelay = ABSOLUTE(0xb8c8);
  udelay = ABSOLUTE(0xb8a4);
  OSRdyTbl = ABSOLUTE(0x121d84);
  OSTCBCur = ABSOLUTE(0x121d90);
  OSTCBPrioTbl = ABSOLUTE(0x121d94);
  OSNextTaskUnblockTime = ABSOLUTE(0x121e94);
  clr_wdt = ABSOLUTE(0xfeac);

EXTERN(
_start








sdfile_vfs_ops
jla_encoder







connected_event_sync
connected_data_transmit
connected_key_sync




audio_dac_driver
);

UPDATA_SIZE = 0x80;
UPDATA_BEG = _MASK_EXPORT_MEM_BEGIN - UPDATA_SIZE;

UPDATA_BREDR_BASE_BEG = 0x114000;


RAM_LIMIT_L = _RAM_LIMIT_L;
RAM_LIMIT_H = UPDATA_BEG;
PHY_RAM_SIZE = RAM_LIMIT_H - RAM_LIMIT_L;


ISR_BASE = _IRQ_MEM_ADDR;
ROM_RAM_SIZE = _MASK_EXPORT_MEM_SIZE;
ROM_RAM_BEG = _MASK_EXPORT_MEM_BEGIN;





RAM0_BEG = RAM_LIMIT_L;
RAM0_END = RAM_LIMIT_H;
RAM0_SIZE = RAM0_END - RAM0_BEG;

TEXT_CODE_BEG = 0x7E00100;




MEMORY
{
 code0(rx) : ORIGIN = TEXT_CODE_BEG, LENGTH = 0x100000
 ram0(rwx) : ORIGIN = RAM0_BEG, LENGTH = RAM0_SIZE

}


ENTRY(_start)

SECTIONS
{
    . = ORIGIN(code0);
    .text ALIGN(4):
    {
        PROVIDE(text_rodata_begin = .);

        *(.startup.text)

  *(.text)

  . = ALIGN(4);
     update_target_begin = .;
     PROVIDE(update_target_begin = .);
     KEEP(*(.update_target))
     update_target_end = .;
     PROVIDE(update_target_end = .);
  . = ALIGN(4);

  *(.LOG_TAG_CONST*)
        *(.rodata*)

  . = ALIGN(4);
        PROVIDE(text_rodata_end = .);

        clock_critical_handler_begin = .;
        KEEP(*(.clock_critical_txt))
        clock_critical_handler_end = .;

        chargestore_handler_begin = .;
        KEEP(*(.chargestore_callback_txt))
        chargestore_handler_end = .;


        *(.opcore_table_maskrom)
        *(.bfilt_table_maskroom)
        *(.opcore_maskrom)
        *(.bfilt_code)
        *(.bfilt_const)


  . = ALIGN(4);
  __VERSION_BEGIN = .;
        KEEP(*(.sys.version))
        __VERSION_END = .;

        *(.noop_version)
  . = ALIGN(4);

  . = ALIGN(4);
  tool_interface_begin = .;
  KEEP(*(.tool_interface))
  tool_interface_end = .;
        . = ALIGN(4);
        cmd_interface_begin = .;
        KEEP(*(.eff_cmd))
        cmd_interface_end = .;
  . = ALIGN(4);

  . = ALIGN(32);
  m_code_addr = . ;
  *(.m.code*)
  *(.movable.code*)
   m_code_size = ABSOLUTE(. - m_code_addr) ;
  . = ALIGN(32);
   } > code0

    . = ORIGIN(ram0);

 .mmu_tlb ALIGN(0x10000):
 {
  *(.mmu_tlb_segment);
 } > ram0

 .boot_info ALIGN(32):
 {
  *(.boot_info)
        . = ALIGN(32);
 } > ram0

 .irq_stack ALIGN(32):
    {
        _cpu0_sstack_begin = .;
        *(.cpu0_stack)
        _cpu0_sstack_end = .;
  . = ALIGN(4);

        _cpu1_sstack_begin = .;
        *(.cpu1_stack)
        _cpu1_sstack_end = .;
  . = ALIGN(4);

    } > ram0

 .data ALIGN(32):
 {
        data_run_addr = .;

        . = ALIGN(4);
        *(.data_magic)

  . = ALIGN(4);
        *(.data*)
        *(.mem_var.data)

  . = ALIGN(4);
 } > ram0

 .bss ALIGN(32):
    {
        . = ALIGN(4);
        *(.bss)
        . = ALIGN(4);
        *(.volatile_ram)

  *(.btstack_pool)

        *(.mem_heap)
  *(.memp_memory_x)

        . = ALIGN(4);
  *(.non_volatile_ram)

        . = ALIGN(32);

    } > ram0

 .data_code ALIGN(32):
 {
        data_code_run_addr = .;
  data_code_pc_limit_begin = .;
  *(*.text.cache.L1)
  *(*.text.cache.L2)
  *(.flushinv_icache)
        *(.cache)
        *(.os_critical_code)
        *(.volatile_ram_code)
        *(.fat_data_code)
        *(.fat_data_code_ex)

        *(.ui_ram)

  . = ALIGN(4);
        _SPI_CODE_START = . ;
        *(.spi_code)
  . = ALIGN(4);
        _SPI_CODE_END = . ;
  . = ALIGN(4);

 } > ram0

 .moveable_slot ALIGN(4):
 {
     *(movable.slot.*)

 } >ram0

 __report_overlay_begin = .;



 OVERLAY : AT(0x200000) SUBALIGN(4)
    {
  .overlay_aec
  {
   aec_code_begin = . ;
   *(.text._*)
   *(.data._*)
   *(.aec_code)
   *(.aec_const)
   *(.res_code)
   *(.res_const)
   *(.ns_code)
   *(.ns_const)
   *(.fft_code)
   *(.fft_const)
            *(.dms_code)
            *(.dms_const)
            *(.dms_sparse_code)
   aec_code_end = . ;
   aec_code_size = aec_code_end - aec_code_begin ;

   *(.msbc_enc)
   *(.cvsd_codec)
   *(.aec_bss)
   *(.aec_data)
   *(.res_data)
   *(.ns_data)
   *(.res_bss)
   *(.ns_bss)
   *(.aec_mem)
  }
  .overlay_aac
  {
   aac_dec_code_begin = .;
   *(.bt_aac_dec_code)
   *(.bt_aac_dec_sparse_code)
   aac_dec_code_end = .;
   aac_dec_code_size = aac_dec_code_end - aac_dec_code_begin ;

   . = ALIGN(4);
   bt_aac_dec_const_begin = .;
   *(.bt_aac_dec_const)
   *(.bt_aac_dec_sparse_const)
   bt_aac_dec_const_end = .;
   bt_aac_dec_const_size = bt_aac_dec_const_end - bt_aac_dec_const_begin ;

   *(.bt_aac_dec_data)
   *(.bt_aac_dec_bss)

   . = ALIGN(4);
   *(.aac_mem)
   *(.aac_ctrl_mem)


  }
  .overlay_mp3
  {
   *(.mp3_mem)
   *(.mp3_ctrl_mem)
   *(.mp3pick_mem)
   *(.mp3pick_ctrl_mem)
   *(.dec2tws_mem)
  }
  .overlay_wma
  {
   *(.wma_mem)
   *(.wma_ctrl_mem)
   *(.wmapick_mem)
   *(.wmapick_ctrl_mem)
  }
  .overlay_wav
  {
   *(.wav_mem)
   *(.wav_ctrl_mem)
  }
  .overlay_ape
        {
            *(.ape_mem)
            *(.ape_ctrl_mem)
  }
  .overlay_flac
        {
            *(.flac_mem)
            *(.flac_ctrl_mem)
  }
  .overlay_m4a
        {
            *(.m4a_mem)
            *(.m4a_ctrl_mem)
  }
  .overlay_amr
        {
            *(.amr_mem)
            *(.amr_ctrl_mem)
  }
  .overlay_dts
        {
            *(.dts_mem)
            *(.dts_ctrl_mem)
  }
  .overlay_fm
  {
   *(.fm_mem)
  }
        .overlay_pc
  {



  }
    } > ram0

 data_code_pc_limit_end = .;
 __report_overlay_end = .;

 _HEAP_BEGIN = . ;
 _HEAP_END = RAM0_END;

}





SECTIONS
{
    .text : ALIGN(4)
    {
        gsensor_dev_begin = .;
        KEEP(*(.gsensor_dev))
        gsensor_dev_end = .;

        fm_dev_begin = .;
        KEEP(*(.fm_dev))
        fm_dev_end = .;

        fm_emitter_dev_begin = .;
        KEEP(*(.fm_emitter_dev))
        fm_emitter_dev_end = .;

        storage_device_begin = .;
        KEEP(*(.storage_device))
        storage_device_end = .;

        wireless_dev_ops_begin = .;
        KEEP(*(.wireless_dev))
        wireless_dev_ops_end = .;

        connected_sync_channel_begin = .;
        KEEP(*(.connected_call_sync))
        connected_sync_channel_end = .;

        conn_sync_call_func_begin = .;
        KEEP(*(.connected_sync_call_func))
        conn_sync_call_func_end = .;

        conn_data_trans_stub_begin = .;
        KEEP(*(.conn_data_trans_stub))
        conn_data_trans_stub_end = .;


  . = ALIGN(4);
        *(.gpio_ram)
  . = ALIGN(4);
        *(.LED_code)
  . = ALIGN(4);
        *(.LED_const)


  . = ALIGN(4);
        *(.usb.text)
        *(.usb.text.const)

  . = ALIGN(4);
        broadcast_text_begin = . ;
        *(.broadcast_text)
        *(.broadcast_const)
        *(.app_broadcast_text)
        *(.app_broadcast_const)
        broadcast_text_end = . ;

  . = ALIGN(32);
    } > code0

 .data ALIGN(32):
 {
  . = ALIGN(4);
        *(.usb.data)

        . = ALIGN(4);
        broadcast_data_begin = . ;
        *(.broadcast_data)
        *(.app_broadcast_data)
        broadcast_data_end = . ;

        . = ALIGN(4);
 } > ram0

 .bss ALIGN(32):
    {
        *(.usb.data.bss.host_mem)
        *(.usb.data.bss)

        *(.usb.data.bss.exchange)


        . = ALIGN(4);
        broadcast_bss_begin = . ;
        *(.broadcast_bss)
        *(.app_broadcast_bss)
        broadcast_bss_end = . ;

        . = ALIGN(32);
    } > ram0

 .data_code ALIGN(32):
 {
  . = ALIGN(4);
 } > ram0

}

broadcast_text_size = broadcast_text_end - broadcast_text_begin;
broadcast_bss_size = broadcast_bss_end - broadcast_bss_begin;
broadcast_data_size = broadcast_data_end - broadcast_data_begin;
app_broadcast_total_size = broadcast_text_size + broadcast_data_size + broadcast_bss_size;



SECTIONS
{
    .data : ALIGN(4)
    {
        update_data_start = .;

        update_data_end = .;
        . = ALIGN(4);
    } > ram0

    .bss (NOLOAD) :ALIGN(4)
    {
        update_bss_start = .;

        *(.update_bss)
        update_bss_end = .;
        . = ALIGN(4);
    } > ram0

    .text : ALIGN(4)
    {
  update_code_start = .;

  *(.bt_updata_ram_code)
  *(.update_const)
  *(.update_code)

  update_code_end = .;
        . = ALIGN(4);
    } > code0


    UPDATE_CODE_TOTAL_SIZE = update_code_end - update_code_start;
}
SECTIONS
{
    .data : ALIGN(4)
    {
        btstack_data_start = .;
        *(.bt_stack_data)
        *(.ble_db_data)
        *(.ble_sm_data)
        *(.ble_att_data)
        *(.ble_gatt_data)


        BTSTACK_LE_HOST_MESH_DATA_START = .;
  . = (. +3) & ~ 3;
        _net_buf_pool_list = .;
        *(._net_buf_pool.static.*)

        *(.ble_mesh_data)
        *(.ble_mesh_tinycrypt_data)
        BTSTACK_LE_HOST_MESH_DATA_SIZE = ABSOLUTE(. - BTSTACK_LE_HOST_MESH_DATA_START);


        BTSTACK_LE_AUDIO_BIG_DATA_START = .;
        *(.big_rx_data)
        *(.big_tx_data)
        *(.big_common_data)
        BTSTACK_LE_AUDIO_BIG_DATA_SIZE = ABSOLUTE(. - BTSTACK_LE_AUDIO_BIG_DATA_START);


        BTSTACK_LE_AUDIO_CIG_DATA_START = .;
        *(.cig_central_data)
        *(.cig_peripheral_data)
        *(.cig_common_data)
        BTSTACK_LE_AUDIO_CIG_DATA_SIZE = ABSOLUTE(. - BTSTACK_LE_AUDIO_CIG_DATA_START);

        btstack_data_end = .;

        . = ALIGN(4);
    } > ram0

    .bss (NOLOAD) :ALIGN(4)
    {
        btstack_bss_start = .;
        *(.bt_stack_bss)
        *(.ble_db_bss)
        *(.ble_sm_bss)
        *(.ble_att_bss)
        *(.ble_gatt_bss)
        *(.btstack_pool)


        BTSTACK_LE_HOST_MESH_BSS_START = .;
        *(.ble_mesh_bss)
        *(.ble_mesh_tinycrypt_bss)
        BTSTACK_LE_HOST_MESH_BSS_SIZE = ABSOLUTE(. - BTSTACK_LE_HOST_MESH_BSS_START);


        BTSTACK_LE_AUDIO_BIG_BSS_START = .;
        *(.big_rx_bss)
        *(.big_tx_bss)
        *(.big_common_bss)
        BTSTACK_LE_AUDIO_BIG_BSS_SIZE = ABSOLUTE(. - BTSTACK_LE_AUDIO_BIG_BSS_START);


        BTSTACK_LE_AUDIO_CIG_BSS_START = .;
        *(.cig_central_bss)
        *(.cig_peripheral_bss)
        *(.cig_common_bss)
        BTSTACK_LE_AUDIO_CIG_BSS_SIZE = ABSOLUTE(. - BTSTACK_LE_AUDIO_CIG_BSS_START);

        btstack_bss_end = .;
        . = ALIGN(4);
    } > ram0

    .text : ALIGN(4)
    {
        btstack_code_start = .;

        . = ALIGN(4);

        a2dp_source_media_codec_begin = .;
        KEEP(*(.a2dp_source_media_codec))
            a2dp_source_media_codec_end = .;
        a2dp_sink_media_probe_begin = .;
        KEEP(*(.a2dp_sink_media_probe))
            a2dp_sink_media_probe_end = .;

        a2dp_sink_media_codec_begin = .;
        KEEP(*(.a2dp_sink_media_codec))
            a2dp_sink_media_codec_end = .;

        a2dp_event_handler_begin = .;
        KEEP(*(.a2dp_event_handler))
            a2dp_event_handler_end = .;

        sdp_record_item_begin = .;
        KEEP(*(.sdp_record_item))
            sdp_record_item_end = .;

        bt_sleep_begin = .;
        KEEP(*(.bt_sleep))
            bt_sleep_end = .;

  *(.bt_stack_const)
  *(.bt_stack_code)
        *(.ble_db_const)
        *(.ble_db_code)
        *(.ble_sm_const)
        *(.ble_sm_code)
        *(.ble_att_const)
        *(.ble_att_code)
        *(.ble_gatt_const)
        *(.ble_gatt_code)


        BTSTACK_LE_HOST_MESH_CODE_START = .;
        *(.ble_mesh_code)
        *(.ble_mesh_tinycrypt_code)

        *(.ble_mesh_const)
        *(.ble_mesh_tinycrypt_const)
        BTSTACK_LE_HOST_MESH_CODE_SIZE = ABSOLUTE(. - BTSTACK_LE_HOST_MESH_CODE_START);


        BTSTACK_LE_AUDIO_BIG_TEXT_START = .;
        *(.big_rx_const)
        *(.big_tx_const)
        *(.big_common_const)

        *(.big_rx_code)
        *(.big_tx_code)
        *(.big_common_code)
        BTSTACK_LE_AUDIO_BIG_TEXT_SIZE = ABSOLUTE(. - BTSTACK_LE_AUDIO_BIG_TEXT_START);


        BTSTACK_LE_AUDIO_CIG_TEXT_START = .;
        *(.cig_central_const)
        *(.cig_peripheral_const)
        *(.cig_common_const)

        *(.cig_central_code)
        *(.cig_peripheral_code)
        *(.cig_common_code)
        BTSTACK_LE_AUDIO_CIG_TEXT_SIZE = ABSOLUTE(. - BTSTACK_LE_AUDIO_CIG_TEXT_START);

        btstack_code_end = .;
  . = ALIGN(4);
    } > code0
}

BTSTACK_LE_HOST_MESH_RAM_TOTAL = BTSTACK_LE_HOST_MESH_DATA_SIZE + BTSTACK_LE_HOST_MESH_BSS_SIZE;
BTSTACK_LE_HOST_MESH_FLASH_TOTAL = BTSTACK_LE_HOST_MESH_CODE_SIZE;

BTSTACK_CODE_SIZE = (btstack_code_end - btstack_code_start) + (btstack_data_end - btstack_data_start);
SECTIONS
{
    .data : ALIGN(4)
    {
  . = ALIGN(4);
        system_data_start = .;

  _video_subdev_begin = .;
  PROVIDE(video_subdev_begin = .);
  KEEP(*(.video_subdev.0))
  KEEP(*(.video_subdev.1))
  KEEP(*(.video_subdev.2))
  KEEP(*(.video_subdev.3))
  KEEP(*(.video_subdev.4))
  KEEP(*(.video_subdev.5))
  _video_subdev_end = .;
  PROVIDE(video_subdev_end = .);

  _audio_subdev_begin = .;
  PROVIDE(audio_subdev_begin = .);
  KEEP(*(.audio_subdev.0))
  KEEP(*(.audio_subdev.1))
  KEEP(*(.audio_subdev.2))
  KEEP(*(.audio_subdev.3))
  _audio_subdev_end = .;
  PROVIDE(audio_subdev_end = .);

  _iic_device_begin = .;
  PROVIDE(iic_device_begin = .);
  KEEP(*(.iic))
  _iic_device_end = .;
  PROVIDE(iic_device_end = .);

        _avin_spi_device_begin = .;
  PROVIDE(avin_spi_device_begin = .);
  KEEP(*(.sw_spi))
  _avin_spi_device_end = .;
  PROVIDE(avin_spi_device_end = .);

  _video_dev_begin = .;
  PROVIDE(video_dev_begin = .);
  KEEP(*(.video_device))
  _video_dev_end = .;
  PROVIDE(video_dev_end = .);

  _key_driver_ops_begin = .;
  PROVIDE(key_driver_ops_begin = .);
  KEEP(*(.key_driver_ops))
  _key_driver_ops_end = .;
  PROVIDE(key_driver_ops_end = .);

  _touch_driver_begin = .;
  PROVIDE(touch_driver_begin = .);
  KEEP(*(.touch_driver))
  _touch_driver_end = .;
  PROVIDE(touch_driver_end = .);

  _static_hi_timer_begin = .;
  PROVIDE(static_hi_timer_begin = .);
  KEEP(*(.hi_timer))
  _static_hi_timer_end = .;
  PROVIDE(static_hi_timer_end = .);

  _sys_cpu_timer_begin = .;
  PROVIDE(sys_cpu_timer_begin = .);
  KEEP(*(.sys_cpu_timer))
  _sys_cpu_timer_end = .;
  PROVIDE(sys_cpu_timer_end = .);

     _sys_config_begin = .;
     PROVIDE(sys_config_begin = .);
     KEEP(*(.sys_cfg))
     _sys_config_end = .;
     PROVIDE(sys_config_end = .);

     _sys_fat_begin = .;
     PROVIDE(sys_fat_begin = .);
     KEEP(*(.fs_fat))
     _sys_fat_end = .;
     PROVIDE(sys_fat_end = .);

  _app_begin = .;
  PROVIDE(app_begin = .);
  KEEP(*(.app))
  _app_end = .;
  PROVIDE(app_end = .);

     *(.crypto_ecdh_data)
     *(.crypto_data)

     *(.mem_data)
        *(.os_port_data)


        *(.os_data)

     *(.uECC_data)
     *(.ECDH_sample_data)

         __movable_slot_start = .;
         *(movable.slot.*);
         __movable_slot_end = .;

        system_data_end = .;

    } > ram0

    .bss (NOLOAD) :ALIGN(4)
    {
        system_bss_start = .;
        . = ALIGN(4);
        *(.os_bss)
        *(.mem_heap)
  *(.memp_memory_x)
        *(.mem_bss)
        *(.os_port_bss)

        *(.uECC_bss)
        *(.ECDH_sample_bss)

        system_bss_end = .;

    } > ram0

    .text : ALIGN(4)
    {
  . = ALIGN(4);
        system_text_start = .;

   _device_node_begin = .;
     PROVIDE(device_node_begin = .);
     KEEP(*(.device))
     _device_node_end = .;
     PROVIDE(device_node_end = .);

  config_target_begin = .;
     PROVIDE(config_target_begin = .);
     KEEP(*(.config_target))
  config_target_end = .;
     PROVIDE(config_target_end = .);

     system_code_begin = .;
     KEEP(*(.system.*.text))
     system_code_end = .;
  . = ALIGN(4);
  system_code_size = system_code_end - system_code_begin;

  vfs_ops_begin = .;
  KEEP(*(.vfs_operations))
  vfs_ops_end = .;

     _lib_version_begin = .;
     PROVIDE(lib_version_begin = .);
     KEEP(*(.lib_version))
     _lib_version_end = .;
     PROVIDE(lib_version_end = .);

  _initcall_begin = .;
  PROVIDE(initcall_begin = .);
     KEEP(*(.initcall))
  _initcall_end = .;
  PROVIDE(initcall_end = .);

  _early_initcall_begin = .;
  PROVIDE(early_initcall_begin = .);
  KEEP(*(.early.initcall))
  _early_initcall_end = .;
  PROVIDE(early_initcall_end = .);

  _late_initcall_begin = .;
  PROVIDE(late_initcall_begin = .);
  KEEP(*(.late.initcall))
  _late_initcall_end = .;
  PROVIDE(late_initcall_end = .);

  _platform_initcall_begin = .;
  PROVIDE(platform_initcall_begin = .);
  KEEP(*(.platform.initcall))
  _platform_initcall_end = .;
  PROVIDE(platform_initcall_end = .);

  _module_initcall_begin = .;
  PROVIDE(module_initcall_begin = .);
  KEEP(*(.module.initcall))
  _module_initcall_end = .;
  PROVIDE(module_initcall_end = .);

  _sys_event_handler_begin = .;
  PROVIDE(sys_event_handler_begin = .);
  KEEP(*(.sys_event.4.handler))
  KEEP(*(.sys_event.3.handler))
  KEEP(*(.sys_event.2.handler))
  KEEP(*(.sys_event.1.handler))
  KEEP(*(.sys_event.0.handler))
  _sys_event_handler_end = .;
  PROVIDE(sys_event_handler_end = .);

  _syscfg_arg_begin = .;
  PROVIDE(syscfg_arg_begin = .);
  KEEP(*(.syscfg.arg))
  _syscfg_arg_end = .;
  PROVIDE(syscfg_arg_end = .);

  _syscfg_handler_begin = .;
  PROVIDE(syscfg_handler_begin = .);
  KEEP(*(.syscfg.handler))
  _syscfg_handler_end = .;
  PROVIDE(syscfg_handler_end = .);

  _syscfg_ops_begin = .;
  PROVIDE(syscfg_ops_begin = .);
  KEEP(*(.syscfg.2.ops))
  KEEP(*(.syscfg.1.ops))
  KEEP(*(.syscfg.0.ops))
  _syscfg_ops_end = .;
  PROVIDE(syscfg_ops_end = .);

  _server_info_begin = .;
  PROVIDE(server_info_begin = .);
  KEEP(*(.server_info))
  _server_info_end = .;
  PROVIDE(server_info_end = .);

  _bus_device_begin = .;
  PROVIDE(bus_device_begin = .);
  KEEP(*(.bus_device))
  _bus_device_end = .;
  PROVIDE(bus_device_end = .);

     _sys_power_hal_ops_begin = .;
     PROVIDE(sys_power_hal_ops_begin = .);
     KEEP(*(.sys_power_hal_ops))
     _sys_power_hal_ops_end = .;
     PROVIDE(sys_power_hal_ops_end = .);


  . = ALIGN(4);
     lp_target_begin = .;
     PROVIDE(lp_target_begin = .);
     KEEP(*(.lp_target))
     lp_target_end = .;
     PROVIDE(lp_target_end = .);

  . = ALIGN(4);
        deepsleep_target_begin = .;
        PROVIDE(deepsleep_target_begin = .);
        KEEP(*(.deepsleep_target))
        deepsleep_target_end = .;
        PROVIDE(deepsleep_target_end = .);

     crypto_begin = .;
     *(.crypto_ecdh_code)
     *(.crypto_ecdh_const)

     *(.crypto_bigint_code)
     *(.crypto_bigint_const)

     *(.crypto_code)
     *(.crypto_const)

     *(.ECDH_sample_code)
     *(.ECDH_sample_const)

     *(.uECC_code)
     *(.uECC_const)

     *(.hmac_code)
     *(.hmac_const)

     *(.hash_sample_code)
     *(.hash_sample_const)

     *(.aes_cmac_sample_code)
     *(.aes_cmac_sample_const)
     crypto_end = .;
     crypto_size = . - crypto_begin;

        *(.mem_code)
        *(.mem_const)

  *(.os_code)
        *(.os_port_code)
        *(.os_port_const)

        *(.os_const)
  *(.os_str)

        *(.math_fast_funtion_code)

        __movable_function_start = .;
        *(movable.text.*);
        *(movable.stub.*);
        *(movable.region.*);

        __movable_function_end = .;
        __movable_function_size = __movable_function_end - __movable_function_start;

        system_text_end = .;


        system_code_total_size = system_text_end - system_text_start;
    } > code0

 .data_code ALIGN(32):
 {

    } > ram0
}
SECTIONS
{
    .data : ALIGN(4)
    {
        btctler_data_start = .;

        BTCTLER_CONTROLLER_DATA_START = .;
        *(.bt_rf_data)
  *(.vendor_manager_data)
  *(.device_manager_data)
  *(.hci_controller_data)
  *(.hci_interface_data)
        BTCTLER_CONTROLLER_DATA_SIZE = ABSOLUTE(. - BTCTLER_CONTROLLER_DATA_START);

        BTCTLER_LE_CONTROLLER_DATA_START = .;
        *(.ble_ll_data)
        *(.ble_hci_data)
        *(.ble_rf_data)
        BTCTLER_LE_CONTROLLER_DATA_SIZE = ABSOLUTE(. - BTCTLER_LE_CONTROLLER_DATA_START);

        BTCTLER_CL_DATA_START = .;
        *(.classic_hci_data)
        *(.classic_lmp_data)
        *(.classic_lmp_auth_data)
        *(.classic_lmp_bigint_data)
        *(.classic_lmp_crypt_data)
        *(.classic_lmp_ecdh_data)
        *(.classic_lmp_linkbulk_data)
        *(.classic_lmp_hmac_data)
        *(.classic_rf_data)
        *(.classic_bb_data)
        BTCTLER_CL_DATA_SIZE = ABSOLUTE(. - BTCTLER_CL_DATA_START);

        btctler_data_end = .;

        . = ALIGN(4);
    } > ram0

    .bss (NOLOAD) :ALIGN(4)
    {
        btctler_bss_start = .;

        BTCTLER_CONTROLLER_BSS_START = .;

        *(.bd_base)
        *(.bredr_rxtx_bulk)
        acl_tx_pool = .;
        *(.bredr_tx_bulk)



        acl_tx_pool_end = acl_tx_pool;

        . = acl_tx_pool_end;

        acl_rx_pool = .;
        *(.bredr_rx_bulk)



        acl_rx_pool_end = acl_rx_pool;

        . = acl_rx_pool_end;

        tws_bulk_pool = .;



        tws_bulk_pool_end = tws_bulk_pool;

        . = tws_bulk_pool_end;

        *(.bt_rf_bss)
  *(.hci_controller_bss)
  *(.hci_interface_bss)
  *(.device_manager_bss)
  *(.vendor_manager_bss)
        BTCTLER_CONTROLLER_BSS_SIZE = ABSOLUTE(. - BTCTLER_CONTROLLER_BSS_START);

        BTCTLER_LE_CONTROLLER_BSS_START = .;
        *(.ble_hci_bss)
        *(.ble_ll_bss)
        *(.ble_rf_bss)
        BTCTLER_LE_CONTROLLER_BSS_SIZE = ABSOLUTE(. - BTCTLER_LE_CONTROLLER_BSS_START);

        BTCTLER_CL_BSS_START = .;
        *(.classic_rf_bss)
        *(.classic_lmp_bss)
        *(.classic_lmp_auth_bss)
        *(.classic_lmp_bigint_bss)
        *(.classic_lmp_crypt_bss)
        *(.classic_lmp_ecdh_bss)
        *(.classic_lmp_linkbulk_bss)
        *(.classic_lmp_hmac_bss)
        *(.classic_bb_bss)
        *(.classic_hci_bss)
        BTCTLER_CL_BSS_SIZE = ABSOLUTE(. - BTCTLER_CL_BSS_START);

        btctler_bss_end = .;
        . = ALIGN(4);
    } > ram0

    .text : ALIGN(4)
    {
        btctler_code_start = .;

        BTCTLER_CONTROLLER_CODE_START = .;
  *(.bt_rf_const)
  *(.bt_rf_code)
  *(.vendor_manager_const)
  *(.vendor_manager_code)
  *(.device_manager_const)
  *(.device_manager_code)
  *(.hci_controller_const)
  *(.hci_controller_code)
  *(.hci_interface_const)
  *(.hci_interface_code)
        BTCTLER_CONTROLLER_CODE_SIZE = ABSOLUTE(. - BTCTLER_CONTROLLER_CODE_START);

        BTCTLER_LE_CONTROLLER_CODE_START = .;
        *(.ble_rf_const)
        *(.ble_rf_code)

     *(.ble_ll_const)
     *(.ble_ll_code)
  *(.ble_hci_const)
  *(.ble_hci_code)
        BTCTLER_LE_CONTROLLER_CODE_SIZE = ABSOLUTE(. - BTCTLER_LE_CONTROLLER_CODE_START);

        BTCTLER_CL_CODE_START = .;
        *(.bredr_irq)
        *(.bredr_irq_code)
        *(.bredr_irq_const)

  *(.classic_hci_const)
  *(.classic_hci_code)
        *(.classic_lmp_const)
        *(.classic_lmp_auth_const)
        *(.classic_lmp_bigint_const)
        *(.classic_lmp_crypt_const)
        *(.classic_lmp_ecdh_const)
        *(.classic_lmp_hmac_const)
        *(.classic_lmp_code)
        *(.classic_lmp_auth_code)
        *(.classic_lmp_bigint_code)
        *(.classic_lmp_crypt_code)
        *(.classic_lmp_ecdh_code)
        *(.classic_lmp_hmac_code)
        *(.classic_rf_const)
        *(.classic_rf_code)
        *(.classic_bb_const)
        *(.classic_bb_code)
        BTCTLER_CL_CODE_SIZE = ABSOLUTE(. - BTCTLER_CL_CODE_START);

  . = ALIGN(4);

    } > code0
 .text ALIGN(4):
 {
        *(.classic_tws_const)
        *(.classic_tws_code)
        *(.classic_tws_code.esco)

  . = ALIGN(4);

        *(.lmp_irq_code)
        *(.link_bulk_code)
        *(.frame_irq_code)

  . = ALIGN(4);
        *(.link_task_const)
        *(.link_task_code)

  . = ALIGN(4);
        *(.classic_irq_const)
        *(.classic_irq_code)


  . = ALIGN(4);
        *(.tws_irq_code)

  . = ALIGN(4);
        tws_sync_call_begin = .;
        KEEP(*(.tws_sync_call))
        tws_sync_call_end = .;

  . = ALIGN(4);
        tws_func_stub_begin = .;
        KEEP(*(.tws_func_stub))
        tws_func_stub_end = .;

        *(.tws_media_sync_code)
        *(.tws_media_sync_const)

        *(.tws_data_forward_code)
        *(.tws_data_forward_const)

  . = ALIGN(4);
        tws_sync_channel_begin = .;
        KEEP(*(.tws_sync_channel.0))
        KEEP(*(.tws_sync_channel.1))
        KEEP(*(.tws_sync_channel.2))
        KEEP(*(.tws_sync_channel.3))
        KEEP(*(.tws_sync_channel.4))
        KEEP(*(.tws_sync_channel.5))
        KEEP(*(.tws_sync_channel.6))
        KEEP(*(.tws_sync_channel.7))
        KEEP(*(.tws_sync_channel.8))
        KEEP(*(.tws_sync_channel.9))
        KEEP(*(.tws_sync_channel.10))
        KEEP(*(.tws_sync_channel.11))
        KEEP(*(.tws_sync_channel.12))
        KEEP(*(.tws_sync_channel.13))
        tws_sync_channel_end = .;

        btctler_code_end = .;

  . = ALIGN(4);
    } > code0




    BTCTLER_LE_RAM_TOTAL = BTCTLER_LE_CONTROLLER_DATA_SIZE + BTCTLER_LE_CONTROLLER_BSS_SIZE;
    BTCTLER_LE_CODE_TOTAL = BTCTLER_LE_CONTROLLER_CODE_SIZE;

    BTCTLER_CL_RAM_TOTAL = BTCTLER_CL_DATA_SIZE + BTCTLER_CL_BSS_SIZE;
    BTCTLER_CL_CODE_TOTAL = BTCTLER_CL_CODE_SIZE;

    BTCTLER_COMMON_RAM_TOTAL = BTCTLER_CONTROLLER_BSS_SIZE + BTCTLER_CONTROLLER_DATA_SIZE;
    BTCTLER_COMMON_CODE_TOTAL = BTCTLER_CONTROLLER_CODE_SIZE ;

    BTCTLER_RAM_TOTAL = (btctler_data_end - btctler_data_start) + (btctler_bss_end - btctler_bss_start);
    BTCTLER_CODE_TOTAL = (btctler_code_end - btctler_code_start);
}
SECTIONS
{
    .data : ALIGN(4)
    {
        driver_data_start = .;


        CLOCK_DATA_START = .;
        *(.clock_data)
        CLOCK_DATA_SIZE = ABSOLUTE(. - CLOCK_DATA_START);

        SDMMC_DATA_START = .;
        *(.sdmmc.data)
        SDMMC_DATA_SIZE = ABSOLUTE(. - SDMMC_DATA_START);


        *(.debug_data)
        *(.power_data)

        *(.uart_data)

        driver_data_end = .;
        . = ALIGN(32);
    } > ram0

    .bss (NOLOAD) :ALIGN(4)
    {
        driver_bss_start = .;

        CLOCK_BSS_START = .;
        *(.clock_bss)
        CLOCK_BSS_SIZE = ABSOLUTE(. - CLOCK_BSS_START);

        SDMMC_BSS_START = .;
        *(.sdmmc.data.bss)
        SDMMC_BSS_SIZE = ABSOLUTE(. - SDMMC_BSS_START);

        *(.debug_bss)
        *(.power_bss)

        *(.uart_bss)

        *(.sd_var)
        driver_bss_end = .;
        . = ALIGN(4);
    } > ram0

    .text : ALIGN(4)
    {
        driver_code_start = .;

        *(.fm_code)

        CLOCK_CODE_START = .;
        *(.clock_code)
        *(.clock_const)
        CLOCK_CODE_SIZE = ABSOLUTE(. - CLOCK_CODE_START);

        SDMMC_CODE_START = .;
        *(.sdmmc.text)
        *(.sdmmc.text.const)
        SDMMC_CODE_SIZE = ABSOLUTE(. - SDMMC_CODE_START);


        *(.debug_code)
        *(.debug_const)

        *(.power_code)
        *(.power_const)

        *(.uart_code)
        *(.uart_const)

        driver_code_end = .;
  . = ALIGN(4);
    } > code0


    DRIVER_RAM_TOTAL = (driver_data_end - driver_data_start) + (driver_bss_end - driver_bss_start);
    DRIVER_CODE_TOTAL = (driver_code_end - driver_code_start);
}
SECTIONS
{
    .data : ALIGN(4)
    {



        media_data_start = .;

        . = ALIGN(4);
        audio_sync_data_begin = .;
            *(.audio_sync_data)
        audio_sync_data_end = .;

        . = ALIGN(4);



        . = ALIGN(4);
        *(.aec_data)
        . = ALIGN(4);
        *(.res_data)
        . = ALIGN(4);
        *(.ns_data)
        . = ALIGN(4);
        *(.der_data)
        . = ALIGN(4);
        *(.nlp_data)
        . = ALIGN(4);
        *(.qmf_data)
        . = ALIGN(4);
        *(.fft_data)
        . = ALIGN(4);
  *(.bt_audioplc_data)
     *(.jla_decoder_data)
     *(.jla_encoder_data)
     *(.jla_codec_c_data)

  *(.sms_data)

        *(.pcm_data)
        *(.jla_ll_data)
        *(.jla_ll_codec.data)
  *(.cvsd_data)
  *(.g729_data)
  *(.g726_data)
  *(.wtg_dec_data)
  *(.wtgv2_data)
  *(.wtgv2dec_data)
  *(.mp3_data)
  *(.mp3_dec_data)
  *(.msbc_data)
  *(.mty_data)
  *(.mp3tsy_dec_data)
  *(.sbc_data)
  *(.sine_data)
        *(.wav_dec_data)
  *(.wav_data)
  *(.wma_data)
  *(.wma_dec_data)
  *(.amr_data)
  *(.amr_dec_data)
  *(.midi_data)
  *(.ape_data)
  *(.ape_dec_data)
  *(.audio_decoder_data)
  *(.dts_data)
  *(.dts_dec_data)
  *(.flac_data)
  *(.flac_dec_data)
  *(.m4a_data)
  *(.m4a_dec_data)
        *(.mp3_encode_data)
  *(.media_device_data)
  *(.audio_encoder_data)
  *(.mixer_data)
  *(.stream_data)
  *(.dec_server_data)
  *(.rec_server_data)
  *(.auto_mute_data)
  *(.plc_data)
  *(.wireless_sync_data)

  *(.resample_cal_data)
  *(.bt_compressor_data)
  *(.bt_crossOver_data)
  *(.bt_limiter_data)
  *(.bt_compressor_sparse_data)
  *(.bt_limiter_sparse_data)
  *(.bt_crossOver_sparse_data)

  *(.iap_data)
  *(.audio_bfilt_data)
  *(.audio_buf_sync_data)
  *(.audio_dec_app_data)
  *(.audio_dec_app_sync_data)
  *(.audio_dig_vol_data)
  *(.audio_echo_reverb_data)
  *(.audio_echo_src_data)
  *(.audio_energy_detect_data)
  *(.audio_eq_data)
  *(.audio_drc_data)
  *(.audio_eq_drc_apply_data)
  *(.audio_equalloudness_data)
  *(.audio_howling_data)
  *(.audio_noisegate_data)
  *(.audio_output_dac_data)
  *(.audio_pemafrow_data)
  *(.audio_pitch_data)
  *(.audio_pitchspeed_data)
  *(.audio_surround_data)
  *(.audio_vbass_data)
  *(.audio_vocal_remove_data)
  *(.audio_vocal_tract_synthesis_data)
  *(.eq_config_data)
  *(.eq_config_protocol_data)
  *(.spectrum_eq_data)
  *(.spectrum_fft_data)
  *(.audio_localtws_data)
  *(.audio_dec_localtws_data)
  *(.audio_dec_a2dp_data)
  *(.audio_dec_esco_data)
  *(.audio_dec_file_data)
  *(.audio_dec_pcm_data)
  *(.audio_resample_data)
  *(.audio_splicing_data)
  *(.channel_switch_data)
  *(.sw_drc_data)
  *(.aptx_decoder_data)
  *(.ldac_decoder_data)
  *(.speex_encoder_data)
  *(.sbc_encoder_data)
        *(.sbc_eng_data)
  *(.opus_encoder_data)
  *(.prevent_task_fill_data)
  *(.audio_cfifo_data)
  *(.audio_track_data)
  *(.audio_adc_data)
  *(.audio_dac_data)
  *(.audio_linein_data)
  *(.audio_link_data)
  *(.audio_sample_sync_data)
  *(.audio_wireless_sync_data)
  *(.audio_spdif_data)
  *(.audio_src_data)
  *(.audio_src_base_data)
  *(.effectrs_sync_data)
  *(.eq_design_data)
  *(.hw_eq_data)
  *(.iic_data)
  *(.mic_cap_data)
  *(.sbc_hwaccel_data)

  *(.resample_fastcal_data)
  *(.aac_data)
  *(.alac_data)
        . = ALIGN(4);
  *(.dec2tws_mem)
        *(.midi_mem)
  *(.midi_ctrl_mem)
  *(.lfwordana_data)
        *(.anc_data)
        *(.anc_user_data)
        *(.anc_box_data)
        *(.anc_btspp_data)
        *(.anc_debug_data)

        *(.dac_buff)
        *(.dec_mix_buff)
  *(.pitchshifter_data)

        . = ALIGN(4);
     *(.llns.data)
        *(.jlsp_data)

        media_data_end = .;
 } > ram0

    .bss (NOLOAD) :ALIGN(4)
    {


        media_bss_start = .;
        . = ALIGN(4);
        audio_sync_bss_begin = .;
        *(.audio_sync_bss)
        audio_sync_bss_end = .;

        . = ALIGN(4);
  *(.aec_bss)
  *(.res_bss)
  *(.ns_bss)
  *(.der_bss)
  *(.nlp_bss)
  *(.qmf_bss)
  *(.bt_audioplc_bss)
  *(.jla_decoder_bss)
  *(.jla_encoder_bss)
  *(.jla_codec_c_bss)
  *(.jla_codec_ari_c_bss)

  *(.fft_bss)
  *(.sms_bss)
        *(.pcm_bss)
        *(.jla_ll_bss)
  *(.cvsd_bss)
  *(.g729_bss)
  *(.g726_bss)
  *(.wtg_dec_bss)
  *(.wtgv2_bss)
  *(.wtgv2dec_bss)
  *(.mp3_bss)
  *(.mp3_dec_bss)
  *(.msbc_bss)
  *(.mty_bss)
  *(.mp3tsy_dec_bss)
  *(.sbc_bss)
  *(.sine_bss)
        *(.wav_bss)
  *(.wav_dec_bss)
  *(.alac_bss)
  *(.alac_dec_bss)
  *(.wma_bss)
  *(.wma_dec_bss)
  *(.aac_bss)
  *(.amr_bss)
  *(.amr_dec_bss)
  *(.midi_bss)
  *(.ape_bss)
  *(.ape_dec_bss)
  *(.audio_decoder_bss)
  *(.dts_bss)
  *(.dts_dec_bss)
  *(.flac_bss)
  *(.flac_dec_bss)
  *(.m4a_bss)
  *(.m4a_dec_bss)
  *(.alac_bss)
        *(.mp3_encode_bss)
  *(.media_device_bss)
  *(.audio_encoder_bss)
  *(.mixer_bss)
  *(.stream_bss)
  *(.dec_server_bss)
  *(.rec_server_bss)
  *(.auto_mute_bss)
  *(.plc_bss)
  *(.wireless_sync_bss)

  *(.audio_dec_bss)
  *(.audio_buf)

  *(.resample_cal_bss)
  *(.bt_crossOver_bss)
  *(.bt_compressor_bss)
  *(.bt_limiter_bss)
  *(.bt_limiter_sparse_bss)
  *(.bt_crossOver_sparse_bss)
  *(.bt_compressor_sparse_bss)

  *(.iap_bss)
  *(.audio_bfilt_bss)
  *(.audio_buf_sync_bss)
  *(.audio_dec_app_bss)
  *(.audio_dec_app_sync_bss)
  *(.audio_dig_vol_bss)
  *(.audio_echo_reverb_bss)
  *(.audio_echo_src_bss)
  *(.audio_energy_detect_bss)
  *(.audio_eq_bss)
  *(.audio_drc_bss)
  *(.audio_eq_drc_apply_bss)
  *(.audio_equalloudness_bss)
  *(.audio_howling_bss)
  *(.audio_noisegate_bss)
  *(.audio_output_dac_bss)
  *(.audio_pemafrow_bss)
  *(.audio_pitch_bss)
  *(.audio_pitchspeed_bss)
  *(.audio_surround_bss)
  *(.audio_vbass_bss)
  *(.audio_vocal_remove_bss)
  *(.audio_vocal_tract_synthesis_bss)
  *(.eq_config_bss)
  *(.eq_config_protocol_bss)
  *(.spectrum_eq_bss)
  *(.spectrum_fft_bss)
  *(.audio_localtws_bss)
  *(.audio_dec_localtws_bss)
  *(.audio_dec_a2dp_bss)
  *(.audio_dec_esco_bss)
  *(.audio_dec_file_bss)
  *(.audio_dec_pcm_bss)
  *(.audio_resample_bss)
  *(.audio_splicing_bss)
  *(.channel_switch_bss)
  *(.sw_drc_bss)
  *(.aptx_decoder_bss)
  *(.ldac_decoder_bss)
  *(.speex_encoder_bss)
  *(.sbc_encoder_bss)
        *(.sbc_eng_bss)
  *(.opus_encoder_bss)
  *(.prevent_task_fill_bss)
  *(.audio_cfifo_bss)
  *(.audio_track_bss)
  *(.audio_adc_bss)
  *(.audio_dac_bss)
  *(.audio_linein_bss)
  *(.audio_link_bss)
  *(.audio_sample_sync_bss)
  *(.audio_wireless_sync_bss)
  *(.audio_spdif_bss)
  *(.audio_src_bss)
  *(.audio_src_base_bss)
        *(.src_filt)
        *(.src_dma)
  *(.effectrs_sync_bss)
  *(.eq_design_bss)
  *(.hw_eq_bss)
  *(.iic_bss)
  *(.mic_cap_bss)
  *(.sbc_hwaccel_bss)
  *(.resample_fastcal_bss)
  *(.lfwordana_bss)

        *(.anc_bss)
        *(.anc_user_bss)
        *(.anc_box_bss)
        *(.anc_btspp_bss)
        *(.anc_debug_bss)

        *(.llns.data.bss)
        *(.jlsp_bss)
        media_bss_end = .;
 } > ram0

    .text : ALIGN(4)
    {


        media_text_start = .;

  *(.aec_code)
  *(.aec_const)
  *(.der_code)
  *(.der_const)
  *(.nlp_code)
  *(.nlp_const)
  *(.qmf_code)
  *(.qmf_const)
  *(.bt_audioplc_sparse_code)
  *(.bt_audioplc_code)
  *(.bt_audioplc_const)
        *(.lf_audioplc_code)
  *(.lf_audioplc_sparse_code)
  *(.lf_audioplc_const)
  . = ALIGN(4);
  *(.res_code)
  *(.res_const)
        *(.bark_const)

  . = ALIGN(4);
  *(.ns_code)
  *(.ns_const)
  *(.opcore_maskrom)

  . = ALIGN(4);
        *(.pcm_code)
        *(.pcm_const)
        *(.jla_ll_codec.text)
        *(.jla_ll_codec.text.const)
  *(.g729_code)
  *(.g729_const)
  *(.g726_code)
  *(.g726_const)

  *(.wtg_dec_code)
  *(.wtg_dec_const)
  *(.wtg_dec_sparse_code)
  *(.wtg_dec_sparse_const)
  *(.lib_wtg_dec_code )
  *(.lib_wtg_dec_const)
  *(.wtgv2_code)
  *(.wtgv2_const)
  *(.wtgv2dec_code)
  *(.wtgv2dec_const)
  *(.wtgv2dec_str)
  *(.wtg_decv2_sparse_code)
        *(.bfilt_code)
        . = ALIGN(4);
  *(.mp3_decstream_const)
  *(.mp3_decstream_code)
  *(.mp3_decstream_sparse_code)
  *(.mp3_decstream_sparse_const)

  *(.mp3_dec_sparse_code)
  *(.mp3_dec_sparse_const)

  *(.mp3_dec_code)
  *(.mp3_dec_const)
  *(.mp3_const)
  *(.mp3_code)

  *(.msbc_code)
  *(.msbc_const)
  *(.mty_code)
  *(.mty_const)
  *(.mp3tsy_dec_code)
  *(.mp3tsy_dec_const)
  *(.mp3tsy_dec_sparse_code)
  *(.mp3tsy_dec_sparse_const)
  *(.sbc_code)
  *(.sbc_const)
  *(.sine_code)
  *(.sine_const)

        *(.wav_dec_sparse_code)
  *(.wav_dec_sparse_const)
  *(.wav_dec_code)
  *(.wav_dec_const)
     *(.wav_const)
  *(.wav_code)

  *(.alac_const)
  *(.alac_code)
  *(.alac_dec_code)

  *(.wma_code)
  *(.wma_const)
  *(.wma_dec_code)
  *(.wma_dec_const)
  *(.wma_dec_sparse_code)
  *(.wma_dec_sparse_const)
  *(.wma_decstream_const)
  *(.wma_decstream_code)
  *(.wma_decstream_sparse_code)
  *(.wma_decstream_sparse_const)
  *(.aac_code)
  *(.aac_const)
  *(.amr_dec_sparse_code)
  *(.amr_dec_sparse_const)
  *(.amr_dec_code)
  *(.amr_dec_const)
  *(.amr_code)
  *(.amr_const)
  *(.midi_code)
  *(.midi_const)

     *(.lfwordnan_const)
     *(.lfwordana_const)
  *(.lfwordana_code)
  *(.lfwordana_sparse_code)

  *(.ape_dec_sparse_code)
  *(.ape_dec_sparse_const)
  *(.ape_dec_code)
  *(.ape_dec_const)
  *(.ape_code)
  *(.ape_const)
  *(.audio_decoder_code)
  *(.audio_decoder_const)
  *(.cvsd_code)
  *(.cvsd_const)
  *(.dts_dec_ff_const)
  *(.dts_dec_sparse_code)
  *(.dts_dec_sparse_const)
  *(.dts_dec_code)
  *(.dts_dec_const)
  *(.dts_code)
  *(.dts_const)
  *(.flac_dec_sparse_code)
  *(.flac_dec_sparse_const)
  *(.flac_dec_code)
  *(.flac_dec_const)
  *(.flac_const)
  *(.flac_code)
  *(.m4a_dec_code)
  *(.m4a_dec_ff_const)
  *(.m4a_dec_const)
  *(.m4a_code)
  *(.m4a_const)
        *(.mp3_encode_code)
        *(.mp3_encode_const)
  *(.media_device_code)
  *(.media_device_const)
  *(.audio_encoder_code)
  *(.audio_encoder_const)
  *(.mixer_code)
  *(.mixer_const)

  *(.stream_code)
  *(.stream_const)

  *(.dec_server_code)
  *(.dec_server_const)
  *(.rec_server_code)
  *(.rec_server_const)
  *(.auto_mute_code)
  *(.auto_mute_const)
  *(.plc_code)
  *(.plc_const)
  *(.wireless_sync_code)
  *(.wireless_sync_const)
  *(.fft_code)
  *(.fft_const)


  *(.mp3_enc_code)
  *(.mp3_enc_const)
  *(.mp3_enc_sparse_code)
  *(.mp3_enc_sparse_const)
  . = ALIGN(4);
  *(.mp2_encode_code)
  *(.mp2_encode_const)
  *(.mp2_encode_sparse_code)
  *(.mp2_encode_sparse_const)
  . = ALIGN(4);
  *(.adpcm_encode_code)
  *(.adpcm_encode_const)
  *(.adpcm_encode_sparse_code)
  *(.adpcm_encode_sparse_const)
  . = ALIGN(4);


  *(.m4a_dec_sparse_code)
  *(.m4a_dec_sparse_const)

   *(.aac_dec_sparse_code)
  *(.aac_dec_sparse_const)
  *(.bt_aac_dec_core_const)
  *(.bt_aac_dec_eng_const)
  *(.bt_aac_dec_eng_code)
  *(.bt_aac_dec_core_code)
  *(.bt_aac_dec_core_sparse_code)


  *(.res_sparse_code)
  *(.ns_sparse_code )
  *(.aec_sparse_code)
  *(.nlp_sparse_code)
  *(.der_sparse_code)
  *(.qmf_sparse_code)

  *(.sms_const);
  *(.sms_sparse_code);
  *(.sms_code);


  *(.resample_cal_code)
  *(.resample_cal_sparse_code)
  *(.resample_cal_const)

  *(.bt_compressor_sparse_const)
  *(.bt_compressor_sparse_code)
  *(.compressor_sparse_code)
  *(.compressor_sparse_const)

  *(.bt_limiter_sparse_const)
  *(.bt_limiter_sparse_code)
  *(.limiter_sparse_code)
  *(.limiter_sparse_const)

  *(.bt_crossOver_sparse_const)
  *(.bt_crossOver_sparse_code)
  *(.crossOver_sparse_code)
  *(.crossOver_sparse_const)

  *(.iap_code)
  *(.iap_const)
  *(.audio_bfilt_code)
  *(.audio_bfilt_const)
  *(.audio_buf_sync_code)
  *(.audio_buf_sync_const)
  *(.audio_dec_app_code)
  *(.audio_dec_app_const)
  *(.audio_dec_app_sync_code)
  *(.audio_dec_app_sync_const)
  *(.audio_dec_app_event_callback)
  *(.audio_dec_app_dec_run)
  *(.audio_dec_app_normal_run)
  *(.audio_dec_tone_event_callback)
  *(.audio_dig_vol_code)
  *(.audio_dig_vol_const)
  *(.audio_echo_reverb_code)
  *(.audio_echo_reverb_const)
        *(.echo_cal_code)
  *(.echo_cal_const)


  *(.platereverb_code)
  *(.platereverb_const)

  *(.reverb0_code)
  *(.reverb0_const)
  *(.reverb_cal_sparse_code)
  *(.audio_echo_src_code)
  *(.audio_echo_src_const)
  *(.audio_energy_detect_code)
  *(.audio_energy_detect_const)

  *(.audio_eq_code)
  *(.audio_eq_const)


     *(.audio_drc_code)
  *(.audio_drc_const)
        *(.drc_const)
  *(.drc_code)
  *(.sw_drc_code)
  *(.sw_drc_const)

  *(.audio_eq_drc_apply_code)
  *(.audio_eq_drc_apply_const)
  *(.audio_equalloudness_code)
  *(.audio_equalloudness_const)
  *(.audio_howling_code)
  *(.audio_howling_const)
        *(.howlings_phf_sparse_code)

        *(.howlings_phf_code)
  *(.howlings_phf_const)

        *(.notchhowling_code)
  *(.notchhowling_const)
  *(.notchhowling_sparse_code)
  *(.notchhowling_sparse_const)
  *(.audio_output_dac_code)
  *(.audio_output_dac_const)
  *(.audio_pemafrow_code)
  *(.audio_pemafrow_const)
  *(.audio_pitch_code)
  *(.audio_pitch_const)
  *(.audio_pitchspeed_code)
  *(.audio_pitchspeed_const)
  *(.audio_surround_code)
  *(.audio_surround_const)

  *(.audio_vocal_remove_code)
  *(.audio_vocal_remove_const)
  *(.audio_vocal_tract_synthesis_code)
  *(.audio_vocal_tract_synthesis_const)
  *(.eq_config_code)
  *(.eq_config_const)
  *(.eq_config_protocol_code)
  *(.eq_config_protocol_const)
  *(.spectrum_eq_code)
  *(.spectrum_eq_const)
  *(.spectrum_fft_code)
  *(.spectrum_fft_const)
  *(.audio_localtws_code)
  *(.audio_localtws_const)
  *(.audio_dec_localtws_code)
  *(.audio_dec_localtws_const)
  *(.audio_dec_a2dp_code)
  *(.audio_dec_a2dp_const)
  *(.audio_dec_esco_code)
  *(.audio_dec_esco_const)
  *(.audio_dec_file_code)
  *(.audio_dec_file_const)
  *(.audio_dec_pcm_code)
  *(.audio_dec_pcm_const)
  *(.audio_resample_code)
  *(.audio_resample_const)
  *(.audio_splicing_code)
  *(.audio_splicing_const)
  *(.channel_switch_code)
  *(.channel_switch_const)

  *(.aptx_decoder_code)
  *(.aptx_decoder_const)
  *(.ldac_decoder_code)
  *(.ldac_decoder_const)
  *(.speex_encoder_code)
  *(.speex_encoder_const)
  *(.sbc_encoder_code)
  *(.sbc_encoder_const)
  *(.sbc_eng_code)
  *(.sbc_eng_const)
  *(.opus_encoder_code)
  *(.opus_encoder_const)
  *(.prevent_task_fill_code)
  *(.prevent_task_fill_const)
  *(.audio_cfifo_code)
  *(.audio_cfifo_const)
  *(.audio_track_code)
  *(.audio_track_const)
  *(.audio_adc_code)
  *(.audio_adc_const)
  *(.audio_dac_code)
  *(.audio_dac_const)
  *(.audio_linein_code)
  *(.audio_linein_const)
  *(.audio_link_code)
  *(.audio_link_const)
  *(.audio_sample_sync_code)
  *(.audio_sample_sync_const)
  *(.audio_wireless_sync_code)
  *(.audio_wireless_sync_const)
  *(.audio_spdif_code)
  *(.audio_spdif_const)
  *(.audio_src_code)
  *(.audio_src_const)
  *(.audio_src_base_code)
  *(.audio_src_base_const)
  *(.effectrs_sync_code)
  *(.effectrs_sync_const)
  *(.eq_design_code)
  *(.eq_design_const)
  *(.hw_eq_code)
  *(.hw_eq_const)
  *(.iic_code)
  *(.iic_const)
  *(.mic_cap_code)
  *(.mic_cap_const)
  *(.sbc_hwaccel_code)
  *(.sbc_hwaccel_const)
  *(.audio_gain_code)
  *(.audio_gain_const)
  *(.plateReverb_adv.text.cache.L2.run)
  *(.plateReverb_adv.text)
  *(.plateReverb_adv.text.const)

  . = ALIGN(4);
        *(.llns.text.const)
        *(.llns.text)
        *(.jlsp_code)
        *(.jlsp_const)

  . = ALIGN(4);
     _audio_decoder_begin = .;
     PROVIDE(audio_decoder_begin = .);
         KEEP(*(.audio_decoder))
     _audio_decoder_end = .;
     PROVIDE(audio_decoder_end = .);

     _audio_encoder_begin = .;
     PROVIDE(audio_encoder_begin = .);
         KEEP(*(.audio_encoder))
     _audio_encoder_end = .;
     PROVIDE(audio_encoder_end = .);

     _audio_package_begin = .;
     PROVIDE(audio_package_begin = .);
         KEEP(*(.audio_package))
     _audio_package_end = .;
     PROVIDE(audio_package_end = .);

     _audio_dev_begin = .;
     PROVIDE(audio_dev_begin = .);
         KEEP(*(.audio_device))
     _audio_dev_end = .;
     PROVIDE(audio_dev_end = .);

     _audio_hwaccel_begin = .;
     PROVIDE(audio_hwaccel_begin = .);
         KEEP(*(.audio_hwaccel))
     _audio_hwaccel_end = .;
     PROVIDE(audio_hwaccel_end = .);


        _sound_platform_driver_begin = .;
        PROVIDE(sound_platform_driver_begin = .);
            KEEP(*(.sound_platform_driver))
        _sound_platform_driver_end = .;
        PROVIDE(sound_platform_driver_end = .);


  . = ALIGN(4);
     media_code_begin = .;
         *(.media.*.text)

  . = ALIGN(4);
  *(.crossOver_code)
  *(.crossOver_const)
  *(.bt_crossOver_const)
  *(.bt_crossOver_code)
  . = ALIGN(4);

  *(.compressor_code)
  *(.compressor_const)
  *(.bt_compressor_const)
  *(.bt_compressor_code)
  . = ALIGN(4);

  *(.limiter_code)
  *(.limiter_const)
  *(.bt_limiter_const)
  *(.bt_limiter_code)
  *(.drc_sparse_code)
  *(.drc_sparse_const)

        *(.jla_decoder_const)
        *(.jla_decoder_code)
     *(.jla_codec_d_code)
     *(.jla_codec_c_sparse_code)
     *(.jla_codec_t8k_code)
     *(.jla_codec_t16k_code)
     *(.jla_codec_t24k_code)
     *(.jla_codec_t32k_code)
     *(.jla_ll_codec.text.cache.L2.encode)
     *(.jla_ll_codec.text.cache.L2.const)
     *(.jla_ll_codec.text.cache.L2.run)
        *(.jla_ll_const)
        *(.jla_ll_code)


  . = ALIGN(4);
  *(.lib_pitchshift_code)
  *(.lib_pitchshift_const)
  . = ALIGN(4);
  *(.sur_cal_const)
  *(.sur_cal_code)
  . = ALIGN(4);
  *(.reverb_cal_sparse_code)
        . = ALIGN(4);
  *(.dvol_ram_code)
        . = ALIGN(4);
  *(.reverb_cal_const)
        . = ALIGN(4);
     *(.reverb_cal_code)
        . = ALIGN(4);


  *(.stream_code_nor_run)
  *(.mixer_code_nor_run)
  *(.audio_codec_code)
  *(.audio_codec_code_read_file)
  *(.audio_src_code_nor_run)
  *(.src_base_code_nor_run)
  *(.audio_dac_code_nor_run)
  *(.audio_cfifo_code_nor_run)

  *(.stream_code_copy_run)
  *(.mixer_code_out_run)

  *(.audio_codec_code_frame_file)

  *(.buf_sync_code_nor_run)

  *(.resample_fastcal_const)
  *(.resample_fastcal_code)
  *(.resample_fastcal_sparse_code)


  *(.dynamic_eq_code)
  *(.dynamic_eq_const)
     *(.dynamic_eq_detect_code)
  *(.dynamic_eq_detect_const)
  *(.dynamic_eq_sparse_code)
  *(.dynamic_eq_sparse_const)
     *(.dynamic_eq_detect_sparse_code)
  *(.dynamic_eq_detect_sparse_const)




  *(.audio_mic_effect_const)
  *(.audio_mic_effect_code)


  *(.audio_mic_stream_const)
  *(.audio_mic_stream_code)


  *(.audio_effect_adj_const)
  *(.audio_effect_adj_code)


     *(.pitchshifter_code)
     *(.pitchshifter_const)
        *(.pitchshifter_sparse_code)


  *(.audio_vbass_code)
  *(.audio_vbass_const)
  *(.vbss_code)
  *(.vbss_const)
        *(.vbss_sparse_code)
  *(.vbss_sparse_const)


        *(.specshow_code)
        *(.specshow_const)
        *(.specshow_sparse_code)
        *(.specshow_sparse_const)

  *(.media.audio*)
  *(.media.mixer*)
  *(.media.media*)

     audio_sync_code_begin = .;
        *(.audio_sync_code)
     audio_sync_code_end = .;

        . = ALIGN(4);

     media_code_end = .;
  . = ALIGN(4);
  media_code_size = media_code_end - media_code_begin;

  . = ALIGN(4);
        media_text_end = .;

        media_code_total_size = media_text_end - media_text_start;
 } > code0

 .data_code ALIGN(32):
 {



        media_data_code_start = .;
        . = ALIGN(4);
    *(.media.aec.text)
        . = ALIGN(4);


 AudioEffects_data_code_begin = .;
  *(.audio_noisegate_code)
  *(.audio_noisegate_const)

  *(.noisegate_code)
  *(.noisegate_const)
  *(.noisegate_sparse_code)
  *(.noisegate_sparse_const)
        AudioEffects_data_code_end = .;
        AudioEffects_data_code_size = AudioEffects_data_code_end - AudioEffects_data_code_begin;


     *(.jla_encoder_const)
     *(.jla_encoder_code)
     *(.jla_codec_e_code)
     *(.jla_codec_c_const)
     *(.jla_codec_c_code)
     *(.jla_codec_ari_c_const)
     *(.jla_codec_ari_c_code)
     *(.jla_codec_t441k_code)
  . = ALIGN(4);

        media_data_code_end = .;
 } > ram0
}

SECTIONS
{
    .data : ALIGN(4)
    {
        system_ui_font_data_start = .;
        *(.font_Arabic.data)
        *(.font_ascii.data)
        *(.font_big5.data)
        *(.font_gbk.data)
        *(.font_Hebrew.data)
        *(.font_ksc.data)
        *(.font_other_language.data)
        *(.font_sdfs.data)
        *(.font_sjis.data)
        *(.font_textout.data)
        *(.font_thai.data)
        *(.font_utf8_2_unicode.data)
        system_ui_font_data_end = .;
        system_ui_font_data_total_size = system_ui_font_data_end - system_ui_font_data_start;

        system_ui_framework_data_start = .;
        *(.layer.data)
        *(.layout.data)
        *(.lyrics.data)
        *(.ui_battery.data)
        *(.ui_button.data)
        *(.ui_core.data)
        *(.ui_core_api.data)
        *(.ui_core_dot.data)
        *(.ui_file_attrs.data)
        *(.ui_file_browser.data)
        *(.ui_grid.data)
        *(.ui_number.data)
        *(.ui_p.data)
        *(.ui_pic.data)
        *(.ui_progress.data)
        *(.ui_progress_multi.data)
        *(.ui_rotatable_pic.data)
        *(.ui_rotate.data)
        *(.ui_slider.data)
        *(.ui_slider_vert.data)
        *(.ui_text.data)
        *(.ui_time.data)
        *(.ui_watch.data)
        *(.window.data)
        system_ui_framework_data_end = .;
        system_ui_framework_data_total_size = system_ui_framework_data_end - system_ui_framework_data_start;

        system_ui_resource_data_start = .;
        *(.ascii.data)
        *(.memvar.data)
        *(.quicklz.data)
        *(.resfile.data)
        *(.rle.data)
        system_ui_resource_data_end = .;
        system_ui_resource_data_total_size = system_ui_resource_data_end - system_ui_resource_data_start;

        system_ui_timer_data_start = .;
        *(.timer.data)
        system_ui_timer_data_end = .;
        system_ui_timer_data_total_size = system_ui_timer_data_end - system_ui_timer_data_start;

        system_ui_draw_data_start = .;
        *(.buffer_manager.data)
        *(.image_process.data)
        *(.ui_bar_chart.data)
        *(.ui_circle.data)
        *(.ui_figure.data)
        *(.ui_wave.data)
        system_ui_draw_data_end = .;
        system_ui_draw_data_total_size = system_ui_draw_data_end - system_ui_draw_data_start;

        system_ui_simple_data_start = .;
        *(.ap_res.data)
        system_ui_simple_data_end = .;
        system_ui_simple_data_total_size = system_ui_simple_data_end - system_ui_simple_data_start;

        system_ui_vm_data_start = .;
        *(.ui_vm_common.data)
        system_ui_vm_data_end = .;
        system_ui_vm_data_total_size = system_ui_vm_data_end - system_ui_vm_data_start;
        . = ALIGN(4);
    } > ram0

    .bss (NOLOAD) :ALIGN(4)
    {
        system_ui_font_data_bss_start = .;
        *(.font_Arabic.data.bss)
        *(.font_ascii.data.bss)
        *(.font_big5.data.bss)
        *(.font_gbk.data.bss)
        *(.font_Hebrew.data.bss)
        *(.font_ksc.data.bss)
        *(.font_other_language.data.bss)
        *(.font_sdfs.data.bss)
        *(.font_sjis.data.bss)
        *(.font_textout.data.bss)
        *(.font_thai.data.bss)
        *(.font_utf8_2_unicode.data.bss)
        system_ui_font_data_bss_end = .;
        system_ui_font_data_bss_total_size = system_ui_font_data_bss_end - system_ui_font_data_bss_start;

        system_ui_framework_data_bss_start = .;
        *(.layer.data.bss)
        *(.layout.data.bss)
        *(.lyrics.data.bss)
        *(.ui_battery.data.bss)
        *(.ui_button.data.bss)
        *(.ui_core.data.bss)
        *(.ui_core_api.data.bss)
        *(.ui_core_dot.data.bss)
        *(.ui_file_attrs.data.bss)
        *(.ui_file_browser.data.bss)
        *(.ui_grid.data.bss)
        *(.ui_number.data.bss)
        *(.ui_p.data.bss)
        *(.ui_pic.data.bss)
        *(.ui_progress.data.bss)
        *(.ui_progress_multi.data.bss)
        *(.ui_rotatable_pic.data.bss)
        *(.ui_rotate.data.bss)
        *(.ui_slider.data.bss)
        *(.ui_slider_vert.data.bss)
        *(.ui_text.data.bss)
        *(.ui_time.data.bss)
        *(.ui_watch.data.bss)
        *(.window.data.bss)
        system_ui_framework_data_bss_end = .;
        system_ui_framework_data_bss_total_size = system_ui_framework_data_bss_end - system_ui_framework_data_bss_start;

        system_ui_resource_data_bss_start = .;
        *(.ascii.data.bss)
        *(.mem_var.data.bss)
        *(.quicklz.data.bss)
        *(.resfile.data.bss)
        *(.rle.data.bss)
        system_ui_resource_data_bss_end = .;
        system_ui_resource_data_bss_total_size = system_ui_resource_data_bss_end - system_ui_resource_data_bss_start;

        system_ui_timer_data_bss_start = .;
        *(.timer.data.bss)
        system_ui_timer_data_bss_end = .;
        system_ui_timer_data_bss_total_size = system_ui_timer_data_bss_end - system_ui_timer_data_bss_start;

        system_ui_draw_data_bss_start = .;
        *(.buffer_manager.data.bss)
        *(.image_process.data.bss)
        *(.ui_bar_chart.data.bss)
        *(.ui_circle.data.bssa)
        *(.ui_figure.data.bss)
        *(.ui_wave.data.bss)
        system_ui_draw_data_bss_end = .;
        system_ui_draw_data_bss_total_size = system_ui_draw_data_bss_end - system_ui_draw_data_bss_start;

        system_ui_simple_data_bss_start = .;
        *(.ap_res.data.bss)
        system_ui_simple_data_bss_end = .;
        system_ui_simple_data_bss_total_size = system_ui_simple_data_bss_end - system_ui_simple_data_bss_start;

        system_ui_vm_data_bss_start = .;
        *(.ui_vm_common.data.bss)
        system_ui_vm_data_bss_end = .;
        system_ui_vm_data_bss_total_size = system_ui_vm_data_bss_end - system_ui_vm_data_bss_start;
        . = ALIGN(4);
    } > ram0

    .text : ALIGN(4)
    {
        system_ui_font_text_start = .;
        *(.ui_ram)
        *(.font_Arabic.text)
        *(.font_Arabic.text.const)
        *(.font_ascii.text)
        *(.font_ascii.text.const)
        *(.font_big5.text)
        *(.font_big5.text.const)
        *(.font_gbk.text)
        *(.font_gbk.text.const)
        *(.font_Hebrew.text)
        *(.font_Hebrew.text.const)
        *(.font_ksc.text)
        *(.font_ksc.text.const)
        *(.font_other_language.text)
        *(.font_other_language.text.const)
        *(.font_sdfs.text)
        *(.font_sdfs.text.const)
        *(.font_sjis.text)
        *(.font_sjis.text.const)
        *(.font_textout.text)
        *(.font_textout.text.const)
        *(.font_thai.text)
        *(.font_thai.text.const)
        *(.font_utf8_2_unicode.text)
        *(.font_utf8_2_unicode.text.const)
        system_ui_font_text_end = .;
        system_ui_font_text_total_size = system_ui_font_text_end - system_ui_font_text_start;

        system_ui_framework_text_start = .;
        *(.layer.text)
        *(.layout.text)
        *(.lyrics.text)
        *(.ui_battery.text)
        *(.ui_button.text)
        *(.ui_core.text)
        *(.ui_core_api.text)
        *(.ui_core_dot.text)
        *(.ui_file_attrs.text)
        *(.ui_file_browser.text)
        *(.ui_grid.text)
        *(.ui_number.text)
        *(.ui_p.text)
        *(.ui_pic.text)
        *(.ui_progress.text)
        *(.ui_progress_multi.text)
        *(.ui_rotatable_pic.text)
        *(.ui_rotate.text)
        *(.ui_slider.text)
        *(.ui_slider_vert.text)
        *(.ui_text.text)
        *(.ui_time.text)
        *(.ui_watch.text)
        *(.window.text)
        *(.layer.text.const)
        *(.layout.text.const)
        *(.lyrics.text.const)
        *(.ui_battery.text.const)
        *(.ui_button.text.const)
        *(.ui_core.text.const)
        *(.ui_core_api.text.const)
        *(.ui_core_dot.text.const)
        *(.ui_file_attrs.text.const)
        *(.ui_file_browser.text.const)
        *(.ui_grid.text.const)
        *(.ui_number.text.const)
        *(.ui_p.text.const)
        *(.ui_pic.text.const)
        *(.ui_progress.text.const)
        *(.ui_progress_multi.text.const)
        *(.ui_rotatable_pic.text.const)
        *(.ui_rotate.text.const)
        *(.ui_slider.text.const)
        *(.ui_slider_vert.text.const)
        *(.ui_text.text.const)
        *(.ui_time.text.const)
        *(.ui_watch.text.const)
        *(.window.text.const)
        system_ui_framework_text_end = .;
        system_ui_framework_text_total_size = system_ui_framework_text_end - system_ui_framework_text_start;

        system_ui_resource_text_start = .;
        *(.ascii.text)
        *(.mem_var.text)
        *(.quicklz.text)
        *(.resfile.text)
        *(.rle.text)
        *(.ascii.text.const)
        *(.memvar.text.const)
        *(.quicklz.text.const)
        *(.resfile.text.const)
        *(.rle.text.const)
        system_ui_resource_text_end = .;
        system_ui_resource_text_total_size = system_ui_resource_text_end - system_ui_resource_text_start;

        system_ui_timer_text_start = .;
        *(.timer.text)
        *(.timer.text.const)
        system_ui_timer_text_end = .;
        system_ui_timer_text_total_size = system_ui_timer_text_end - system_ui_timer_text_start;

        system_ui_draw_text_start = .;
        *(.buffer_manager.text)
        *(.image_process.text)
        *(.ui_bar_chart.text)
        *(.ui_circle.text)
        *(.ui_figure.text)
        *(.ui_wave.text)
        *(.buffer_manager.text.const)
        *(.image_process.text.const)
        *(.ui_bar_chart.text.const)
        *(.ui_circle.text.const)
        *(.ui_figure.text.const)
        *(.ui_wave.text.const)
        system_ui_draw_text_end = .;
        system_ui_draw_text_total_size = system_ui_draw_text_end - system_ui_draw_text_start;

        system_ui_simple_text_start = .;
        *(.ap_res.text)
        *(.ap_res.text.const)
        system_ui_simple_text_end = .;
        system_ui_simple_text_total_size = system_ui_simple_text_end - system_ui_simple_text_start;

        system_ui_vm_text_start = .;
        *(.ui_vm_common.text)
        *(.ui_vm_common.text.const)
        system_ui_vm_text_end = .;
        system_ui_vm_text_total_size = system_ui_vm_text_end - system_ui_vm_text_start;

        . = ALIGN(4);
        lcd_interface_begin = .;
        KEEP(*(.lcd_if_info))
        lcd_interface_end = .;

  ui_style_begin = .;
  KEEP(*(.ui_style))
  ui_style_end = .;


        elm_event_handler_begin_JL = .;
        KEEP(*(.elm_event_handler_JL))
  elm_event_handler_end_JL = .;

        elm_event_handler_begin_UPGRADE = .;
        KEEP(*(.elm_event_handler_UPGRADE))
        elm_event_handler_end_UPGRADE = .;



  elm_event_handler_begin_DIAL = .;
        KEEP(*(.elm_event_handler_DIAL))
  elm_event_handler_end_DIAL = .;


  control_event_handler_begin = .;
        KEEP(*(.control_event_handler))
  control_event_handler_end = .;

        control_ops_begin = .;
        KEEP(*(.control_ops))
        control_ops_end = .;

        battery_notify_begin = .;
        *(.battery_notify)
        battery_notify_end = .;
        . = ALIGN(4);
    } > code0
}


text_begin = ADDR(.text);
text_size = SIZEOF(.text);
text_end = text_begin + text_size;
ASSERT((text_size % 4) == 0,"!!! text_size Not Align 4 Bytes !!!");

bss_begin = ADDR(.bss);
bss_size = SIZEOF(.bss);
bss_end = bss_begin + bss_size;
ASSERT((bss_size % 4) == 0,"!!! bss_size Not Align 4 Bytes !!!");

data_addr = ADDR(.data);
data_begin = text_begin + text_size;
data_size = SIZEOF(.data);
ASSERT((data_size % 4) == 0,"!!! data_size Not Align 4 Bytes !!!");

moveable_slot_addr = ADDR(.moveable_slot);
moveable_slot_begin = data_begin + data_size;
moveable_slot_size = SIZEOF(.moveable_slot);

data_code_addr = ADDR(.data_code);
data_code_begin = moveable_slot_begin + moveable_slot_size;
data_code_size = SIZEOF(.data_code);
ASSERT((data_code_size % 4) == 0,"!!! data_code_size Not Align 4 Bytes !!!");




aec_addr = ADDR(.overlay_aec);
aec_begin = data_code_begin + data_code_size;
aec_size = SIZEOF(.overlay_aec);

aac_addr = ADDR(.overlay_aac);
aac_begin = aec_begin + aec_size;
aac_size = SIZEOF(.overlay_aac);
bank_code_load_addr = text_end;


ASSERT(_HEAP_BEGIN > bss_begin,"_HEAP_BEGIN < bss_begin");
ASSERT(_HEAP_BEGIN > data_addr,"_HEAP_BEGIN < data_addr");
ASSERT(_HEAP_BEGIN > data_code_addr,"_HEAP_BEGIN < data_code_addr");



PROVIDE(HEAP_BEGIN = _HEAP_BEGIN);
PROVIDE(HEAP_END = _HEAP_END);
_MALLOC_SIZE = _HEAP_END - _HEAP_BEGIN;
PROVIDE(MALLOC_SIZE = _HEAP_END - _HEAP_BEGIN);






report_text_beign = ADDR(.text);
report_text_size = SIZEOF(.text);
report_text_end = report_text_beign + report_text_size;

report_mmu_tlb_begin = ADDR(.mmu_tlb);
report_mmu_tlb_size = SIZEOF(.mmu_tlb);
report_mmu_tlb_end = report_mmu_tlb_begin + report_mmu_tlb_size;

report_boot_info_begin = ADDR(.boot_info);
report_boot_info_size = SIZEOF(.boot_info);
report_boot_info_end = report_boot_info_begin + report_boot_info_size;

report_irq_stack_begin = ADDR(.irq_stack);
report_irq_stack_size = SIZEOF(.irq_stack);
report_irq_stack_end = report_irq_stack_begin + report_irq_stack_size;

report_data_begin = ADDR(.data);
report_data_size = SIZEOF(.data);
report_data_end = report_data_begin + report_data_size;

report_bss_begin = ADDR(.bss);
report_bss_size = SIZEOF(.bss);
report_bss_end = report_bss_begin + report_bss_size;

report_data_code_begin = ADDR(.data_code);
report_data_code_size = SIZEOF(.data_code);
report_data_code_end = report_data_code_begin + report_data_code_size;

report_overlay_begin = __report_overlay_begin;
report_overlay_size = __report_overlay_end - __report_overlay_begin;
report_overlay_end = __report_overlay_end;

report_heap_beign = _HEAP_BEGIN;
report_heap_size = _HEAP_END - _HEAP_BEGIN;
report_heap_end = _HEAP_END;

BR28_PHY_RAM_SIZE = PHY_RAM_SIZE;
BR28_SDK_RAM_SIZE = report_mmu_tlb_size + report_boot_info_size + report_irq_stack_size + report_data_size + report_bss_size + report_overlay_size + report_data_code_size + report_heap_size;

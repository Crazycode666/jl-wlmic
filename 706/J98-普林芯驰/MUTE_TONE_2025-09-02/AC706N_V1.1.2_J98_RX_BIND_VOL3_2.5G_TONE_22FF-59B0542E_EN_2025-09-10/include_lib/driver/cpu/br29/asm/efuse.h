#ifndef  __EFUSE_H__
#define  __EFUSE_H__


u16 get_chip_id();
u16 get_vbat_trim();
u16 get_vbat_trim_435();
u16 get_charge_cur_trim();
u16 get_vbg_trim();
u16 get_vbat4p35_trim();
u16 get_vbat_chg_curr_trim();
u32 get_wvdd_trim();
u32 get_chip_version();

u16 get_lrc_ws_inc();			//from uboot
u16 get_lrc_ws_init();		//from uboot
u16 get_btosc_ws_inc();		//from uboot
u16 get_btosc_ws_init();		//from uboot
u8 get_lrc_change_mode();	//from uboot

u32 get_boot_flag();
void set_boot_flag(u32 flag);
#endif  /*EFUSE_H*/

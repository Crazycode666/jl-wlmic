@echo off

@echo *********************************************************************
@echo 			                BR29 SDK 
@echo *********************************************************************
@echo %date%
cd %~dp0

copy ..\..\script.ver .
copy ..\..\tone.cfg .
copy ..\..\br29loader.bin .
copy ..\..\ota.bin .

..\..\isd_download.exe ..\..\isd_config.ini -tonorflash -dev br29 -boot 0x102000 -div8 -wait 300 -uboot ..\..\uboot.boot -app ..\..\app.bin -res tone.cfg ..\..\cfg_tool.bin -uboot_compress -key kph_7247.key
:: -format all


@REM 常用命令说明
@rem -format vm         // 擦除VM 区域
@rem -format all        // 擦除所有
@rem -reboot 500        // reset chip, valid in JTAG debug

@rem 删除临时文件-format all
if exist *.mp3 del *.mp3 
if exist *.PIX del *.PIX
if exist *.TAB del *.TAB
if exist *.res del *.res
if exist *.sty del *.sty

@rem 生成固件升级文件
..\..\fw_add.exe -noenc -fw jl_isd.fw -add ..\..\ota.bin -type 100 -out jl_isd.fw

..\..\ufw_maker.exe -fw_to_ufw jl_isd.fw
copy jl_isd.ufw update.ufw
del jl_isd.ufw

@REM 生成配置文件升级文件
::ufw_maker.exe -chip AC800X %ADD_KEY% -output config.ufw -res bt_cfg.cfg

::IF EXIST jl_696x.bin del jl_696x.bin 

if exist script.ver del script.ver
if exist tone.cfg del tone.cfg
if exist br29loader.bin del br29loader.bin

@rem 常用命令说明
@rem -format vm        //擦除VM 区域
@rem -format cfg       //擦除BT CFG 区域
@rem -format 0x3f0-2   //表示从第 0x3f0 个 sector 开始连续擦除 2 个 sector(第一个参数为16进制或10进制都可，第二个参数必须是10进制)

ping /n 2 127.1>null
IF EXIST null del null


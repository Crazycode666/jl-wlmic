// *INDENT-OFF*
#include "app_config.h"

#ifdef __SHELL__

##!/bin/sh

${OBJDUMP} -D -address-mask=0x7ffffff -print-imm-hex -print-dbg -mcpu=r3 $1.elf > $1.lst
${OBJCOPY} -O binary -j .text $1.elf text.bin
${OBJCOPY} -O binary -j .data  $1.elf data.bin
${OBJCOPY} -O binary -j .data_code $1.elf data_code.bin
${OBJCOPY} -O binary -j .overlay_aec $1.elf aec.bin
${OBJCOPY} -O binary -j .overlay_aac $1.elf aac.bin
${OBJCOPY} -O binary -j .ps_ram_data_code $1.elf ps_ram_data_code.bin

#ifdef CONFIG_LZ4_DATA_CODE_ENABLE
lz4_packet -dict text.bin -input data.bin 0 data_code.bin 0 -o bank.bin
#endif

${OBJDUMP} -section-headers -address-mask=0x7ffffff $1.elf
${OBJSIZEDUMP} -lite -skip-zero -enable-dbg-info $1.elf | sort -k 1 >  symbol_tbl.txt

#ifdef CONFIG_LZ4_DATA_CODE_ENABLE
cat text.bin bank.bin aec.bin aac.bin ps_ram_data_code.bin > app.bin
#else
cat text.bin data.bin data_code.bin aec.bin aac.bin ps_ram_data_code.bin > app.bin
#endif


/opt/utils/strip-ini -i isd_config.ini -o isd_config.ini

files="app.bin ${CPU}loader.* uboot* isd_config.ini ota.*"

NICKNAME="${CPU}_sdk"

host-client -project ${NICKNAME}$2_${APP_CASE} -f ${files} $1.elf

#else


@echo off
Setlocal enabledelayedexpansion
@echo ********************************************************************************
@echo           SDK BR29
@echo ********************************************************************************
@echo %date%

cd /d %~dp0

if not %KEY_FILE_PATH%A==A set KEY_FILE=-key %KEY_FILE_PATH%

set OBJDUMP=C:\JL\pi32\bin\llvm-objdump.exe
set OBJCOPY=C:\JL\pi32\bin\llvm-objcopy.exe
set ELFFILE=sdk.elf
set LZ4_PACKET=.\lz4_packet.exe

REM %OBJDUMP% -D -address-mask=0x7ffffff -print-imm-hex -mcpu=r3 -print-dbg sdk.elf > sdk.lst
%OBJCOPY% -O binary -j .text %ELFFILE% text.bin
%OBJCOPY% -O binary -j .data %ELFFILE% data.bin
%OBJCOPY% -O binary -j .data_code %ELFFILE% data_code.bin
%OBJCOPY% -O binary -j .overlay_aac %ELFFILE% aac.bin

#ifdef CONFIG_LZ4_DATA_CODE_ENABLE
LZ4_PACKET -dict text.bin -input data.bin 0 data_code.bin 0 -o bank.bin
#endif



%OBJDUMP% -section-headers -address-mask=0x7ffffff %ELFFILE%
REM %OBJDUMP% -t %ELFFILE% >  symbol_tbl.txt

#ifdef CONFIG_LZ4_DATA_CODE_ENABLE
copy /b text.bin + bank.bin + aac.bin app.bin
#else
copy /b text.bin + data.bin + data_code.bin + aac.bin app.bin
#endif

del aac.bin
del data_code.bin
del data.bin
del text.bin


call download/soundbox/download.bat

#endif


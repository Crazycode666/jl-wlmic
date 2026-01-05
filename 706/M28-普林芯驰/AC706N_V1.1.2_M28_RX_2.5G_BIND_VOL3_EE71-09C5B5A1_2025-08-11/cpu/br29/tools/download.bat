









@echo off
Setlocal enabledelayedexpansion
@echo ********************************************************************************
@echo SDK BR29
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


LZ4_PACKET -dict text.bin -input data.bin 0 data_code.bin 0 -o bank.bin




%OBJDUMP% -section-headers -address-mask=0x7ffffff %ELFFILE%
REM %OBJDUMP% -t %ELFFILE% > symbol_tbl.txt


copy /b text.bin + bank.bin + aac.bin app.bin




del aac.bin
del data_code.bin
del data.bin
del text.bin


call download/soundbox/download.bat

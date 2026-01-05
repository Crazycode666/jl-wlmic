SECTIONS
{
    .data : ALIGN(4)
    {
		#include "audio_lib_data.ld"
    } > ram0

    .bss (NOLOAD) :ALIGN(4)
    {
		#include "audio_lib_bss.ld"
    } > ram0

    .text : ALIGN(4)
    {
		#include "audio_lib_text.ld"
    } > code0

	.data_code ALIGN(32):
	{
		#include "audio_lib_data_text.ld"
	} > ram0
}


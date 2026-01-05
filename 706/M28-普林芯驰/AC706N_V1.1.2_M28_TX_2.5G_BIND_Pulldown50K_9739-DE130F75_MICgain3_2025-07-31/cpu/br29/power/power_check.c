#include "asm/power_interface.h"

AT_VOLATILE_RAM_CODE
u8 soff_safety_check(void)
{
    return 0;
}


AT_VOLATILE_RAM_CODE
u8 sleep_safety_check(void)
{
    return 0;
}

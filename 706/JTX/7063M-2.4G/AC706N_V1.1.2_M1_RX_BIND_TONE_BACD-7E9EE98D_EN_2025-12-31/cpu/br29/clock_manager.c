#include "system/includes.h"
#include "app_config.h"
#include "clock_cfg.h"
#include "asm/dac.h"
struct clock_type {
    u8 type;
    u32 clock;
    const char *name;
};

/*****

idle clk : 模式里面的空闲时钟

时钟通过idle clk 加上每一种解码或者运算的时钟
累加总和来设置需要的时钟

模式空闲时钟设置
void clock_idle(u32 type)


把时钟设置加入到ext中，但是不是立刻设置时钟
需要最后调用clock_set_cur来最后设置时钟
用于连续地方设置时钟
用完后必须把时钟remove
void clock_add(u32 type)
void clock_remove(u32 type)
void clock_set_cur(void)


把时钟设置加入到ext中，立刻设置时钟
需要立刻添加时钟
用完后必须把时钟remove
void clock_add_set(u32 type)
void clock_remove_set(u32 type)

*****/
////  如果clock_fix 为0 就按照配置设置时钟，如果有值就固定频率
#if (TCFG_MIC_EFFECT_ENABLE)
#define CLOCK_FIX   (192)
#else
#define CLOCK_FIX   0
#endif

#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_FRONT_LR_REAR_LR) && TCFG_EQ_DIVIDE_ENABLE
#define  EQ4_CLK  (24)
#else
#define  EQ4_CLK  (0)
#endif

#if TCFG_EQ_ONLINE_ENABLE
#define  EQ_ONLINE_CLK   (64)
#else
#define  EQ_ONLINE_CLK   (0)
#endif

#define  MIX_SRC_CLK (8)//src

const struct clock_type  clock_enum[] = {

    { LINEIN_IDLE_CLOCK 	, (96), "	LINEIN_IDLE_CLOCK  " },

#if TCFG_APP_PC_EN
    { PC_IDLE_CLOCK     	, (120), "	PC_IDLE_CLOCK      " },
#endif

#if (TCFG_EQ_ENABLE || TCFG_DRC_ENABLE)
    { EQ_CLK	, (24 + EQ_ONLINE_CLK + EQ4_CLK), " EQ_CLK	     "   },
    { EQ_DRC_CLK	, (60), " EQ_DRC_CLK	 "   },
#endif

#if TCFG_MIC_EFFECT_ENABLE
    { REVERB_CLK	, (64), " REVERB_CLK	 "   },
    { REVERB_HOWLING_CLK	, (32), " REVERB_HOWLING_CLK	 "   },
    { REVERB_PITCH_CLK	, (24), " REVERB_PITCH_CLK	 "   },
#endif

    { DEC_PCM_CLK	, (24), "DEC_PCM_CLK	 " },
    { DEC_MIX_CLK, (6 + MIX_SRC_CLK),	"DEC_MIX_CLK"   },

#if TCFG_UI_ENABLE
    { DEC_UI_CLK, (160),	"DEC_UI_CLK"   },
#endif

#if TCFG_APP_LIVE_IIS_EN
    { DEC_IIS_CLK, (64),	"DEC_IIS_CLK"   },
#endif

#if AUDIO_SURROUND_CONFIG
    { DEC_3D_CLK, (24),	"DEC_3D_CLK"   },
#endif

#if AUDIO_VBASS_CONFIG
    { DEC_VBASS_CLK, (8),	"DEC_VBASS_CLK"   },
#endif

#if AUDIO_EQUALLOUDNESS_CONFIG
    { DEC_LOUDNES_CLK, (108),	"DEC_LOUDNES_CLK"},
#endif

#if AUDIO_SPECTRUM_CONFIG
    { SPECTRUM_CLK, (5),	"SPECTRUM_CLK"   },
#endif

#if TCFG_BROADCAST_ENABLE
    {BROADCAST_CLK, (32),	"BROADCAST_CLK"   },
    {LIVE_STREAM_DEC_CLK, (32),	"LIVE_STREAM_DEC_CLK"   },
#endif

#if TCFG_CONNECTED_ENABLE
    {CONNECTED_CIG_CLK, (32),	"CONNECTED_CIG_CLK"   },
    {LIVE_STREAM_DEC_CLK, (32),	"LIVE_STREAM_DEC_CLK"   },
#endif

    /* { AEC8K_CLK	, (10), "AEC8K_CLK	     " }, */
    /* { AEC8K_ADV_CLK	, (50), "AEC8K_ADV_CLK	 " }, */
    /* { AEC16K_CLK	, (10), "AEC16K_CLK	 " }, */
    /* { AEC16K_ADV_CLK, (50), "AEC16K_ADV_CLK " }, */
    /* { AEC8K_SPX_CLK	, (10), "AEC8K_SPX_CLK	 " }, */
    /* { AEC16K_SPX_CLK, (10), "AEC16K_SPX_CLK " }, */
};



const u8 clock_tb[] = {
    24,
    32,
    48,
    64,
    96,
    128,
    160,
    192,
};

static u8 ext_clk_tb[10];
static u32 idle_type = 0;

static void clock_ext_dump()
{
    u8 i, j;
    for (i = 0; i < ARRAY_SIZE(clock_enum); i++) {
        if (idle_type ==  clock_enum[i].type) {
            y_printf("--- %d  %s \n", clock_enum[i].clock, clock_enum[i].name);
            break;
        }
    }

    for (i = 0; i < ARRAY_SIZE(ext_clk_tb); i++) {
        if (ext_clk_tb[i]) {
            for (j = 0; j < ARRAY_SIZE(clock_enum); j++) {
                if (ext_clk_tb[i] == clock_enum[j].type) {
                    y_printf("--- %d  %s \n", clock_enum[j].clock, clock_enum[j].name);
                    continue;
                }
            }
        }
    }
}


u8 clock_idle_selet(u32 type)
{
    u8 i;

#if (CLOCK_FIX )
    return CLOCK_FIX;
#endif

    for (i = 0; i < ARRAY_SIZE(clock_enum); i++) {
        if (type ==  clock_enum[i].type) {
            /* y_printf("--- %d  %s \n", clock_enum[i].clock, clock_enum[i].name); */
            return clock_enum[i].clock;
        }
    }

    return 24;
}

u8 clock_ext_push(u8 ext_type)
{
    u8 i;
    for (i = 0; i < ARRAY_SIZE(ext_clk_tb); i++) {
        if (ext_type == ext_clk_tb[i]) {
            return 0;
        }
    }

    for (i = 0; i < ARRAY_SIZE(ext_clk_tb); i++) {
        if (!ext_clk_tb[i]) {
            ext_clk_tb[i] = ext_type;
            return 1;
        }
    }

    y_printf("clock ext over!!! \n");
    return 0;
}

u8 clock_ext_pop(u8 ext_type)
{
    u8 i;
    for (i = 0; i < ARRAY_SIZE(ext_clk_tb); i++) {
        if (ext_type == ext_clk_tb[i]) {
            ext_clk_tb[i] = 0;
            return 1;
        }
    }
    return 0;
}

u16 clock_match(u16 clk)
{
    u8 i;
    for (i = 0; i < ARRAY_SIZE(clock_tb); i++) {
        if (clk <= clock_tb[i]) {
            return clock_tb[i];
        }
    }
    y_printf("clock overlimit!!! %d\n", clock_tb[ARRAY_SIZE(clock_tb) - 1]);
    return clock_tb[ARRAY_SIZE(clock_tb) - 1];
}


u16 clock_ext_cal()
{
    u32 ext_clk = 0 ;
    u8 i, j;

    for (i = 0; i < ARRAY_SIZE(ext_clk_tb); i++) {
        if (ext_clk_tb[i]) {
            for (j = 0; j < ARRAY_SIZE(clock_enum); j++) {
                if (ext_clk_tb[i] == clock_enum[j].type) {
                    /* y_printf("--- %d  %s \n", clock_enum[j].clock, clock_enum[j].name); */
                    ext_clk += clock_enum[j].clock;
                    continue;
                }
            }
        }
    }

    return ext_clk;
}

u32 clock_cur_cal()
{
    u32 idle_clk, cur_clk, ext_clk;

#if (CLOCK_FIX )
    return CLOCK_FIX;
#endif

    local_irq_disable();
    idle_clk = clock_idle_selet(idle_type);
    ext_clk = clock_ext_cal();
    cur_clk = idle_clk + ext_clk;
    cur_clk = clock_match(cur_clk);
    local_irq_enable();
    return cur_clk ;
}

void clock_pause_play(u8 mode)
{
    u32 rets = 0;
    __asm__ volatile("%0 = rets" : "=r"(rets));
    ASSERT(!(cpu_in_irq() || cpu_irq_disabled()), "clock set in irq or irq disabled,rets =%x\n", rets);


    u32 idle_clk, cur_clk ;
    if (mode) {
        idle_clk = clock_idle_selet(idle_type);
        clk_set("sys", idle_clk * 1000000L);
    } else {
        clock_ext_dump();
        cur_clk = clock_cur_cal();
        clk_set("sys", cur_clk * 1000000L);
    }
}

void clock_idle(u32 type)
{
    u32 rets = 0;
    __asm__ volatile("%0 = rets" : "=r"(rets));
    ASSERT(!(cpu_in_irq() || cpu_irq_disabled()), "clock set in irq or irq disabled,rets =%x\n", rets);


    u32 cur_clk;
    local_irq_disable();
    idle_type = type;
    cur_clk = clock_cur_cal();
    local_irq_enable();
    clock_ext_dump();
    clk_set("sys", cur_clk * 1000000L);
}

//////把时钟设置加入到ext中，但是不是立刻设置时钟
void  clock_add(u32 type)
{
    u32 cur_clk ;
    u8 resoult = clock_ext_push(type);
    if (!resoult) {
        return;
    }
}

void clock_remove(u32 type)
{
    u32 cur_clk ;

    u8 resoult = clock_ext_pop(type);
    if (!resoult) {
        return;
    }
}

void clock_set_cur(void)
{
    u32 rets = 0;
    __asm__ volatile("%0 = rets" : "=r"(rets));
    ASSERT(!(cpu_in_irq() || cpu_irq_disabled()), "clock set in irq or irq disabled,rets =%x\n", rets);


    u32 cur_clk ;
    clock_ext_dump();
    cur_clk = clock_cur_cal();
    clk_set("sys", cur_clk * 1000000L);
}

//////把时钟设置加入到ext中，立刻设置时钟
void clock_add_set(u32 type)
{
    u32 rets = 0;
    __asm__ volatile("%0 = rets" : "=r"(rets));
    ASSERT(!(cpu_in_irq() || cpu_irq_disabled()), "clock set in irq  %d or irq disabled %d,rets =%x\n", cpu_in_irq(), cpu_irq_disabled(), rets);


    u32 cur_clk ;
    u8 resoult = clock_ext_push(type);
    if (!resoult) {
        return;
    }
    clock_ext_dump();
    cur_clk = clock_cur_cal();
    clk_set("sys", cur_clk * 1000000L);
}

void clock_remove_set(u32 type)
{
    u32 rets = 0;
    __asm__ volatile("%0 = rets" : "=r"(rets));
    ASSERT(!(cpu_in_irq() || cpu_irq_disabled()), "clock set in irq or irq disabled,rets =%x\n", rets);

    u32 cur_clk ;

    u8 resoult = clock_ext_pop(type);
    if (!resoult) {
        return;
    }

    clock_ext_dump();
    cur_clk = clock_cur_cal();
    clk_set("sys", cur_clk * 1000000L);
}

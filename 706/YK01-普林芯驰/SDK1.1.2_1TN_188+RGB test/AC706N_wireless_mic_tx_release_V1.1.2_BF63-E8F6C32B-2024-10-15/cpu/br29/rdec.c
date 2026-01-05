#include "asm/includes.h"
#include "generic/gpio.h"
#include "asm/rdec.h"

#define QDEC_DEBUG_ENABLE
#ifdef QDEC_DEBUG_ENABLE
#define rdec_debug(fmt, ...) 	printf("[QDEC] "fmt, ##__VA_ARGS__)
#else
#define rdec_debug(...)
#endif

#define rdec_error(fmt, ...) 	printf("[QDEC] ERR: "fmt, ##__VA_ARGS__)


struct rdec {
    u8 init;
    const struct rdec_platform_data *user_data;
};

static struct rdec _rdec = {0};

#define __this 			(&_rdec)

typedef struct {
    u32 con;
    int dat;
    int smp;
    u32 dbe;
} QDEC_REG;


QDEC_REG *rdec_get_reg(u8 index)
{
    QDEC_REG *reg = NULL;

    switch (index) {
    case 0:
        reg = (QDEC_REG *)JL_QDEC0;
        break;
    default :
        ASSERT(0, "only one qdec module");
    }


    return reg;
}

static void __rdec_port_init(u8 port)
{
    gpio_set_pull_down(port, 0);
    gpio_set_pull_up(port, 1);
    gpio_set_die(port, 1);
    gpio_set_direction(port, 1);
}

static void rdec_port_init(const struct rdec_device *rdec)
{
    u32 index = rdec->index;

    __rdec_port_init(rdec->sin_port0);
    __rdec_port_init(rdec->sin_port1);

    if (rdec->sin_port0 == IO_PORTB_04 && rdec->sin_port1 == IO_PORTB_07) {
        JL_IOMC->IOMC0 &= ~BIT(25);
    } else if (rdec->sin_port0 == IO_PORTB_07 && rdec->sin_port1 == IO_PORTB_04) {
        JL_IOMC->IOMC0 &= ~BIT(25);
    } else {
        JL_IOMC->IOMC0 |=  BIT(25);
        gpio_ich_sel_iutput_signal(rdec->sin_port0, INPUT_CH_SIGNAL_QDEC_SIN0, INPUT_CH_TYPE_GP_ICH);
        gpio_ich_sel_iutput_signal(rdec->sin_port1, INPUT_CH_SIGNAL_QDEC_SIN0, INPUT_CH_TYPE_GP_ICH);
    }

    printf("rdec->sin_port0 = %x", rdec->sin_port0);
    printf("rdec->sin_port1 = %x", rdec->sin_port1);
    printf("ICH_CON0 = %x", JL_IOMC->ICH_CON0);
    printf("GP_ICH0 = %x", JL_IMAP->FI_GP_ICH0);
    printf("GP_ICH1 = %x", JL_IMAP->FI_GP_ICH1);

}

static void log_rdec_info(u32 index)
{
    QDEC_REG *reg = NULL;
    reg = rdec_get_reg(index);
    rdec_debug("QDEC CON = 0x%x", reg->con);

    printf("PB_DIR = 0x%x", JL_PORTB->DIR);
    printf("PB_OUT = 0x%x", JL_PORTB->OUT);
    printf("PB_PU = 0x%x",  JL_PORTB->PU0);
    printf("PB_PD = 0x%x",  JL_PORTB->PD0);
}
#define     PHASE_1_MODE   (0<<8)   // 全马
#define     PHASE_2_MODE   (1<<8)   // 半码, 鼠标
#define     PHASE_4_MODE   (2<<8)

int rdec_init(const struct rdec_platform_data *user_data)
{
    u8 i;
    QDEC_REG *reg = NULL;
    const struct rdec_device *rdec;
    rdec_debug("rdec init...");
    __this->init = 0;
    if (user_data == NULL) {
        return -1;
    }
    for (i = 0; i < user_data->num; i++) {
        //rdec = &(user_data->rdec[i]);
        u32 index = user_data->rdec[i].index;
        reg = rdec_get_reg(index);
        rdec_port_init(&(user_data->rdec[i]));
        //module init

        u32 qdec_con = 0;
        qdec_con |= (0xf << 2);
        qdec_con &= ~BIT(1); //pol = 0, io should pull up
        qdec_con |= BIT(1); //pol = 1, io should pull down
        qdec_con |= PHASE_1_MODE;

        reg->con = qdec_con;
        reg->con |= BIT(0); //QDECx EN
        log_rdec_info(index);
    }
    __this->init = 1;
    __this->user_data = user_data;
    return 0;
}

s8 get_rdec_rdat(i)
{
    QDEC_REG *reg = NULL;
    s8 _rdat = 0;
    u8 dbe = 0;
    reg = rdec_get_reg(i);
    if (reg->con & BIT(7)) {
        reg->con |= BIT(6);
        __asm__ volatile("csync");
        _rdat = reg->dat;
        dbe = reg->dbe;
    }
    if (_rdat != 0) {
        rdec_debug("QDEC: %d , dat: %d", i, _rdat);
        rdec_debug("QDEC: %d , dbe: %d", i, dbe);
    }
    return _rdat;
}

#define JL_RDEC_SEL 0
#define RDEC_TEST   0
#if RDEC_TEST
const struct rdec_device rdec_device_list[] = {
    {
        .index = JL_RDEC_SEL,
        .sin_port0 = IO_PORTB_05,
        .sin_port1 = IO_PORTB_06,
    },
};

// *INDENT-OFF*
RDEC_PLATFORM_DATA_BEGIN(rdec_data)
.enable = 1,
.num = ARRAY_SIZE(rdec_device_list),
.rdec = rdec_device_list,
RDEC_PLATFORM_DATA_END()
#endif


static void rdec_poll(void *p)
{
    get_rdec_rdat(0);
}

void rdec_test()
{
#if RDEC_TEST
    rdec_init(&rdec_data);
    sys_timer_add(NULL, rdec_poll, 100);
#endif
}

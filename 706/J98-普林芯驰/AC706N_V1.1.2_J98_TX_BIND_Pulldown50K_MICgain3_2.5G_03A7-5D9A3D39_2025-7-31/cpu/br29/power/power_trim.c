#include "asm/power_interface.h"
#include "syscfg_id.h"


static u8 wvdd_trim()
{
    return 0;
}

static u8 pvdd_trim()
{
    return 0;
}

typedef struct {
    u8 except_cnt;
} pmu_voltage_type;

void update_vdd_table(u8 val);

void volatage_trim_init()
{

    pmu_voltage_type pmu_voltage;
    memset((u8 *)&pmu_voltage, 0xff, sizeof(pmu_voltage));

    int vm_len = syscfg_read(VM_PMU_VOLTAGE, (u8 *)&pmu_voltage, sizeof(pmu_voltage_type));

    /*上电第一次trim*/
    if (vm_len != sizeof(pmu_voltage_type)) {

        pmu_voltage.except_cnt = 0;
    }

    if (is_reset_source(P33_WDT_RST) || is_reset_source(P11_WDT_RST) || is_reset_source(P33_EXCEPTION_SOFT_RST)) {
        if ((pmu_voltage.except_cnt + 1) < 0xff) {
            pmu_voltage.except_cnt++;
        }
        syscfg_write(VM_PMU_VOLTAGE, (u8 *)&pmu_voltage, sizeof(pmu_voltage_type));
    }

    update_vdd_table(((pmu_voltage.except_cnt <= 2) ? pmu_voltage.except_cnt : 2));
}

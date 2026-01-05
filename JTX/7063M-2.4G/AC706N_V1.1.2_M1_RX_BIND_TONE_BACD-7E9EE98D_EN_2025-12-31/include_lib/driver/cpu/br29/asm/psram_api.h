#ifndef __PSRAM_API_H__
#define __PSRAM_API_H__

enum PSRAM_PORT_SEL_TABLE {
    PSRAM_PORT_SEL_PORTA = 0,
    PSRAM_PORT_SEL_PORTB,
};

enum PSRAM_MODE_SEL_TABLE {
    PSRAM_MODE_1_WIRE = 0,
    PSRAM_MODE_4_WIRE_CMD1_ADR4_DAT4,
    PSRAM_MODE_4_WIRE_CMD4_ADR4_DAT4,
};


struct psram_platform_data {
    u8 power_port;
    enum PSRAM_PORT_SEL_TABLE port;
    enum PSRAM_MODE_SEL_TABLE mode;
};

#define PSRAM_PLATFORM_DATA_BEGIN(data) \
		static const struct psram_platform_data data = {

#define PSRAM_PLATFORM_DATA_END()  \
};


/* ---------------------------------------------------------------------------- */
/**
 * @brief psram硬件初始化函数
 *
 * @param config: psram配置参数
 */
/* ---------------------------------------------------------------------------- */
void psram_hw_init(const struct psram_platform_data *config);

#endif /* #ifndef __PSRAM_API_H__ */

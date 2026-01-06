
//===============================================================================//
//
//      output function define
//
//===============================================================================//
#define FO_GP_OCH0        ((0 << 2)|BIT(1))
#define FO_GP_OCH1        ((1 << 2)|BIT(1))
#define FO_GP_OCH2        ((2 << 2)|BIT(1))
#define FO_GP_OCH3        ((3 << 2)|BIT(1))
#define FO_GP_OCH4        ((4 << 2)|BIT(1))
#define FO_GP_OCH5        ((5 << 2)|BIT(1))
#define FO_SD0_CLK        ((6 << 2)|BIT(1)|BIT(0))
#define FO_SD0_CMD        ((7 << 2)|BIT(1)|BIT(0))
#define FO_SD0_DA0        ((8 << 2)|BIT(1)|BIT(0))
#define FO_UART1_TX       ((9 << 2)|BIT(1)|BIT(0))

//===============================================================================//
//
//      IO output select sfr
//
//===============================================================================//
typedef struct {
    __RW __u8 PA0_OUT;
    __RW __u8 PA1_OUT;
    __RW __u8 PA2_OUT;
    __RW __u8 PA3_OUT;
    __RW __u8 PA4_OUT;
    __RW __u8 PA5_OUT;
    __RW __u8 PA6_OUT;
    __RW __u8 PB0_OUT;
    __RW __u8 PB1_OUT;
    __RW __u8 PB2_OUT;
    __RW __u8 PB3_OUT;
    __RW __u8 PB4_OUT;
    __RW __u8 PB5_OUT;
    __RW __u8 PB6_OUT;
    __RW __u8 PB7_OUT;
    __RW __u8 PC0_OUT;
    __RW __u8 PC1_OUT;
    __RW __u8 PC2_OUT;
    __RW __u8 PC3_OUT;
    __RW __u8 PC4_OUT;
    __RW __u8 PC5_OUT;
    __RW __u8 USBDP_OUT;
    __RW __u8 USBDM_OUT;
    __RW __u8 PP0_OUT;
} JL_OMAP_TypeDef;

#define JL_OMAP_BASE      (ls_base + map_adr(0x36, 0x00))
#define JL_OMAP           ((JL_OMAP_TypeDef   *)JL_OMAP_BASE)



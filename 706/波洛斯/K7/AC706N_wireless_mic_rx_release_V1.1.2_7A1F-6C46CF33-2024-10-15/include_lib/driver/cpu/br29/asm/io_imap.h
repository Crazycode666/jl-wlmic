
//===============================================================================//
//
//      input IO define
//
//===============================================================================//
#define PA0_IN  1
#define PA1_IN  2
#define PA2_IN  3
#define PA3_IN  4
#define PA4_IN  5
#define PA5_IN  6
#define PA6_IN  7
#define PB0_IN  8
#define PB1_IN  9
#define PB2_IN  10
#define PB3_IN  11
#define PB4_IN  12
#define PB5_IN  13
#define PB6_IN  14
#define PB7_IN  15
#define PC0_IN  16
#define PC1_IN  17
#define PC2_IN  18
#define PC3_IN  19
#define PC4_IN  20
#define PC5_IN  21
#define USBDP_IN  22
#define USBDM_IN  23
#define PP0_IN  24

//===============================================================================//
//
//      function input select sfr
//
//===============================================================================//
typedef struct {
    __RW __u8 FI_GP_ICH0;
    __RW __u8 FI_GP_ICH1;
    __RW __u8 FI_GP_ICH2;
    __RW __u8 FI_GP_ICH3;
    __RW __u8 FI_GP_ICH4;
    __RW __u8 FI_GP_ICH5;
    __RW __u8 FI_SD0_CMD;
    __RW __u8 FI_SD0_DA0;
    __RW __u8 FI_UART1_RX;
    __RW __u8 FI_TOTAL;
} JL_IMAP_TypeDef;

#define JL_IMAP_BASE      (ls_base + map_adr(0x3a, 0x00))
#define JL_IMAP           ((JL_IMAP_TypeDef   *)JL_IMAP_BASE)



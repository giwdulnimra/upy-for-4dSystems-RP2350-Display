// Board and hardware specific configuration
#define MICROPY_HW_BOARD_NAME           "4d Systems RP2350 70CT-CLB"
// #define MICROPY_HW_MCU_NAME "rp2350b"
#define PICO_FLASH_SIZE_BYTES           (16 * 1024 * 1024)
#define MICROPY_HW_FLASH_STORAGE_BYTES  (PICO_FLASH_SIZE_BYTES - (2 * 1024 * 1024))
//#define MICROPY_HW_FLASH_STORAGE_BYTES  (PICO_FLASH_SIZE_BYTES - 1024 * 1024)

// TODO: Split PSRAM option off as a variant
#define MICROPY_HW_PSRAM_CS_PIN (0)
#define MICROPY_HW_ENABLE_PSRAM (0)

// Override machine_uart.c definitions.
// See weactstudio_rp2350b.h and note that the PICO_DEFAULT_UART configuration
// is not currently referenced in machine_uart.c.
#define MICROPY_HW_UART0_TX     (4)
#define MICROPY_HW_UART0_RX     (5)
#define MICROPY_HW_UART0_CTS    (-1)
#define MICROPY_HW_UART0_RTS    (-1)
#define MICROPY_HW_UART1_TX     (8)

#include "hardware/regs/sio.h"
#include "hardware/regs/io_banks0.h"

#ifndef _u
    #include "hardware/platform_defs.h"
#endif

// from hardware/regs/addressmap.h
#ifndef SIO_BASE
    #define SIO_BASE      _u(0xd0000000)
#endif
#ifndef IO_BANK0_BASE
    #define IO_BANK0_BASE _u(0x40028000)
#endif

// from hardware/regs/io_banks0.h
#ifndef IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SIOB_PROC_0
    #define IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SIOB_PROC_0 _u(0x05)
#endif
#define GPIO_FUNC_SIO   IO_BANK0_GPIO0_CTRL_FUNCSEL_VALUE_SIOB_PROC_0
// defines Offset of GPIO0, Offsets can be calculated due to difference of 8 bit between GPIO's
#ifndef IO_BANK0_GPIO0_CTRL_OFFSET
    #define IO_BANK0_GPIO0_CTRL_OFFSET _u(0x00000004)
#endif
#define CTRL_OFFSET     IO_BANK0_GPIO0_CTRL_OFFSET

// from hardware/regs/sio.h
#ifndef SIO_GPIO_OE_SET_OFFSET
    #define SIO_GPIO_OE_SET_OFFSET     _u(0x00000038)
#endif
#ifndef SIO_GPIO_OUT_CLR_OFFSET
    #define SIO_GPIO_OUT_CLR_OFFSET    _u(0x00000020)
#endif
#ifndef SIO_GPIO_OUT_SET_OFFSET
    #define SIO_GPIO_OUT_SET_OFFSET    _u(0x00000018)
#endif
#ifndef SIO_GPIO_IN_OFFSET
    #define SIO_GPIO_IN_OFFSET         _u(0x00000004)
#endif

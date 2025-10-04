#include "Graphics4D.h"
#include "pico_4d_modules.h"
// #include "psram_setup.h"

#include "pico/stdlib.h"
#include "hardware/regs/addressmap.h"
#include "hardware/structs/xip_ctrl.h"
#include <hardware/pwm.h>
#include "hardware/i2c.h"
#include "hardware/flash.h"
#include <cstdlib>
#include <cstdio>
#include <stdarg.h>
#include <cmath>

#ifdef GEN4_RP2350_RGB

#include "hardware/dma.h"
#include "hardware/claim.h"

#if defined(GEN4_RP2350_43) || defined(GEN4_RP2350_43T) || defined(GEN4_RP2350_43CT)
#include "rgb43.pio.h"
#elif defined(GEN4_RP2350_50) || defined(GEN4_RP2350_50T) || defined(GEN4_RP2350_50CT)
#include "rgb50.pio.h"
#elif defined(GEN4_RP2350_70) || defined(GEN4_RP2350_70T) || defined(GEN4_RP2350_70CT)
#include "rgb70.pio.h"
#elif defined(RP2350_90) || defined(RP2350_90T) || defined(RP2350_90CT)
#include "rgb90.pio.h"
#endif

#else
#include "bus_2040.pio.h"
#endif

#include "psram_tools/rp_pico_alloc.h"

// Max Settings
#define MAX_BUFFER_SIZE 8192
#define FB_SIZE LCD_WIDTH *LCD_HEIGHT * 2
#define MAX_FB_IN_SRAM 480 * 320 * 2
#define MAX_AUX_IN_SRAM 480 * 320
#define MAX_PXL_IN_AUX 480 * 320 / 2

#ifdef GEN4_RP2350_RGB
#define BOUNCE_BUFFER_LINES 80 // ****** seems to be fastest
#define BOUNCE_BUFFER_SIZE LCD_WIDTH *BOUNCE_BUFFER_LINES
#define BOUNCE_BUFFER_LINES_COUNT LCD_HEIGHT / BOUNCE_BUFFER_LINES
#endif

#define SWAP(x, y)          \
    do                      \
    {                       \
        typeof(x) SWAP = x; \
        x = y;              \
        y = SWAP;           \
    } while (0)

#define MAP(x, in_min, in_max, out_min, out_max) \
    (((x) - (in_min)) * ((out_max) - (out_min)) / ((in_max) - (in_min)) + (out_min))

#ifndef USE_SD_SPI

/* SDIO Interface */
static sd_sdio_if_t sdio_if = {
    /*
    Pins CLK_gpio, D1_gpio, D2_gpio, and D3_gpio are at offsets from pin D0_gpio.
    The offsets are determined by sd_driver\SDIO\rp2040_sdio.pio.
        CLK_gpio = (D0_gpio + SDIO_CLK_PIN_D0_OFFSET) % 32;
        As of this writing, SDIO_CLK_PIN_D0_OFFSET is 30,
            which is -2 in mod32 arithmetic, so:
        CLK_gpio = D0_gpio -2.
        D1_gpio = D0_gpio + 1;
        D2_gpio = D0_gpio + 2;
        D3_gpio = D0_gpio + 3;
    */
    .CMD_gpio = LCD_SD_CMD,
    .D0_gpio = LCD_SD_D0,
    .SDIO_PIO = pio0,
    .DMA_IRQ_num = DMA_IRQ_0,
    .use_exclusive_DMA_IRQ_handler = true,
    // .baud_rate = 10000000 // default
    .baud_rate = 25000000 // most stable
    // .baud_rate = 40000000 // target max
    // .baud_rate = 150000000 / 4 // max default
};

/* Hardware Configuration of the SD Card socket "object" */
static sd_card_t sd_card = {
    .type = SD_IF_SDIO,
    .sdio_if_p = &sdio_if
};

#else

/* Configuration of hardware SPI object */
static spi_t spi = {
    .hw_inst = spi1,  // SPI component
    .miso_gpio = LCD_SD_D0,
    .mosi_gpio = LCD_SD_CMD,
    .sck_gpio = LCD_SD_CLK,
    // .baud_rate = 125 * 1000 * 1000 / 4,  // 31250000 Hz
    .baud_rate = 125 * 1000 * 1000 / 2  // 62500000 Hz
};

/* SPI Interface */
static sd_spi_if_t spi_if = {
    .spi = &spi,  // Pointer to the SPI driving this card
    .ss_gpio = LCD_SD_D3  // The SPI slave select GPIO for this SD card  
};

/* Configuration of the SD Card socket object */
static sd_card_t sd_card = {
    .type = SD_IF_SPI,
    .spi_if_p = &spi_if  // Pointer to the SPI interface driving this card
};

#endif

/* Callbacks used by the library: */
size_t sd_get_num() { return 1; }

sd_card_t *sd_get_by_num(size_t num)
{
    if (0 == num)
        return &sd_card;
    else
        return NULL;
}

#if defined(GEN4_RP2350_RGB) && !defined(RGB_TEST)

bool initial = true;
int rgb_chan_0;
int rgb_chan_2;
int rgb_chan_1;
dma_channel_config c0;
uint16_t *rgb_data_array = NULL;
static uint16_t *__address_ptr = NULL;
static uint16_t *bounce_buff0 = NULL;
static uint16_t *bounce_buff1 = NULL;
static uint32_t dmaArrayPos;
static uint32_t bounce_buffer_size = BOUNCE_BUFFER_SIZE;
static int num_lines;
static uint16_t *__linebuffer;

void __no_inline_not_in_flash_func(dma_complete_handler(void))
{
    if(dma_channel_get_irq1_status(rgb_chan_0) || initial){
    //if (dma_hw->ints0 == 1 || initial)
    //{ // are we called for our DMA channel?
        initial = false;
        hw_set_bits(&dma_hw->ints1, 0);
        uint32_t arrayPosCount = dmaArrayPos * bounce_buffer_size;
        if ((dmaArrayPos & 0x1) == 1)
        {
            dma_channel_set_read_addr(rgb_chan_0, &bounce_buff1[0], true);
            if (dmaArrayPos == num_lines - 1)
            {
                bounce_buff0[bounce_buffer_size - 1] = rgb_data_array[0];
                // memcpy(bounce_buff0 + bounce_buffer_size - 1, rgb_data_array, 2);
                memcpy(bounce_buff0, rgb_data_array + arrayPosCount + 1, (bounce_buffer_size - 1) << 1);
            }
            else
            {
                memcpy(bounce_buff0, rgb_data_array + arrayPosCount + 1, bounce_buffer_size << 1);
            }
        }
        else
        {
            dma_channel_set_read_addr(rgb_chan_0, &bounce_buff0[0], true);
            memcpy(&bounce_buff1[0], rgb_data_array + arrayPosCount + 1, bounce_buffer_size << 1);
        }
        dmaArrayPos++;
        if (dmaArrayPos >= num_lines)
            dmaArrayPos = 0;
    }
}
#endif

TextArea::TextArea(int _x1, int _y1, int _x2, int _y2, uint16_t fg, uint16_t bg)
{
    x1 = _x1;
    y1 = _y1;
    x2 = _x2;
    y2 = _y2;

    fg_color = fg;
    bg_color = bg;

    // ensure x1 is less than x2
    if (x1 > x2)
        SWAP(x1, x2);
    // ensure y1 is less than y2
    if (y1 > y2)
        SWAP(y1, y2);
}

TextArea::~TextArea()
{
}

Graphics4D::Graphics4D()
{
    // display properties
    __display.orientation = LCD_ORIENTATION;
    __display.rotation = 0;
    __display.width = LCD_WIDTH;
    __display.height = LCD_HEIGHT;
    __display.pixel_count = LCD_WIDTH * LCD_HEIGHT;
    __display.clip.x2 = LCD_WIDTH - 1;
    __display.clip.y2 = LCD_HEIGHT - 1;
    __settings.buffer = (char *)malloc(__settings.buffer_size + 1);
    __textarea = new TextArea(0, 0, __display.width, __display.height, 0x07E0, 0x0000);
    SetFont(Font1);
}

Graphics4D::~Graphics4D()
{
    if (__framebuffer)
        free(__framebuffer);
}

void Graphics4D::DrawWidget(int num, int f, int x, int y, const uint8_t *gciArray)
{
    uint32_t dpos = (y * 800) + x;
    uint32_t fpos;

    int frame = 0;
    int x1, y1;
    fpos = (f * 28800) + 8;
    for (y1 = 0; y1 < 120; y1++)
    {
        for (x1 = 0; x1 < 120; x1++)
        {
            __framebuffer[dpos + x1] = (gciArray[fpos++] << 8) + gciArray[fpos++];
        }
        dpos += 800;
    }
}

void Graphics4D::SetFramebuffer(uint16_t *buffer)
{
    __framebuffer = buffer;
}

bool Graphics4D::Initialize()
{
    stdio_usb_init();
#ifdef GEN4_RP2350_RGB
    // Force clock to 248MHz in RGB mode
#if defined(GEN4_RP2350_70) || defined(GEN4_RP2350_70T) || defined(GEN4_RP2350_70CT) || defined(RP2350_90) || defined(RP2350_90T)  || defined(RP2350_90CT)
    set_sys_clock_khz(258000, true);
#else
    set_sys_clock_khz(258000, true);
#endif
#endif

#if defined(GEN4_RP2350_RGB) && !defined(RGB_TEST)
    bounce_buff0 = (uint16_t *)malloc(BOUNCE_BUFFER_SIZE << 1);
    bounce_buff1 = (uint16_t *)malloc(BOUNCE_BUFFER_SIZE << 1);
    rgb_data_array = (uint16_t *)rp_mem_malloc((LCD_WIDTH * LCD_HEIGHT + 1) * 2);
    __framebuffer = &rgb_data_array[0];
    num_lines = LCD_HEIGHT / BOUNCE_BUFFER_LINES;
    __linebuffer = (uint16_t *)malloc(LCD_WIDTH);
    memset(__framebuffer, 0, 2 * LCD_WIDTH * LCD_HEIGHT);
#else
#if (FB_SIZE <= MAX_FB_IN_SRAM) && !defined(FB_IN_PSRAM)
    __framebuffer = (uint16_t *)malloc(FB_SIZE);
#else
    __framebuffer = (uint16_t *)rp_mem_malloc(FB_SIZE);
#endif
#endif

    gpio_set_function(LCD_BACKLIGHT, GPIO_FUNC_PWM);
    uint sliceNum = pwm_gpio_to_slice_num(LCD_BACKLIGHT);

    float sys_clk = clock_get_hz(clk_sys);
    float divider = sys_clk / (25000.0 * 1024);

    // Configure PWM
    pwm_config pwmconfig = pwm_get_default_config();
    pwm_config_set_clkdiv(&pwmconfig, divider);
    pwm_config_set_wrap(&pwmconfig, 1024);
    pwm_init(sliceNum, &pwmconfig, true);

#ifndef GEN4_RP2350_RGB
    // this must be MCU mode

    pio_set_gpio_base(pio1, 16);

    if (!pio_can_add_program(pio1, &bus_write_program))
        return false;

    // Save some config
    __bus_pio = pio1;
    __bus_sm = 0;

    // claim the state machine
    pio_sm_claim(__bus_pio, __bus_sm);

    // load the PIO program
    __bus_prog_offset = pio_add_program(__bus_pio, &bus_write_program);
    __bus_jmp_read = pio_encode_jmp(__bus_prog_offset + bus_write_offset_read);

    // initialize pins for the PIO
    for (int i = 0; i < 16; i++)
    {
        pio_gpio_init(__bus_pio, LCD_DATA0_PIN + i); // D0 to D15 pins
    }

    pio_sm_set_consecutive_pindirs(__bus_pio, __bus_sm, LCD_DATA0_PIN, 16, true);

    // initialize control pins
#ifdef GEN4_PIO_TEST
    for (int i = 1; i < 3; i++) // don't include WR pin
#else
    for (int i = 0; i < 3; i++)
#endif
    {
        gpio_init(LCD_RS_PIN + i);
        gpio_set_dir(LCD_RS_PIN + i, GPIO_OUT);
        gpio_put(LCD_RS_PIN + i, 1);
    }

#ifdef GEN4_PIO_TEST
    pio_sm_set_consecutive_pindirs(__bus_pio, __bus_sm, LCD_WR_PIN, 1, true);
#endif

    // initialize reset pin
    gpio_init(LCD_RESET);
    gpio_set_dir(LCD_RESET, GPIO_OUT);
    gpio_put(LCD_RESET, 1);

    // configure the state machine.
    pio_sm_config sm_config = bus_write_program_get_default_config(__bus_prog_offset);

    // set the bus pins starting from data_pin
    sm_config_set_out_pins(&sm_config, LCD_DATA0_PIN, 16);

#ifdef GEN4_PIO_TEST
    // set the WR pin
    sm_config_set_set_pins(&sm_config, LCD_WR_PIN, 1);
#endif

    // set the clock divider
    sm_config_set_clkdiv_int_frac(&sm_config, 1, 0);

    // added since we don't need to receive when using framebuffers
    // sm_config_set_fifo_join(&sm_config, PIO_FIFO_JOIN_TX); // TODO: use this when not using SRAM/PSRAM framebuffer

    pio_sm_init(__bus_pio, __bus_sm, __bus_prog_offset + bus_write_offset_main, &sm_config);

    // Enable state machine
    pio_sm_set_enabled(__bus_pio, __bus_sm, true);

    // written based on https://github.com/zapta/pio_tft/blob/main/platformio/src/pio_tft.cpp

    delay(10);
    Reset();

    size_t len = sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]);
    for (size_t i = 0; i < len; i++)
    {
        const lcd_init_command *init = &lcd_init_cmds[i];
        __write_command(init->command);
        for (uint j = 0; j < init->len; j++)
        {
            __write_data(init->data[j]);
        }
        delay(init->delay_ms);
    }

#elif !defined(RGB_TEST) // RGB mode

    dmaArrayPos = 1;

    pio_set_gpio_base(pio1, 16);
    pio_set_gpio_base(pio2, 16);

    uint hsync_sm = 0;
    uint vsync_sm = 1;
    uint den_sm = 2;
    uint rgb_sm = 0; // pio_claim_unused_sm(pio2, true);

    #if defined(GEN4_RP2350_70) || defined(GEN4_RP2350_70T) || defined(GEN4_RP2350_70CT) || defined(RP2350_90) || defined(RP2350_90T)  || defined(RP2350_90CT)
    float pio_freq = sys_clk / ((float)/*LCD_CLK_FREQ*/ 33000000.0);
    #else
    float pio_freq = sys_clk / ((float)/*LCD_CLK_FREQ*/ 32000000.0);
    #endif

    uint vsync_offset = pio_add_program(pio1, &vsync_program);
    uint hsync_offset = pio_add_program(pio1, &hsync_program);
    uint den_offset = pio_add_program(pio1, &RGBframe_program);
    uint rgb_offset = pio_add_program(pio2, &rgb_program);

    // stdio_printf("Setting up RGB PIO with CLKDIV of %d\n", syn_div);

    // Initialize pio statemachines
    hsync_program_init(pio1, hsync_sm, hsync_offset, LCD_HSYNC_PIN, pio_freq);
    vsync_program_init(pio1, vsync_sm, vsync_offset, LCD_VSYNC_PIN, pio_freq);
    RGBframe_program_init(pio1, den_sm, den_offset, LCD_DE_PIN, pio_freq);
    #if defined(GEN4_RP2350_70) || defined(GEN4_RP2350_70T) || defined(GEN4_RP2350_70CT) || defined(RP2350_90) || defined(RP2350_90T)  || defined(RP2350_90CT)
    rgb_program_init(pio2, rgb_sm, rgb_offset, LCD_DATA0_PIN, (pio_freq / 7.0 * 2.7));
    #else
    rgb_program_init(pio2, rgb_sm, rgb_offset, LCD_DATA0_PIN, (pio_freq / 7.0 * 2.7));
    #endif

    // setup dma
    rgb_chan_0 = dma_claim_unused_channel(true);

    memcpy(bounce_buff0, rgb_data_array, BOUNCE_BUFFER_SIZE << 1);
    memcpy(bounce_buff1, rgb_data_array + (BOUNCE_BUFFER_SIZE << 1), BOUNCE_BUFFER_SIZE << 1);

    /*dma_channel_config*/ c0 = dma_channel_get_default_config(rgb_chan_0);
    channel_config_set_transfer_data_size(&c0, DMA_SIZE_16); // 16-bit transfers
    channel_config_set_read_increment(&c0, true);
    channel_config_set_write_increment(&c0, false);
    channel_config_set_dreq(&c0, DREQ_PIO2_TX0); // set for pio2 sm 0
    // channel_config_set_chain_to(&c0, rgb_chan_1);

    dma_channel_configure(
        rgb_chan_0,
        &c0,
        &pio2->txf[rgb_sm],           // RGB PIO2 TX FIFO
        &bounce_buff0,                // frame buffer
        BOUNCE_BUFFER_SIZE /* >> 1*/, // FB_SIZE >> 1,       // size of frame buffer
        false);

    // irq_add_shared_handler(DMA_IRQ_1, dma_complete_handler, PICO_SHARED_IRQ_HANDLER_HIGHEST_ORDER_PRIORITY); 
    irq_set_exclusive_handler(DMA_IRQ_1, dma_complete_handler); // after DMA all data, raise an interrupt
    dma_channel_set_irq1_enabled(rgb_chan_0, true);             // map DMA channel to interrupt
    irq_set_enabled(DMA_IRQ_1, true);                           // enable interrupt

    // Initialize counters in pio & pio state machines
    pio_sm_put_blocking(pio1, hsync_sm, LCD_WIDTH - 1);
    pio_sm_put_blocking(pio1, vsync_sm, LCD_HEIGHT - 1);
    pio_sm_put_blocking(pio1, den_sm, (LCD_WIDTH * 2) - 1);
    pio_sm_put_blocking(pio2, rgb_sm, LCD_WIDTH - 1);

    // start pio0 state machines in sync
    pio_sm_set_enabled(pio2, rgb_sm, true);
    pio_enable_sm_mask_in_sync(pio1, ((1u << hsync_sm) | (1u << vsync_sm) | (1u << den_sm)));
    dma_complete_handler();

#endif
    delay(200);
    Cls();
    Contrast(15);
    stdio_usb_init();

    return true;
}

#ifndef GEN4_RP2350_RGB
void Graphics4D::Reset()
{
    gpio_put(LCD_RESET, 0);
    delay(120);
    gpio_put(LCD_RESET, 1);
    delay(220);
}
#endif

uint16_t Graphics4D::BlendColor(uint16_t base_color, uint16_t new_color, uint8_t alpha)
{
    // Scale the alpha value to a range of 0-31
    uint32_t __alphatemp = alpha >> 3;

    // Prepare the colors by extending them to 32 bits and masking their components
    uint32_t fg = (new_color | (new_color << 16)) & 0x07e0f81f;
    uint32_t bg = (base_color | (base_color << 16)) & 0x07e0f81f;

    // Perform the blending operation
    bg += (fg - bg) * __alphatemp >> 5;
    bg &= 0x07e0f81f;

    // Combine the high and low 16-bit portions to form the final color
    uint16_t result_color = (uint16_t)(bg | (bg >> 16));

    return result_color;
}

uint Graphics4D::GetWidth()
{
    return __display.width;
}
uint Graphics4D::GetHeight()
{
    return __display.height;
}

void Graphics4D::SetBacklightLevel(uint16_t level)
{
    pwm_set_gpio_level(LCD_BACKLIGHT, level);
}

void Graphics4D::Contrast(uint8_t level)
{
    switch (level)
    {
    case 0:
        pwm_set_gpio_level(LCD_BACKLIGHT, 0);
        break;
    case 1:
        pwm_set_gpio_level(LCD_BACKLIGHT, 28);
        break;
    case 2:
        pwm_set_gpio_level(LCD_BACKLIGHT, 35);
        break;
    case 3:
        pwm_set_gpio_level(LCD_BACKLIGHT, 47);
        break;
    case 4:
        pwm_set_gpio_level(LCD_BACKLIGHT, 62);
        break;
    case 5:
        pwm_set_gpio_level(LCD_BACKLIGHT, 78);
        break;
    case 6:
        pwm_set_gpio_level(LCD_BACKLIGHT, 105);
        break;
    case 7:
        pwm_set_gpio_level(LCD_BACKLIGHT, 140);
        break;
    case 8:
        pwm_set_gpio_level(LCD_BACKLIGHT, 188);
        break;
    case 9:
        pwm_set_gpio_level(LCD_BACKLIGHT, 224);
        break;
    case 10:
        pwm_set_gpio_level(LCD_BACKLIGHT, 276);
        break;
    case 11:
        pwm_set_gpio_level(LCD_BACKLIGHT, 352);
        break;
    case 12:
        pwm_set_gpio_level(LCD_BACKLIGHT, 460);
        break;
    case 13:
        pwm_set_gpio_level(LCD_BACKLIGHT, 608);
        break;
    case 14:
        pwm_set_gpio_level(LCD_BACKLIGHT, 820);
        break;
    default:
        pwm_set_gpio_level(LCD_BACKLIGHT, 1023);
        break;
    }
}

uint8_t Graphics4D::ScreenMode(uint8_t orientation)
{
    if (__display.orientation == orientation)
        return __display.orientation;

    uint8_t last_orientation = __display.orientation;

    __display.orientation = orientation;
    __display.rotation = (4 + orientation - LCD_ORIENTATION) % 4;

#ifndef GEN4_RP2350_RGB
    __write_command(ST7789_MADCTL);
#endif
    switch (__display.rotation)
    {
    case 0:
#ifndef GEN4_RP2350_RGB
        __write_data(0x48);
#endif
        __display.width = LCD_WIDTH;
        __display.height = LCD_HEIGHT;
        break; // Portrait
    case 1:
#ifndef GEN4_RP2350_RGB
        __write_data(0x28);
#endif
        __display.width = LCD_HEIGHT;
        __display.height = LCD_WIDTH;
        break; // Landscape
    case 2:
#ifndef GEN4_RP2350_RGB
        __write_data(0x88);
#endif
        __display.width = LCD_WIDTH;
        __display.height = LCD_HEIGHT;
        break; // Portrait (flipped)
    case 3:
#ifndef GEN4_RP2350_RGB
        __write_data(0xE8);
#endif
        __display.width = LCD_HEIGHT;
        __display.height = LCD_WIDTH;
        break; // Landscape (flipped)
    }
    // reset clipping
    __display.clip.x1 = 0;
    __display.clip.y1 = 0;
    __display.clip.x2 = __display.width - 1;
    __display.clip.y2 = __display.height - 1;
    // reset text area position
    __textarea->cursor_x = 0;
    __textarea->cursor_y = 0;
    __textarea->x1 = 0;
    __textarea->y1 = 0;
    __textarea->x2 = __display.width - 1;
    __textarea->y2 = __display.height - 1;

    return last_orientation;
}

#ifndef GEN4_RP2350_RGB
void Graphics4D::SetAddressWindow(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end)
{
    __write_command(ST7789_CASET);
    __write_data(x_start >> 8);
    __write_data(x_start & 0xFF);
    __write_data(x_end >> 8);
    __write_data(x_end & 0xFF);

    __write_command(ST7789_RASET);
    __write_data(y_start >> 8);
    __write_data(y_start & 0xFF);
    __write_data(y_end >> 8);
    __write_data(y_end & 0xFF);

    __write_command(ST7789_RAMWR);
}

void Graphics4D::SendFrameBuffer(uint x1, uint y1, uint x2, uint y2)
{
    // x1 must be less than or equal to x2
    // y1 must be less than or equal to y2
    // must be within limits of the __framebuffer
    SetAddressWindow(x1, y1, x2, y2);

    uint w = x2 - x1 + 1;
    uint pos = (y1 * __display.width) + x1;
    for (uint y = y1; y <= y2; y++)
    {
        for (uint n = 0; n < w; n++)
        {
            __write_data(__framebuffer[pos + n]);
        }
        pos += __display.width;
    }
}
#endif

uint16_t *Graphics4D::GetFrameBuffer()
{
    return __framebuffer;
}

uint16_t Graphics4D::SetBackgroundColor(uint16_t color)
{
    uint16_t old_color = __display.bg_color;
    __display.bg_color = color;
    return old_color;
}

bool Graphics4D::ClipWindow(int x1, int y1, int x2, int y2)
{

    // ensure x1 is less than x2
    if (x1 > x2)
        SWAP(x1, x2);
    // ensure y1 is less than y2
    if (y1 > y2)
        SWAP(y1, y2);

    // if clip window is fully out of screen size, return immediately
    if (x1 >= __display.width || y1 >= __display.height || x2 < 0 || y2 < 0)
        return false;

    // ensure dimensions are within limits and store
    __display.clip.x1 = (x1 < 0) ? 0 : x1;
    __display.clip.y1 = (y1 < 0) ? 0 : y1;
    __display.clip.x2 = (x2 >= __display.width) ? __display.width - 1 : x2;
    __display.clip.y2 = (y2 >= __display.height) ? __display.height - 1 : y2;

    return true;
}

bool Graphics4D::MoveTo(int x, int y)
{
    if (x < 0 || y < 0 || x >= __display.width || y >= __display.height)
        return false;
    __textarea->cursor_x = x;
    __textarea->cursor_y = y;
    return true;
}

bool Graphics4D::MoveRel(int x_offset, int y_offset)
{
    int new_x = __textarea->cursor_x + x_offset;
    int new_y = __textarea->cursor_x + y_offset;
    return MoveTo(new_x, new_y);
}

void Graphics4D::Cls(bool draw_fb)
{

    // reset clip window
    __display.clip.x1 = 0;
    __display.clip.y1 = 0;
    __display.clip.x2 = __display.width - 1;
    __display.clip.y2 = __display.height - 1;

    // reset cursor position
    __textarea->cursor_x = 0;
    __textarea->cursor_y = 0;

#ifndef GEN4_RP2350_RGB
    if (draw_fb)
        SetAddressWindow(0, 0, __display.width - 1, __display.height - 1);
#endif
    for (uint i = 0; i <= __display.pixel_count; i++)
    {
        __framebuffer[i] = __display.bg_color;
#ifndef GEN4_RP2350_RGB
        if (draw_fb)
            __write_data(__display.bg_color);
#endif
    }
}

void Graphics4D::RectangleFilled(int x1, int y1, int x2, int y2, uint16_t color, bool draw_fb)
{

    // ensure x1 is less than x2
    if (x1 > x2)
        SWAP(x1, x2);
    // ensure y1 is less than y2
    if (y1 > y2)
        SWAP(y1, y2);

    // if rectangle is fully out of clip window, return immediately
    if (x1 > __display.clip.x2 || y1 > __display.clip.y2 || x2 < __display.clip.x1 || y2 < __display.clip.y1)
        return;

    // ensure dimensions are within limits
    if (x1 < __display.clip.x1)
        x1 = __display.clip.x1;
    if (x2 > __display.clip.x2)
        x2 = __display.clip.x2;
    if (y1 < __display.clip.y1)
        y1 = __display.clip.y1;
    if (y2 > __display.clip.y2)
        y2 = __display.clip.y2;

#ifndef GEN4_RP2350_RGB
    uint w = x2 - x1 + 1;
    uint pos = (y1 * __display.width) + x1;
    for (int y = y1; y <= y2; y++)
    {
        for (uint n = 0; n < w; n++)
        {
            __framebuffer[pos + n] = color;
        }
        pos += __display.width;
    }
#else
    // Translate coordinates based on orientation
    int x1_translated, y1_translated, x2_translated, y2_translated;

    switch (__display.rotation)
    {
    case 0:
        x1_translated = x1;
        y1_translated = y1;
        x2_translated = x2;
        y2_translated = y2;
        break;
    case 1:
        x1_translated = LCD_WIDTH - 1 - y2;
        y1_translated = x1;
        x2_translated = LCD_WIDTH - 1 - y1;
        y2_translated = x2;
        break;
    case 2:
        x1_translated = LCD_WIDTH - 1 - x2;
        y1_translated = LCD_HEIGHT - 1 - y2;
        x2_translated = LCD_WIDTH - 1 - x1;
        y2_translated = LCD_HEIGHT - 1 - y1;
        break;
    case 3:
        x1_translated = y1;
        y1_translated = LCD_HEIGHT - 1 - x2;
        x2_translated = y2;
        y2_translated = LCD_HEIGHT - 1 - x1;
        break;
    }

    uint w = x2_translated - x1_translated + 1;
    uint pos = (y1_translated * LCD_WIDTH) + x1_translated;

    for (int y = y1_translated; y <= y2_translated; y++)
    {
        for (uint n = 0; n < w; n++)
        {
            __framebuffer[pos + n] = color;
        }
        pos += LCD_WIDTH;
    }
#endif

#ifndef GEN4_RP2350_RGB
    if (draw_fb)
        SendFrameBuffer(x1, y1, x2, y2);
#endif
}

void Graphics4D::RectangleFilled(int x1, int y1, int x2, int y2, const uint16_t *buffer, bool draw_fb)
{

    // ensure x1 is less than x2
    if (x1 > x2)
        SWAP(x1, x2);
    // ensure y1 is less than y2
    if (y1 > y2)
        SWAP(y1, y2);

    // if rectangle is fully out of clip window, return immediately
    if (x1 > __display.clip.x2 || y1 > __display.clip.y2 || x2 < __display.clip.x1 || y2 < __display.clip.y1)
        return;

    // source width
    int original_width = x2 - x1 + 1;

    // clipping adjustments
    int sx1 = 0, sy1 = 0;
    if (x1 < __display.clip.x1)
    {
        sx1 += __display.clip.x1 - x1;
        x1 = __display.clip.x1;
    }
    if (x2 > __display.clip.x2)
    {
        x2 = __display.clip.x2;
    }
    if (y1 < __display.clip.y1)
    {
        sy1 += __display.clip.y1 - y1;
        y1 = __display.clip.y1;
    }
    if (y2 > __display.clip.y2)
    {
        y2 = __display.clip.y2;
    }

    // destination width and height after clipping
    uint w = x2 - x1 + 1;
    uint h = y2 - y1 + 1;
    uint sw = original_width;

    uint src = sy1 * sw + sx1;

#ifndef GEN4_RP2350_RGB
    // initial source and destination positions
    uint pos = (y1 * __display.width) + x1;

    for (uint y = 0; y < h; y++)
    {
        for (uint n = 0; n < w; n++)
        {
            uint src2 = src + n;
            __framebuffer[pos + n] = buffer[src2];
        }
        pos += __display.width;
        src += sw; // move to the next line in the source buffer
    }

    if (draw_fb)
        SendFrameBuffer(x1, y1, x2, y2);
#else

    if (__display.rotation == 0)
    {
        // no need to translate based on orientation
        uint pos = y1 * LCD_WIDTH + x1;
        size_t bytes_per_line = 2 * w;
        for (uint i = 0; i < h; i++)
        {
            memcpy(__framebuffer + pos, buffer + i * w, bytes_per_line);
            pos += LCD_WIDTH;
        }
        return;
    }

    for (uint i = 0; i < h; i++)
    {
        for (uint n = 0; n < w; n++)
        {
            uint src2 = src + n;
            int x_translated, y_translated;
            int x = n + x1;
            int y = i + y1;

            // Apply the translation based on the orientation
            switch (__display.rotation)
            {
            case 1:
                x_translated = LCD_WIDTH - 1 - y;
                y_translated = x;
                break;
            case 2:
                x_translated = LCD_WIDTH - 1 - x;
                y_translated = LCD_HEIGHT - 1 - y;
                break;
            case 3:
                x_translated = y;
                y_translated = LCD_HEIGHT - 1 - x;
                break;
            }

            uint pos = y_translated * LCD_WIDTH + x_translated;
            __framebuffer[pos] = buffer[src2];
        }
        src += sw; // move to the next line in the source buffer
    }

#endif
}

void Graphics4D::RectangleFilled(int x1, int y1, int x2, int y2, const uint8_t *buffer, bool draw_fb)
{

    // ensure x1 is less than x2
    if (x1 > x2)
        SWAP(x1, x2);
    // ensure y1 is less than y2
    if (y1 > y2)
        SWAP(y1, y2);

    // if rectangle is fully out of clip window, return immediately
    if (x1 > __display.clip.x2 || y1 > __display.clip.y2 || x2 < __display.clip.x1 || y2 < __display.clip.y1)
        return;

    // source width
    int original_width = x2 - x1 + 1;

    // clipping adjustments
    int sx1 = 0, sy1 = 0;
    if (x1 < __display.clip.x1)
    {
        sx1 = __display.clip.x1 - x1;
        x1 = __display.clip.x1;
    }
    if (x2 > __display.clip.x2)
    {
        x2 = __display.clip.x2;
    }
    if (y1 < __display.clip.y1)
    {
        sy1 = __display.clip.y1 - y1;
        y1 = __display.clip.y1;
    }
    if (y2 > __display.clip.y2)
    {
        y2 = __display.clip.y2;
    }

    // destination width and height after clipping
    uint w = x2 - x1 + 1;
    uint h = y2 - y1 + 1;
    uint sw = original_width;

    uint src = (sy1 * sw + sx1) << 1;

#ifndef GEN4_RP2350_RGB
    // initial source and destination positions
    uint pos = (y1 * __display.width) + x1;

    for (uint y = 0; y < h; y++)
    {
        for (uint n = 0; n < w; n++)
        {
            uint src2 = src + (n << 1);
            __framebuffer[pos + n] = (buffer[src2 + 1] << 8) | buffer[src2];
        }
        pos += __display.width;
        src += sw << 1; // move to the next line in the source buffer
    }

    if (draw_fb)
        SendFrameBuffer(x1, y1, x2, y2);
#else

    if (__display.rotation == 0)
    {
        // no need to translate based on orientation
        uint pos = y1 * LCD_WIDTH + x1;
        size_t bytes_per_line = 2 * w;
        for (uint i = 0; i < h; i++)
        {
            memcpy(__framebuffer + pos, buffer + (2 * i * w), bytes_per_line);
            pos += LCD_WIDTH;
        }
        return;
    }

    for (uint i = 0; i < h; i++)
    {
        for (uint n = 0; n < w; n++)
        {
            uint src2 = src + (n << 1);
            int x_translated, y_translated;
            int x = n + x1;
            int y = i + y1;

            // Apply the translation based on the orientation
            switch (__display.rotation)
            {
            case 1:
                x_translated = LCD_WIDTH - 1 - y;
                y_translated = x;
                break;
            case 2:
                x_translated = LCD_WIDTH - 1 - x;
                y_translated = LCD_HEIGHT - 1 - y;
                break;
            case 3:
                x_translated = y;
                y_translated = LCD_HEIGHT - 1 - x;
                break;
            }

            uint pos = y_translated * LCD_WIDTH + x_translated;
            __framebuffer[pos] = (buffer[src2 + 1] << 8) | buffer[src2];
        }
        src += sw << 1; // move to the next line in the source buffer
    }
#endif
}

void Graphics4D::RectangleFilledWithAlpha(int x1, int y1, int x2, int y2, const uint8_t *buffer, bool draw_fb)
{
    // ensure x1 is less than x2
    if (x1 > x2)
        SWAP(x1, x2);
    // ensure y1 is less than y2
    if (y1 > y2)
        SWAP(y1, y2);

    // if rectangle is fully out of clip window, return immediately
    if (x1 > __display.clip.x2 || y1 > __display.clip.y2 || x2 < __display.clip.x1 || y2 < __display.clip.y1)
        return;

    // source width and height
    int original_width = x2 - x1 + 1;
    int original_height = y2 - y1 + 1;

    uint pixel_count = original_width * original_height;

    // clipping adjustments
    if (x1 < __display.clip.x1)
        x1 = __display.clip.x1;
    if (x2 > __display.clip.x2)
        x2 = __display.clip.x2;
    if (y1 < __display.clip.y1)
        y1 = __display.clip.y1;
    if (y2 > __display.clip.y2)
        y2 = __display.clip.y2;

    // initial source and destination positions
    uint src = 0;
    int x = x1, y = y1;

    while (pixel_count > 0)
    {
        uint8_t alpha = buffer[src++];
        uint8_t len;
        switch (alpha)
        {
        case 0:
            len = buffer[src++];
            pixel_count -= len;
            while (len--)
            {
                x++;
                if (x > x2)
                {
                    y++;
                    if (y > y2)
                        break;
                    x = x1;
                }
            }
            break;
        case 255:
            len = buffer[src++];
            pixel_count -= len;
            while (len--)
            {
                if (x < __display.clip.x1 || x > __display.clip.x2 || y < __display.clip.y1 || y > __display.clip.y2)
                {
                    src += 2;
                }
                else
                {
                    uint16_t color = (buffer[src + 1] << 8) | buffer[src];
#ifndef GEN4_RP2350_RGB
                    uint pos = y * __display.width + x;
#else
                    int x_translated, y_translated;

                    // Apply the translation based on the orientation
                    switch (__display.rotation)
                    {
                    case 0:
                        x_translated = x;
                        y_translated = y;
                        break;
                    case 1:
                        x_translated = LCD_WIDTH - 1 - y;
                        y_translated = x;
                        break;
                    case 2:
                        x_translated = LCD_WIDTH - 1 - x;
                        y_translated = LCD_HEIGHT - 1 - y;
                        break;
                    case 3:
                        x_translated = y;
                        y_translated = LCD_HEIGHT - 1 - x;
                        break;
                    }
                    uint pos = y_translated * LCD_WIDTH + x_translated;
#endif
                    __framebuffer[pos] = color;
                    src += 2;
                }
                x++;
                if (x > x2)
                {
                    y++;
                    if (y > y2)
                        break;
                    x = x1;
                }
            }
            break;
        default:
            if (x < __display.clip.x1 || x > __display.clip.x2 || y < __display.clip.y1 || y > __display.clip.y2)
            {
                src += 2;
            }
            else
            {
                uint16_t color = (buffer[src + 1] << 8) | buffer[src];
#ifndef GEN4_RP2350_RGB
                uint pos = y * __display.width + x;
#else
                int x_translated, y_translated;

                // Translate coordinates based on orientation
                switch (__display.rotation)
                {
                case 0:
                    x_translated = x;
                    y_translated = y;
                    break;
                case 1:
                    x_translated = LCD_WIDTH - 1 - y;
                    y_translated = x;
                    break;
                case 2:
                    x_translated = LCD_WIDTH - 1 - x;
                    y_translated = LCD_HEIGHT - 1 - y;
                    break;
                case 3:
                    x_translated = y;
                    y_translated = LCD_HEIGHT - 1 - x;
                    break;
                }
                uint pos = y_translated * LCD_WIDTH + x_translated;
#endif
                __framebuffer[pos] = BlendColor(__framebuffer[pos], color, alpha);
                src += 2;
            }
            x++;
            if (x > x2)
            {
                y++;
                if (y > y2)
                    break;
                x = x1;
            }
            pixel_count--;
            break;
        }
    }

#ifndef GEN4_RP2350_RGB
    if (draw_fb)
        SendFrameBuffer(x1, y1, x2, y2);
#endif
}

void Graphics4D::Hline(int y, int x1, int x2, uint16_t color, bool draw_fb)
{
    // ensure x1 is less than x2
    if (x1 > x2)
        SWAP(x1, x2);

    // if line is fully out of clip window, return immediately
    if (x1 > __display.clip.x2 || y > __display.clip.y2 || x2 < __display.clip.x1 || y < __display.clip.y1)
        return;

    // ensure dimensions are within limits
    if (x1 < __display.clip.x1)
        x1 = __display.clip.x1;
    if (x2 > __display.clip.x2)
        x2 = __display.clip.x2;

#ifndef GEN4_RP2350_RGB
    uint pos = y * __display.width;
    for (int x = x1; x <= x2; x++)
    {
        __framebuffer[pos + x] = color;
    }

    if (draw_fb)
        SendFrameBuffer(x1, y, x2, y);
#else
    // Translate coordinates based on orientation
    int x1_translated, y1_translated, x2_translated, y2_translated;

    switch (__display.rotation)
    {
    case 0:
        x1_translated = x1;
        y1_translated = y;
        x2_translated = x2;
        y2_translated = y;
        break;
    case 1:
        x1_translated = LCD_WIDTH - 1 - y;
        y1_translated = x1;
        x2_translated = LCD_WIDTH - 1 - y;
        y2_translated = x2;
        break;
    case 2:
        x1_translated = LCD_WIDTH - 1 - x2;
        y1_translated = LCD_HEIGHT - 1 - y;
        x2_translated = LCD_WIDTH - 1 - x1;
        y2_translated = LCD_HEIGHT - 1 - y;
        break;
    case 3:
        x1_translated = y;
        y1_translated = LCD_HEIGHT - 1 - x2;
        x2_translated = y;
        y2_translated = LCD_HEIGHT - 1 - x1;
        break;
    }

    uint w = x2_translated - x1_translated + 1;
    uint pos = (y1_translated * LCD_WIDTH) + x1_translated;

    for (int y = y1_translated; y <= y2_translated; y++)
    {
        for (uint n = 0; n < w; n++)
        {
            __framebuffer[pos + n] = color;
        }
        pos += LCD_WIDTH;
    }
#endif
}

void Graphics4D::Vline(int x, int y1, int y2, uint16_t color, bool draw_fb)
{
    // ensure y1 is less than y2
    if (y1 > y2)
        SWAP(y1, y2);

    // if line is fully out of clip window, return immediately
    if (x > __display.clip.x2 || y1 > __display.clip.y2 || x < __display.clip.x1 || y2 < __display.clip.y1)
        return;

    // ensure dimensions are within limits
    if (y1 < __display.clip.y1)
        y1 = __display.clip.y1;
    if (y2 > __display.clip.y2)
        y2 = __display.clip.y2;

#ifndef GEN4_RP2350_RGB
    uint pos = (y1 * __display.width) + x;
    for (int y = y1; y <= y2; y++)
    {
        __framebuffer[pos] = color;
        pos += __display.width;
    }

    if (draw_fb)
        SendFrameBuffer(x, y1, x, y2);
#else
    // Translate coordinates based on orientation
    int x1_translated, y1_translated, x2_translated, y2_translated;

    switch (__display.rotation)
    {
    case 0:
        x1_translated = x;
        y1_translated = y1;
        x2_translated = x;
        y2_translated = y2;
        break;
    case 1:
        x1_translated = LCD_WIDTH - 1 - y2;
        y1_translated = x;
        x2_translated = LCD_WIDTH - 1 - y1;
        y2_translated = x;
        break;
    case 2:
        x1_translated = LCD_WIDTH - 1 - x;
        y1_translated = LCD_HEIGHT - 1 - y2;
        x2_translated = LCD_WIDTH - 1 - x;
        y2_translated = LCD_HEIGHT - 1 - y1;
        break;
    case 3:
        x1_translated = y1;
        y1_translated = LCD_HEIGHT - 1 - x;
        x2_translated = y2;
        y2_translated = LCD_HEIGHT - 1 - x;
        break;
    }

    uint w = x2_translated - x1_translated + 1;
    uint pos = (y1_translated * LCD_WIDTH) + x1_translated;

    for (int y = y1_translated; y <= y2_translated; y++)
    {
        for (uint n = 0; n < w; n++)
        {
            __framebuffer[pos + n] = color;
        }
        pos += LCD_WIDTH;
    }
#endif
}

void Graphics4D::Rectangle(int x1, int y1, int x2, int y2, uint16_t color, bool draw_fb)
{
    Hline(y1, x1, x2, color, false);
    Hline(y2, x1, x2, color, false);
    Vline(x1, y1 + 1, y2 - 1, color, false);
    Vline(x2, y1 + 1, y2 - 1, color, false);
#ifndef GEN4_RP2350_RGB
    if (draw_fb)
        SendFrameBuffer(x1, y1, x2, y1);
#endif
}

void Graphics4D::PutPixel(int x, int y, uint16_t color, bool draw_fb)
{
    // if pixel is fully out of screen area, return immediately
    if (x > __display.clip.x2 || y > __display.clip.y2 || x < __display.clip.x1 || y < __display.clip.y1)
        return;
#ifndef GEN4_RP2350_RGB
    __framebuffer[(y * __display.width) + x] = color;
    if (draw_fb)
        SendFrameBuffer(x, y, x, y);
#else
    // Translate coordinates based on orientation
    int32_t x_translated, y_translated;

    // Apply the translation based on the orientation
    switch (__display.rotation)
    {
    case 0:
        x_translated = x;
        y_translated = y;
        break;
    case 1:
        x_translated = LCD_WIDTH - 1 - y;
        y_translated = x;
        break;
    case 2:
        x_translated = LCD_WIDTH - 1 - x;
        y_translated = LCD_HEIGHT - 1 - y;
        break;
    case 3:
        x_translated = y;
        y_translated = LCD_HEIGHT - 1 - x;
        break;
    }

    // Write the pixel to the framebuffer
    __framebuffer[(y_translated * __display.width) + x_translated] = color;
    // stdio_printf("xy %u", x_translated); stdio_printf(" %u\n", y_translated);
#endif
}

void Graphics4D::PutPixel(int x, int y, uint16_t color, uint8_t alpha, bool draw_fb)
{
    if (alpha == 0) return;        // If alpha is 0, do not draw the pixel
    // if pixel is fully out of screen area, return immediately
    if (x > __display.clip.x2 || y > __display.clip.y2 || x < __display.clip.x1 || y < __display.clip.y1)
        return;
#ifndef GEN4_RP2350_RGB
    int pos = (y * __display.width) + x;
    __framebuffer[pos] = BlendColor(__framebuffer[pos], color, alpha);
    if (draw_fb)
        SendFrameBuffer(x, y, x, y);
#else
    // Translate coordinates based on orientation
    int32_t x_translated, y_translated;

    // Apply the translation based on the orientation
    switch (__display.rotation)
    {
    case 0:
        x_translated = x;
        y_translated = y;
        break;
    case 1:
        x_translated = LCD_WIDTH - 1 - y;
        y_translated = x;
        break;
    case 2:
        x_translated = LCD_WIDTH - 1 - x;
        y_translated = LCD_HEIGHT - 1 - y;
        break;
    case 3:
        x_translated = y;
        y_translated = LCD_HEIGHT - 1 - x;
        break;
    }

    // Write the pixel to the framebuffer
    int pos = (y_translated * __display.width) + x_translated;
    __framebuffer[pos] = BlendColor(__framebuffer[pos], color, alpha);
    // stdio_printf("xy %u", x_translated); stdio_printf(" %u\n", y_translated);
#endif
}


void Graphics4D::Line(int x1, int y1, int x2, int y2, uint16_t color, bool draw_fb)
{

    int angH = abs(y2 - y1) > abs(x2 - x1);
    if (angH)
    {
        SWAP(x1, y1);
        SWAP(x2, y2);
    }
    if (x1 > x2)
    {
        SWAP(x1, x2);
        SWAP(y1, y2);
    }
    int xx;
    int yy;
    xx = x2 - x1;
    yy = abs(y2 - y1);
    int edx = xx / 2;
    int incy;
    if (y1 < y2)
    {
        incy = 1;
    }
    else
    {
        incy = -1;
    }
    for (; x1 <= x2; x1++)
    {
        if (angH)
        {
            PutPixel(y1, x1, color, draw_fb);
        }
        else
        {
            PutPixel(x1, y1, color, draw_fb);
        }
        edx = edx - yy;
        if (edx < 0)
        {
            y1 = y1 + incy;
            edx = edx + xx;
        }
    }
}

void Graphics4D::Ellipse(int xe, int ye, uint radx, uint rady, uint16_t color, bool draw_fb)
{

    if (radx < 2)
        return;
    if (rady < 2)
        return;

    int x, y;
    // maybe change the variables below to long?
    int es;
    int radxx = radx * radx;
    int radyy = rady * rady;
    int xr = 4 * radxx;
    int yr = 4 * radyy;

    for (x = 0, y = rady, es = 2 * radyy + radxx * (1 - 2 * rady);
         radyy * x <= radxx * y; x++)
    {
        PutPixel(xe + x, ye + y, color, draw_fb);
        PutPixel(xe - x, ye + y, color, draw_fb);
        PutPixel(xe - x, ye - y, color, draw_fb);
        PutPixel(xe + x, ye - y, color, draw_fb);
        if (es >= 0)
        {
            es += xr * (1 - y);
            y--;
        }
        es += radyy * ((4 * x) + 6);
    }

    for (x = radx, y = 0, es = 2 * radxx + radyy * (1 - 2 * radx);
         radxx * y <= radyy * x; y++)
    {
        PutPixel(xe + x, ye + y, color, draw_fb);
        PutPixel(xe - x, ye + y, color, draw_fb);
        PutPixel(xe - x, ye - y, color, draw_fb);
        PutPixel(xe + x, ye - y, color, draw_fb);
        if (es >= 0)
        {
            es += yr * (1 - x);
            x--;
        }
        es += radxx * ((4 * y) + 6);
    }
}

void Graphics4D::EllipseFilled(int xe, int ye, uint radx, uint rady, uint16_t color, bool draw_fb)
{
    if (radx < 2)
        return;
    if (rady < 2)
        return;

    int x, y;
    // maybe change the variables below to long?
    int es;
    int radxx = radx * radx;
    int radyy = rady * rady;
    int xr = 4 * radxx;
    int yr = 4 * radyy;

    for (x = 0, y = rady, es = 2 * radyy + radxx * (1 - 2 * rady);
         radyy * x <= radxx * y; x++)
    {
        Hline(ye - y, xe - x, xe + x + 1, color, draw_fb);
        Hline(ye + y, xe - x, xe + x + 1, color, draw_fb);
        if (es >= 0)
        {
            es += xr * (1 - y);
            y--;
        }
        es += radyy * ((4 * x) + 6);
    }
    for (x = radx, y = 0, es = 2 * radxx + radyy * (1 - 2 * radx);
         radxx * y <= radyy * x; y++)
    {
        Hline(ye - y, xe - x, xe + x + 1, color, draw_fb);
        Hline(ye + y, xe - x, xe + x + 1, color, draw_fb);
        if (es >= 0)
        {
            es += yr * (1 - x);
            x--;
        }
        es += radxx * ((4 * y) + 6);
    }
}

void Graphics4D::Circle(int xc, int yc, uint radius, uint16_t color, bool draw_fb)
{
    int c = 1 - radius;
    int xx = 1;
    int yy = -2 * radius;
    int x = 0;
    int y = radius;
    PutPixel(xc, yc + radius, color, draw_fb);
    PutPixel(xc, yc - radius, color, draw_fb);
    PutPixel(xc + radius, yc, color, draw_fb);
    PutPixel(xc - radius, yc, color, draw_fb);
    while (x < y)
    {
        if (c >= 0)
        {
            y--;
            yy = yy + 2;
            c = c + yy;
        }
        x++;
        xx = xx + 2;
        c = c + xx;
        PutPixel(xc + x, yc + y, color, draw_fb);
        PutPixel(xc - x, yc + y, color, draw_fb);
        PutPixel(xc + x, yc - y, color, draw_fb);
        PutPixel(xc - x, yc - y, color, draw_fb);
        PutPixel(xc + y, yc + x, color, draw_fb);
        PutPixel(xc - y, yc + x, color, draw_fb);
        PutPixel(xc + y, yc - x, color, draw_fb);
        PutPixel(xc - y, yc - x, color, draw_fb);
    }
}

void Graphics4D::Arc(int xa, int ya, uint r, int sa, uint16_t color, bool draw_fb)
{
    int c = 1 - r;
    int xx = 1;
    int yy = -2 * r;
    int x = 0;
    int y = r;
    while (x < y)
    {
        if (c >= 0)
        {
            y--;
            yy = yy + 2;
            c = c + yy;
        }
        x++;
        xx = xx + 2;
        c = c + xx;
        if (sa & 0x4)
        {
            PutPixel(xa + x, ya + y, color, draw_fb);
            PutPixel(xa + y, ya + x, color, draw_fb);
        }
        if (sa & 0x2)
        {
            PutPixel(xa + x, ya - y, color, draw_fb);
            PutPixel(xa + y, ya - x, color, draw_fb);
        }
        if (sa & 0x8)
        {
            PutPixel(xa - y, ya + x, color, draw_fb);
            PutPixel(xa - x, ya + y, color, draw_fb);
        }
        if (sa & 0x1)
        {
            PutPixel(xa - y, ya - x, color, draw_fb);
            PutPixel(xa - x, ya - y, color, draw_fb);
        }
    }
}

void Graphics4D::ArcFilled(int xa, int ya, uint r, int sa, int ea, uint16_t color, bool draw_fb)
{
    int c = 1 - r;
    int x = 0;
    int y = r;
    int xx = 1;
    int yy = -2 * r;
    while (x < y)
    {
        if (c >= 0)
        {
            y--;
            yy = yy + 2;
            c = c + yy;
        }
        x++;
        xx = xx + 2;
        c = c + xx;
        if (sa & 0x1)
        {
            Vline(xa + x, ya - y, ya + y + 1 + ea, color, draw_fb /*false*/);
            Vline(xa + y, ya - x, ya + x + 1 + ea, color, draw_fb /*false*/);
        }
        if (sa & 0x2)
        {
            Vline(xa - x, ya - y, ya + y + 1 + ea, color, draw_fb /*false*/);
            Vline(xa - y, ya - x, ya + x + 1 + ea, color, draw_fb /*false*/);
        }
    }

    // TODO: maybe send the bounding rectangle at once
}

void Graphics4D::CircleFilled(int xc, int yc, uint radius, uint16_t color, bool draw_fb)
{
    Vline(xc, yc - radius, yc + radius + 1, color, draw_fb);
    ArcFilled(xc, yc, radius, 3, 0, color, draw_fb);
}

void Graphics4D::Triangle(int x1, int y1, int x2, int y2, int x3, int y3, uint16_t color, bool draw_fb)
{
    Line(x1, y1, x2, y2, color, draw_fb);
    Line(x1, y1, x3, y3, color, draw_fb);
    Line(x3, y3, x2, y2, color, draw_fb);
}

void Graphics4D::TriangleFilled(int x1, int y1, int x2, int y2, int x3, int y3, uint16_t color, bool draw_fb)
{
    int p0, p1, y, last;
    if (y1 > y2)
    {
        SWAP(y1, y2);
        SWAP(x1, x2);
    }
    if (y2 > y3)
    {
        SWAP(y3, y2);
        SWAP(x3, x2);
    }
    if (y1 > y2)
    {
        SWAP(y1, y2);
        SWAP(x1, x2);
    }
    if (y1 == y3)
    {
        p0 = p1 = x1;
        if (x2 < p0)
            p0 = x2;
        else if (x2 > p1)
            p1 = x2;
        if (x3 < p0)
            p0 = x3;
        else if (x3 > p1)
            p1 = x3;
        Hline(y1, p0, p1, color, draw_fb);
        return;
    }

    int xx01 = x2 - x1, yy01 = y2 - y1, xx02 = x3 - x1, yy02 = y3 - y1;
    int xx12 = x3 - x2, yy12 = y3 - y2;
    int z1 = 0, z2 = 0; // make this long?
    if (y2 == y3)
    {
        last = y2;
    }
    else
    {
        last = y2 - 1;
    }
    for (y = y1; y <= last; y++)
    {
        p0 = x1 + z1 / yy01;
        p1 = x1 + z2 / yy02;
        z1 += xx01;
        z2 += xx02;
        if (p0 > p1)
        {
            SWAP(p0, p1);
        }
        Hline(y, p0, p1, color, draw_fb);
    }
    z1 = xx12 * (y - y2);
    z2 = xx02 * (y - y1);
    for (; y <= y3; y++)
    {
        p0 = x2 + z1 / yy12;
        p1 = x1 + z2 / yy02;
        z1 += xx12;
        z2 += xx02;
        if (p0 > p1)
        {
            SWAP(p0, p1);
        }
        Hline(y, p0, p1, color, draw_fb);
    }
}

void Graphics4D::Polyline(uint len, const int *vx, const int *vy, uint16_t color, bool draw_fb)
{
    // Draw lines between consecutive vertices
    for (uint i = 0; i < len - 1; ++i)
    {
        Line(vx[i], vy[i], vx[i + 1], vy[i + 1], color, draw_fb);
    }
}

void Graphics4D::Polygon(uint len, const int *vx, const int *vy, uint16_t color, bool draw_fb)
{
    // Draw lines between consecutive vertices
    Polyline(len, vx, vy, color, draw_fb);

    // Draw line connecting the last vertex to the first one to close the polygon
    Line(vx[len - 1], vy[len - 1], vx[0], vy[0], color, draw_fb);
}

void Graphics4D::PolygonFilled(uint len, const int *x, const int *y, uint16_t color, bool draw_fb)
{
    // Find y_min and y_max
    int y_min = y[0];
    int y_max = y[0];
    for (uint i = 1; i < len; i++)
    {
        if (y[i] < y_min)
        {
            y_min = y[i];
        }
        if (y[i] > y_max)
        {
            y_max = y[i];
        }
    }

    // Loop through scanlines from y_min to y_max
    for (int y_scan = y_min; y_scan <= y_max; y_scan++)
    {
        // Initialize an array to store intersections
        int intersections[len];
        int num_intersections = 0;

        // Loop through edges of the polygon
        for (uint i = 0; i < len; i++)
        {
            int j = (i + 1) % len; // Index of the next vertex

            int y1 = y[i];
            int y2 = y[j];
            int x1 = x[i];
            int x2 = x[j];

            // Check if the edge crosses the scanline
            if ((y1 <= y_scan && y2 > y_scan) || (y2 <= y_scan && y1 > y_scan))
            {
                // Calculate the intersection point
                int x_intersect = (int)(((double)(y_scan - y1) / (double)(y2 - y1)) * (x2 - x1)) + x1;

                // Add the intersection to the list
                intersections[num_intersections++] = x_intersect;
            }
        }

        // Sort the intersection points
        for (int i = 0; i < num_intersections - 1; i++)
        {
            for (int j = 0; j < num_intersections - i - 1; j++)
            {
                if (intersections[j] > intersections[j + 1])
                {
                    int temp = intersections[j];
                    intersections[j] = intersections[j + 1];
                    intersections[j + 1] = temp;
                }
            }
        }

        // Draw horizontal lines between pairs of intersections
        for (int i = 0; i < num_intersections; i += 2)
        {
            Hline(y_scan, intersections[i], intersections[i + 1], color, draw_fb);
        }
    }
}

#ifndef GEN4_RP2350_RGB

void Graphics4D::__write_command(uint8_t c)
{
    gpio_put(LCD_RS_PIN, 0);
#ifndef GEN4_PIO_TEST
    gpio_put(LCD_WR_PIN, 0);
#endif
    pio_sm_put_blocking(__bus_pio, __bus_sm, c);
#ifndef GEN4_PIO_TEST
    gpio_put(LCD_WR_PIN, 1);
#else
    sleep_us(10);
#endif
    gpio_put(LCD_RS_PIN, 1);
}

void Graphics4D::__write_data(uint16_t c)
{
#ifndef GEN4_PIO_TEST
    gpio_put(LCD_WR_PIN, 0);
#endif
    pio_sm_put_blocking(__bus_pio, __bus_sm, c);
#ifndef GEN4_PIO_TEST
    gpio_put(LCD_WR_PIN, 1);
#else
    sleep_us(10);
#endif
}

uint16_t Graphics4D::__read_data()
{
#ifndef GEN4_PIO_TEST
    gpio_put(LCD_RD_PIN, 0);
#endif
    pio_sm_exec_wait_blocking(__bus_pio, __bus_sm, __bus_jmp_read);
    uint16_t result = pio_sm_get_blocking(__bus_pio, __bus_sm);
#ifndef GEN4_PIO_TEST
    gpio_put(LCD_RD_PIN, 1);
#else
    sleep_us(10);
#endif
    return result;
}

#endif
// #else // RGB

uint16_t *Graphics4D::__get_aux_buffer(uint width, uint height)
{
    return __get_aux_buffer(2 * width * height);
}

uint16_t *Graphics4D::__get_aux_buffer(size_t buffer_size)
{

    if (__aux_size >= buffer_size)
    {
        return __aux_buffer;
    }

    if (__aux_size == 0)
    {
        if (buffer_size > MAX_AUX_IN_SRAM) {
            __aux_buffer = (uint16_t *)rp_mem_malloc(buffer_size);
            if (__aux_buffer)
            {
                __aux_size = buffer_size;
                __aux_buffer_in_psram = true;
            }
        } else {
            __aux_buffer = (uint16_t *)malloc(buffer_size);
            if (__aux_buffer)
            {
                __aux_size = buffer_size;
                __aux_buffer_in_psram = false;
            }
        }
        return __aux_buffer;
    }

    uint16_t *tmp;

    if (buffer_size > MAX_AUX_IN_SRAM) {
        if (__aux_buffer_in_psram) {
            // last buffer was allocated in PSRAM
            tmp = (uint16_t *)rp_mem_realloc(__aux_buffer, buffer_size);
        } else {
            // if last buffer was allocated in SRAM
            free(__aux_buffer); // free it first
            // allocate new buffer in PSRAM
            tmp = (uint16_t *)rp_mem_malloc(buffer_size);
            __aux_buffer_in_psram = true;
        }
    } else {
        tmp = (uint16_t *)realloc(__aux_buffer, buffer_size);
    }
    if (tmp == NULL)
    {
        stdio_printf("Failed reallocating buffer: %d/%d\n", buffer_size, MAX_AUX_IN_SRAM);
        return NULL;
    }
    __aux_buffer = tmp;
    __aux_size = buffer_size;
    return __aux_buffer;
}

// #endif

/*
 * Text/String Related functions
 */

const uint8_t *Graphics4D::SetFont(const uint8_t *f)
{
    if (!f)
        return __font.ptr;

    const uint8_t *last_font = __font.ptr;
    __font.ptr = f;

    int i = 1;

    __font.char_count = f[i++];
    if (f[0] == 3 || f[0] == 4)
    {
        __font.char_count |= f[i++] << 8; // if font is 3 or 4, char_count is 2 bytes
    } 
    __font.first_char = f[i++];
    if (f[0] == 4)
    {
        __font.first_char |= f[i++] << 8; // if font is 4, first_char is 2 bytes
    }
    __font.max_width = f[i++];
    __font.height = f[i++];

    __font.last_char = __font.char_count + __font.first_char;

    __font.bytes_per_char = __font.height * ((__font.max_width + 7) >> 3);

    switch (f[0])
    {
    case 0:
        __font.data = &f[i];
        break;
    case 1:
        __font.data = &f[i];
        __font.bytes_per_char++;
        break;
    case 2:
        __font.widths = &f[i];
        __font.data = __font.widths + __font.char_count;
        __font.bytes_per_char++;
        break;
    case 3:
        __font.widths = &f[i];
        __font.data = __font.widths + __font.char_count;
        __font.bytes_per_char++;
        break;
    case 4: // alpha font (4-bit per pixel)
        __font.data = &f[i];
        __font.bytes_per_char = (__font.height * __font.max_width + 3) >> 1; // 4-bits per pixel
        // 4-bits per pixel means each byte contains 2 pixels
        // so we need to divide the height * width by 2
        // +1 ensures that if the pixel count is odd, we still have enough space for the last pixel
        // and we need to add 1 byte for alpha (+2 before shifting/dividing by 2)
        break;
    default:
        break;
    }

    return last_font;
}

uint16_t Graphics4D::SetFontForeground(TextArea4D area, uint16_t color)
{
    uint16_t old_color = area->fg_color;
    area->fg_color = color;
    return old_color;
}

uint16_t Graphics4D::SetFontBackground(TextArea4D area, uint16_t color, bool transparent)
{
    uint16_t old_color = area->bg_color;
    area->bg_color = color;
    area->bg_transparent = transparent;
    return old_color;
}

uint16_t Graphics4D::SetFontForeground(uint16_t color)
{
    uint16_t old_color = __textarea->fg_color;
    __textarea->fg_color = color;
    return old_color;
}

uint16_t Graphics4D::SetFontBackground(uint16_t color, bool transparent)
{
    uint16_t old_color = __textarea->bg_color;
    __textarea->bg_color = color;
    __textarea->bg_transparent = transparent;
    return old_color;
}


uint Graphics4D::GetStringWidth(const char * ts) {
    size_t len = strlen(ts);
    return len * __font.max_width;
}

uint Graphics4D::GetFontHeight() {
    return __font.height;
}


size_t Graphics4D::__putch(TextArea4D area, uint16_t c, bool draw_fb)
{

    if (c == '\r')
    {
        area->cursor_x = area->x1;
        return 0;
    }

    if (c == '\n')
    {
        area->cursor_y += __font.height;
        area->cursor_x = area->x1; // ensures that \n works the same as \r\n
        return 0;
    }

    // if character is not available
    if (c >= __font.last_char || c < __font.first_char)
    {
        // TODO: draw a placeholder, perhaps a rectangle to denote missing char

        return 0; // change to 1 when todo is done
    }

    uint16_t fontMode = __font.ptr[0];

    uint8_t width = __font.max_width;
    uint8_t bytes_per_row;
    switch (fontMode) {
        case 4:
            bytes_per_row = __font.max_width >> 1; // 4-bits per pixel (ignored)
            break;
        default:
            bytes_per_row = (__font.max_width + 7) >> 3;
            // +7 ensures the bits will be >= next bit count in multiple of 8
            // >> 3 divides it by 8
            break;
    }

    const uint8_t *data = __font.data;

    data += (c - __font.first_char) * __font.bytes_per_char;

    if (fontMode != 0)
    {
        // if not simple
        // width is different for each character
        width = data[0];
        data++;
    }

    if (area->wrap && area->cursor_x + width > __display.width)
    {
        // if next character overflows, move to next line
        area->cursor_y += __font.height;
        area->cursor_x = 0;
    }

    int _x, _y, i = 0;

    if (fontMode == 4) {
        int j = 0;
        // loop here
        for (uint8_t _y = 0; _y < __font.height; _y++)
        {
            for (uint8_t _x = 0; _x < __font.max_width; _x++)
            {
                if (_x >= width) {
                    // skip pixels that are not part of the character
                    if (j == 1) i++; // move to next byte
                    j++; j %= 2; // toggle between high and low nibbles
                    continue;
                }
                uint8_t alpha = (j == 0) ? (data[i] & 0xF0) : ((data[i] & 0x0F) << 4);
                if (area->bg_transparent)
                {   // merge with current pixel
                    if (alpha > 0) PutPixel(area->cursor_x + _x, area->cursor_y + _y, area->fg_color, alpha, draw_fb);
                } else {
                    // merge with background color
                    PutPixel(area->cursor_x + _x, area->cursor_y + _y, BlendColor(area->bg_color, area->fg_color, alpha), draw_fb);
                }
                if (j == 1) i++; // move to next byte
                j++; j %= 2; // toggle between high and low nibbles
            }
        }

    } else {
        // loop here
        for (int i = 0; i < __font.height; i++)
        {
            int byteIndex = 0;
            int bitIndex = 7;
            int numBits = width;

            while (numBits-- > 0)
            {
                _x = area->cursor_x + width - numBits - 1;
                _y = area->cursor_y + i;
                if ((data[byteIndex] >> bitIndex) & 0x01)
                {
                    PutPixel(_x, _y, area->fg_color, draw_fb);
                }
                else
                {
                    if (!area->bg_transparent)
                    {
                        PutPixel(_x, _y, area->bg_color, draw_fb);
                    }
                }
                if (--bitIndex < 0)
                {
                    byteIndex++;
                    bitIndex = 7;
                }
            }

            data += bytes_per_row;
        }
    }

    area->cursor_x += width;

    return 1;
}

size_t Graphics4D::print(const char *str, bool draw_fb)
{
    return print(__textarea, str, draw_fb);
}

size_t Graphics4D::printf(const char *format, ...)
{
    size_t len = __settings.buffer_size;
    va_list args;

    va_start(args, format);
    int result = vsnprintf(__settings.buffer, len, format, args);
    va_end(args);

    if (result < 0)
    {
        // Error in formatting
        return 0;
    }

    if ((size_t)result >= len)
    {
        // String was truncated, reallocate buffer
        len = (size_t)result + 1; // Add 1 for null terminator

        if (len > MAX_BUFFER_SIZE)
        {
            return 0;
        }

        __settings.buffer = (char *)realloc(__settings.buffer, len);

        if (__settings.buffer == NULL)
        {
            // Memory reallocation failed
            return 0;
        }

        va_start(args, format);
        result = vsnprintf(__settings.buffer, len, format, args);
        va_end(args);
    }

    return print(__textarea, __settings.buffer);
}

TextArea4D Graphics4D::CreateTextArea(int x1, int y1, int x2, int y2, uint16_t fg_color, uint16_t bg_color)
{
    return new TextArea(x1, y1, x2, y2, fg_color, bg_color);
}

size_t Graphics4D::print(TextArea4D area, const char *str, bool draw_fb)
{
    size_t len = 0;
    uint16_t u16chr = 0;
    uint8_t utf8expLen = 0;
    uint32_t utf8codepoint = 0;

    for (const char *p = str; *p != 0; p++)
    {

        const char c = *p;

        // First we build the Utf8 character
        if (utf8expLen)
        {
            // If we already started building the utf-8, we continue
            // we can check this by checking expected length
            if ((c & 0xC0) != 0x80)
            {
                // Invalid UTF-8 sequence, handle error or ignore
                utf8expLen = 0;
                utf8codepoint = 0;
                return 0; // Indicate failure
            }
            utf8codepoint = (utf8codepoint << 6) | (c & 0x3F);
            utf8expLen--;
            if (utf8expLen != 0)
                continue; // not yet complete
            u16chr = static_cast<uint16_t>(utf8codepoint);
        }
        else
        {
            // Otherwise, let's figure out how many bytes to expect
            if ((c & 0x80) == 0)
            {
                // If the character is ASCII, directly write its Unicode value
                u16chr = static_cast<uint16_t>(c);
            }
            else if ((c & 0xE0) == 0xC0)
            {
                utf8codepoint = c & 0x1F;
                utf8expLen = 1;
                continue;
            }
            else if ((c & 0xF0) == 0xE0)
            {
                utf8codepoint = c & 0x0F;
                utf8expLen = 2;
                continue;
            }
            else if ((c & 0xF8) == 0xF0)
            {
                utf8codepoint = c & 0x07;
                utf8expLen = 3;
                continue;
            }
            else
            {
                // Invalid UTF-8 sequence, handle error or ignore
                return 0; // Indicate failure
            }
        }

        len += __putch(area, u16chr, draw_fb);
    }
    return len;
}

size_t Graphics4D::printf(TextArea4D area, const char *format, ...)
{
    size_t len = __settings.buffer_size;
    va_list args;

    va_start(args, format);
    int result = vsnprintf(__settings.buffer, len, format, args);
    va_end(args);

    if (result < 0)
    {
        // Error in formatting
        return 0;
    }

    if ((size_t)result >= len)
    {
        // String was truncated, reallocate buffer
        len = (size_t)result + 1; // Add 1 for null terminator

        if (len > MAX_BUFFER_SIZE)
        {
            return 0;
        }

        __settings.buffer = (char *)realloc(__settings.buffer, len);

        if (__settings.buffer == NULL)
        {
            // Memory reallocation failed
            return 0;
        }

        va_start(args, format);
        result = vsnprintf(__settings.buffer, len, format, args);
        va_end(args);
    }

    return print(area, __settings.buffer);
}

/*
 * uSD Media (Image/Video) Related functions
 */

ImageControl::ImageControl(const uint8_t *ptr) : ptr(ptr)
{

    count = (ptr[2] << 8) | ptr[3];
    const uint8_t *data = ptr + 4;
    info = new MediaInfo[count];

    uint32_t nextOffset;
    uint8_t nextWidgetFormat;

    formCount = 0;

    MediaInfo4D currentForm;

    for (uint j = 0; j < count; j++)
    {
        uint i = j * 15;

        MediaInfo4D _info = &info[j];

        _info->offset = data[i++] << 24;
        _info->offset |= data[i++] << 16;
        _info->offset |= data[i++] << 8;
        _info->offset |= data[i++];

        uint8_t widgetFormat = data[i++];
        _info->type = (widgetFormat & 0xF8) >> 3;
        _info->option = (widgetFormat & 0x06) >> 1;
        _info->mode = (widgetFormat & 0x01) ? 24 : 16;

        if (_info->type == MediaType::FORM)
        {
            currentForm = _info;
            _info->form = NULL;
            formCount++;
        }
        else
        {
            _info->form = currentForm;
        }

        switch (_info->type)
        {
        case MediaType::BUTTON:
        case MediaType::SLIDER:
        case MediaType::KNOB:
            _info->touch_enabled = true;
            break;
        }

        _info->x = data[i++] << 8;
        _info->x |= data[i++];

        _info->y = data[i++] << 8;
        _info->y |= data[i++];

        _info->width = data[i++] << 8;
        _info->width |= data[i++];

        _info->height = data[i++] << 8;
        _info->height |= data[i++];

        _info->frames = data[i++] << 8;
        _info->frames |= data[i++];

        nextOffset = data[i++] << 24;
        nextOffset |= data[i++] << 16;
        nextOffset |= data[i++] << 8;
        nextOffset |= data[i++];

        nextWidgetFormat = data[i++];
        uint8_t nextWidgetType = (nextWidgetFormat & 0xF8) >> 3;
        uint8_t nextWidgetOption = (nextWidgetFormat & 0x06) >> 1;

        if (nextWidgetType == MediaType::FORM && nextWidgetOption == 0x00)
        {
            i += 10;
            nextOffset = data[i++] << 24;
            nextOffset |= data[i++] << 16;
            nextOffset |= data[i++] << 8;
            nextOffset |= data[i++];
        }

        _info->bytes_per_frame = (nextOffset - _info->offset) / _info->frames;
    }

    forms = new MediaInfo4D[formCount];

    nextForm = count;

    // Find the forms
    for (uint i = 0, j = 0; i < count; i++)
    {
        MediaInfo4D _info = &info[i];
        if (_info->type == MediaType::FORM)
        {
            forms[j] = _info;
            j++;
        }
    }
}

ImageControl::ImageControl(FIL *fil) : fil(fil)
{
    UINT bytes_read;
    uint8_t count_data[2];

    FRESULT fr = f_read(fil, count_data, 2, &bytes_read);

    count = (count_data[0] << 8) | count_data[1];
    uint dataSize = (count * 15) + 4;
    uint dataOffset = 4;
    uint8_t data[dataSize];

    fr = f_read(fil, data, dataSize, &bytes_read);

    info = new MediaInfo[count];

    uint32_t nextOffset;
    uint8_t nextWidgetFormat;

    formCount = 0;

    MediaInfo4D currentForm;

    for (uint j = 0; j < count; j++)
    {
        uint i = j * 15;

        MediaInfo4D _info = &info[j];

        _info->offset = data[i++] << 24;
        _info->offset |= data[i++] << 16;
        _info->offset |= data[i++] << 8;
        _info->offset |= data[i++];

        uint8_t widgetFormat = data[i++];
        _info->type = (widgetFormat & 0xF8) >> 3;
        _info->option = (widgetFormat & 0x06) >> 1;
        _info->mode = (widgetFormat & 0x01) ? 24 : 16;

        if (_info->type == MediaType::FORM)
        {
            currentForm = _info;
            _info->form = NULL;
            formCount++;
        }
        else
        {
            _info->form = currentForm;
        }

        switch (_info->type)
        {
        case MediaType::BUTTON:
        case MediaType::SLIDER:
        case MediaType::KNOB:
            _info->touch_enabled = true;
            break;
        }

        _info->x = data[i++] << 8;
        _info->x |= data[i++];

        _info->y = data[i++] << 8;
        _info->y |= data[i++];

        _info->width = data[i++] << 8;
        _info->width |= data[i++];

        _info->height = data[i++] << 8;
        _info->height |= data[i++];

        _info->frames = data[i++] << 8;
        _info->frames |= data[i++];

        nextOffset = data[i++] << 24;
        nextOffset |= data[i++] << 16;
        nextOffset |= data[i++] << 8;
        nextOffset |= data[i++];

        nextWidgetFormat = data[i++];
        uint8_t nextWidgetType = (nextWidgetFormat & 0xF8) >> 3;
        uint8_t nextWidgetOption = (nextWidgetFormat & 0x06) >> 1;

        if (nextWidgetType == MediaType::FORM && nextWidgetOption == 0x00)
        {
            i += 10;
            nextOffset = data[i++] << 24;
            nextOffset |= data[i++] << 16;
            nextOffset |= data[i++] << 8;
            nextOffset |= data[i++];
        }

        _info->bytes_per_frame = (nextOffset - _info->offset) / _info->frames;
    }

    forms = new MediaInfo4D[formCount];

    nextForm = count;

    // Find the forms and calculate data size
    size_t lastOffset = 0;
    for (uint i = 0, j = 0; i < count; i++)
    {
        MediaInfo4D _info = &info[i];
        if (_info->type == MediaType::FORM)
        {
            forms[j] = _info;

            if (_info->option == 0) {
                stdio_printf("Form is colored, skipping to the first form widget\n");
                _info = &info[i + 1];
            }

            if (j > 0) {
                // if it is not the first form
                forms[j - 1]->formDataSize = _info->offset - lastOffset;
                stdio_printf("Form %02d Offset: 0x%08X Size: %lu\n", j - 1, lastOffset, forms[j - 1]->formDataSize);
                if (maxFormDataSize < forms[j - 1]->formDataSize) maxFormDataSize = forms[j - 1]->formDataSize;
            }

            lastOffset = _info->offset;

            j++;
        }
    }

    // For the last form size, subtract file size with lastOffset
    forms[formCount - 1]->formDataSize = f_size(fil) - lastOffset;
    stdio_printf("Form %02d Offset: 0x%08X Size: %lu\n", formCount - 1, lastOffset, forms[formCount - 1]->formDataSize);
    if (maxFormDataSize < forms[formCount - 1]->formDataSize) maxFormDataSize = forms[formCount - 1]->formDataSize;
}

ImageControl::~ImageControl()
{
    free((void *)info);
}

GraphicsMedia4D::GraphicsMedia4D()
{
}

GraphicsMedia4D::~GraphicsMedia4D()
{
}

ImageControl4D GraphicsMedia4D::LoadImageControl(const uint8_t *gcx, uint formIndex)
{
    if (gcx[0] != 'M' || gcx[1] != 'E')
        return NULL;

    ImageControl4D hndl = new ImageControl(gcx);
    
    if (formIndex != -1) ShowForm(hndl, formIndex);

    return hndl;
}

ImageControl4D GraphicsMedia4D::LoadImageControl(const char *filename, uint formIndex)
{
    FRESULT fr;
    // Check if SD is already mounted
    if (fs.fs_type == 0)
    {
#ifndef USE_SD_SPI        
        sdio_if.baud_rate = clock_get_hz(clk_sys) / 4; // 4 is current CLKDIV in SDIO PIO
        if (sdio_if.baud_rate > 20000000)
        {
            sdio_if.baud_rate = 20000000;
        }
        stdio_printf("Mounting SD card @%lu baud\n", sdio_if.baud_rate);
#endif
        fr = f_mount(&fs, "", 1);
        if (fr != FR_OK) {
            stdio_printf("microSD card not mounted\n");
            return NULL;
        }

    }

    // Open file for reading
    static FIL fil;
    fr = f_open(&fil, filename, FA_READ);

    if (FR_OK != fr && FR_EXIST != fr)
    {
        stdio_printf("f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
        return NULL;
    }

    UINT bytes_read;
    uint8_t gcx[2];
    fr = f_read(&fil, gcx, 2, &bytes_read);

    if (gcx[0] != 'M' || gcx[1] != 'E') {
        stdio_printf("Invalid GCX file\n");
        return NULL;
    }

    stdio_printf("Creating file handle for %s...\n", filename);
    ImageControl4D hndl = new ImageControl(&fil);
    
    stdio_printf("__max_form_data < hndl->maxFormDataSize: %lu < %lu\n", __max_form_data, hndl->maxFormDataSize);
    if (__max_form_data < hndl->maxFormDataSize) {
        __max_form_data = hndl->maxFormDataSize;
        if (__form_data == NULL) {
            stdio_printf("Allocating %lu bytes for form data buffer\n", __max_form_data);
            __form_data = (uint8_t *) rp_mem_malloc(__max_form_data);
        } else {
            stdio_printf("Reallocating %lu bytes for form data buffer\n", __max_form_data);
            __form_data = (uint8_t *) rp_mem_realloc(__form_data, __max_form_data);
        }
    }

    if (formIndex != -1) ShowForm(hndl, formIndex);

    return hndl;
}

uint GraphicsMedia4D::GetCount(ImageControl4D hndl)
{
    return hndl->count;
}

FIL *GraphicsMedia4D::GetFile(ImageControl4D hndl)
{
    return hndl->fil;
}

MediaInfo4D GraphicsMedia4D::GetInfo(ImageControl4D hndl, uint index)
{
    return &hndl->info[index];
}

void GraphicsMedia4D::SetProperties(ImageControl4D hndl, uint index, const uint16_t *properties)
{
    hndl->info[index].properties = properties;
}

uint16_t GraphicsMedia4D::SetValue(ImageControl4D hndl, uint index, uint16_t value)
{
    uint16_t last_val = hndl->info[index].value;
    hndl->info[index].value = value;
    return last_val;
}

uint16_t GraphicsMedia4D::GetValue(ImageControl4D hndl, uint index)
{
    return hndl->info[index].value;
}

uint16_t GraphicsMedia4D::GetFrames(ImageControl4D hndl, uint index)
{
    return hndl->info[index].frames;
}

void GraphicsMedia4D::SetPosition(ImageControl4D hndl, uint index, int16_t x, int16_t y)
{
    hndl->info[index].x = x;
    hndl->info[index].y = y;
}

void GraphicsMedia4D::Clear(ImageControl4D hndl, uint index, uint16_t color, bool draw_fb)
{
    MediaInfo4D info = &hndl->info[index];
    int x1 = info->x;
    int y1 = info->y;
    int x2 = x1 + info->width - 1;
    int y2 = y1 + info->height - 1;
    gfx.RectangleFilled(x1, y1, x2, y2, color, draw_fb);
}

void GraphicsMedia4D::Show(ImageControl4D hndl, uint index, bool draw_fb)
{
    // only need to draw widgets from active form
    if (!redrawForm && (index < hndl->currentForm || index >= hndl->nextForm))
        return;

    MediaInfo4D info = &hndl->info[index];

    switch (info->type)
    {
    case MediaType::DIGITS:
        return __show_digits(hndl, info);
    case MediaType::DUAL_FRAME_GAUGE:
        return __show_2_frame_gauge(hndl, info);
    case MediaType::LINEAR_GAUGE:
    case MediaType::SLIDER:
        return __show_linear_gauge(hndl, info);
    case MediaType::KNOB:
        return __show_knob(hndl, info);
    case MediaType::FORM:
        if (info->option == 0)
        {
            gfx.RectangleFilled(0, 0, gfx.__display.width - 1, gfx.__display.height - 1, (uint16_t)info->offset, draw_fb);
            return;
        }
    }

    FRESULT fr;
    UINT read_count;

    uint16_t value = info->value;

    if (value >= info->frames)
        value = info->frames - 1;

    if (value < 0)
        value = 0;

    if (!redrawForm && info->last_value == value)
        return;

    info->last_value = value;

    int x1 = info->x;
    int y1 = info->y;
    int x2 = x1 + info->width - 1;
    int y2 = y1 + info->height - 1;

    if (info->mode == 24 && hndl->ptr == NULL)
    {
#ifdef GEN4_RP2350_RGB
        stdio_printf("Alpha support for GCX files not yet supported\n");
#else
        __redraw_form_region(hndl, info->form, x1, y1, x2, y2);
        uint8_t *buf = (uint8_t *)gfx.__get_aux_buffer(info->bytes_per_frame);
        if (buf == NULL)
        {
            stdio_printf("Unable to allocate buffer\n");
            return;
        }

        // read frame into framebuffer
        FSIZE_t offset = info->offset + (info->value * info->bytes_per_frame);
        fr = f_lseek(hndl->fil, offset);
        if (FR_OK != fr)
        {
            stdio_printf("f_lseek error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        fr = f_read(hndl->fil, buf, info->bytes_per_frame, &read_count);
        if (FR_OK != fr)
        {
            stdio_printf("f_read error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        gfx.RectangleFilledWithAlpha(x1, y1, x2, y2, buf);
#endif
        return;
    }

    if (hndl->ptr)
    {
        const uint8_t *ptr = hndl->ptr + info->offset + (value * info->bytes_per_frame);
        if (info->mode != 24)
        {
            gfx.RectangleFilled(x1, y1, x2, y2, ptr, draw_fb);
            return;
        }
        // with alpha

        size_t pixel_count = info->width * info->height;
        uint src = 0;

#ifndef GEN4_RP2350_RGB
        __redraw_form_region(hndl, info->form, x1, y1, x2, y2);
        gfx.RectangleFilledWithAlpha(x1, y1, x2, y2, ptr, draw_fb);
        return;
#else
        uint16_t *dest = __copy_form_to_aux(hndl, info->form, x1, y1, x2, y2);
        if (!dest)
        {
            // stdio_printf("Unable to allocate aux frame buffer\n");
            return; // TODO: handle properly when not able to get aux buffer
        }

        // initial source and destination positions
        int x = 0, y = 0;
        uint dest_width = info->width;

        while (pixel_count > 0)
        {
            uint8_t alpha = ptr[src++];
            uint8_t len;
            switch (alpha)
            {
            case 0:
                len = ptr[src++];
                pixel_count -= len;
                while (len--)
                {
                    x++;
                    if (x >= info->width)
                    {
                        y++;
                        if (y >= info->height)
                            break;
                        x = 0;
                    }
                }
                break;
            case 255:
                len = ptr[src++];
                pixel_count -= len;
                while (len--)
                {
                    uint16_t color = (ptr[src + 1] << 8) | ptr[src];
                    uint pos = y * dest_width + x;
                    dest[pos] = color;
                    src += 2;
                    x++;
                    if (x >= info->width)
                    {
                        y++;
                        if (y >= info->height)
                            break;
                        x = 0;
                    }
                }
                break;
            default:
                uint16_t color = (ptr[src + 1] << 8) | ptr[src];
                uint pos = y * dest_width + x;
                dest[pos] = gfx.BlendColor(dest[pos], color, alpha);
                src += 2;
                x++;
                if (x >= info->width)
                {
                    y++;
                    if (y >= info->height)
                        break;
                    x = 0;
                }
                pixel_count--;
                break;
            }
        }

        gfx.RectangleFilled(x1, y1, x2, y2, dest, draw_fb);
        return;
#endif
    }

    if (info->mode != 24 && x1 == 0 && y1 == 0 && info->width == gfx.__display.width && info->height == gfx.__display.height
#ifdef GEN4_RP2350_RGB
        && gfx.__display.rotation == 0
#endif
    )
    {
        FSIZE_t offset = info->offset + (value * info->bytes_per_frame);
        // We can draw directly to framebuffer if endianess aligns
        fr = f_lseek(hndl->fil, offset);
        if (FR_OK != fr)
        {
            stdio_printf("f_lseek error: %s (%d)\n", FRESULT_str(fr), fr);
        }

        fr = f_read(hndl->fil, gfx.__framebuffer, info->bytes_per_frame, &read_count);
        if (FR_OK != fr)
        {
            stdio_printf("f_read error: %s (%d)\n", FRESULT_str(fr), fr);
        }

#ifndef GEN4_RP2350_RGB
        gfx.SendFrameBuffer(0, 0, gfx.__display.width - 1, gfx.__display.height - 1);
#endif
        return;
    }

    if (info->mode != 24)
    {
        UINT bytes_per_line = 2 * info->width;
        FSIZE_t offset = info->offset + (value * info->bytes_per_frame);

        UINT bytes_per_read = info->bytes_per_frame;
        if (bytes_per_read > MAX_AUX_IN_SRAM)
            bytes_per_read = MAX_AUX_IN_SRAM;

        UINT lines_per_read = bytes_per_read / bytes_per_line;
        bytes_per_read = lines_per_read * bytes_per_line;

        uint16_t *ptr = gfx.__get_aux_buffer(bytes_per_read);

        UINT remaining_lines = info->height;

        fr = f_lseek(hndl->fil, offset);
        while (remaining_lines > 0)
        {
            if (lines_per_read > remaining_lines)
            {
                lines_per_read = remaining_lines;
            }

            UINT current_bytes_per_read = lines_per_read * bytes_per_line;

            // fr = f_lseek(hndl->fil, offset);
            fr = f_read(hndl->fil, ptr, current_bytes_per_read, &read_count);

            for (UINT lines_read = 0; lines_read < lines_per_read; lines_read++)
            {
                int y = y1 + (info->height - remaining_lines + lines_read);
                gfx.RectangleFilled(x1, y, x2, y, ptr + (lines_read * info->width));
                // TODO: Implement Hline
            }

            offset += current_bytes_per_read;
            remaining_lines -= lines_per_read;
        }

        return;
    }

    // gfx.RectangleFilledWithAlpha(x1, y1, x2, y2, ptr, draw_fb);
    //   TODO: need to add HlineWithAlpha
    stdio_printf("Not yet supported!\n");
    // }
}

void GraphicsMedia4D::ShowForm(ImageControl4D hndl, uint index)
{
    if (hndl->currentForm == index)
    {
        return;
    }
    stdio_printf("Changing form from %d to %d\n", hndl->currentForm, index);

    MediaInfo4D info = &hndl->info[index];

    if (info->type != MediaType::FORM)
    {
        stdio_printf("Form type %d not valid, expecting %d\n", info->type, MediaType::FORM);
        return; // Not a valid form
    }

    if (!hndl->ptr)
    {
        FRESULT fr;
        UINT read_count;    

        // Load Form to PSRAM
        stdio_printf("Loading form data to PSRAM\n");

        MediaInfo4D _info = (info->option == 0) ? &hndl->info[index + 1] : info;

        fr = f_lseek(hndl->fil, _info->offset);
        if (FR_OK != fr)
        {
            stdio_printf("f_lseek error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        stdio_printf("Reading from 0x%08X: %lu bytes\n", _info->offset, info->formDataSize);
        fr = f_read(hndl->fil, __form_data, info->formDataSize, &read_count);
        if (FR_OK != fr)
        {
            stdio_printf("f_read error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        if (info->formDataSize != read_count) {
            stdio_printf("Failed loading form data to PSRAM\n");
        } else {
            stdio_printf("Successfully loaded form data to PSRAM\n");
        }
    }

    redrawForm = true;

    Show(hndl, index, false);

    uint i = index + 1;
    for (; i < hndl->count; i++)
    {
        info = &hndl->info[i];

        if (info->type == MediaType::FORM)
            break;
        Show(hndl, i, false);

        switch (info->type)
        {
        case MediaType::KNOB:
        case MediaType::DIGITS:
            i += 1;
            break;
        case MediaType::LINEAR_GAUGE:
        case MediaType::SLIDER:
            i += 2;
            break;
        }
    }

#ifndef GEN4_RP2350_RGB
    gfx.SendFrameBuffer(0, 0, gfx.__display.width - 1, gfx.__display.height - 1);
#endif

    hndl->currentForm = index;
    hndl->nextForm = i;

    redrawForm = false;

    stdio_printf("Changed form %d\n", index);
}

#ifndef LCD_TOUCH_NONE
uint GraphicsMedia4D::Touched(ImageControl4D hndl, uint index)
{
    // only need to check active form
    if (index != -1 && (index <= hndl->currentForm || index >= hndl->nextForm))
        return -1;

    // get touch points here
    int16_t touchX = touch.GetX();
    int16_t touchY = touch.GetY();

    if (index != -1)
    {
        // Check only this specific index
        MediaInfo4D info = &hndl->info[index];

        if (!info->touch_enabled)
            return -1;

        int16_t x1 = info->x;
        int16_t y1 = info->y;
        int16_t x2 = x1 + info->width - 1;
        int16_t y2 = y1 + info->height - 1;

        if (!(touchX >= x1 && touchX <= x2 && touchY >= y1 && touchY <= y2))
        {
            return -1;
        }

        int currentValue;

        switch (info->type)
        {
        case MediaType::BUTTON:
            return index;
        case MediaType::SLIDER:
        {
            uint16_t range = info->properties ? info->properties[0] : 100;
            MediaInfo4D thumb = info + 2;
            int16_t offset, startPos, endPos, currentPos;
            if (info->width > info->height)
            {
                // horizontal
                offset = thumb->width / 2;
                startPos = x1 + offset;
                endPos = x2 - offset;
                currentPos = touchX;
            }
            else
            {
                // vertical
                offset = thumb->height / 2;
                endPos = y1 + offset;
                startPos = y2 - offset;
                currentPos = touchY;
            }

            // Map currentPos to the value in the range
            currentValue = range * (currentPos - startPos) / (endPos - startPos);
            if (currentValue < 0)
                currentValue = 0;
            if (currentValue > range)
                currentValue = range;
            break;
        }
        case MediaType::KNOB:
        {
            MediaInfo4D thumb = info + 1;
            uint16_t range = thumb->frames - 1;
            int16_t centerX = info->x + info->properties[0];
            int16_t centerY = info->y + info->properties[1];
            int16_t startPos = info->properties[2];
            int16_t endPos = info->properties[3];

            // Calculate the differences between the touch point and the center
            double deltaX = centerX - touchX;
            double deltaY = touchY - centerY; // Y increases downwards on the LCD screen

            // Calculate the angle in radians from the X-axis
            double angleRadians = atan2(deltaX, deltaY);

            // Convert the angle to degrees
            double angleDegrees = angleRadians * (180.0 / 3.14159265);

            // Ensure the angle is in the range [0, 360)
            if (angleDegrees < 0)
            {
                angleDegrees += 360.0;
            }

            currentValue = range * (angleDegrees - startPos) / (endPos - startPos);
            if (currentValue < 0)
                currentValue = 0;
            if (currentValue > range)
                currentValue = range;
            break;
        }
        // TODO: add keyboard
        default:
            break;
        }
        return index;
    }

    for (uint i = hndl->currentForm + 1; i < hndl->nextForm; i++)
    {
        MediaInfo4D info = &hndl->info[i];

        if (!info->touch_enabled)
            continue;

        int16_t x1 = info->x;
        int16_t y1 = info->y;
        int16_t x2 = x1 + info->width - 1;
        int16_t y2 = y1 + info->height - 1;

        if (touchX >= x1 && touchX <= x2 && touchY >= y1 && touchY <= y2)
        {
            int currentValue;
            switch (info->type)
            {
            case MediaType::BUTTON:
                return i;
            case MediaType::SLIDER:
            {
                uint16_t range = info->properties ? info->properties[0] : 100;
                MediaInfo4D thumb = info + 2;
                int16_t offset, startPos, endPos, currentPos;
                if (info->width > info->height)
                {
                    // horizontal
                    offset = thumb->width / 2;
                    startPos = x1 + offset;
                    endPos = x2 - offset;
                    currentPos = touchX;
                }
                else
                {
                    // vertical
                    offset = thumb->height / 2;
                    endPos = y1 + offset;
                    startPos = y2 - offset;
                    currentPos = touchY;
                }

                // Map currentPos to the value in the range
                currentValue = range * (currentPos - startPos) / (endPos - startPos);
                if (currentValue < 0)
                    currentValue = 0;
                if (currentValue > range)
                    currentValue = range;
                break;
            }
            case MediaType::KNOB:
            {
                MediaInfo4D thumb = info + 1;
                uint16_t range = thumb->frames - 1;
                int16_t centerX = info->x + info->properties[0];
                int16_t centerY = info->y + info->properties[1];
                int16_t startPos = info->properties[2];
                int16_t endPos = info->properties[3];

                // Calculate the differences between the touch point and the center
                double deltaX = centerX - touchX;
                double deltaY = touchY - centerY; // Y increases downwards on the LCD screen

                // Calculate the angle in radians from the X-axis
                double angleRadians = atan2(deltaX, deltaY);

                // Convert the angle to degrees
                double angleDegrees = angleRadians * (180.0 / 3.14159265);

                // Ensure the angle is in the range [0, 360)
                if (angleDegrees < 0)
                {
                    angleDegrees += 360.0;
                }

                currentValue = range * (angleDegrees - startPos) / (endPos - startPos);
                if (currentValue < 0)
                    currentValue = 0;
                if (currentValue > range)
                    currentValue = range;
                break;
            }
            // TODO: add keyboard
            default:
                return i;
            }
            SetValue(hndl, i, currentValue);
            Show(hndl, i);
            return i;
        }
    }

    return -1;
}
#endif

void GraphicsMedia4D::__draw_to_buffer(MediaInfo4D info, const uint8_t *ptr, uint16_t *dest, int bufferWidth, int bufferHeight, int x1, int y1, int value, BufferRegion *drawArea)
{

#ifdef GEN4_RP2350_RGB
    // if (!hndl->ptr) {
    //     stdio_printf("__draw_to_buffer not yet supported in RGB displays in uSD Mode!\n");
    //     return; // only use this when using flash GCX in RGB displays
    //     // to use uSD in RGB mode, need to use more RAM, ideally the PSRAM
    // }
#endif

    // initial source and destination positions
    uint src = info->bytes_per_frame * value;

    if (info->mode != 24)
    {
        int x = x1, y = y1;
        uint bytes_per_line = 2 * info->width;
        uint pos = (y1 * bufferWidth) + x1;
        for (uint i = 0; i < info->height; i++)
        {
            memcpy(&dest[pos], &ptr[src], bytes_per_line);
            pos += bufferWidth;
            src += bytes_per_line;
        }
        return;
    }

    int x = 0, y = 0;
    size_t pixel_count = info->width * info->height;

    while (pixel_count > 0)
    {
        uint8_t alpha = ptr[src++];
        uint8_t len;
        switch (alpha)
        {
        case 0:
            len = ptr[src++];
            pixel_count -= len;
            while (len--)
            {
                x++;
                if (x >= info->width)
                {
                    y++;
                    if (y >= info->height)
                        break;
                    x = 0;
                }
            }
            break;
        case 255:
            len = ptr[src++];
            pixel_count -= len;
            while (len--)
            {
                if (!drawArea || (x >= drawArea->x1 && x <= drawArea->x2 && y >= drawArea->y1 && y <= drawArea->y2))
                {
                    uint16_t color = (ptr[src + 1] << 8) | ptr[src];
                    uint pos = (y1 + y) * bufferWidth + x1 + x;
                    dest[pos] = color;
                }
                src += 2;
                x++;
                if (x >= info->width)
                {
                    y++;
                    if (y >= info->height)
                        break;
                    x = 0;
                }
            }
            break;
        default:
            if (!drawArea || (x >= drawArea->x1 && x <= drawArea->x2 && y >= drawArea->y1 && y <= drawArea->y2))
            {
                uint16_t color = (ptr[src + 1] << 8) | ptr[src];
                uint pos = (y1 + y) * bufferWidth + x1 + x;
                dest[pos] = gfx.BlendColor(dest[pos], color, alpha);
            }
            src += 2;
            x++;
            if (x >= info->width)
            {
                y++;
                if (y >= info->height)
                    break;
                x = 0;
            }
            pixel_count--;
            break;
        }
    }
}

void GraphicsMedia4D::__draw_to_buffer(ImageControl4D hndl, MediaInfo4D info, uint16_t *dest, int width, int height, int x1, int y1, int value, BufferRegion *drawArea)
{

#ifdef GEN4_RP2350_RGB
    if (!hndl->ptr)
    {
        stdio_printf("__draw_to_buffer not yet supported in RGB displays in uSD Mode!\n");
        return; // only use this when using flash GCX in RGB displays
        // to use uSD in RGB mode, need to use more RAM, ideally the PSRAM
    }
#endif

    // initial source and destination positions
    uint src = 0;

    // stdio_printf("Drawing from %08X\n", info->offset);
    // stdio_printf("Aux dimensions %04d x %04d\n", width, height);
    // stdio_printf("Media dimensions %04d x %04d\n", info->width, info->height);

    size_t pixel_count = info->width * info->height;

    const uint8_t *ptr = hndl->ptr ? hndl->ptr + info->offset + (value * info->bytes_per_frame) : (uint8_t *)gfx.__get_aux_buffer(info->bytes_per_frame);

    FRESULT fr;
    UINT read_count;

    if (!hndl->ptr)
    {
        fr = f_lseek(hndl->fil, info->offset + (value * info->bytes_per_frame));
        if (FR_OK != fr)
        {
            stdio_printf("f_lseek error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        fr = f_read(hndl->fil, (uint8_t *)ptr, info->bytes_per_frame, &read_count);
        if (FR_OK != fr)
        {
            stdio_printf("f_read error: %s (%d)\n", FRESULT_str(fr), fr);
        }
    }

    if (info->mode != 24)
    {
        int x = x1, y = y1;
        uint bytes_per_line = 2 * info->width;
        uint pos = (y1 * width) + x1;
        for (uint i = 0; i < info->height; i++)
        {
            memcpy(&dest[pos], &ptr[src], bytes_per_line);
            pos += width;
            src += bytes_per_line;
        }
        return;
    }

    int x = 0, y = 0;

    while (pixel_count > 0)
    {
        uint8_t alpha = ptr[src++];
        uint8_t len;
        switch (alpha)
        {
        case 0:
            len = ptr[src++];
            pixel_count -= len;
            while (len--)
            {
                x++;
                if (x >= info->width)
                {
                    y++;
                    if (y >= info->height)
                        break;
                    x = 0;
                }
            }
            break;
        case 255:
            len = ptr[src++];
            pixel_count -= len;
            while (len--)
            {
                if (!drawArea || (x >= drawArea->x1 && x <= drawArea->x2 && y >= drawArea->y1 && y <= drawArea->y2))
                {
                    uint16_t color = (ptr[src + 1] << 8) | ptr[src];
                    uint pos = (y1 + y) * width + x1 + x;
                    dest[pos] = color;
                }
                src += 2;
                x++;
                if (x >= info->width)
                {
                    y++;
                    if (y >= info->height)
                        break;
                    x = 0;
                }
            }
            break;
        default:
            if (!drawArea || (x >= drawArea->x1 && x <= drawArea->x2 && y >= drawArea->y1 && y <= drawArea->y2))
            {
                uint16_t color = (ptr[src + 1] << 8) | ptr[src];
                uint pos = (y1 + y) * width + x1 + x;
                dest[pos] = gfx.BlendColor(dest[pos], color, alpha);
            }
            src += 2;
            x++;
            if (x >= info->width)
            {
                y++;
                if (y >= info->height)
                    break;
                x = 0;
            }
            pixel_count--;
            break;
        }
    }
}

void GraphicsMedia4D::__show_digits(ImageControl4D hndl, MediaInfo4D digits, bool draw_fb)
{
    // prepare aux buffer
    size_t pixel_count = digits->width * digits->height;
    uint16_t *dest;

    int x1 = digits->x;
    int y1 = digits->y;
    int x2 = x1 + digits->width - 1;
    int y2 = y1 + digits->height - 1;

    MediaInfo4D frames = digits + 1;

    if (hndl->ptr && (digits->mode != 24))
    {
        // if fully opaque, simply draw base image
#ifdef GEN4_RP2350_RGB
        dest = gfx.__get_aux_buffer(digits->width, digits->height);
        if (!dest)
            return;
        memcpy(dest, hndl->ptr + digits->offset, pixel_count * 2);
#else
        gfx.RectangleFilled(x1, y1, x2, y2, hndl->ptr + digits->offset, false);
#endif
    }

    if (hndl->ptr && (digits->mode == 24))
    {
        // if with alpha, get background from form
        // and draw base image on top of it
#ifdef GEN4_RP2350_RGB
        dest = __copy_form_to_aux(hndl, digits->form, x1, y1, x2, y2);
        if (!dest)
            return;
        __draw_to_buffer(hndl, digits, gfx.__aux_buffer, digits->width, digits->height, 0, 0, 0);
#else
        __redraw_form_region(hndl, digits->form, x1, y1, x2, y2);
        gfx.RectangleFilledWithAlpha(x1, y1, x2, y2, hndl->ptr + digits->offset, false);
#endif
    }

    UINT read_count;

    if (!hndl->ptr && (digits->mode != 24))
    {
        // if fully opaque, simply draw base image
        dest = gfx.__get_aux_buffer(digits->bytes_per_frame);
        if (!dest)
            return;
#ifdef GEN4_RP2350_RGB
        stdio_printf("Fully opaque digits from uSD not yet supported\n");
        return;
#else
        FRESULT fr = f_lseek(hndl->fil, digits->offset);
        if (FR_OK != fr)
        {
            stdio_printf("f_lseek error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        fr = f_read(hndl->fil, dest, digits->bytes_per_frame, &read_count);
        if (FR_OK != fr)
        {
            stdio_printf("f_read error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        gfx.RectangleFilled(x1, y1, x2, y2, dest, false);
#endif
    }

    if (!hndl->ptr && (digits->mode == 24))
    {
        // if with alpha, get background from form
        // and draw base image on top of it
#ifdef GEN4_RP2350_RGB
        stdio_printf("Digits with alpha from uSD not yet supported\n");
        return;
#else
        __redraw_form_region(hndl, digits->form, x1, y1, x2, y2);

        dest = gfx.__get_aux_buffer(digits->bytes_per_frame);
        if (!dest)
            return;

        FRESULT fr = f_lseek(hndl->fil, digits->offset);
        if (FR_OK != fr)
        {
            stdio_printf("f_lseek error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        fr = f_read(hndl->fil, dest, digits->bytes_per_frame, &read_count);
        if (FR_OK != fr)
        {
            stdio_printf("f_read error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        gfx.RectangleFilledWithAlpha(x1, y1, x2, y2, (uint8_t *)dest, false);
#endif
    }

    // get digit properties
    uint16_t Digits = digits->properties[0];
    uint16_t MinDigits = digits->properties[1];
    uint16_t decimals = digits->properties[2];
    uint16_t WidthDigit = digits->properties[3];
    uint16_t widthPoint = digits->properties[4];
    bool Unsigned = (digits->option & 0x1) == 0;
    bool LeadingBlanks = (digits->option & 0x2) == 0;
    int16_t value = (int16_t)digits->value;

    // Manually calculate powers of 10
    uint32_t powerOfTen = 1;
    for (uint16_t i = 0; i < decimals; i++)
    {
        powerOfTen *= 10;
    }

    // Compose string based on value
    char digitString[8] = {0}; // Assuming 7 digits + null terminator
    int len = 0;

    if (decimals > 0)
    {
        int intValue = value / powerOfTen;
        int fracValue = value % powerOfTen;
        len = snprintf(digitString, sizeof(digitString), "%0*d.%0*d",
                       Digits - decimals, intValue,
                       decimals, fracValue);
    }
    else
    {
        len = snprintf(digitString, sizeof(digitString), "%0*d", Digits, value);
    }

    // Handle leading blanks or zeros
    if (LeadingBlanks)
    {
        for (int i = 0; i < len; i++)
        {
            if (digitString[i] == '0' && (digitString[i + 1] != '.' || digitString[i + 1] != '\0'))
            {
                digitString[i] = ' ';
            }
            else
            {
                break;
            }
        }
    }

    // Draw each digit
#ifdef GEN4_RP2350_RGB
    int drawX = 0;
#else
    int drawX = x1;
#endif
    int charIndex;

    if (!hndl->ptr)
    {
        FSIZE_t framesSize = frames->bytes_per_frame * 12;
        dest = gfx.__get_aux_buffer(framesSize);
        if (!dest)
            return;
        FRESULT fr = f_lseek(hndl->fil, frames->offset);
        if (FR_OK != fr)
        {
            stdio_printf("f_lseek error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        fr = f_read(hndl->fil, dest, framesSize, &read_count);
        if (FR_OK != fr)
        {
            stdio_printf("f_read error: %s (%d)\n", FRESULT_str(fr), fr);
        }
    }
#ifndef GEN4_RP2350_RGB
    else
    {
        dest = (uint16_t *)(hndl->ptr + frames->offset);
    }
#endif

    for (int i = 0; i < len; i++)
    {
        // Adjust for spacing if needed, especially for decimals
        switch (digitString[i])
        {
        case '.':
            drawX += widthPoint;
            continue;
        case ' ':
            charIndex = 10;
            break;
        case '-':
            charIndex = 11;
            break;
        default:
            charIndex = digitString[i] - '0';
            break;
        }
#ifdef GEN4_RP2350_RGB
        __draw_to_buffer(hndl, frames, gfx.__aux_buffer, digits->width, digits->height, drawX, 0, charIndex);
#else
        __draw_to_buffer(frames, (uint8_t *)dest, gfx.__framebuffer, gfx.__display.width, gfx.__display.height, drawX, y1, charIndex);
#endif
        drawX += WidthDigit;
    }

#ifdef GEN4_RP2350_RGB
    return gfx.RectangleFilled(x1, y1, x2, y2, dest);
#else
    return gfx.SendFrameBuffer(x1, y1, x2, y2);
#endif
}

void GraphicsMedia4D::__show_2_frame_gauge(ImageControl4D hndl, MediaInfo4D gauge, bool draw_fb)
{

    uint16_t range = gauge->properties ? gauge->properties[0] : 100;

    uint16_t value = gauge->value;

    if (value > range)
        value = range;

    if (value < 0)
        value = 0;

    if (!redrawForm && gauge->last_value == value)
        return;

    gauge->last_value = value;

    size_t pixel_count = gauge->width * gauge->height;
    uint16_t *dest;

    int x1 = gauge->x;
    int y1 = gauge->y;
    int x2 = x1 + gauge->width - 1;
    int y2 = y1 + gauge->height - 1;

    if (hndl->ptr && (gauge->mode != 24))
    {
        // if fully opaque, simply draw base image
#ifdef GEN4_RP2350_RGB
        dest = gfx.__get_aux_buffer(gauge->width, gauge->height);
        if (!dest)
            return;
        memcpy(dest, hndl->ptr + gauge->offset, pixel_count * 2);
#else
        gfx.RectangleFilled(x1, y1, x2, y2, hndl->ptr + gauge->offset, false);
#endif
    }

    if (hndl->ptr && (gauge->mode == 24))
    {
        // if with alpha, get background from form
        // and draw base image on top of it
#ifdef GEN4_RP2350_RGB
        dest = __copy_form_to_aux(hndl, gauge->form, x1, y1, x2, y2);
        if (!dest)
            return;
        __draw_to_buffer(hndl, gauge, gfx.__aux_buffer, gauge->width, gauge->height, 0, 0, 0);
#else
        __redraw_form_region(hndl, gauge->form, x1, y1, x2, y2);
        gfx.RectangleFilledWithAlpha(x1, y1, x2, y2, hndl->ptr + gauge->offset, false);
#endif
    }

    UINT read_count;

    // Add fill
    if (!hndl->ptr)
    {
        dest = gfx.__get_aux_buffer(gauge->bytes_per_frame * 2);
        if (!dest)
            return;
        FRESULT fr = f_lseek(hndl->fil, gauge->offset);
        if (FR_OK != fr)
        {
            stdio_printf("f_lseek error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        fr = f_read(hndl->fil, dest, gauge->bytes_per_frame * 2, &read_count);
        if (FR_OK != fr)
        {
            stdio_printf("f_read error: %s (%d)\n", FRESULT_str(fr), fr);
        }
    }
#ifndef GEN4_RP2350_RGB
    else
    {
        dest = (uint16_t *)(hndl->ptr + gauge->offset);
    }
#endif

    BufferRegion *drawArea = new BufferRegion();
    drawArea->x1 = 0;
    drawArea->y1 = 0;
    drawArea->x2 = gauge->width - 1;
    drawArea->y2 = gauge->height - 1;

    if (gauge->width > gauge->height)
    {
        // horizontal
        drawArea->x2 = value * gauge->width / range;
    }
    else
    {
        // vertical
        drawArea->y1 = gauge->height - (value * gauge->height / range);
    }

    if (drawArea->y2 > drawArea->y1 && drawArea->x2 > drawArea->x1)
    {
#ifdef GEN4_RP2350_RGB
        __draw_to_buffer(hndl, gauge, gfx.__aux_buffer, gauge->width, gauge->height, 0, 0, 1, drawArea);
#else
        __draw_to_buffer(gauge, (uint8_t *)dest, gfx.__framebuffer, gfx.__display.width, gfx.__display.height, x1, y1, 1, drawArea);
#endif
    }

    if (gauge->width > gauge->height)
    {
        // horizontal
        drawArea->x1 = drawArea->x2 + 1;
        drawArea->x2 = gauge->width - 1;
    }
    else
    {
        // vertical
        drawArea->y2 = drawArea->y1 - 1;
        drawArea->y1 = 0;
    }

    // stdio_printf("Fill area\n");

    if (drawArea->y2 > drawArea->y1 && drawArea->x2 > drawArea->x1)
    {
#ifdef GEN4_RP2350_RGB
        __draw_to_buffer(hndl, gauge, gfx.__aux_buffer, gauge->width, gauge->height, 0, 0, 0, drawArea);
#else
        __draw_to_buffer(gauge, (uint8_t *)dest, gfx.__framebuffer, gfx.__display.width, gfx.__display.height, x1, y1, 0, drawArea);
#endif
    }

    delete drawArea;

#ifdef GEN4_RP2350_RGB
    return gfx.RectangleFilled(x1, y1, x2, y2, dest);
#else
    return gfx.SendFrameBuffer(x1, y1, x2, y2);
#endif
}

void GraphicsMedia4D::__show_linear_gauge(ImageControl4D hndl, MediaInfo4D gauge, bool draw_fb)
{

    uint16_t range = gauge->properties ? gauge->properties[0] : 100;

    uint16_t value = gauge->value;

    if (value > range)
        value = range;

    if (value < 0)
        value = 0;

    if (!redrawForm && gauge->last_value == value)
        return;

    gauge->last_value = value;

    // prepare aux buffer
    size_t pixel_count = gauge->width * gauge->height;
    uint16_t *dest;

    int x1 = gauge->x;
    int y1 = gauge->y;
    int x2 = x1 + gauge->width - 1;
    int y2 = y1 + gauge->height - 1;

    MediaInfo4D fill = gauge + 1;
    MediaInfo4D thumb = gauge + 2;

    if (hndl->ptr && (gauge->mode != 24))
    {
        // stdio_printf("Linear gauge/slider from Flash without transparency\n");
        // if fully opaque, simply draw base image
#ifdef GEN4_RP2350_RGB
        dest = gfx.__get_aux_buffer(gauge->width, gauge->height);
        if (!dest)
            return;
        memcpy(dest, hndl->ptr + gauge->offset, pixel_count * 2);
#else
        gfx.RectangleFilled(x1, y1, x2, y2, hndl->ptr + gauge->offset, false);
#endif
    }

    if (hndl->ptr && (gauge->mode == 24))
    {
        // stdio_printf("Linear gauge/slider from Flash with transparency\n");
        // if with alpha, get background from form
        // and draw base image on top of it
#ifdef GEN4_RP2350_RGB
        dest = __copy_form_to_aux(hndl, gauge->form, x1, y1, x2, y2);
        if (!dest)
            return;
        __draw_to_buffer(hndl, gauge, gfx.__aux_buffer, gauge->width, gauge->height, 0, 0, 0);
#else
        __redraw_form_region(hndl, gauge->form, x1, y1, x2, y2);
        gfx.RectangleFilledWithAlpha(x1, y1, x2, y2, hndl->ptr + gauge->offset, false);
#endif
    }

    UINT read_count;

    if (!hndl->ptr && (gauge->mode != 24))
    {
        // if fully opaque, simply draw base image
        dest = gfx.__get_aux_buffer(gauge->bytes_per_frame);
        if (!dest)
            return;
#ifdef GEN4_RP2350_RGB
        stdio_printf("Fully opaque gauge from uSD not yet supported\n");
        return;
#else
        FRESULT fr = f_lseek(hndl->fil, gauge->offset);
        if (FR_OK != fr)
        {
            stdio_printf("f_lseek error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        fr = f_read(hndl->fil, dest, gauge->bytes_per_frame, &read_count);
        if (FR_OK != fr)
        {
            stdio_printf("f_read error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        gfx.RectangleFilled(x1, y1, x2, y2, dest, false);
#endif
    }

    if (!hndl->ptr && (gauge->mode == 24))
    {
        // if with alpha, get background from form
        // and draw base image on top of it

#ifdef GEN4_RP2350_RGB
        stdio_printf("Gauge with alpha from uSD not yet supported\n");
        return;
#else

        __redraw_form_region(hndl, gauge->form, x1, y1, x2, y2);

        dest = gfx.__get_aux_buffer(gauge->bytes_per_frame);
        if (!dest)
            return;

        FRESULT fr = f_lseek(hndl->fil, gauge->offset);
        if (FR_OK != fr)
        {
            stdio_printf("f_lseek error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        fr = f_read(hndl->fil, dest, gauge->bytes_per_frame, &read_count);
        if (FR_OK != fr)
        {
            stdio_printf("f_read error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        gfx.RectangleFilledWithAlpha(x1, y1, x2, y2, (uint8_t *)dest, false);
#endif
    }

    BufferRegion *drawArea = new BufferRegion();

    // Add fill
    if (!hndl->ptr)
    {
        dest = gfx.__get_aux_buffer(fill->bytes_per_frame);
        if (!dest)
            return;
        FRESULT fr = f_lseek(hndl->fil, fill->offset);
        if (FR_OK != fr)
        {
            stdio_printf("f_lseek error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        fr = f_read(hndl->fil, dest, fill->bytes_per_frame, &read_count);
        if (FR_OK != fr)
        {
            stdio_printf("f_read error: %s (%d)\n", FRESULT_str(fr), fr);
        }
    }
#ifndef GEN4_RP2350_RGB
    else
    {
        dest = (uint16_t *)(hndl->ptr + fill->offset);
    }
#endif

    drawArea->x1 = 0;
    drawArea->y1 = 0;
    drawArea->x2 = fill->width - 1;
    drawArea->y2 = fill->height - 1;

    uint length, trackLen, trackPos, xOffset, yOffset;

    if (gauge->width > gauge->height)
    {
        // horizontal
        length = fill->width;
        trackLen = length - thumb->width;
        trackPos = value * trackLen / range;
        drawArea->x2 = trackPos + (thumb->width / 2);
        xOffset = trackPos;
        yOffset = 0;
    }
    else
    {
        // vertical
        length = fill->height;
        trackLen = length - thumb->height;
        trackPos = trackLen - (value * trackLen / range);
        drawArea->y1 = trackPos + (thumb->width / 2);
        xOffset = 0;
        yOffset = trackPos;
    }

#ifdef GEN4_RP2350_RGB
    __draw_to_buffer(hndl, fill, gfx.__aux_buffer, fill->width, fill->height, 0, 0, 0, drawArea);
#else
    __draw_to_buffer(fill, (uint8_t *)dest, gfx.__framebuffer, gfx.__display.width, gfx.__display.height, x1, y1, 0, drawArea);
#endif

    // Add thumb
    if (!hndl->ptr)
    {
        dest = gfx.__get_aux_buffer(thumb->bytes_per_frame);
        if (!dest)
        {
            delete drawArea;
            return;
        }

        FRESULT fr = f_lseek(hndl->fil, thumb->offset);
        if (FR_OK != fr)
        {
            stdio_printf("f_lseek error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        fr = f_read(hndl->fil, dest, thumb->bytes_per_frame, &read_count);
        if (FR_OK != fr)
        {
            stdio_printf("f_read error: %s (%d)\n", FRESULT_str(fr), fr);
        }
    }
#ifndef GEN4_RP2350_RGB
    else
    {
        dest = (uint16_t *)(hndl->ptr + thumb->offset);
    }
#endif

#ifdef GEN4_RP2350_RGB
    __draw_to_buffer(hndl, thumb, gfx.__aux_buffer, gauge->width, gauge->height, xOffset, yOffset, 0);
#else
    __draw_to_buffer(thumb, (uint8_t *)dest, gfx.__framebuffer, gfx.__display.width, gfx.__display.height, x1 + xOffset, y1 + yOffset, 0);
#endif

    delete drawArea;

#ifdef GEN4_RP2350_RGB
    gfx.RectangleFilled(x1, y1, x2, y2, dest);
#else
    gfx.SendFrameBuffer(x1, y1, x2, y2);
#endif
}

void GraphicsMedia4D::__show_knob(ImageControl4D hndl, MediaInfo4D knob, bool draw_fb)
{

    MediaInfo4D thumb = knob + 1;

    uint16_t value = knob->value;

    if (value >= thumb->frames)
        value = thumb->frames - 1;

    if (value < 0)
        value = 0;

    if (!redrawForm && knob->last_value == value)
        return;

    knob->last_value = value;

    // prepare aux buffer
    size_t pixel_count = knob->width * knob->height;
    uint16_t *dest;

    int x1 = knob->x;
    int y1 = knob->y;
    int x2 = x1 + knob->width - 1;
    int y2 = y1 + knob->height - 1;

    if (hndl->ptr && (knob->mode != 24))
    {
        // if fully opaque, simply draw base image
#ifdef GEN4_RP2350_RGB
        dest = gfx.__get_aux_buffer(knob->width, knob->height);
        if (!dest)
            return;
        memcpy(dest, hndl->ptr + knob->offset, pixel_count * 2);
#else
        gfx.RectangleFilled(x1, y1, x2, y2, hndl->ptr + knob->offset, false);
#endif
    }

    if (hndl->ptr && (knob->mode == 24))
    {
        // if with alpha, get background from form
        // and draw base image on top of it
#ifdef GEN4_RP2350_RGB
        dest = __copy_form_to_aux(hndl, knob->form, x1, y1, x2, y2);
        if (!dest)
            return;
        __draw_to_buffer(hndl, knob, gfx.__aux_buffer, knob->width, knob->height, 0, 0, 0);
#else
        __redraw_form_region(hndl, knob->form, x1, y1, x2, y2);
        gfx.RectangleFilledWithAlpha(x1, y1, x2, y2, hndl->ptr + knob->offset, false);
#endif
    }

    UINT read_count;

    if (!hndl->ptr && (knob->mode != 24))
    {
        // if fully opaque, simply draw base image
        dest = gfx.__get_aux_buffer(knob->bytes_per_frame);
        if (!dest)
            return;
#ifdef GEN4_RP2350_RGB
        stdio_printf("Fully opaque knob from uSD not yet supported\n");
        return;
#else
        FRESULT fr = f_lseek(hndl->fil, knob->offset);
        if (FR_OK != fr)
        {
            stdio_printf("f_lseek error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        fr = f_read(hndl->fil, dest, knob->bytes_per_frame, &read_count);
        if (FR_OK != fr)
        {
            stdio_printf("f_read error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        gfx.RectangleFilled(x1, y1, x2, y2, dest, false);
#endif
    }

    if (!hndl->ptr && (knob->mode == 24))
    {
        // if with alpha, get background from form
        // and draw base image on top of it

#ifdef GEN4_RP2350_RGB
        stdio_printf("Knob with alpha from uSD not yet supported\n");
        return;
#else

        __redraw_form_region(hndl, knob->form, x1, y1, x2, y2);

        dest = gfx.__get_aux_buffer(knob->bytes_per_frame);
        if (!dest)
            return;

        FRESULT fr = f_lseek(hndl->fil, knob->offset);
        if (FR_OK != fr)
        {
            stdio_printf("f_lseek error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        fr = f_read(hndl->fil, dest, knob->bytes_per_frame, &read_count);
        if (FR_OK != fr)
        {
            stdio_printf("f_read error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        gfx.RectangleFilledWithAlpha(x1, y1, x2, y2, (uint8_t *)dest, false);
#endif
    }

    if (!hndl->ptr)
    {
        dest = gfx.__get_aux_buffer(thumb->bytes_per_frame);
        if (!dest)
            return;
        FRESULT fr = f_lseek(hndl->fil, thumb->offset + (thumb->bytes_per_frame * knob->value));
        if (FR_OK != fr)
        {
            stdio_printf("f_lseek error: %s (%d)\n", FRESULT_str(fr), fr);
        }
        fr = f_read(hndl->fil, dest, thumb->bytes_per_frame, &read_count);
        if (FR_OK != fr)
        {
            stdio_printf("f_read error: %s (%d)\n", FRESULT_str(fr), fr);
        }
    }
#ifndef GEN4_RP2350_RGB
    else
    {
        dest = (uint16_t *)(hndl->ptr + thumb->offset + (thumb->bytes_per_frame * knob->value));
    }
#endif

#ifdef GEN4_RP2350_RGB
    __draw_to_buffer(hndl, thumb, gfx.__aux_buffer, knob->width, knob->height, 0, 0, value);
#else
    __draw_to_buffer(thumb, (uint8_t *)dest, gfx.__framebuffer, gfx.__display.width, gfx.__display.height, x1, y1, 0);
#endif

#ifdef GEN4_RP2350_RGB
    return gfx.RectangleFilled(x1, y1, x2, y2, dest);
#else
    return gfx.SendFrameBuffer(x1, y1, x2, y2);
#endif
}

void GraphicsMedia4D::__redraw_form_region(ImageControl4D hndl, MediaInfo4D form, int x1, int y1, int x2, int y2)
{
    if (form->form)
    {
        return;
    }

    uint width = x2 - x1 + 1;
    uint height = y2 - y1 + 1;
    uint pixel_count = width * height;

    if (form->option == 0)
    {
        return gfx.RectangleFilled(x1, y1, x2, y2, (uint16_t)(form->offset & 0xFFFF), false);
    }

    uint bytes_per_line = 2 * width;
    uint bytes_per_width = 2 * gfx.__display.width;
    uint16_t *dest = gfx.__framebuffer + (gfx.__display.width * y1) + x1;

    if (hndl->ptr)
    {
        const uint8_t *src = hndl->ptr + form->offset + (bytes_per_width * y1) + (x1 * 2);

        for (uint i = 0; i < height; i++)
        {
            memcpy(dest, src, bytes_per_line);
            src += bytes_per_width;
            dest += gfx.__display.width;
        }

        return;
    }

    uint offset = form->offset + (bytes_per_width * y1) + (x1 * 2);

    // calculate number of bytes to read from form
    UINT bytes_per_read = bytes_per_width * height;
    if (bytes_per_read > MAX_AUX_IN_SRAM)
        bytes_per_read = MAX_AUX_IN_SRAM;

    UINT lines_per_read = bytes_per_read / bytes_per_width;
    bytes_per_read = lines_per_read * bytes_per_width;

    uint16_t *ptr = gfx.__get_aux_buffer(bytes_per_read);

    UINT remaining_lines = height;

    FRESULT fr = f_lseek(hndl->fil, offset);
    UINT read_count;

    int y = y1;

    while (remaining_lines > 0)
    {

        if (lines_per_read > remaining_lines)
        {
            lines_per_read = remaining_lines;
        }

        UINT current_bytes_per_read = lines_per_read * bytes_per_width;
        fr = f_read(hndl->fil, ptr, current_bytes_per_read, &read_count);

        for (UINT lines_read = 0; lines_read < lines_per_read; lines_read++)
        {
            // stdio_printf("Drawing line: %d\n", y);
            gfx.RectangleFilled(x1, y, x2, y, ptr + (lines_read * gfx.__display.width), false);
            y++;
            // TODO: Implement Hline
        }

        // offset += current_bytes_per_read;
        remaining_lines -= lines_per_read;
    }
}

uint16_t *GraphicsMedia4D::__copy_form_to_aux(ImageControl4D hndl, MediaInfo4D form, int x1, int y1, int x2, int y2)
{

    if (form->form)
    {
        return NULL;
    }

    uint width = x2 - x1 + 1;
    uint height = y2 - y1 + 1;
    uint pixel_count = width * height;

    if (!gfx.__get_aux_buffer(width, height))
        return NULL;

    if (form->option == 0)
    {
        uint16_t color = form->offset & 0xFFFF;
        for (uint i = 0; i < pixel_count; i++)
            gfx.__aux_buffer[i] = color;
        return gfx.__aux_buffer;
    }

    if (hndl->ptr)
    {
        uint bytes_per_line = 2 * width;
        uint bytes_per_width = 2 * gfx.__display.width;

        uint16_t *dest = gfx.__aux_buffer;
        const uint8_t *src = hndl->ptr + form->offset + (bytes_per_width * y1) + (x1 * 2);

        for (uint i = 0; i < height; i++)
        {
            memcpy(dest, src, bytes_per_line);
            src += bytes_per_width;
            dest += width;
        }

        return gfx.__aux_buffer;
    }

    stdio_printf("Copy form from uSD still not supported\n");
    return gfx.__aux_buffer;
}

// TODO: Add image/video functions
//       Waiting on SDIO

#ifndef LCD_TOUCH_NONE

/*
 * Touch Related functions
 */

#ifndef LCD_TOUCH_NS2009
#define LCD_TOUCH_ADDRESS   0x38
#else
#define LCD_TOUCH_ADDRESS   0x48
#endif

uint8_t GraphicsTouch4D::__points;
TouchInfo GraphicsTouch4D::__touch[LCD_TOUCH_POINTS];

Graphics4D &GraphicsTouch4D::gfx = Graphics4D::GetInstance();

GraphicsTouch4D::GraphicsTouch4D()
{
}

GraphicsTouch4D::~GraphicsTouch4D()
{
}

bool GraphicsTouch4D::Initialize()
{

#if defined(LCD_TOUCH_4WIRE) || defined(LCD_TOUCH_NS2009)
    if (!(__get_calibration())){
        __calib_start_x = LCD_TOUCH_START_X;
        __calib_start_y = LCD_TOUCH_START_Y;
        __calib_end_x   = LCD_TOUCH_END_X;
        __calib_end_y   = LCD_TOUCH_END_Y;
    }
#endif

#ifdef LCD_TOUCH_4WIRE

    // Initialize GPIO for digital I/O pins
    gpio_init(LCD_TOUCH_YD);
    gpio_init(LCD_TOUCH_XL);

    // Initialize ADC
    adc_init();

#else

    gpio_init(LCD_TOUCH_RST);
    gpio_init(LCD_TOUCH_INT);
    gpio_set_dir(LCD_TOUCH_RST, GPIO_OUT);
    gpio_set_dir(LCD_TOUCH_INT, GPIO_IN);
    gpio_pull_up(LCD_TOUCH_INT); // Enable pull-up resistor

    gpio_put(LCD_TOUCH_RST, 0);
    delay(100);
    gpio_put(LCD_TOUCH_RST, 1);

    i2c_init(LCD_TOUCH_I2C, 400 * 1000);
    gpio_set_function(LCD_TOUCH_SDA, GPIO_FUNC_I2C);
    gpio_set_function(LCD_TOUCH_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(LCD_TOUCH_SDA);
    gpio_pull_up(LCD_TOUCH_SCL);

#ifndef LCD_TOUCH_NS2009
    // keep active mode even when no touch event (FTxxxx)
    i2c_write_blocking(LCD_TOUCH_I2C, LCD_TOUCH_ADDRESS, (const uint8_t[]){0x86}, 1, true);
    i2c_write_blocking(LCD_TOUCH_I2C, LCD_TOUCH_ADDRESS, (const uint8_t[]){0x00}, 1, false);
#endif

#endif

    return true;
}

#if defined(LCD_TOUCH_4WIRE) || defined(LCD_TOUCH_NS2009)
#define TOUCH_CALIBRATION_OFFSET            PICO_FLASH_SIZE_BYTES - (2 * FLASH_SECTOR_SIZE)
#define TOUCH_CALIB_POINTS                  20  // Number of calibration points
#if defined(LCD_TOUCH_4WIRE)
#define TOUCH_4WIRE_THRESHOLD               200  // Threshold for filtering
#else
#define TOUCH_4WIRE_THRESHOLD               50  // Threshold for filtering
#endif

int GraphicsTouch4D::__calib_start_x;
int GraphicsTouch4D::__calib_start_y;
int GraphicsTouch4D::__calib_end_x;
int GraphicsTouch4D::__calib_end_y;

bool GraphicsTouch4D::Calibrate()
{
    uint8_t last_orientation = gfx.ScreenMode(LCD_ORIENTATION);

    const uint8_t *last_font = gfx.SetFont(Font1);

    bool redo = true;
    int bak_start_x = __calib_start_x;
    int bak_start_y = __calib_start_y;
    int bak_end_x = __calib_end_x;
    int bak_end_y = __calib_end_y;

    const int threshold = TOUCH_4WIRE_THRESHOLD;  // Set a reasonable threshold for filtering
    uint8_t status;
    int num_points = 0;
    int x_raw[TOUCH_CALIB_POINTS];
    int y_raw[TOUCH_CALIB_POINTS];

    int x_sum = 0;
    int y_sum = 0;
    int x_avg = 0;
    int y_avg = 0;

    struct TouchInfo *touch = &__touch[0]; // 4-wire can only have single touch point

    do {

        gfx.SetBackgroundColor(BLACK);
        gfx.Cls();

        delay(500);

        // Record start test point    
        // Draw touch points at (20, 20) and (LCD_WIDTH - 21, LCD_HEIGHT - 21)
        gfx.Circle(20, 20, 5, 0x07E0);
        gfx.Vline(20, 10, 30, 0x07E0);
        gfx.Hline(20, 10, 30, 0x07E0);

        gfx.MoveTo(35, 17);
        gfx.print("<<< hold here");

        num_points = 0;
        x_sum = 0;
        y_sum = 0;
        x_avg = 0;
        y_avg = 0;

        while (num_points < TOUCH_CALIB_POINTS) {

            uint8_t points = GetPoints();

            if (points == 0) continue;

            status = GetStatus();

            gfx.MoveTo(0, 100);
            gfx.printf("Status  : %d  \n", status);
            gfx.printf("Points  : %d  \n", num_points);

            if (/* status == TOUCH_PRESSED || */status == TOUCH_MOVING) {
            // if (status == TOUCH_PRESSED || status == TOUCH_MOVING) {
                int x = touch->xraw;
                int y = touch->yraw;

                gfx.printf("Cur X : %d  \n", x);
                gfx.printf("Cur Y : %d  \n", y);

                // If we've already collected some points, check if the new point is within the threshold
                if (num_points > 0) {
                    if (abs(x - x_avg) > threshold || abs(y - y_avg) > threshold) {
                        // Disregard the point if it's too far from the current average
                        continue;
                    }
                }

                // Accept the point
                x_raw[num_points] = x;
                y_raw[num_points] = y;
                x_sum += x;
                y_sum += y;
                num_points++;

                // Recompute the average with the new point
                x_avg = x_sum / num_points;
                y_avg = y_sum / num_points;

                gfx.printf("Avg X : %d  \n", x_avg);
                gfx.printf("Avg Y : %d  \n", y_avg);

            }
        }

        gfx.Cls();
        
        do {
            uint8_t points = GetPoints();
            if (points == 0) continue;
            status = GetStatus();
        } while (status != TOUCH_RELEASED && status != NO_TOUCH);

        delay(500);

        int x = LCD_WIDTH - 21;
        int y = LCD_HEIGHT - 21;
        gfx.Circle(x, y, 5, 0x07E0);
        gfx.Vline(x, y - 10, y + 10, 0x07E0);
        gfx.Hline(y, x - 10, x + 10, 0x07E0);
        
        gfx.MoveTo(x - 105, y - 3);
        gfx.print("hold here >>>");

        __calib_start_x = x_avg;
        __calib_start_y = y_avg;    

        num_points = 0;
        x_sum = 0;
        y_sum = 0;
        x_avg = 0;
        y_avg = 0;

        while (num_points < TOUCH_CALIB_POINTS) {

            uint8_t points = GetPoints();

            if (points == 0) continue;

            status = GetStatus();

            gfx.MoveTo(0, 100);
            gfx.printf("Status  : %d  \n", status);
            gfx.printf("Points  : %d  \n", num_points);

            if (/* status == TOUCH_PRESSED || */status == TOUCH_MOVING) {
            // if (status == TOUCH_PRESSED || status == TOUCH_MOVING) {
                int x = touch->xraw;
                int y = touch->yraw;

                gfx.printf("Cur X : %d  \n", x);
                gfx.printf("Cur Y : %d  \n", y);

                // If we've already collected some points, check if the new point is within the threshold
                if (num_points > 0) {
                    if (abs(x - x_avg) > threshold || abs(y - y_avg) > threshold) {
                        // Disregard the point if it's too far from the current average
                        continue;
                    }
                }

                // Accept the point
                x_raw[num_points] = x;
                y_raw[num_points] = y;
                x_sum += x;
                y_sum += y;
                num_points++;

                // Recompute the average with the new point
                x_avg = x_sum / num_points;
                y_avg = y_sum / num_points;

                gfx.printf("Avg X : %d  \n", x_avg);
                gfx.printf("Avg Y : %d  \n", y_avg);

            }
        }

        gfx.Cls();

        do {
            uint8_t points = GetPoints();
            if (points == 0) continue;
            status = GetStatus();
        } while (status != TOUCH_RELEASED && status != NO_TOUCH);

        __calib_end_x = x_avg;
        __calib_end_y = y_avg;

        delay(500);

        gfx.print("Touch test");
        
        uint16_t x_center = LCD_WIDTH >> 1;
        uint16_t y_center = LCD_HEIGHT >> 1;

        gfx.Circle(x_center - 50, y_center - 12, 7, LIME);
        gfx.Circle(x_center - 50, y_center + 12, 7, RED);

        gfx.MoveTo(x_center - 30, y_center - 14);
        gfx.print("<<< CONFIRM");

        gfx.MoveTo(x_center - 30, y_center + 10);
        gfx.print("<<< REDO");

        uint16_t ok_x1 = x_center - 57;
        uint16_t ok_x2 = x_center - 43;
        uint16_t ok_y1 = y_center - 19;
        uint16_t ok_y2 = y_center - 5;
        uint16_t re_x1 = x_center - 57;
        uint16_t re_x2 = x_center - 43;
        uint16_t re_y1 = y_center + 5;
        uint16_t re_y2 = y_center + 19;

        // Perform touch test
        uint16_t x_old = -1, y_old = -1, x_new, y_new;

        bool accepted = false;
        do {
            uint8_t points = GetPoints();
            if (points == 0) continue;

            uint8_t status = touch->status;
            x_new = touch->xpos;
            y_new = touch->ypos;

            gfx.MoveTo(0, 100);
            gfx.printf("Status   : %d  \n", status);
            gfx.printf("Raw X/Y  : %d, %d  \n", touch->xraw, touch->yraw);
            gfx.printf("Position : %d, %d  \n", x_new, y_new);
 
            switch (status) {
                case TOUCH_PRESSED:
                    gfx.CircleFilled(x_new, y_new, 3, RED);
                    x_old = x_new;
                    y_old = y_new;
                    if (x_new >= ok_x1 && x_new <= ok_x2 && y_new >= ok_y1 && y_new <= ok_y2) {
                        accepted = true;
                        redo = false;
                    }
                    if (x_new >= re_x1 && x_new <= re_x2 && y_new >= re_y1 && y_new <= re_y2) {
                        accepted = true;
                    }
                    break;
                case TOUCH_MOVING:
                    if (x_old != -1 && y_old != -1) {
                        gfx.Line(x_old, y_old, x_new, y_new, CYAN);
                    } else {
                        gfx.CircleFilled(x_new, y_new, 3, RED);
                    }
                    x_old = x_new;
                    y_old = y_new;
                    if (x_new >= ok_x1 && x_new <= ok_x2 && y_new >= ok_y1 && y_new <= ok_y2) {
                        accepted = true;
                        redo = false;
                    }
                    if (x_new >= re_x1 && x_new <= re_x2 && y_new >= re_y1 && y_new <= re_y2) {
                        accepted = true;
                    }
                    break;
                case TOUCH_RELEASED:
                    gfx.CircleFilled(x_new, y_new, 3, AQUA);
                    x_old = -1;
                    y_old = -1;
                    break;
            }
        } while (!accepted);

    } while (redo);

    __set_calibration();

    gfx.ScreenMode(last_orientation);
    gfx.SetFont(last_font);

    return true;
}

void GraphicsTouch4D::__set_calibration() {
    uint8_t CalibData[FLASH_SECTOR_SIZE];
    memset(CalibData, 0xff, FLASH_SECTOR_SIZE);
    CalibData[0] = '$';
    CalibData[1] = __calib_start_x & 0xff;
    CalibData[2] = __calib_start_x >> 8;
    CalibData[3] = __calib_start_y & 0xff;
    CalibData[4] = __calib_start_y >> 8;
    CalibData[5] = __calib_end_x & 0xff;
    CalibData[6] = __calib_end_x >> 8;
    CalibData[7] = __calib_end_y & 0xff;
    CalibData[8] = __calib_end_y >> 8;
    CalibData[9] = '$';
    for (int i = 1; i < 9; i++) {
        CalibData[9] ^= CalibData[i];
    }

    gfx.print("Saving calibration values... ");

    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(TOUCH_CALIBRATION_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(TOUCH_CALIBRATION_OFFSET, CalibData, FLASH_PAGE_SIZE);
    restore_interrupts(interrupts);

    gfx.print("Done.\n");
}

bool GraphicsTouch4D::__get_calibration() {
    uint8_t CalibDataR[10];
    const uint8_t* flash_target_contents = (const uint8_t *) (XIP_BASE + TOUCH_CALIBRATION_OFFSET);
    memcpy(&CalibDataR, flash_target_contents, 10);
    __calibrated = false;

    if (CalibDataR[0] != '$') {
        // stdio_printf("Not Calibrated.\n");
        return false;
    }

    uint8_t checksum = 0;

    for (int i = 0; i < 10; i++) {
        checksum ^= CalibDataR[i];
    }

    if (checksum) {    
        // stdio_printf("Not Calibrated.\n");
        return false;
    } 

    __calib_start_x = CalibDataR[1] + (CalibDataR[2] << 8);
    __calib_start_y = CalibDataR[3] + (CalibDataR[4] << 8);
    __calib_end_x = CalibDataR[5] + (CalibDataR[6] << 8);
    __calib_end_y = CalibDataR[7] + (CalibDataR[8] << 8);

    __calibrated = true;
    // stdio_printf("Calibration OK.\n");
    return true;

}
#endif

#define TOUCH_RTP_ATTEMPT               20 // Number of attempts to get samples to take for RTP
#define TOUCH_RTP_SAMPLES               10 // Number of samples required
#define PRESS_TRIP                      60000

uint8_t GraphicsTouch4D::GetPoints()
{

#ifdef LCD_TOUCH_4WIRE
    static uint32_t last_touch_time = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    // int32_t xpos_sum = 0;
    // int32_t ypos_sum = 0;
    // int32_t zval_sum = 0;
    // int valid_samples = 0;

    int m = 0;
    uint16_t xl[TOUCH_RTP_ATTEMPT], yl[TOUCH_RTP_ATTEMPT];
    int z0avg = 0;
    int x_value = 0;
    int y_value = 0;

    for (int i = 0; i < TOUCH_RTP_ATTEMPT; i++) {

        // To read Y, set YD and YU to 0 and 1 respectively and read from XR
        gpio_init(LCD_TOUCH_YU);
        gpio_set_dir(LCD_TOUCH_YU, GPIO_OUT);
        gpio_set_dir(LCD_TOUCH_YD, GPIO_OUT);
        gpio_set_dir(LCD_TOUCH_XL, GPIO_IN); // setting this as input for now
        gpio_disable_pulls(LCD_TOUCH_XL);

        gpio_put(LCD_TOUCH_YU, 1);
        gpio_put(LCD_TOUCH_YD, 0);
        adc_gpio_init(LCD_TOUCH_XR);

        busy_wait_us(DEBOUNCE_DELAY); // Small delay for the ADC to settle

        // Read ADC values
        adc_select_input(LCD_TOUCH_XR - 40);
        int16_t y_val = adc_read();

        // To read X, set XL and XR to 0 and 1 respectively and read from YU
        gpio_init(LCD_TOUCH_XR);
        gpio_set_dir(LCD_TOUCH_XR, GPIO_OUT);
        gpio_set_dir(LCD_TOUCH_XL, GPIO_OUT);
        gpio_set_dir(LCD_TOUCH_YD, GPIO_IN); // setting this as input for now
        gpio_disable_pulls(LCD_TOUCH_YD);
        gpio_put(LCD_TOUCH_XR, 1);
        gpio_put(LCD_TOUCH_XL, 0);
        adc_gpio_init(LCD_TOUCH_YU);

        busy_wait_us(DEBOUNCE_DELAY); // Small delay for the ADC to settle

        // Read ADC values
        adc_select_input(LCD_TOUCH_YU - 40);
        int16_t x_val = adc_read();

        // To verify validity, first set XL to 0 and YD to 1
        gpio_set_dir(LCD_TOUCH_YD, GPIO_OUT);
        gpio_put(LCD_TOUCH_XL, 0);
        gpio_put(LCD_TOUCH_YD, 1);

        // Measure XR while YU is floating
        gpio_init(LCD_TOUCH_YU);
        gpio_set_dir(LCD_TOUCH_YU, GPIO_IN); // setting this as input for now
        gpio_disable_pulls(LCD_TOUCH_YU);
        adc_gpio_init(LCD_TOUCH_XR);

        busy_wait_us(DEBOUNCE_DELAY); // Small delay for the ADC to settle

        // Read ADC values
        adc_select_input(LCD_TOUCH_XR - 40);
        int16_t z0 = adc_read();

        // Comment these out since we only want to confirm a press

        // // Measure YU with XR floating
        // gpio_init(LCD_TOUCH_XR);
        // gpio_set_dir(LCD_TOUCH_XR, GPIO_IN); // setting this as input for now
        // gpio_disable_pulls(LCD_TOUCH_XR);
        // adc_gpio_init(LCD_TOUCH_YU);

        // busy_wait_us(DEBOUNCE_DELAY); // Small delay for the ADC to settle

        // // Read ADC values
        // adc_select_input(LCD_TOUCH_YU - 40);
        // int16_t z1 = adc_read();

        // gpio_disable_pulls(LCD_TOUCH_XR);

        // }

        // gfx.MoveTo(0, 0);
        // gfx.printf("Z: %d  \n", z0);
        
        if (z0 > LCD_TOUCH_THRESHOLD) {
            z0avg += z0;
            x_value += x_val;
            y_value += y_val;
            m++;
        }
    }

    struct TouchInfo *touch = &__touch[0]; // 4-wire can only have single touch point

    if (m < TOUCH_RTP_SAMPLES) {
        // let's treat this as the released state
        if ((touch->status == TOUCH_PRESSED || touch->status == TOUCH_MOVING))
        {
            touch->status = TOUCH_RELEASED;
            __points = 1;
        }
        else
        {
            touch->status = NO_TOUCH;
            __points = 0;
        }
        return __points;
    }

    z0avg /= m;
    x_value /= m;
    y_value /= m;

    // gfx.MoveTo(0, 0);
    // gfx.printf("Z: %d, M: %d  \n", z0avg, m);

    if (z0avg < LCD_TOUCH_THRESHOLD)
    {
        // || touch->xpos < 0 || touch->ypos < 0 || touch->xpos >= LCD_WIDTH || touch->ypos >= LCD_HEIGHT
        if ((touch->status == TOUCH_PRESSED || touch->status == TOUCH_MOVING) /* && current_time - last_touch_time > RELEASE_THRESHOLD*/)
        {
            touch->status = TOUCH_RELEASED;
            __points = 1;
        }
        else
        {
            touch->status = NO_TOUCH;
            __points = 0;
        }
    }
    else
    {
        __points = 1;
        touch->xraw = x_value;
        touch->yraw = y_value;
        x_value = MAP(x_value, __calib_start_x, __calib_end_x, 20, LCD_WIDTH - 21);
        y_value = MAP(y_value, __calib_start_y, __calib_end_y, 20, LCD_HEIGHT - 21);

        if (x_value >= 0 && y_value >= 0 && x_value < LCD_WIDTH && y_value < LCD_HEIGHT)
        {
            touch->xpos = x_value;
            touch->ypos = y_value;
        }
        last_touch_time = current_time;
        if (touch->status == NO_TOUCH || touch->status == TOUCH_RELEASED)
        {
            touch->status = TOUCH_PRESSED;
        }
        else
        {
            touch->status = TOUCH_MOVING;
        }
    }

#elif defined(LCD_TOUCH_NS2009)

    int m = 0;
    uint8_t rxdata[2];
    uint16_t xl[TOUCH_RTP_SAMPLES], yl[TOUCH_RTP_SAMPLES];
    int txpos = 0; int typos = 0; int z1 = 0; int z2 = 0;
    int txraw = 0; int tyraw = 0;
    // txpos1 = 0; // seems to be unused, only being set to 0, nothing else
        //int xl[READS], yl[READS];

    long p = 0;
    int mnx = 0; int mny = 0; int mnxi = 0; int mnyi = 0; int mxx = 0; int mxy = 0; int mxxi = 0; int mxyi = 0;
    int i;
    m = 0;

    // Read multiple samples
    for (i = 0; i < TOUCH_RTP_ATTEMPT; i++) {

        uint16_t x, y, z1, z2;

        // Read x
        i2c_write_blocking(LCD_TOUCH_I2C, LCD_TOUCH_ADDRESS, (const uint8_t[]){0xc4}, 1, true);
        i2c_read_burst_blocking(LCD_TOUCH_I2C, LCD_TOUCH_ADDRESS, rxdata, 2);
        x = (rxdata[0] << 4) | (rxdata[1] >> 4);

        // Get touch pressure z1 sample
        i2c_write_blocking(LCD_TOUCH_I2C, LCD_TOUCH_ADDRESS, (const uint8_t[]){0xe4}, 1, true);
        i2c_read_burst_blocking(LCD_TOUCH_I2C, LCD_TOUCH_ADDRESS, rxdata, 2);
        z1 = (rxdata[0] << 4) | (rxdata[1] >> 4);

        // Get touch pressure z2 sample
        i2c_write_blocking(LCD_TOUCH_I2C, LCD_TOUCH_ADDRESS, (const uint8_t[]){0xf4}, 1, true);
        i2c_read_burst_blocking(LCD_TOUCH_I2C, LCD_TOUCH_ADDRESS, rxdata, 2);
        z2 = (rxdata[0] << 4) | (rxdata[1] >> 4);

        // Get touch y sample
        i2c_write_blocking(LCD_TOUCH_I2C, LCD_TOUCH_ADDRESS, (const uint8_t[]){0xd4}, 1, true);
        i2c_read_burst_blocking(LCD_TOUCH_I2C, LCD_TOUCH_ADDRESS, rxdata, 2);
        y = (rxdata[0] << 4) | (rxdata[1] >> 4);

        // Calculate pressure
        if ((z1 < 20) || (z2 > 4075))
            p = PRESS_TRIP + 1;
        else
            p = (long)x * ((long)z2 / (long)z1 - 1l);

        // Keep correct pressure values
        if (p <= PRESS_TRIP) {
            xl[m] = x;
            yl[m] = y;
            m++;
        }

        if (m == TOUCH_RTP_SAMPLES)
            break;
    }
    if (m == TOUCH_RTP_SAMPLES) // only interested if we got 10 good reads
        {
            mnx = mny = 10000; // real max is 4095
            mxx = mxy = 0;
            txpos = typos = 0;
            for (i = 0; i < TOUCH_RTP_SAMPLES; i++) // work out min and max
            {
                if (xl[i] < mnx) {
                    mnx = xl[i];
                    mnxi = i;
                }
                if (xl[i] > mxx) {
                    mxx = xl[i];
                    mxxi = i;
                }
                if (yl[i] < mny) {
                    mny = yl[i];
                    mnyi = i;
                }
                if (yl[i] > mxy) {
                    mxy = yl[i];
                    mxyi = i;
                }
            }
            if (mnxi == mxxi)
                mxxi++; // only true if all the same, (will both be 0), need to adjust
            // as we need to ignore two values
            if (mnyi == mxyi)
                mxyi++; // only true if all the same, (will both be 0), need to adjust
            // as we need to ignore two values
            for (i = 0; i < TOUCH_RTP_SAMPLES; i++) // discard min and max
            {
                if ((i != mnxi) && (i != mxxi))
                    txpos += xl[i];
                if ((i != mnyi) && (i != mxyi))
                    typos += yl[i];
            }
            txpos /= 8;
            typos /= 8;
            txraw = txpos;
            tyraw = typos;
            
            txpos = MAP(txraw, __calib_start_x, __calib_end_x, 20, LCD_WIDTH - 21);
            typos = MAP(tyraw, __calib_start_y, __calib_end_y, 20, LCD_HEIGHT - 21);

            if (txpos >= 0 && typos >= 0 && txpos < LCD_WIDTH && typos < LCD_HEIGHT)
            {
                txpos = txpos;
                typos = typos;
            }

        }

    if (m == TOUCH_RTP_SAMPLES) {
        //tPen = 1;
        __points = 1;
        if (__touch[0].status == 0 || __touch[0].status == TOUCH_RELEASED)
        {
            __touch[0].status = TOUCH_PRESSED;
        }
        else
        {
            __touch[0].status = TOUCH_MOVING;
        }
    } else {
        if ((__touch[0].status == TOUCH_PRESSED || __touch[0].status == TOUCH_MOVING) /* && current_time - last_touch_time > RELEASE_THRESHOLD*/)
        {
            __touch[0].status = TOUCH_RELEASED;
            __points = 1;
        }
        else
        {
            __touch[0].status = 0;
            __points = 0;
        }
        //__points = 0;
        //return 0;
    }

#else // FTxxxx

    i2c_write_blocking(LCD_TOUCH_I2C, LCD_TOUCH_ADDRESS, (const uint8_t[]){0x02}, 1, true);
    i2c_read_blocking(LCD_TOUCH_I2C, LCD_TOUCH_ADDRESS, &__points, 1, false);

    if (__points == 0xff)
    {
        __points = 0;
        return 0; // some FT chips return garbage before first valid read,
    }

    __points &= 0x0F;

#endif

    if (__points == 0)
    {
        for (int i = 0; i < LCD_TOUCH_POINTS; i++) // TODO: this should be old number of points or max number of points
        {
            __touch[i].status = TOUCH_RELEASED;
        }
        return 0;
    };

#if !defined(LCD_TOUCH_4WIRE) && !defined(LCD_TOUCH_NS2009)
    int len = 6 * __points;
    uint8_t rxdata[len];
    i2c_write_blocking(LCD_TOUCH_I2C, LCD_TOUCH_ADDRESS, (const uint8_t[]){0x03}, 1, true);
    i2c_read_burst_blocking(LCD_TOUCH_I2C, LCD_TOUCH_ADDRESS, rxdata, len); //, false);
#endif

// Disable the warning around the loop
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waggressive-loop-optimizations"

#ifndef LCD_TOUCH_4WIRE
    for (int i = 0, j = 0; i < __points; i++)
    {
#ifdef LCD_TOUCH_NS2009
        __touch[i].xraw = txraw;
        __touch[i].yraw = tyraw;
        __touch[i].xpos = txpos;
        __touch[i].ypos = typos;
#else
        __touch[i].id = rxdata[j + 2] >> 4;
        int8_t oldStatus = __touch[i].status;
        __touch[i].status = rxdata[j] >> 6;
        if ((oldStatus == TOUCH_RELEASED || oldStatus == NO_TOUCH) && __touch[i].status == TOUCH_MOVING) {
            __touch[i].status = TOUCH_PRESSED; // press has to be manually triggered
        }
        __touch[i].xraw = ((rxdata[j] & 0x0F) << 8) | rxdata[j + 1];
        __touch[i].yraw = ((rxdata[j + 2] & 0x0F) << 8) | rxdata[j + 3];
        __touch[i].xpos = __touch[i].xraw;
        __touch[i].ypos = __touch[i].yraw;
        __touch[i].weight = rxdata[j + 4];
        __touch[i].area = rxdata[j + 5];
        j += 6;
#endif
#else
    int i = 0;
    // only need to translate pressed and moving points
    if (__touch[i].status == TOUCH_PRESSED || __touch[i].status == TOUCH_MOVING) {
#endif

        // translate x and y based on default orientation
#ifdef LCD_TOUCH_SWAP_XY
        SWAP(__touch[i].xpos, __touch[i].ypos);
#endif
#ifdef LCD_TOUCH_MIRROR_Y
        __touch[i].ypos = LCD_HEIGHT - __touch[i].ypos - 1;
#endif
#ifdef LCD_TOUCH_MIRROR_X
        __touch[i].xpos = LCD_WIDTH - __touch[i].xpos - 1;
#endif

        // translate x and y position based on current orientation
        switch (gfx.__display.rotation)
        {
        case 1:
            SWAP(__touch[i].xpos, __touch[i].ypos);
            __touch[i].ypos = gfx.__display.height - __touch[i].ypos - 1;
            break;
        case 2:
            __touch[i].xpos = gfx.__display.width - __touch[i].xpos - 1;
            __touch[i].ypos = gfx.__display.height - __touch[i].ypos - 1;
            break;
        case 3:
            SWAP(__touch[i].xpos, __touch[i].ypos);
            __touch[i].xpos = gfx.__display.width - __touch[i].xpos - 1;
            break;
        default:
            break;
        }
    }

// Restore warnings to their previous state
#pragma GCC diagnostic pop

    return __points;
}

int8_t GraphicsTouch4D::GetStatus(uint8_t point)
{
    return __touch[point].status;
}

int16_t GraphicsTouch4D::GetID(uint8_t point)
{
    return __touch[point].id;
}

int16_t GraphicsTouch4D::GetX(uint8_t point)
{
    return __touch[point].xpos;
}

int16_t GraphicsTouch4D::GetY(uint8_t point)
{
    return __touch[point].ypos;
}

int16_t GraphicsTouch4D::__get_raw_x(uint8_t point)
{
    return __touch[point].xraw;
}

int16_t GraphicsTouch4D::__get_raw_y(uint8_t point)
{
    return __touch[point].yraw;
}

int16_t GraphicsTouch4D::GetWeight(uint8_t point)
{
    return __touch[point].weight;
}

int16_t GraphicsTouch4D::GetArea(uint8_t point)
{
    return __touch[point].area;
}

#endif

// Define these instances
Graphics4D &gfx = Graphics4D::GetInstance();
GraphicsMedia4D &img = GraphicsMedia4D::GetInstance();
#ifndef LCD_TOUCH_NONE
GraphicsTouch4D &touch = GraphicsTouch4D::GetInstance();
#endif

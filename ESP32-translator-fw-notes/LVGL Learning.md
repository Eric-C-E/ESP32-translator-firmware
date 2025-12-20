Was looking into LVGL_port for ESP32, but it appears outdated and users have reported the program no longer works for newer vers. esp.idf (v5) whereas I am on V6.0.

Looking into TFT_eSPI, which is a HW driver with some fast-drawing primitives. Draw and it will put it on the screen.

[TFT_eSPI](https://github.com/Bodmer/TFT_eSPI)

LVGL is a full UI framework for animated and nice looking displays.

TFT_eSPI provides a quick way to do the hardware flush + some primitives drawing like strings. LVGL requires objects, has a layout engine abstracted from us.

TFT_eSPI has some nice tricks.

Even though TFT_eSPI is nice, we probably require Arduino IDE or PlatformIO to run it, which may not be worth it.

Therefore we go back to using LVGL with the Espressif Provided official lcd_drivers library which does support GC9A01.

After drawing, it may be not possible to flip the thing in software - so play with the MADCTL register to see which setting has flipped text for AR application!

[ESP_LCD](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/lcd/index.html)

**Display Bringup**

Setting up SPI:

With [ESP_LCD_GC9A01](https://github.com/espressif/esp-bsp/tree/618b05297dae1aa3fab986d5ca7b89b328e1919a/components/lcd/esp_lcd_gc9a01), it is possible to do it quickly, however I must set up the SPI first.
We use SPI2 because SPI0/1 are RAM/FLASH access lanes.
The peripheral pin assignments of ESP32 are set by priority - fastest are through Priority 1 (P1) that is routed directly  through IOMUX or RTC IOMUX not GPIOMUX. 
For SPI2, these pins are: 

| Pin     | Physical | GPIO | Function                        |
| ------- | -------- | ---- | ------------------------------- |
| FSPIHD  |          | 09   | HOLD *not connected*            |
| FSPICS0 |          | 10   | SELECT                          |
| FSPID   |          | 11   | MOSI                            |
| FSPICLK |          | 12   | CLOCK                           |
| FSPIQ   |          | 13   | MISO *not connected*            |
| FSPIWP  |          | 14   | WRITE PROTECT *not connected*   |
| *RST*   | 3V3      | 3V3  | RST pin pulled high to function |
| *DC*    | 8        | 8    | DC data command mux             |
Any other GPIO pin can be made into a chip select. Can do
`cs_gpio_num` per panel.

To bring up the display after this connections:

Initialize SPI (spi enable)
Initialize the GC9A01 using esp_lcd_gc9a01 (send stuff GC9A01 expects to SPI)
Write some useless stuff to it (test program)

Using codex,
It produced some files in main and updated the CMakeLists. 
It created a light scaffolding of stuff like queues that will be used later, as well as app_display.c that is the task we want.

Upon build and test,
Correct sequence is:
Ensure that we have 
`const spi_bus_config_t buscfg = stuff`

`esp_lcd_panel_io_handle_t io_handle = NULL`

`const esp_lcd_panel_io_spi_config_t io_config = stuff`

Attach LCD to SPI Bus

`esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)TEST_LCD_HOST, &io_config, &io_handle));`

Set up the panel

`esp_lcd_panel_handle_t panel_handle = NULL`

`esp_lcd_panel_dev_config_t panel_config = stuff`

Then, the GC9A01 specific stuff:

`esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &panel_handle)`

and the general panel commands:

`ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));

`ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));`

`ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));`

Hereonafter the display is initialized, and the rest of the code is just for drawing stuff to the display. Wasted about 3 hours dealing with the color being off.

Troubleshooting: Colors were off with default settings as part of example code.
Sending RED 0xf800 results in max values for RED and GREEN, 0 for blue.
Effectively RGB565 1111111111100000 
Instead of 1111100000000000
Which is odd.

Patterns like gradients also showed signs of byte-level mismatch in form of interference patterns in X/Y axes.

Changing the delivery direction of the bitstream by setting lsb-first in flags FIXED colors but kept the interference.

Changing RGB to BGR inverts the problem interestingly, has the same effect as inversion of screen color (command).

Thinking of an endianness solution: since changing bitstream delivery did fix colors, maybe it has to be changed but on the byte level. This also corroborates with the fact that RGB to BGR inverts the problem (reading from other side) but doesn't present correct colors. The colors are being sent in the wrong byte order.
The GC9A01 expects 16 bits delivered with bits delivered MSB but the bytes swapped. 

FOR REFERENCE:


Commands for lcd_esp_gc9a01

static esp_err_t panel_gc9a01_del(esp_lcd_panel_t *panel);
static esp_err_t panel_gc9a01_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_gc9a01_init(esp_lcd_panel_t *panel);
static esp_err_t panel_gc9a01_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data);
static esp_err_t panel_gc9a01_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t panel_gc9a01_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t panel_gc9a01_swap_xy(esp_lcd_panel_t *panel, bool swap_axes);
static esp_err_t panel_gc9a01_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap);
static esp_err_t panel_gc9a01_disp_on_off(esp_lcd_panel_t *panel, bool off);

**Integrating lvgl_esp_port and lvgl**

Now that the display works we have confirmed 1. SPI bus functionality
and 2. lcd_esp and lcd_esp_gc9a01 functionality

Included dependencies for lvgl and lvgl_esp_port















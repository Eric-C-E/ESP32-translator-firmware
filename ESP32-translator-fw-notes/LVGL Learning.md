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

Standard Includes: 

```C
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"
#include "esp_lcd_gc9a01.h"

```

**Using LVGL_PORT, the glue layer**

[lvgl_port](https://github.com/espressif/esp-bsp/tree/d15cf39696339b128eceb75665d806d8bd10959a/components/esp_lvgl_port)

lvgl port takes care of 
- initializing lvgl
- adding and removing displays
- adding and removing touch
- buttons
- encoders
- keyboard and mouse bindings

General workflow is to add screen to lvgl

`static lv_disp_t * disp handle`

`esp_lcd_panel_io_handle_t io_handle = NULL;` standard lcd driver init
call `ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t) 1, &ioconfig, &io_h)`

`esp_lcd_panel_handle_t lcd_panel_handle;`

`ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &lcd_panel_handle));`

add the LCD screen

const lvgl_port_display_cfg_t disp_cfg = { set a bunch of stuff}

`disp_handle = lvgl_port_add_disp(&disp_cfg)`

Makes the screen handle.

And more initialization steps following.

When removing displays, use 
`lvgl_port_remove_disp(disp_handle)`

LVGL calls must be protected with lock/unlock commands

```C
lvgl_port_lock(0);
...
lv_obj_t * screen = lv_disp_get_scr_act(disp_handle);
lv_obj_t *obj = lv_label_screate(screen);
....
lvgl_port_unlock();
```

if SRAM insufficient, PSRAM used as a canvas a using a small transbuffer to carry it.
However, LVGL is supposedly capable of chunking using 1/10th the SRAM, which means ESP32 has more than enough.

Making images into transferable format:

```C
lvgl_port_create_c_image("images/logo.png" "images/" "RGB565" "NONE")
lvgl_port_create_c_image("images/image.png" "images/" "RGB565" "NONE")
# Add Generated Images to build
lvgl_port_add_images(${COMPONENT_LIB} "images/")
```
With formats:
L8,I1,I2,I4,I8,A1,A2,A4,A8,ARGB8888,XRGB8888,RGB565,RGB565A8,RGB888,TRUECOLOR,TRUECOLOR_ALPHA,AUTO

**Using LVGL**

LVGL still must be used. LVGL Port is just glue to make LVGL work very well with ESP-IDF.

LVGL_PORT handles the ESP specific functions, like DMA buffers, inputs, and locking.

you can use LVGL **REMEMBER TO LOCK**

```C
lv_obj_t *label = lv_label_create(lv_scr_act());
lv_label_set_text(label, "Hello");
```

LVGL will render into buffer and the port flushes the buffer via esp_lcd.

esp_lcd_panel_draw_bitmap() is now never called by user.

lvgl_port may do some optimization in background.

If you must do something with the screen that is not LVGL, you can lock LVGL and write directly with your own drivers using draw_bitmap().

Mostly taken from example at: [example program by espressif](https://github.com/espressif/esp-bsp/blob/d15cf39696339b128eceb75665d806d8bd10959a/components/esp_lvgl_port/test_apps/lvgl_port/main/test.c)

define lcd size
define spi parameters and other params
define io pins

(optional) define touch settings
(optional) define lcd touch pins

`static char *TAG = "whatever you want"`

make esp_lcd handles

```c
/* LCD IO and panel */
static esp_lcd_panel_io_handle_t lcd_io = NULL;
static esp_lcd_panel_handle_t lcd_panel = NULL;
static esp_lcd_panel_io_handle_t tp_io_handle = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;

/* LVGL display and touch */
static lv_display_t *lvgl_disp = NULL;
static lv_indev_t *lvgl_touch_indev = NULL;
static i2c_master_bus_handle_t i2c_handle = NULL;
```

make a vendor specific init chain if you need. For GC9A01 this is in the esp_lcd_gc9a01.

set buscfg

init bus

set io_config

initialize esp_lcd new panel

install lcd driver 

by initializing the display (esp_lcd_gc9a01)

set panel reset, init, mirror, disp_on_off

(optional) backlight

free spi bus if needed

```c
static esp_err_t app_lcd_deinit(void)
{
    ESP_RETURN_ON_ERROR(esp_lcd_panel_del(lcd_panel), TAG, "LCD panel deinit failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_del(lcd_io), TAG, "LCD IO deinit failed");
    ESP_RETURN_ON_ERROR(spi_bus_free(EXAMPLE_LCD_SPI_NUM), TAG, "SPI BUS free failed");
    ESP_RETURN_ON_ERROR(gpio_reset_pin(EXAMPLE_LCD_GPIO_BL), TAG, "Reset BL pin failed");
    return ESP_OK;
}
```

init lvgl

```c
static esp_err_t app_lvgl_init(void)
{
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd_io,
        .panel_handle = lcd_panel,
        .buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_DRAW_BUFF_HEIGHT * sizeof(uint16_t),
        .double_buffer = EXAMPLE_LCD_DRAW_BUFF_DOUBLE,
        .hres = EXAMPLE_LCD_H_RES,
        .vres = EXAMPLE_LCD_V_RES,
        .monochrome = false,
        /* Rotation values must be same as used in esp_lcd for initial settings of the screen */
        .rotation = {
            .swap_xy = false,
            .mirror_x = true,
            .mirror_y = true,
        },
        .flags = {
            .buff_dma = true,
#if LVGL_VERSION_MAJOR >= 9
            .swap_bytes = true,
#endif
        }
    };
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);

    /* Add touch input (for selected screen) */
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = lvgl_disp,
        .handle = touch_handle,
    };
    lvgl_touch_indev = lvgl_port_add_touch(&touch_cfg);

    return ESP_OK;
}
```

lvgl de init

```c
static esp_err_t app_lvgl_deinit(void)
{
    ESP_RETURN_ON_ERROR(lvgl_port_remove_touch(lvgl_touch_indev), TAG, "LVGL touch removing failed");
    gpio_uninstall_isr_service();

    ESP_RETURN_ON_ERROR(lvgl_port_remove_disp(lvgl_disp), TAG, "LVGL disp removing failed");
    ESP_RETURN_ON_ERROR(lvgl_port_deinit(), TAG, "LVGL deinit failed");

    return ESP_OK;
}
```

make main display code:

```c
static void app_main_display(void)
{
    lv_obj_t *scr = lv_scr_act();

    /* Task lock */
    lvgl_port_lock(0);

    /* Your LVGL objects code here .... */

    /* Label */
    lv_obj_t *label = lv_label_create(scr);
    lv_obj_set_width(label, EXAMPLE_LCD_H_RES);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
#if LVGL_VERSION_MAJOR == 8
    lv_label_set_recolor(label, true);
    lv_label_set_text(label, "#FF0000 "LV_SYMBOL_BELL" Hello world Espressif and LVGL "LV_SYMBOL_BELL"#\n#FF9400 "LV_SYMBOL_WARNING" For simplier initialization, use BSP "LV_SYMBOL_WARNING" #");
#else
    lv_label_set_text(label, LV_SYMBOL_BELL" Hello world Espressif and LVGL "LV_SYMBOL_BELL"\n "LV_SYMBOL_WARNING" For simplier initialization, use BSP "LV_SYMBOL_WARNING);
#endif
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -30);

    /* Button */
    lv_obj_t *btn = lv_btn_create(scr);
    label = lv_label_create(btn);
    lv_label_set_text_static(label, "Rotate screen");
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_add_event_cb(btn, _app_button_cb, LV_EVENT_CLICKED, NULL);

    /* Task unlock */
    lvgl_port_unlock();
}
```

memory leak test

```c
#define TEST_MEMORY_LEAK_THRESHOLD (50)

static void check_leak(size_t start_free, size_t end_free, const char *type)
{
    ssize_t delta = start_free - end_free;
    printf("MALLOC_CAP_%s: Before %u bytes free, After %u bytes free (delta %d)\n", type, start_free, end_free, delta);
    TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE (delta, TEST_MEMORY_LEAK_THRESHOLD, "memory leak");
}
```

(optional) use [[Unity Unit Testing]] to test for memleaks:

```c
TEST_CASE("Main test LVGL port", "[lvgl port]")
{
    size_t start_freemem_8bit = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    size_t start_freemem_32bit = heap_caps_get_free_size(MALLOC_CAP_32BIT);

    ESP_LOGI(TAG, "Initilize LCD.");

    /* LCD HW initialization */
    TEST_ASSERT_EQUAL(app_lcd_init(), ESP_OK);

    /* Touch initialization */
    TEST_ASSERT_EQUAL(app_touch_init(), ESP_OK);

    size_t start_lvgl_freemem_8bit = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    size_t start_lvgl_freemem_32bit = heap_caps_get_free_size(MALLOC_CAP_32BIT);

    ESP_LOGI(TAG, "Initilize LVGL.");

    /* LVGL initialization */
    TEST_ASSERT_EQUAL(app_lvgl_init(), ESP_OK);

    /* Show LVGL objects */
    app_main_display();

    vTaskDelay(5000 / portTICK_PERIOD_MS);

    /* LVGL deinit */
    TEST_ASSERT_EQUAL(app_lvgl_deinit(), ESP_OK);

    /* When using LVGL8, it takes some time to release all memory */
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "LVGL deinitialized.");

    size_t end_lvgl_freemem_8bit = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    size_t end_lvgl_freemem_32bit = heap_caps_get_free_size(MALLOC_CAP_32BIT);
    check_leak(start_lvgl_freemem_8bit, end_lvgl_freemem_8bit, "8BIT LVGL");
    check_leak(start_lvgl_freemem_32bit, end_lvgl_freemem_32bit, "32BIT LVGL");

    /* Touch deinit */
    TEST_ASSERT_EQUAL(app_touch_deinit(), ESP_OK);

    /* LCD deinit */
    TEST_ASSERT_EQUAL(app_lcd_deinit(), ESP_OK);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "LCD deinitilized.");

    size_t end_freemem_8bit = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    size_t end_freemem_32bit = heap_caps_get_free_size(MALLOC_CAP_32BIT);
    check_leak(start_freemem_8bit, end_freemem_8bit, "8BIT");
    check_leak(start_freemem_32bit, end_freemem_32bit, "32BIT");


}

void app_main(void)
{
    printf("TEST ESP LVGL port\n\r");
    unity_run_menu();
}
```

**general program execution format, and future "taskitization"**

The program as it appears now is running in the main freeRTOS task.

The program hands control to Unity's test menu.
Unity can run the test case.
baselines mem
Which inits lcd
(N/A) inits touch
checks mem
inits lvgl
runs app_main_display() which draws objects
de-init lvgl
delay for 1000ms because memory takes time to release
check mem
de-init touch
de-init screen
delay for 1000ms because memory takes time to release
check mem
end

Unity's task runs in the same main task.

Concurrency starts when init LVGL port. 

lvgl init will make an LVGL engine + tick source.

User does not flush pixels rather create lvgl objects that lvgl handles into drawbuffer.

**integration into software architecture of LLL**

We use a .c file and .h file.
Call the functions in main, they get run in freeRTOS maintask

A writer task will touch LVGL objects. the TCP RX task does NOT call these, but it shoud send messages to GUItask.

In side the task, we shall have port lock and unlock.
LVGL engine runs concurrently.

Data Flow is TCP RX TASK -> Parse Sockets, and Posts Messages to a Queue
GUITASK dequeues and updates LVGL objects under lock.

Message types can be: "set status text", "add new line to term", "update prog bar", "show/hide alert"

We don't directly write to LVGL's buffer.

The core philosophy is that network stack is living on core 0.
Application logic shall live on core 1. This ensures TCP doesn't wait on UI.

If we need persistent memory, then we need to make sure the memory is not on a stack of a returning function.

Memory on task stack and heap (static, malloc) is good.

so if the task was

```c
void gui_task(void *pvParameters){
	char text[128];
	static char status[64];
	strcpy(text, "hello");
	
	while(1){
		vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}
```
they stay valid.

Where a freeRTOS task is just a C function with signature 

`void TASK_NAME(void *pvParameters)`

that never returns. freeRTOS will make a stack, task control block, and scheduler entry.

for above guitask, 

```c
void app_main(void)
{
    xTaskCreate(
        gui_task,          // function = task
        "GUI",              // name
        4096,               // stack size (bytes in ESP-IDF, words in vanilla FreeRTOS)
        NULL,               // pvParameters
        5,                  // priority
        NULL                // optional handle
    );
}
```

Where, to pass data into the task,  pass via pointer or struct via pvParameters

```c
typedef struct {
    QueueHandle_t gui_queue;
} gui_ctx_t;

// accessed within like this:
void gui_task(void *arg){  //void pointer
gui_ctx_t *ctx = arg;
}

xTaskCreate(gui_task, "GUI", 4096, &ctx, 5, NULL);
```

where in static context (heap) the variable can be declared OUTSIDE the task as

`static gui_ctx_t gui_ctx`

where in the initializations, the app_main has already:

`gui_ctx.gui_queue = xQueueCreate(10, sizeof(gui_msg_t));`

and while creating the task, has passed in the thing as &gui_ctx (address of gui_ctx)


**My LVGL layout**

The layout is subject to expansion on re-requisition of system hardware for other tasks, but for the current task, it is enough to have text in a terminal-like upwards scrolling fashion.




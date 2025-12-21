Together with [[LVGL Learning]] and [[TCP Learning]]

INMP 441 microphones (L/R) channels both used.

The microphones have their SCK, WS, SD tied together. By suggested layout, the SD line shall have 100kOhm pull down resistor -> which makes the creation of a PCB likely.

Left microphone has L/R pulled to GND. Right microphone is pulled to VCC.

| IMNP441 Pin | ESP-32 Pin              | Remarks                  |
| ----------- | ----------------------- | ------------------------ |
| SCK         | 4                       | P2                       |
| WS          | 5                       | P2                       |
| SD          | 6                       | P2                       |
| L/R         | Individual pull up/down | Different for each - PCB |
| VCC         | 3V3                     | PCB                      |
| GND         | GND                     | PCB                      |
**Behavior of IMNP441**

PCM Modulated, two's complement, MSB first. 

64 clock cycles per stereo data-word. f_sck = 64*f_ws*
WS runs once every stereo cycle since it selects left and right microphone to write to SD.
The microphone is 24 bit data precision.

**Example Code**

The provided audio recorder example is using PDM modulation.

Still gives some clues to operating the microphone:

```c
#define NUM_CHANNELS (1) //for mono
#define SAMPLE_SIZE (CONFIG_EXAMPLE_BIT_SAMPLE *1024)
#define BYTE_RATE (CONFIG_EXAMPLE_SAMPLE_RATE * (CONFIG_EXAMPLE_BIT_SAMPLE / 8)) * NUM CHANNELS
//but this is seemingly unused.
```

Microphone initialization:


```c
void init_microphone(void)
{
#if SOC_I2S_SUPPORTS_PDM2PCM
	ESP_LOGI(TAG, "receive PDM microphone data in PCM format");
#else

ESP_LOGI(TAG, "Receive PDM microphone data in raw PDM format");

#endif // SOC_I2S_SUPPORTS_PDM2PCM

i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);

ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &rx_handle));

  

i2s_pdm_rx_config_t pdm_rx_cfg = {

.clk_cfg = I2S_PDM_RX_CLK_DEFAULT_CONFIG(CONFIG_EXAMPLE_SAMPLE_RATE),

/* The default mono slot is the left slot (whose 'select pin' of the PDM microphone is pulled down) */

#if SOC_I2S_SUPPORTS_PDM2PCM

.slot_cfg = I2S_PDM_RX_SLOT_PCM_FMT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),

#else

.slot_cfg = I2S_PDM_RX_SLOT_RAW_FMT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),

#endif

.gpio_cfg = {

.clk = CONFIG_EXAMPLE_I2S_CLK_GPIO,

.din = CONFIG_EXAMPLE_I2S_DATA_GPIO,

.invert_flags = {

.clk_inv = false,

},

},

};

ESP_ERROR_CHECK(i2s_channel_init_pdm_rx_mode(rx_handle, &pdm_rx_cfg));

ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));
}
```

With a recording function that looks terrible.

Above might be bad example. Better one could be the i2s_std example. In that example:




Espressif Official Documentation Example:



**My PCM Recording**

Need the "standard" libs

`i2s_std.h`
`i2s_pdm.h`
`i2s_tdm.h`

Need to add the driver to REQUIRES esp_driver_i2s.

```c
#include "driver/i2s_std.h"
#include "driver/gpio.h"

i2s_chan_handle_t rx_handle;
//default helper macro
i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);

i2s_new_channel(&chan_cfg, NULL, &rx_handle);

i2s_std_config_t std_cfg = {
	.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000), //was 48000
	.slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_24BIT, I2S_SLOT_MODE_STEREO),
	.gpio_cfg = {
		.mclk = I2S_GPIO_UNUSED,
		.bclk = GPIO_NUM_4,
		.ws = GPIO_NUM_5,
		.dout = I2S_GPIO_UNUSED,
		.din = GPIO_NUM_6,
		.invert_flags = {
			.mclk_inv = false,
			.bclk_inv = false,
			.ws_inv = false,
		},
	},
};
/* initialize channel */
i2s_channel_init_std_mode(rx_handle, &std_cfg);
/* chart RX channel */
i2s_channel_enable(rx_handle);
i2s_channel_read(rx_handle, desc_buf, bytes_to_read, bytes_read, ticks_to_wait);

/* deletion of channel code */
i2s_channel_disable(rx_handle);
/* delete channel to release channel resources if handle un-needed */
i2s_del_channel(rx_handle);

```

We will use I2S0.

We will use the Phillips Format where SD is delayed one clock pulse from WS which is IMNP441 compatible.

A list of helpful functions might be: 

```c
i2s_channel_enable()
//when received data reaches size of the DMA buffer, I2S_IN_SUC_EOF interrupt triggered.
//dma_buffer_size = dma_frame_num * slot_num * slot_bit_width / 8
i2s_channel_read() //waits to receive message queue which contains DMA buffer addr copies from DMA RX to dest buffer.
//i2s_channel_read is a blocking function -> continues waiting until whole buffer sent or whole dest buffer is loaded.
//to use asynchronously
i2s_channel_register_event_callback()
//can access DMA buffer without using the blocking functions.

//initialization
i2s_channel_init_std_mode()

//for updating configs, call
i2s_channel_disable() //then
i2s_channel_reconfig_std_slot()
i2s_channel_reconfig_std_clock()
i2s_channel_reconfig_std_gpio()

//all API threadsafe, do NOT call them from ISR (mutex)
```

Settings Structures:

Structs that contain settings: 

i2s_std_config_t contains all configs.
in order of : clock, slot and gpio configs.

i2s_std_clk_config_t
```c
	.sample_rate_hz //16000 Hz
	.clk_src //choose source
	.ext_clk_freq_hz //unimportant if no external clock
	.mclk_multiple //important for 24 bit this is set to 384
	.bclk_div //unimportant for master role
```
i2s_std_slot_config_t
```c
	.data_bid_width //48
	.slot_bit_width //24
	.slot_mode
	.slot_mask
	.ws_width
	.ws_pol
	.bit_shift
	.left_align
	.big_endian
	.bit_order_lsb
```
i2s_std_gpio_config_t
```c
	.mclk
	.bclk
	.ws
	.dout
	.din
	.mclk_inv
	.bclk_inv
	.ws_inv
	.invert_flags
```


**Architecture**

Calculated data rate for 16kHz sampling stereo @ 24 bits is 96kB/s. Wow that is big.
@ f_clk of 1.024 MHz

It will be easier to get one microphone working first (left channel only) while streaming in "stereo", read mono, then add another one so the software already handles getting each channel. This is such that front channel is recording TGT and rear channel is recording you.

This program won't do any conversion of audio on its own, but on Jetson Side, 
Numpy array will be mono, 16kHz.

We should sample this at 16kHz to start.
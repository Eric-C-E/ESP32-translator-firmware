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
i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER); //data bit width being 24 means dma_frame_num needs to be multiple of 3.

i2s_new_channel(&chan_cfg, NULL, &rx_handle);

i2s_std_config_t std_cfg = {
	.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000), //was 48000
	.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_24BIT, I2S_SLOT_MODE_STEREO),
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
	.mclk_multiple //set to I2S_MCLK_MULTIPLE_384
	.bclk_div //unimportant for master role
```
i2s_std_slot_config_t
```c
	.data_bit_width //I2S_DATA_BIT_WIDTH_24BIT
	.slot_bit_width //I2S_SLOT_BIT_WIDTH_32BIT
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
for reference: 

```c
for I2S Driver Config
#define I2S_STD_CLK_DEFAULT_CONFIG(rate) { \
    .sample_rate_hz = rate, \
    .clk_src = I2S_CLK_SRC_DEFAULT, \
    .mclk_multiple = I2S_MCLK_MULTIPLE_256, \
    .bclk_div = 8, \
}
#define I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(bits_per_sample, mono_or_stereo) { \
    .data_bit_width = bits_per_sample, \
    .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO, \
    .slot_mode = mono_or_stereo, \
    .slot_mask = I2S_STD_SLOT_BOTH, \
    .ws_width = bits_per_sample, \
    .ws_pol = false, \
    .bit_shift = true, \
    .left_align = true, \
    .big_endian = false, \
    .bit_order_lsb = false \
}

for I2S CHANNEL config

#define I2S_CHANNEL_DEFAULT_CONFIG(i2s_num, i2s_role) { \
    .id = i2s_num, \
    .role = i2s_role, \
    .dma_desc_num = 6, \
    .dma_frame_num = 240, \must be multiple of three for 24 bit data!
    .auto_clear_after_cb = false, \
    .auto_clear_before_cb = false, \
    .allow_pd = false, \
    .intr_priority = 0, \
}
```
240 actually does work, we can set DMA buffers to 8 though.

To write into a 3072 byte queue buffer (to TCP stack)

Functions:

i2s_channel_read(i2s_chan_handle_t, void * dest, size_t size, size_t * bytes_read, uin32_t timeout_ms)

dest is the receiving data buffer. Buffer size must respect DMA buffer size (multiple of 2 and 3)
size is max data buffer length.
bytes_read can be NULL
timeout_ms is max block time.

returns ESP_OK if successful. (reads until either it's out of stuff, or rx buffer is full)

TRACK r_bytes so we can enqueue into our TCP ringbuffer the right amount of data and avoid any duplicate data.



**Architecture**

Microphone currently records 16kHz stereo format, 24 bit per "slot".

This program won't do any conversion of audio on its own, but on Jetson Side, 

Numpy array fed to Whisper will be mono, 16kHz.
Which means we will need to make the stereo mono, which numpy probably has a function for.

Ringbuffer audio_rb is currently 32768 bytes (32kB) which means it is capable of handling a few hundred ms of TCP hangup. The TCP sockets running on core 1 shouldn't ever reach that level of delay. Microphone writes to DMA, which is checked by the audio task, and buffered into audio_rb at the exact amount of bytes received from the DMA buffer (most likely allocated heap amount 3072 bytes, due to 500 ms wait, but it should hit that in ~30ms).

Writes into a ring-buffer for the TCP transmit socket!
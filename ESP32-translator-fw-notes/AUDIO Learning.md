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

```



**Architecture**

It will be easier to get one microphone working first (left channel only) while streaming in "stereo", read mono, then add another one so the software already handles getting each channel. This is such that front channel is recording TGT and rear channel is recording you.

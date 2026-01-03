ESP-LCD-NV3041A is nonexistent at time of this project. 

It is now responsibility to create it and upload it for public use.

Resolution of screen - max 480x272 px

Another mode: 320x240 px.

My resolution: 480x128.

The driver has 720 sources and 544 gates. GRAM of 293760 bytes

**Hardware-Side Setup**
IM2:0
For Standard SPI Interface Enable

| IM2 | IM1 | IM0 |
| --- | --- | --- |
| 0   | 0   | 1   |
SPI4W
High for 4 wire

GRB (RESET)
Global Reset - Internally pulled high. Active low.

DISP
User should connect to LOW. Internal pull HIGH.


**Register Level Setup**

Standard 4 wire SPI
Requirements: set IM[2:0], set SPI4W = 1, Register 3AH (COLMOD) = 0x01 (which is default)

Most of the registers here are very common and expected for a "generic" display IC, that does not cause loss of generality.

Set: 
Set the scan window of the timing engine. 
SCAN_HRES default 0b111111111
SCAN_VRES default 0b111111111


GATE_Setting 0x51 -> usually not touched

GATE_SCAN 0x50 -> try default, usually not touched

For sure:
CASET 0x2A Col Set -> if mv = 0, 0-479. If mv = 1, 0-271  | want mv = 0
RASET 0x2B Row Set -> if mv = 0, 0-271, if mv = 0, 0-479  | want mv = 0

SCAN_HRES default 0b111111111
SCAN_VRES default 0b111111111

NORMAL 
COLMOD (explicitly)
MADCTL  -> 0b0000000 (which is default) -> validates CASET and RASET
SLPOUT
DISPON

Optionals: 

IDMOFF 0x38

________________________________________________________________________

Note: for setting registers, send the register address as the command, then data as its contents.
NV3052 has page changes between register settings - this is because it has banked registers. NV3041 does not have banked registers, it can be considered to have a single page and absolute register addresses.
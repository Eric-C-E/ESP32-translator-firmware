The Hardware Design of the ESP-LLL is not too complex on an individual level, but uses professional-grade hardware modules in a very coupled way, raising complexity of integration quite a bit.

Main Blocks: 

**ESP32-S3 Devkit-C**

ESP32-S3 Devkit with AMS1117 LDO removed because it does not provide a low enough dropout voltage to reliably run the system on 1S lithium ion pack.
Also, LDO proved incapable of providing the current required to run all system modules cleanly.

**2x IMNP441 Microphones** 

2x IMNP441 microphones wired as stereo.

| IMNP441 Pin | ESP-32 Pin              | Remarks                  |
| ----------- | ----------------------- | ------------------------ |
| SCK         | 4                       | P2                       |
| WS          | 5                       | P2                       |
| SD          | 6                       | P2                       |
| L/R         | Individual pull up/down | Different for each - PCB |
| VCC         | 3V3                     | PCB                      |
| GND         | GND                     | PCB                      |
**2x GPIO Buttons**

Currently GPIO47, GPIO48.

**Augmented Reality Screen driven by GC9A01**

| Pin     | Pin on Screen | ESP-Side GPIO | Function                        |
| ------- | ------------- | ------------- | ------------------------------- |
| FSPIHD  | --            | 09            | HOLD *not connected*            |
| FSPICS0 | "CS"          | 10            | SELECT                          |
| FSPID   | "SD"          | 11            | MOSI                            |
| FSPICLK | "CLK"         | 12            | CLOCK                           |
| FSPIQ   | --            | 13            | MISO *not connected*            |
| FSPIWP  | --            | 14            | WRITE PROTECT *not connected*   |
| *RST*   | "RS"          | 3V3           | RST pin pulled high to function |
| *DC*    | "DC"          | 8             | DC data command mux             |


**External Display Screen driven by NV3041A**

| Pin  | Pin on Screen (40-FPC) | ESP-Side GPIO |
| ---- | ---------------------- | ------------- |
| MOSI | 13                     | 11            |
| CLK  | 10                     | 12            |
| CS   | 9                      | 17            |
| DC   | 11                     | 8             |
Contains a FPC-40P 0.5mm breakout board that allows wiring of the small pins on the FPC connector. This means it was necessary to bend the FPC with radius of around 3mm in multiple places. If necessary, grounded shielding can be added between the FPC and the breakout to avoid EMI.

**Power Supply Circuit**

The power circuit for the Goggles is complex: 

It contains a battery pack nominal 3.7V li-ion, a battery charger, a 2A max 3V3 LDO, and an SPDT switch that allows user to switch the LDO input from (mutually exclusively) 5V in from USB cable (for programming/long runtime), and Battery/Charger output - to prevent battery feeding back into its own charging circuit. This is a safe way to keep a common ground when reprogramming and to ensure safe operation of the circuit. 
Note: if the SWITCH is set to use battery, the device will be on. Because of common ground between the programmer cable and the battery power circuit, commands should still work, but the 5Vin will only be driving the battery charger, and not powering the circuit as a whole.
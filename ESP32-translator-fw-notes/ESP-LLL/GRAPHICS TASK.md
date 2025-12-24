Priority: Medium

GPIO Task:
- LVGL graphics rendering
- Use [https://github.com/lvgl/lv_port_esp32]
- Use [https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/lcd/index.html]
- uses FIVE layers of abstraction to drive two LCD displays.
- Bottom Level: spi_master
- Layer Up: esp_lcd
- Layer Up: esp_lcd_specific
- Layer Up: esp_lvgl_port
- Top Layer: lvgl
- Display 1: User Facing for AR: scrolling text, ready, recording, wifi RSSI
- Display 2: TGT Facing normal screen: scrolling text

Connects to:
[[TEXT DOWNLINK TASK]]
with Queue

[[GPIO TASK]]
with FSM

[[WIFI TASK]] 
with RSSI


Code complete by unit test - need to be integrated. Outstanding work: 
Display 1: create AR scrolling text
Display 1: create READY and RECORDING graphics

Display 2: configure HAL, esp_lcd_xxxx
Display 2: configure pipeline
Display 2: create scrolling text - TGT 
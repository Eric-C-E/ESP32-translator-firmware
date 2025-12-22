Priority: Medium

GPIO Task:
- LVGL graphics rendering
- UI Framebuffer Update
- Use [https://github.com/lvgl/lv_port_esp32]
- Use [https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/lcd/index.html]
- esp_lcd and lvgl_port. 
- Run two LVGL displays
- SPI DMA needs buffers in SRAM. PSRAM not good. 
- Keep LVGL draw buffers small and in internal RAM.
- esp_lcd might bounce buffer
- Partial Buffer SRAM

Connects to:
[[TEXT DOWNLINK TASK]]
with Queue

[[GPIO TASK]]
with FSM


Code complete by unit test - need to be integrated. Outstanding work: 
Display 1: create AR scrolling text
Display 1: create READY and RECORDING graphics

Display 2: configure HAL, esp_lcd_xxxx
Display 2: configure pipeline
Display 2: create scrolling text - TGT 
# ESP32-translator-firmware

ESP-IDF (FreeRTOS) firmware for the ESP32 side of the Live Translator headset:
- Connects to the Jetson (acting as Wi‑Fi AP)
- Streams audio/control out to Jetson
- Receives text updates back and routes them to “display 1/2” UI

## Quick start (ESP-IDF)

1. Install ESP-IDF (v5.x recommended) and export the environment (per Espressif docs).
2. From `ESP32-translator-firmware/`:
   - `idf.py set-target esp32` (or your exact chip, e.g. `esp32s3`)
   - `idf.py menuconfig` → `Translator firmware` (set SSID/pass + Jetson host/port + GPIOs)
   - `idf.py build flash monitor`

ESP-IDF install docs:
- https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/

## Architecture (scaffold)

- `io_task`: debounced GPIO buttons → button events
- `net_task`: Wi‑Fi STA connect + TCP link to Jetson; parses framed messages; sends button/audio frames
- `audio_task`: placeholder audio source (synthetic PCM for now) → audio chunks
- `ui_task`: placeholder UI that logs per-display text updates (LVGL integration later)

## Repo layout

- `CMakeLists.txt`: ESP-IDF project entrypoint
- `main/`: firmware source (tasks + protocol framing + Kconfig options)
- `docs/PROTOCOL.md`: on-wire framing + message payloads

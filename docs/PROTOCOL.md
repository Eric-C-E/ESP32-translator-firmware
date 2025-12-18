# Protocol (ESP32 ↔ Jetson)

This repo uses a simple framed transport over TCP.

## Endianness

All integer fields are big-endian (network byte order).

## Frame header

Fixed-size header (`translator_frame_header_t` in `main/translator_proto.h`):

- `magic` (`u32`): `0x54524E53` (`'T''R''N''S'`)
- `version` (`u16`): `1`
- `msg_type` (`u16`): see below
- `payload_len` (`u32`): bytes following the header
- `seq` (`u32`): sequence number (used for audio chunks)
- `timestamp_us` (`u64`): sender timestamp (microseconds)

## Message types / payloads

### `TRANSLATOR_MSG_TEXT_UPDATE` (`1`)

Payload:
- `display_id` (`u8`): `1` or `2`
- `text` (`u8[]`): UTF-8 bytes (not NUL-terminated on-wire)

### `TRANSLATOR_MSG_AUDIO_CHUNK` (`2`)

Payload (current scaffold):
- `source_id` (`u8`): `1` or `2` (which “active language” / button)
- `pcm` (`u8[]`): raw PCM bytes (currently expected to be `CONFIG_TRANSLATOR_AUDIO_CHUNK_BYTES`)

### `TRANSLATOR_MSG_BUTTON_EVENT` (`3`)

Payload:
- `button_id` (`u8`): `1` or `2`
- `action` (`u8`): `1` = down, `2` = up

### `TRANSLATOR_MSG_HEARTBEAT` (`4`)

Payload: empty

### `TRANSLATOR_MSG_STOP_AUDIO` (`5`)

Payload:
- `button_id` (`u8`): which source stream is stopping

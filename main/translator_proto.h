#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// On-wire protocol:
// - Big-endian (network byte order) for all integer fields
// - Header is fixed-size, payload is `payload_len` bytes
// - Text payload: [u8 display_id][utf8 bytes...]
// - Audio payload (current scaffold): [u8 source_id][pcm bytes...]

#define TRANSLATOR_MAGIC 0x54524E53u // 'T''R''N''S'
#define TRANSLATOR_PROTO_VERSION 1u

typedef enum {
    TRANSLATOR_MSG_TEXT_UPDATE = 1,
    TRANSLATOR_MSG_AUDIO_CHUNK = 2,
    TRANSLATOR_MSG_BUTTON_EVENT = 3,
    TRANSLATOR_MSG_HEARTBEAT = 4,
    TRANSLATOR_MSG_STOP_AUDIO = 5,
} translator_msg_type_t;

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint16_t version;
    uint16_t msg_type;
    uint32_t payload_len;
    uint32_t seq;
    uint64_t timestamp_us;
} translator_frame_header_t;

uint64_t translator_htonll(uint64_t v);
uint64_t translator_ntohll(uint64_t v);

bool translator_send_frame(int sock,
                           translator_msg_type_t msg_type,
                           uint32_t seq,
                           uint64_t timestamp_us,
                           const void *payload,
                           uint32_t payload_len);

bool translator_recv_frame_header(int sock, translator_frame_header_t *out_hdr);
bool translator_recv_exact(int sock, void *buf, size_t len);

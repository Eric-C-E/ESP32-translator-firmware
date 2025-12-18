#pragma once

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "sdkconfig.h"

typedef enum {
    TRANSLATOR_BTN_1 = 1,
    TRANSLATOR_BTN_2 = 2,
} translator_button_id_t;

typedef enum {
    TRANSLATOR_BTN_DOWN = 1,
    TRANSLATOR_BTN_UP = 2,
} translator_button_action_t;

typedef struct {
    translator_button_id_t button_id;
    translator_button_action_t action;
    uint32_t timestamp_ms;
} translator_button_event_t;

typedef struct {
    uint8_t display_id; // 1 or 2
    char text[CONFIG_TRANSLATOR_TEXT_MAX_BYTES];
} translator_text_update_t;

typedef struct {
    uint8_t source_id; // 1 or 2
    uint32_t seq;
    uint64_t timestamp_us;
    uint16_t pcm_len;
    uint8_t pcm[CONFIG_TRANSLATOR_AUDIO_CHUNK_BYTES];
} translator_audio_chunk_t;

typedef struct {
    QueueHandle_t button_events;
    QueueHandle_t text_updates;
    QueueHandle_t audio_chunks;
    EventGroupHandle_t state_bits;
} translator_runtime_t;

enum {
    TRANSLATOR_STATE_BTN1_ACTIVE = (1u << 0),
    TRANSLATOR_STATE_BTN2_ACTIVE = (1u << 1),
};

translator_runtime_t *translator_runtime_get(void);
void translator_runtime_init(void);

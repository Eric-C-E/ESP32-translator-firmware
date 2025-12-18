#include "translator_tasks.h"

#include <string.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "translator_runtime.h"

static const char *TAG = "audio_task";

static void audio_task(void *arg) {
    (void)arg;
    translator_runtime_t *rt = translator_runtime_get();

    uint32_t seq = 0;
    ESP_LOGI(TAG, "Audio task running (synth=%s)", CONFIG_TRANSLATOR_ENABLE_SYNTH_AUDIO ? "on" : "off");

    while (true) {
        EventBits_t bits = xEventGroupGetBits(rt->state_bits);
        bool any_active = (bits & (TRANSLATOR_STATE_BTN1_ACTIVE | TRANSLATOR_STATE_BTN2_ACTIVE)) != 0;
        if (!any_active) {
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }

        if (!CONFIG_TRANSLATOR_ENABLE_SYNTH_AUDIO) {
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }

        const uint8_t source_id = (bits & TRANSLATOR_STATE_BTN1_ACTIVE) ? 1 : 2;

        translator_audio_chunk_t chunk = {
            .source_id = source_id,
            .seq = seq++,
            .timestamp_us = (uint64_t)esp_timer_get_time(),
            .pcm_len = CONFIG_TRANSLATOR_AUDIO_CHUNK_BYTES,
        };
        memset(chunk.pcm, 0, sizeof(chunk.pcm));

        (void)xQueueSend(rt->audio_chunks, &chunk, pdMS_TO_TICKS(5));
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void translator_start_audio_task(void) {
    xTaskCreate(audio_task, "audio_task", 4096, NULL, 9, NULL);
}

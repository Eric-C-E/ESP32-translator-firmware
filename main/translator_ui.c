#include "translator_tasks.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "translator_runtime.h"

static const char *TAG = "ui_task";

static void ui_task(void *arg) {
    (void)arg;
    translator_runtime_t *rt = translator_runtime_get();

    translator_text_update_t update;
    while (true) {
        if (xQueueReceive(rt->text_updates, &update, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "Display %u: %s", (unsigned)update.display_id, update.text);
        }
    }
}

void translator_start_ui_task(void) {
    xTaskCreate(ui_task, "ui_task", 4096, NULL, 8, NULL);
}

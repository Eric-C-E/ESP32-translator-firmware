#include "translator_tasks.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "translator_runtime.h"

static const char *TAG = "app_main";

void app_main(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    } else {
        ESP_ERROR_CHECK(err);
    }

    translator_runtime_init();

    ESP_LOGI(TAG, "Starting tasks");
    translator_start_io_task();
    translator_start_audio_task();
    translator_start_ui_task();
    translator_start_net_task();
}

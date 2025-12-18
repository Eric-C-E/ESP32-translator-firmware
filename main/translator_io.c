#include "translator_tasks.h"

#include <stdbool.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "translator_runtime.h"

static const char *TAG = "io_task";

typedef struct {
    gpio_num_t gpio;
    translator_button_id_t button_id;
    bool stable_level;
    bool last_read_level;
    int64_t last_change_us;
} button_state_t;

static bool gpio_read_level(gpio_num_t gpio_num) {
    return gpio_get_level(gpio_num) != 0;
}

static void io_task(void *arg) {
    (void)arg;

    translator_runtime_t *rt = translator_runtime_get();

    const int64_t debounce_us = 50 * 1000;

    button_state_t b1 = {
        .gpio = (gpio_num_t)CONFIG_TRANSLATOR_BUTTON1_GPIO,
        .button_id = TRANSLATOR_BTN_1,
        .stable_level = true,
        .last_read_level = true,
        .last_change_us = esp_timer_get_time(),
    };
    button_state_t b2 = {
        .gpio = (gpio_num_t)CONFIG_TRANSLATOR_BUTTON2_GPIO,
        .button_id = TRANSLATOR_BTN_2,
        .stable_level = true,
        .last_read_level = true,
        .last_change_us = esp_timer_get_time(),
    };

    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << b1.gpio) | (1ULL << b2.gpio),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));

    b1.stable_level = gpio_read_level(b1.gpio);
    b1.last_read_level = b1.stable_level;
    b2.stable_level = gpio_read_level(b2.gpio);
    b2.last_read_level = b2.stable_level;

    ESP_LOGI(TAG, "Buttons on GPIO %d/%d (active-low via pull-up assumed)", b1.gpio, b2.gpio);

    while (true) {
        button_state_t *buttons[] = {&b1, &b2};
        for (size_t i = 0; i < 2; i++) {
            button_state_t *b = buttons[i];
            bool level = gpio_read_level(b->gpio);
            int64_t now_us = esp_timer_get_time();

            if (level != b->last_read_level) {
                b->last_read_level = level;
                b->last_change_us = now_us;
            }

            if (level != b->stable_level && (now_us - b->last_change_us) >= debounce_us) {
                b->stable_level = level;

                const bool pressed = (b->stable_level == false); // active-low
                translator_button_event_t evt = {
                    .button_id = b->button_id,
                    .action = pressed ? TRANSLATOR_BTN_DOWN : TRANSLATOR_BTN_UP,
                    .timestamp_ms = (uint32_t)(now_us / 1000),
                };
                (void)xQueueSend(rt->button_events, &evt, 0);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void translator_start_io_task(void) {
    xTaskCreate(io_task, "io_task", 4096, NULL, 10, NULL);
}

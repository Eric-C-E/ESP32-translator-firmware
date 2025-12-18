#include "translator_tasks.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "nvs_flash.h"

#include "translator_proto.h"
#include "translator_runtime.h"

static const char *TAG = "net_task";

static EventGroupHandle_t s_wifi_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data) {
    (void)arg;
    (void)event_data;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        esp_wifi_connect();
        return;
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        return;
    }
}

static void wifi_init_sta(void) {
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta =
            {
                .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            },
    };
    strncpy((char *)wifi_config.sta.ssid, CONFIG_TRANSLATOR_WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password,
            CONFIG_TRANSLATOR_WIFI_PASSWORD,
            sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static int tcp_connect_jetson(void) {
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res = NULL;
    char port_str[8];
    snprintf(port_str, sizeof(port_str), "%d", CONFIG_TRANSLATOR_JETSON_TCP_PORT);

    int err = getaddrinfo(CONFIG_TRANSLATOR_JETSON_HOST, port_str, &hints, &res);
    if (err != 0 || res == NULL) {
        return -1;
    }

    int sock = socket(res->ai_family, res->ai_socktype, 0);
    if (sock < 0) {
        freeaddrinfo(res);
        return -1;
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) != 0) {
        close(sock);
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res);
    return sock;
}

static void publish_text_update(uint8_t display_id, const uint8_t *text, uint32_t text_len) {
    translator_runtime_t *rt = translator_runtime_get();

    translator_text_update_t update = {.display_id = display_id};
    size_t cap = sizeof(update.text) - 1;
    size_t n = (text_len < cap) ? text_len : cap;
    memcpy(update.text, text, n);
    update.text[n] = '\0';

    (void)xQueueSend(rt->text_updates, &update, pdMS_TO_TICKS(10));
}

static void handle_incoming_frames(int sock) {
    translator_frame_header_t hdr;
    uint8_t rx_buf[CONFIG_TRANSLATOR_TCP_RX_BUF];

    while (translator_recv_frame_header(sock, &hdr)) {
        uint16_t msg_type = ntohs(hdr.msg_type);
        uint32_t payload_len = ntohl(hdr.payload_len);

        if (payload_len > sizeof(rx_buf)) {
            ESP_LOGW(TAG, "Dropping frame type=%u payload_len=%u (rx buf=%u)",
                     (unsigned)msg_type,
                     (unsigned)payload_len,
                     (unsigned)sizeof(rx_buf));
            uint32_t remaining = payload_len;
            uint8_t tmp[64];
            while (remaining > 0) {
                size_t chunk = remaining > sizeof(tmp) ? sizeof(tmp) : remaining;
                if (!translator_recv_exact(sock, tmp, chunk)) {
                    return;
                }
                remaining -= (uint32_t)chunk;
            }
            continue;
        }

        if (!translator_recv_exact(sock, rx_buf, payload_len)) {
            return;
        }

        if (CONFIG_TRANSLATOR_LOG_NET_FRAMES) {
            ESP_LOGI(TAG, "RX frame type=%u payload_len=%u", (unsigned)msg_type, (unsigned)payload_len);
        }

        if (msg_type == TRANSLATOR_MSG_TEXT_UPDATE && payload_len >= 1) {
            uint8_t display_id = rx_buf[0];
            publish_text_update(display_id, &rx_buf[1], payload_len - 1);
        }
    }
}

static void process_button_events(int sock, translator_runtime_t *rt) {
    translator_button_event_t evt;
    while (xQueueReceive(rt->button_events, &evt, 0) == pdTRUE) {
        const EventBits_t bit = (evt.button_id == TRANSLATOR_BTN_1) ? TRANSLATOR_STATE_BTN1_ACTIVE
                                                                    : TRANSLATOR_STATE_BTN2_ACTIVE;
        if (evt.action == TRANSLATOR_BTN_DOWN) {
            xEventGroupSetBits(rt->state_bits, bit);
        } else {
            xEventGroupClearBits(rt->state_bits, bit);
        }

        uint8_t payload[2] = {(uint8_t)evt.button_id, (uint8_t)evt.action};
        (void)translator_send_frame(sock,
                                    TRANSLATOR_MSG_BUTTON_EVENT,
                                    0,
                                    (uint64_t)esp_timer_get_time(),
                                    payload,
                                    (uint32_t)sizeof(payload));

        if (evt.action == TRANSLATOR_BTN_UP) {
            uint8_t stop_payload[1] = {(uint8_t)evt.button_id};
            (void)translator_send_frame(sock,
                                        TRANSLATOR_MSG_STOP_AUDIO,
                                        0,
                                        (uint64_t)esp_timer_get_time(),
                                        stop_payload,
                                        (uint32_t)sizeof(stop_payload));
        }
    }
}

static void process_audio_chunks(int sock, translator_runtime_t *rt) {
    translator_audio_chunk_t chunk;
    while (xQueueReceive(rt->audio_chunks, &chunk, 0) == pdTRUE) {
        uint8_t payload[1 + CONFIG_TRANSLATOR_AUDIO_CHUNK_BYTES];
        payload[0] = chunk.source_id;
        memcpy(&payload[1], chunk.pcm, chunk.pcm_len);

        (void)translator_send_frame(sock,
                                    TRANSLATOR_MSG_AUDIO_CHUNK,
                                    chunk.seq,
                                    chunk.timestamp_us,
                                    payload,
                                    1u + (uint32_t)chunk.pcm_len);
    }
}

static void net_task(void *arg) {
    (void)arg;
    translator_runtime_t *rt = translator_runtime_get();

    wifi_init_sta();
    ESP_LOGI(TAG, "Connecting to WiFi SSID '%s'", CONFIG_TRANSLATOR_WIFI_SSID);

    while (true) {
        xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
        ESP_LOGI(TAG, "WiFi connected, connecting TCP to %s:%d",
                 CONFIG_TRANSLATOR_JETSON_HOST,
                 CONFIG_TRANSLATOR_JETSON_TCP_PORT);

        int sock = tcp_connect_jetson();
        if (sock < 0) {
            ESP_LOGW(TAG, "TCP connect failed; retrying");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        ESP_LOGI(TAG, "TCP connected");

        struct timeval tv = {.tv_sec = 0, .tv_usec = 200 * 1000};
        (void)setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        while (true) {
            process_button_events(sock, rt);
            process_audio_chunks(sock, rt);

            handle_incoming_frames(sock);

            if (errno != EWOULDBLOCK && errno != EAGAIN && errno != 0) {
                ESP_LOGW(TAG, "Socket error/closed (errno=%d); reconnecting", errno);
                break;
            }

            vTaskDelay(pdMS_TO_TICKS(10));
        }

        close(sock);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void translator_start_net_task(void) {
    xTaskCreate(net_task, "net_task", 8192, NULL, 11, NULL);
}

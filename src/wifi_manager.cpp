#include "wifi_manager.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include <stdio.h>

static const char *TAG = "WIFI_MANAGER";
static bool wifi_connected = false;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        printf("[WIFI] Station started, connecting...\n");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_connected = false;
        printf("[WIFI] Disconnected, attempting to reconnect...\n");
        ESP_LOGI(TAG, "WiFi disconnected, attempting to reconnect...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        printf("[WIFI] Got IP address: " IPSTR "\n", IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        wifi_connected = true;
    }
}

esp_err_t wifi_init_sta(const char* ssid, const char* password) {
    printf("[WIFI] Initializing NVS...\n");
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        printf("[WIFI] Erasing NVS...\n");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    printf("[WIFI] NVS initialized\n");

    printf("[WIFI] Initializing network interface...\n");
    // Initialize network interface
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    printf("[WIFI] Network interface initialized\n");

    printf("[WIFI] Configuring WiFi...\n");
    // Configure WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    printf("[WIFI] WiFi initialized\n");

    // Register event handlers
    printf("[WIFI] Registering event handlers...\n");
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    printf("[WIFI] Event handlers registered\n");

    // Configure WiFi station
    printf("[WIFI] Setting WiFi credentials...\n");
    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    printf("[WIFI] WiFi started, connecting to: %s\n", ssid);

    ESP_LOGI(TAG, "WiFi initialization finished. Connecting to %s...", ssid);
    
    return ESP_OK;
}

bool wifi_is_connected(void) {
    return wifi_connected;
}


#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "wifi_manager.h"
#include "azure_iot_mqtt.h"
#include "azure_config.h"
#include <cJSON.h>
#include <stdio.h>

static const char *TAG = "MAIN";
#define LED_PIN GPIO_NUM_17

extern "C" void app_main(void) {
    // Small delay to ensure UART is ready
    vTaskDelay(100 / portTICK_PERIOD_MS);
    
    printf("\n\n========================================\n");
    printf("[MAIN] Starting ESP32 Azure IoT Hub application...\n");
    printf("========================================\n");
    fflush(stdout);  // Force flush to ensure output is sent immediately
    ESP_LOGI(TAG, "Starting ESP32 Azure IoT Hub application...");
    
    // Configure LED pin as output
    printf("[MAIN] Configuring LED pin 17...\n");
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);
    printf("[MAIN] LED pin configured\n");
    
    // Initialize WiFi
    printf("[MAIN] Initializing WiFi...\n");
    printf("[MAIN] SSID: %s\n", WIFI_SSID);
    ESP_LOGI(TAG, "Initializing WiFi...");
    esp_err_t ret = wifi_init_sta(WIFI_SSID, WIFI_PASSWORD);
    if (ret != ESP_OK) {
        printf("[MAIN] ERROR: WiFi initialization failed (error: %d)\n", ret);
        ESP_LOGE(TAG, "WiFi initialization failed");
        return;
    }
    printf("[MAIN] WiFi initialization OK\n");
    
    // Wait for WiFi connection
    printf("[MAIN] Waiting for WiFi connection...\n");
    ESP_LOGI(TAG, "Waiting for WiFi connection...");
    int wifi_timeout = 30; // 30 seconds timeout
    while (!wifi_is_connected() && wifi_timeout > 0) {
        printf("[MAIN] WiFi connecting... (%d seconds remaining)\n", wifi_timeout);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        wifi_timeout--;
    }
    
    if (!wifi_is_connected()) {
        printf("[MAIN] ERROR: WiFi connection timeout!\n");
        ESP_LOGE(TAG, "WiFi connection timeout");
        return;
    }
    
    printf("[MAIN] WiFi CONNECTED! IP obtained.\n");
    ESP_LOGI(TAG, "WiFi connected! Initializing Azure IoT Hub connection...");
    
    // Initialize Azure IoT Hub MQTT
    printf("[MAIN] Initializing Azure IoT Hub MQTT connection...\n");
    printf("[MAIN] IoT Hub: %s\n", IOT_HUB_HOSTNAME);
    printf("[MAIN] Device ID: %s\n", DEVICE_ID);
    ret = azure_iot_mqtt_init();
    if (ret != ESP_OK) {
        printf("[MAIN] ERROR: Azure IoT Hub initialization failed (error: %d)\n", ret);
        ESP_LOGE(TAG, "Azure IoT Hub initialization failed");
        return;
    }
    printf("[MAIN] Azure IoT Hub MQTT client initialized\n");
    
    // Wait for MQTT connection
    printf("[MAIN] Waiting for Azure IoT Hub connection...\n");
    ESP_LOGI(TAG, "Waiting for Azure IoT Hub connection...");
    int mqtt_timeout = 30; // 30 seconds timeout
    while (!azure_iot_is_connected() && mqtt_timeout > 0) {
        printf("[MAIN] MQTT connecting... (%d seconds remaining)\n", mqtt_timeout);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        mqtt_timeout--;
    }
    
    if (!azure_iot_is_connected()) {
        printf("[MAIN] ERROR: Azure IoT Hub connection timeout!\n");
        ESP_LOGE(TAG, "Azure IoT Hub connection timeout");
        return;
    }
    
    printf("[MAIN] Azure IoT Hub CONNECTED!\n");
    printf("[MAIN] Starting main loop...\n");
    ESP_LOGI(TAG, "Connected to Azure IoT Hub! Starting main loop...");
    
    // Main loop: wait and process messages
    // LED control is now handled via MQTT messages (ON/OFF commands)
    while (1) {
        // LED blinking disabled - LED is now controlled via MQTT messages
        // gpio_set_level(LED_PIN, 1);
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
        // gpio_set_level(LED_PIN, 0);
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        // Telemetry sending disabled for now
        // counter++;
        // if (counter >= 5 && azure_iot_is_connected()) {
        //     printf("[MAIN] Sending telemetry (counter: %d)...\n", counter);
        //     // Create JSON telemetry message
        //     cJSON *json = cJSON_CreateObject();
        //     cJSON_AddNumberToObject(json, "led_state", gpio_get_level(LED_PIN));
        //     cJSON_AddNumberToObject(json, "counter", counter);
        //     cJSON_AddStringToObject(json, "device_id", DEVICE_ID);
        //     
        //     char *json_string = cJSON_Print(json);
        //     if (json_string != NULL) {
        //         printf("[MAIN] Telemetry JSON: %s\n", json_string);
        //         azure_iot_send_telemetry(json_string);
        //         free(json_string);
        //     }
        //     cJSON_Delete(json);
        //     
        //     counter = 0;
        // }
        
        // Just wait and let MQTT event handler process messages
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
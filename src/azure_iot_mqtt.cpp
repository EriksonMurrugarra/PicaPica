#include "azure_iot_mqtt.h"
#include "azure_config.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_event.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "AZURE_IOT_MQTT";
static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_connected = false;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                                int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            printf("[MQTT] *** CONNECTED to Azure IoT Hub ***\n");
            ESP_LOGI(TAG, "MQTT Connected to Azure IoT Hub");
            mqtt_connected = true;
            
            // Subscribe to device-to-cloud messages (if needed)
            // For now, we'll just publish telemetry
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            printf("[MQTT] Disconnected from Azure IoT Hub\n");
            ESP_LOGI(TAG, "MQTT Disconnected");
            mqtt_connected = false;
            break;

        case MQTT_EVENT_PUBLISHED:
            printf("[MQTT] Message published successfully, msg_id=%d\n", event->msg_id);
            ESP_LOGI(TAG, "MQTT Published, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_ERROR:
            printf("[MQTT] ERROR occurred!\n");
            ESP_LOGE(TAG, "MQTT Error");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                printf("[MQTT] Transport error: %s\n", strerror(event->error_handle->esp_transport_sock_errno));
                ESP_LOGE(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;

        default:
            printf("[MQTT] Other event id: %d\n", event->event_id);
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
}

esp_err_t azure_iot_mqtt_init(void) {
    // Build MQTT URI: mqtts://{hostname}:8883
    char mqtt_uri[256];
    snprintf(mqtt_uri, sizeof(mqtt_uri), "mqtts://%s:8883", IOT_HUB_HOSTNAME);
    
    // Build client ID: {device_id}
    char client_id[128];
    snprintf(client_id, sizeof(client_id), "%s", DEVICE_ID);
    
    // Build username: {hostname}/{device_id}/?api-version=2021-04-12
    char username[256];
    snprintf(username, sizeof(username), "%s/%s/?api-version=2021-04-12", 
             IOT_HUB_HOSTNAME, DEVICE_ID);
    
    printf("[MQTT] Connecting to Azure IoT Hub:\n");
    printf("[MQTT]   URI: %s\n", mqtt_uri);
    printf("[MQTT]   Client ID: %s\n", client_id);
    printf("[MQTT]   Username: %s\n", username);
    ESP_LOGI(TAG, "Connecting to Azure IoT Hub:");
    ESP_LOGI(TAG, "  URI: %s", mqtt_uri);
    ESP_LOGI(TAG, "  Client ID: %s", client_id);
    ESP_LOGI(TAG, "  Username: %s", username);

    printf("[MQTT] Configuring MQTT client...\n");
    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.broker.address.uri = mqtt_uri;
    mqtt_cfg.credentials.client_id = client_id;
    mqtt_cfg.credentials.username = username;
    mqtt_cfg.credentials.authentication.password = SAS_TOKEN;
    mqtt_cfg.session.keepalive = 60;
    
    printf("[MQTT] Initializing MQTT client...\n");
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL) {
        printf("[MQTT] ERROR: Failed to initialize MQTT client\n");
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return ESP_FAIL;
    }
    printf("[MQTT] MQTT client initialized\n");
    
    // Register event handler - register for all MQTT events
    printf("[MQTT] Registering event handlers...\n");
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_CONNECTED, mqtt_event_handler, NULL);
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_DISCONNECTED, mqtt_event_handler, NULL);
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_PUBLISHED, mqtt_event_handler, NULL);
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ERROR, mqtt_event_handler, NULL);
    printf("[MQTT] Event handlers registered\n");
    
    printf("[MQTT] Starting MQTT client...\n");
    esp_err_t err = esp_mqtt_client_start(mqtt_client);
    
    if (err != ESP_OK) {
        printf("[MQTT] ERROR: Failed to start MQTT client: %s\n", esp_err_to_name(err));
        ESP_LOGE(TAG, "Failed to start MQTT client: %s", esp_err_to_name(err));
        return err;
    }
    printf("[MQTT] MQTT client started, waiting for connection...\n");
    
    return ESP_OK;
}

esp_err_t azure_iot_send_telemetry(const char* data) {
    if (!mqtt_connected || mqtt_client == NULL) {
        printf("[MQTT] WARNING: MQTT not connected, cannot send telemetry\n");
        ESP_LOGW(TAG, "MQTT not connected, cannot send telemetry");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Build topic: devices/{device_id}/messages/events/
    char topic[256];
    snprintf(topic, sizeof(topic), "devices/%s/messages/events/", DEVICE_ID);
    
    printf("[MQTT] Publishing to topic: %s\n", topic);
    printf("[MQTT] Data: %s\n", data);
    int msg_id = esp_mqtt_client_publish(mqtt_client, topic, data, 0, 1, 0);
    
    if (msg_id < 0) {
        printf("[MQTT] ERROR: Failed to publish message (msg_id=%d)\n", msg_id);
        ESP_LOGE(TAG, "Failed to publish message");
        return ESP_FAIL;
    }
    
    printf("[MQTT] Telemetry sent successfully (msg_id=%d)\n", msg_id);
    ESP_LOGI(TAG, "Telemetry sent: %s", data);
    return ESP_OK;
}

bool azure_iot_is_connected(void) {
    return mqtt_connected;
}


#include "azure_iot_mqtt.h"
#include "azure_config.h"
#include "led_channels.h"
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
            {
                printf("[MQTT] *** CONNECTED to Azure IoT Hub ***\n");
                ESP_LOGI(TAG, "MQTT Connected to Azure IoT Hub");
                mqtt_connected = true;
                
                // Subscribe to cloud-to-device messages
                // Topic format: devices/{deviceId}/messages/devicebound/#
                char subscribe_topic[256];
                snprintf(subscribe_topic, sizeof(subscribe_topic), "devices/%s/messages/devicebound/#", DEVICE_ID);
                int msg_id = esp_mqtt_client_subscribe(mqtt_client, subscribe_topic, 1);
                if (msg_id >= 0) {
                    printf("[MQTT] Subscribed to cloud-to-device messages: %s\n", subscribe_topic);
                    ESP_LOGI(TAG, "Subscribed to: %s", subscribe_topic);
                } else {
                    printf("[MQTT] ERROR: Failed to subscribe to cloud-to-device messages\n");
                    ESP_LOGE(TAG, "Failed to subscribe");
                }
            }
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

        case MQTT_EVENT_DATA:
            {
                printf("\n========================================\n");
                printf("RECIBIDO MENSAJE\n");
                printf("========================================\n");
                printf("[MQTT] Topic: %.*s\n", event->topic_len, event->topic);
                printf("[MQTT] Data length: %d bytes\n", event->data_len);
                
                // Process the message content
                if (event->data_len > 0) {
                    // Create a null-terminated string from the message
                    char message[256];
                    int len = (event->data_len < sizeof(message) - 1) ? event->data_len : sizeof(message) - 1;
                    memcpy(message, event->data, len);
                    message[len] = '\0';
                    
                    printf("[MQTT] Message content: %s\n", message);
                    
                    // Remove trailing whitespace/newlines
                    while (len > 0 && (message[len-1] == '\n' || message[len-1] == '\r' || message[len-1] == ' ')) {
                        message[len-1] = '\0';
                        len--;
                    }
                    
                    // Control LED channels based on message
                    // Format: "CHANNEL:STATE" (e.g., "RGB:ON", "WHITE:OFF", "VERDE:ON", "FAR_RED:OFF")
                    // Or simple "ON"/"OFF" for backward compatibility (controls RGB channel)
                    char* colon = strchr(message, ':');
                    if (colon != NULL) {
                        // Format: CHANNEL:STATE
                        *colon = '\0';
                        char* channel = message;
                        char* state = colon + 1;
                        
                        gpio_num_t pin;
                        const char* channel_name = NULL;
                        
                        // Map channel name to GPIO pin
                        if (strcmp(channel, "RGB") == 0) {
                            pin = CHANNEL_RGB_PIN;
                            channel_name = "RGB";
                        } else if (strcmp(channel, "WHITE") == 0) {
                            pin = CHANNEL_WHITE_PIN;
                            channel_name = "WHITE";
                        } else if (strcmp(channel, "VERDE") == 0) {
                            pin = CHANNEL_VERDE_PIN;
                            channel_name = "VERDE";
                        } else if (strcmp(channel, "FAR_RED") == 0) {
                            pin = CHANNEL_FAR_RED_PIN;
                            channel_name = "FAR_RED";
                        } else {
                            printf("[MQTT] Canal desconocido: %s (Use: RGB, WHITE, VERDE, FAR_RED)\n", channel);
                            break;
                        }
                        
                        // Set channel state
                        if (strcmp(state, "ON") == 0) {
                            led_channel_set(pin, 1);
                            printf("[MQTT] Canal %s (Pin %d) encendido\n", channel_name, pin);
                            ESP_LOGI(TAG, "Channel %s turned ON", channel_name);
                        } else if (strcmp(state, "OFF") == 0) {
                            led_channel_set(pin, 0);
                            printf("[MQTT] Canal %s (Pin %d) apagado\n", channel_name, pin);
                            ESP_LOGI(TAG, "Channel %s turned OFF", channel_name);
                        } else {
                            printf("[MQTT] Estado desconocido: %s (Use: ON u OFF)\n", state);
                        }
                    } else {
                        // Backward compatibility: simple ON/OFF controls RGB channel
                        if (strcmp(message, "ON") == 0) {
                            printf("[MQTT] Comando ON recibido - Encendiendo canal RGB\n");
                            led_channel_set(CHANNEL_RGB_PIN, 1);
                            ESP_LOGI(TAG, "RGB channel turned ON (backward compatibility)");
                        } else if (strcmp(message, "OFF") == 0) {
                            printf("[MQTT] Comando OFF recibido - Apagando canal RGB\n");
                            led_channel_set(CHANNEL_RGB_PIN, 0);
                            ESP_LOGI(TAG, "RGB channel turned OFF (backward compatibility)");
                        } else {
                            printf("[MQTT] Mensaje desconocido: %s\n", message);
                            printf("[MQTT] Formato esperado: CHANNEL:STATE (ej: RGB:ON, WHITE:OFF)\n");
                            printf("[MQTT] O simplemente ON/OFF para controlar RGB\n");
                        }
                    }
                }
                
                // If message is chunked, print chunk info
                if (event->total_data_len != event->data_len) {
                    printf("[MQTT] Chunked message: %d/%d bytes\n", 
                           event->data_len, event->total_data_len);
                }
                
                printf("========================================\n\n");
                
                ESP_LOGI(TAG, "Received message on topic: %.*s", event->topic_len, event->topic);
                ESP_LOGI(TAG, "Data: %.*s", event->data_len, event->data);
            }
            break;

        case MQTT_EVENT_ERROR:
            printf("[MQTT] ERROR occurred!\n");
            ESP_LOGE(TAG, "MQTT Error");
            if (event->error_handle) {
                printf("[MQTT] Error type: %d\n", event->error_handle->error_type);
                if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                    printf("[MQTT] Transport error: %s (errno: %d)\n", 
                           strerror(event->error_handle->esp_transport_sock_errno),
                           event->error_handle->esp_transport_sock_errno);
                    ESP_LOGE(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
                } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                    printf("[MQTT] Connection refused - check credentials (SAS token may be expired)\n");
                }
            }
            break;

        case MQTT_EVENT_SUBSCRIBED:
            printf("[MQTT] Successfully subscribed to topic, msg_id=%d\n", event->msg_id);
            ESP_LOGI(TAG, "Subscribed, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            printf("[MQTT] Successfully unsubscribed from topic, msg_id=%d\n", event->msg_id);
            ESP_LOGI(TAG, "Unsubscribed, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_BEFORE_CONNECT:
            printf("[MQTT] Before connect event\n");
            ESP_LOGI(TAG, "Before connect");
            break;

        case MQTT_EVENT_DELETED:
            printf("[MQTT] MQTT client deleted\n");
            ESP_LOGI(TAG, "MQTT client deleted");
            break;

        case MQTT_USER_EVENT:
            printf("[MQTT] User event received\n");
            ESP_LOGI(TAG, "User event");
            break;

        // case MQTT_EVENT_ANY:
        //     // This case is included for completeness but shouldn't be reached
        //     // since we register handlers for specific events
        //     printf("[MQTT] MQTT_EVENT_ANY received (event_id: %d)\n", event->event_id);
        //     break;

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
    
    // Configure SSL/TLS - skip certificate verification (for development)
    // With CONFIG_ESP_TLS_INSECURE=y and CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY=y
    // we need to explicitly configure the verification settings
    mqtt_cfg.broker.verification.skip_cert_common_name_check = true;
    // Don't use global CA store when in insecure mode
    mqtt_cfg.broker.verification.use_global_ca_store = false;
    mqtt_cfg.broker.verification.certificate = NULL;
    
    printf("[MQTT] SSL/TLS configured (insecure mode - certificate verification disabled)\n");
    printf("[MQTT] Using CONFIG_ESP_TLS_INSECURE and CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY\n");
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
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_DATA, mqtt_event_handler, NULL);
    esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ERROR, mqtt_event_handler, NULL);
    printf("[MQTT] Event handlers registered (including cloud-to-device message handler)\n");
    
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


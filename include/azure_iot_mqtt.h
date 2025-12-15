#ifndef AZURE_IOT_MQTT_H
#define AZURE_IOT_MQTT_H

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t azure_iot_mqtt_init(void);
esp_err_t azure_iot_send_telemetry(const char* data);
bool azure_iot_is_connected(void);

#ifdef __cplusplus
}
#endif

#endif // AZURE_IOT_MQTT_H


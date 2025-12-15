#ifndef AZURE_CONFIG_H
#define AZURE_CONFIG_H

// WiFi Configuration
#define WIFI_SSID "Odido-474830"
#define WIFI_PASSWORD "D5ETT9WPRW7MPJAK"

// Azure IoT Hub Configuration
// Replace with your IoT Hub hostname (e.g., "myhub.azure-devices.net")
// Example: If your IoT Hub is named "mi-hub-test", use "mi-hub-test.azure-devices.net"
#define IOT_HUB_HOSTNAME "iot-hub-demo-arduino.azure-devices.net"

// Device ID (must match the Device ID you created in Azure IoT Hub)
// Example: "ESP32_001"
// IMPORTANT: Authentication Type in Azure must be "Symmetric Key" (not X.509)
#define DEVICE_ID "ESP32_001"

// Shared Access Signature (SAS) Token
// Generate this using Azure CLI: az iot hub generate-sas-token --hub-name <HUB_NAME> --device-id <DEVICE_ID> --duration 3600
// Format: SharedAccessSignature sr={hub-name}.azure-devices.net%2Fdevices%2F{device-id}&sig={signature}&se={expiry}
// Example: "SharedAccessSignature sr=mi-hub-test.azure-devices.net%2Fdevices%2FESP32_001&sig=ABC123...&se=1735689600"
#define SAS_TOKEN "SharedAccessSignature sr=iot-hub-demo-arduino.azure-devices.net%2Fdevices%2FESP32_001&sig=qHZ0oNQIrSZppuwhk%2FnKwrmef73aL5mvjf3v3fitc5o%3D&se=1765837011"

// Alternative: If you prefer to use connection string, you can parse it
// Connection string format: HostName={hub-name}.azure-devices.net;DeviceId={device-id};SharedAccessKey={key}
// #define CONNECTION_STRING "HostName=..."

#endif // AZURE_CONFIG_H


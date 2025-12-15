# Configuración de Azure IoT Hub para ESP32

Este proyecto conecta un ESP32 a Azure IoT Hub usando MQTT.

## Pasos de Configuración

### 1. Crear un dispositivo en Azure IoT Hub

Si aún no has creado un dispositivo en tu IoT Hub, puedes hacerlo usando Azure CLI:

```bash
# Crear un nuevo dispositivo
az iot hub device-identity create \
  --hub-name <NOMBRE_DE_TU_IOT_HUB> \
  --device-id <ID_DEL_DISPOSITIVO>

# Obtener la cadena de conexión
az iot hub device-identity connection-string show \
  --hub-name <NOMBRE_DE_TU_IOT_HUB> \
  --device-id <ID_DEL_DISPOSITIVO> \
  --output table
```

### 2. Generar un Token SAS (Shared Access Signature)

Azure IoT Hub requiere autenticación usando un token SAS. Puedes generar uno usando Azure CLI:

```bash
# Generar token SAS (válido por 1 hora)
az iot hub generate-sas-token \
  --hub-name iot-hub-demo-arduino \
  --device-id ESP32_001 \
  --duration 3600
```

O puedes usar el Azure IoT Explorer o el portal de Azure para generar el token.

**Nota:** El token SAS tiene una fecha de expiración. Para producción, considera usar certificados X.509 o implementar la renovación automática del token.

### 3. Configurar las credenciales

Edita el archivo `include/azure_config.h` y actualiza las siguientes constantes:

```cpp
// WiFi Configuration
#define WIFI_SSID "Odido-474830"
#define WIFI_PASSWORD "D5ETT9WPRW7MPJAK"

// Azure IoT Hub Configuration
#define IOT_HUB_HOSTNAME "tu-iot-hub.azure-devices.net"
#define DEVICE_ID "tu-device-id"
#define SAS_TOKEN "SharedAccessSignature sr=..."
```

**Importante:** 
- Reemplaza `tu-iot-hub` con el nombre real de tu IoT Hub
- El formato del token SAS debe ser: `SharedAccessSignature sr=...&sig=...&se=...`
- El token SAS expira, así que necesitarás actualizarlo periódicamente

### 4. Compilar y subir

```bash
pio run --target upload
```

### 5. Verificar la conexión

Puedes usar Azure IoT Explorer o el portal de Azure para verificar que tu dispositivo está conectado y recibiendo telemetría.

## Estructura del Proyecto

- `include/azure_config.h` - Configuración de credenciales (WiFi y Azure)
- `include/wifi_manager.h` - Gestor de conexión WiFi
- `include/azure_iot_mqtt.h` - Cliente MQTT para Azure IoT Hub
- `src/wifi_manager.cpp` - Implementación del gestor WiFi
- `src/azure_iot_mqtt.cpp` - Implementación del cliente MQTT
- `src/main.cpp` - Código principal que integra todo

## Funcionalidad

El código:
1. Se conecta a WiFi usando las credenciales configuradas
2. Se conecta a Azure IoT Hub usando MQTT
3. Parpadea el LED en el pin 17
4. Envía telemetría JSON cada 5 segundos con el estado del LED y un contador

## Solución de Problemas

### Error de conexión WiFi
- Verifica que el SSID y la contraseña sean correctos
- Asegúrate de que el ESP32 esté dentro del rango de la red WiFi

### Error de conexión a Azure IoT Hub
- Verifica que el hostname del IoT Hub sea correcto
- Verifica que el Device ID coincida con el creado en Azure
- Verifica que el token SAS no haya expirado
- Revisa los logs del ESP32 para más detalles

### Token SAS expirado
El token SAS tiene una fecha de expiración. Si expira, necesitarás generar uno nuevo y actualizar `azure_config.h`.

## Próximos Pasos

- Implementar renovación automática del token SAS
- Agregar recepción de mensajes desde la nube (cloud-to-device)
- Implementar certificados X.509 para autenticación más segura
- Agregar más sensores y telemetría


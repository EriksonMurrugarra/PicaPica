# Guía: Crear Dispositivo en Azure IoT Hub

## Paso 1: Crear el Dispositivo en Azure Portal

1. Ve a tu **Azure IoT Hub** en el portal de Azure
2. En el menú lateral, busca **"Devices"** o **"Dispositivos"** (en español)
3. Haz clic en **"+ Add device"** o **"+ Agregar dispositivo"**

## Paso 2: Configurar el Dispositivo

Completa el formulario con estos valores:

- **Device ID**: `ESP32_001` (o el que prefieras)
- **Authentication Type**: Selecciona **"Symmetric Key"** (Clave Simétrica)
  - ✅ Esta es la opción correcta para usar SAS Token
  - ❌ NO uses "X.509 Self-Signed" ni "X.509 CA Signed" (esos son para certificados)
- **Auto-generate keys**: Deja marcado ✅ (recomendado)
- **Connect device to IoT Hub**: Puedes dejarlo marcado ✅

Haz clic en **"Save"** o **"Guardar"**

## Paso 3: Obtener las Credenciales

Después de crear el dispositivo, verás dos opciones importantes:

### Opción A: Connection String (Más fácil para empezar)

1. Haz clic en el dispositivo `ESP32_001` que acabas de crear
2. En la página de detalles, verás **"Primary Connection String"**
3. Copia esa cadena completa

Formato: `HostName=tu-hub.azure-devices.net;DeviceId=ESP32_001;SharedAccessKey=xxxxx`

### Opción B: Generar Token SAS (Recomendado para producción)

Necesitas generar un token SAS. Tienes dos formas:

#### Método 1: Usando Azure CLI (Recomendado)

```bash
# Generar token SAS válido por 1 hora (3600 segundos)
az iot hub generate-sas-token \
  --hub-name <NOMBRE_DE_TU_IOT_HUB> \
  --device-id ESP32_001 \
  --duration 3600
```

Esto te dará un token como:
```
SharedAccessSignature sr=tu-hub.azure-devices.net%2Fdevices%2FESP32_001&sig=xxxxx&se=1234567890
```

#### Método 2: Usando Azure IoT Explorer

1. Descarga e instala [Azure IoT Explorer](https://github.com/Azure/azure-iot-explorer/releases)
2. Conecta tu IoT Hub
3. Selecciona tu dispositivo `ESP32_001`
4. Ve a la pestaña "SAS Token"
5. Genera un token con la duración deseada

## Paso 4: Configurar el Código

Actualiza el archivo `include/azure_config.h`:

```cpp
// WiFi Configuration
#define WIFI_SSID "TU_SSID_WIFI"
#define WIFI_PASSWORD "TU_PASSWORD_WIFI"

// Azure IoT Hub Configuration
#define IOT_HUB_HOSTNAME "tu-iot-hub.azure-devices.net"  // Sin el "https://"
#define DEVICE_ID "ESP32_001"

// Token SAS (el que generaste en el paso 3)
#define SAS_TOKEN "SharedAccessSignature sr=tu-hub.azure-devices.net%2Fdevices%2FESP32_001&sig=xxxxx&se=1234567890"
```

## Ejemplo Completo

Si tu IoT Hub se llama `mi-hub-test` y tu dispositivo es `ESP32_001`:

```cpp
#define IOT_HUB_HOSTNAME "mi-hub-test.azure-devices.net"
#define DEVICE_ID "ESP32_001"
#define SAS_TOKEN "SharedAccessSignature sr=mi-hub-test.azure-devices.net%2Fdevices%2FESP32_001&sig=ABC123...&se=1735689600"
```

## Importante sobre el Token SAS

⚠️ **El token SAS expira**. Tienes dos opciones:

1. **Para desarrollo/pruebas**: Genera tokens con duración larga (ej: 1 año = 31536000 segundos)
2. **Para producción**: Implementa renovación automática del token (más complejo pero más seguro)

Para generar un token válido por 1 año:
```bash
az iot hub generate-sas-token \
  --hub-name <NOMBRE_IOT_HUB> \
  --device-id ESP32_001 \
  --duration 31536000
```

## Verificar que Funciona

1. Compila y sube el código al ESP32
2. Abre el monitor serial: `pio device monitor`
3. Deberías ver mensajes como:
   - "WiFi connected!"
   - "MQTT Connected to Azure IoT Hub"
   - "Telemetry sent: {...}"
4. En Azure Portal, ve a tu dispositivo y verás la telemetría llegando

## Resumen Rápido

✅ **Authentication Type**: Symmetric Key  
✅ **Device ID**: ESP32_001 (o el que prefieras)  
✅ **Token**: Genera un SAS Token usando Azure CLI o Azure IoT Explorer  
✅ **Configuración**: Actualiza `azure_config.h` con tus credenciales


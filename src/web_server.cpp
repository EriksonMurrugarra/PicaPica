#include "web_server.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_netif_ip_addr.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "WEB_SERVER";
#define LED_PIN GPIO_NUM_17

static httpd_handle_t server_handle = NULL;

// Forward declarations
static esp_err_t root_handler(httpd_req_t *req);
static esp_err_t led_control_handler(httpd_req_t *req);

// HTML page with buttons to control LED
static const char html_page[] = 
"<!DOCTYPE html>"
"<html>"
"<head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<title>ESP32 LED Control</title>"
"<style>"
"body {"
"font-family: Arial, sans-serif;"
"display: flex;"
"justify-content: center;"
"align-items: center;"
"min-height: 100vh;"
"margin: 0;"
"background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);"
"}"
".container {"
"background: white;"
"padding: 40px;"
"border-radius: 20px;"
"box-shadow: 0 10px 40px rgba(0,0,0,0.2);"
"text-align: center;"
"}"
"h1 {"
"color: #333;"
"margin-bottom: 30px;"
"}"
".button-group {"
"display: flex;"
"gap: 20px;"
"justify-content: center;"
"}"
"button {"
"padding: 15px 40px;"
"font-size: 18px;"
"border: none;"
"border-radius: 10px;"
"cursor: pointer;"
"transition: all 0.3s;"
"font-weight: bold;"
"}"
".btn-on {"
"background: #4CAF50;"
"color: white;"
"}"
".btn-on:hover {"
"background: #45a049;"
"transform: scale(1.05);"
"}"
".btn-off {"
"background: #f44336;"
"color: white;"
"}"
".btn-off:hover {"
"background: #da190b;"
"transform: scale(1.05);"
"}"
".status {"
"margin-top: 30px;"
"padding: 15px;"
"border-radius: 10px;"
"font-weight: bold;"
"}"
".status-success {"
"background: #d4edda;"
"color: #155724;"
"}"
"</style>"
"</head>"
"<body>"
"<div class=\"container\">"
"<h1>ESP32 LED Control</h1>"
"<div class=\"button-group\">"
"<button class=\"btn-on\" onclick=\"controlLED('ON')\">ENCENDER</button>"
"<button class=\"btn-off\" onclick=\"controlLED('OFF')\">APAGAR</button>"
"</div>"
"<div id=\"status\"></div>"
"</div>"
"<script>"
"function controlLED(command) {"
"var statusDiv = document.getElementById('status');"
"statusDiv.innerHTML = '<div class=\"status\">Enviando comando...</div>';"
"fetch('/led?state=' + command, {"
"method: 'GET'"
"})"
".then(function(response) { return response.text(); })"
".then(function(data) {"
"statusDiv.innerHTML = '<div class=\"status status-success\">' + data + '</div>';"
"})"
".catch(function(error) {"
"statusDiv.innerHTML = '<div class=\"status\" style=\"background:#f8d7da;color:#721c24;\">Error: ' + error + '</div>';"
"});"
"}"
"</script>"
"</body>"
"</html>";

// Handler for root path - serves HTML page
static esp_err_t root_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_page, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for LED control endpoint
static esp_err_t led_control_handler(httpd_req_t *req) {
    char query[64];
    char response[128];
    
    // Get query string
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        char state[10];
        if (httpd_query_key_value(query, "state", state, sizeof(state)) == ESP_OK) {
            if (strcmp(state, "ON") == 0) {
                gpio_set_level(LED_PIN, 1);
                snprintf(response, sizeof(response), "LED encendido correctamente");
                printf("[WEB] LED turned ON via web interface\n");
                ESP_LOGI(TAG, "LED turned ON via web");
            } else if (strcmp(state, "OFF") == 0) {
                gpio_set_level(LED_PIN, 0);
                snprintf(response, sizeof(response), "LED apagado correctamente");
                printf("[WEB] LED turned OFF via web interface\n");
                ESP_LOGI(TAG, "LED turned OFF via web");
            } else {
                snprintf(response, sizeof(response), "Comando invalido. Use ON u OFF");
            }
        } else {
            snprintf(response, sizeof(response), "Parametro 'state' no encontrado");
        }
    } else {
        snprintf(response, sizeof(response), "Error al procesar la peticion");
    }
    
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t web_server_start(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 10;
    config.max_open_sockets = 7;
    
    printf("[WEB] Starting HTTP server on port %d...\n", config.server_port);
    ESP_LOGI(TAG, "Starting HTTP server on port %d", config.server_port);
    
    if (httpd_start(&server_handle, &config) == ESP_OK) {
        // Register URI handlers
        httpd_uri_t root = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server_handle, &root);
        
        httpd_uri_t led_control = {
            .uri       = "/led",
            .method    = HTTP_GET,
            .handler   = led_control_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server_handle, &led_control);
        
        printf("[WEB] HTTP server started successfully\n");
        printf("[WEB] Open http://<ESP32_IP>/ in your browser\n");
        ESP_LOGI(TAG, "HTTP server started successfully");
        return ESP_OK;
    }
    
    printf("[WEB] ERROR: Failed to start HTTP server\n");
    ESP_LOGE(TAG, "Failed to start HTTP server");
    return ESP_FAIL;
}

void web_server_stop(void) {
    if (server_handle) {
        httpd_stop(server_handle);
        server_handle = NULL;
        printf("[WEB] HTTP server stopped\n");
        ESP_LOGI(TAG, "HTTP server stopped");
    }
}


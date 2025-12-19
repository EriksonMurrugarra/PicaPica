#include "led_channels.h"
#include "esp_log.h"
#include <stdio.h>

static const char *TAG = "LED_CHANNELS";

void led_channels_init(void) {
    printf("[LED] Initializing LED channels...\n");
    
    // Configure all channel pins as outputs
    gpio_set_direction(CHANNEL_RGB_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(CHANNEL_WHITE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(CHANNEL_VERDE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(CHANNEL_FAR_RED_PIN, GPIO_MODE_OUTPUT);
    
    // Set all channels to OFF initially
    gpio_set_level(CHANNEL_RGB_PIN, 0);
    gpio_set_level(CHANNEL_WHITE_PIN, 0);
    gpio_set_level(CHANNEL_VERDE_PIN, 0);
    gpio_set_level(CHANNEL_FAR_RED_PIN, 0);
    
    printf("[LED] Channel RGB (Pin 17) configured\n");
    printf("[LED] Channel WHITE (Pin 16) configured\n");
    printf("[LED] Channel VERDE (Pin 4) configured\n");
    printf("[LED] Channel FAR_RED (Pin 12) configured\n");
    printf("[LED] All channels initialized and set to OFF\n");
    
    ESP_LOGI(TAG, "All LED channels initialized");
}

void led_channel_set(gpio_num_t pin, int level) {
    gpio_set_level(pin, level);
}

int led_channel_get(gpio_num_t pin) {
    return gpio_get_level(pin);
}


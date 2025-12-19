#ifndef LED_CHANNELS_H
#define LED_CHANNELS_H

#include "driver/gpio.h"

// LED Channel definitions
#define CHANNEL_RGB_PIN     GPIO_NUM_17
#define CHANNEL_WHITE_PIN   GPIO_NUM_16
#define CHANNEL_VERDE_PIN   GPIO_NUM_4
#define CHANNEL_FAR_RED_PIN GPIO_NUM_19

// Channel names
#define CHANNEL_RGB_NAME     "RGB"
#define CHANNEL_WHITE_NAME   "WHITE"
#define CHANNEL_VERDE_NAME   "VERDE"
#define CHANNEL_FAR_RED_NAME "FAR_RED"

#ifdef __cplusplus
extern "C" {
#endif

void led_channels_init(void);
void led_channel_set(gpio_num_t pin, int level);
int led_channel_get(gpio_num_t pin);

#ifdef __cplusplus
}
#endif

#endif // LED_CHANNELS_H


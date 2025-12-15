#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_PIN GPIO_NUM_17

extern "C" void app_main(void) {
  // Configure pin 17 as output
  gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
  
  // Loop to blink LED every second
  while (1) {
    // Turn on the LED
    gpio_set_level(LED_PIN, 1);
    vTaskDelay(5000 / portTICK_PERIOD_MS);  // Wait 1 second
    
    // Turn off the LED
    gpio_set_level(LED_PIN, 0);
    vTaskDelay(5000 / portTICK_PERIOD_MS);  // Wait 1 second
  }
}
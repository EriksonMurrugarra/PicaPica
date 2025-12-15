#include <Arduino.h>

#define LED_PIN 17

void setup() {
  // Configure pin 17 as output
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Turn on the LED
  digitalWrite(LED_PIN, HIGH);
  delay(1000);  // Wait 1 second
  
  // Turn off the LED
  digitalWrite(LED_PIN, LOW);
  delay(1000);  // Wait 1 second
}
#include "config.h"
#if CONFIG_LED_ILLUMINATOR_ENABLED
#ifndef MY_RGB_LED_H
#define MY_RGB_LED_H
#include <Arduino.h>
#include <Adafruit_NeoPixel.h> // Include this header for WS2812 functions

//#define CONFIG_LED_ILLUMINATOR_ENABLED 1
//#define CONFIG_LED_MAX_INTENSITY 255

#define LED_COUNT 1  // Only one WS2812 LED

class My_RGB_Led {
   public:
    int led_duty;
    Adafruit_NeoPixel led;
    My_RGB_Led(int);
    int init();
    void enable_led(bool);
    void setColor(uint8_t r, uint8_t g, uint8_t b);
};

#endif  // MY_RGB_LED_H
#endif

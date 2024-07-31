#include "config.h"
#if CONFIG_LED_ILLUMINATOR_ENABLED
#ifndef LED_H
#define LED_H
#include <Arduino.h>
#include "driver/ledc.h" // Include this header for LED PWM functions

class Led
{
public:
  int pin;
  int led_duty;
  int ledChannel; // Choose a PWM channel (0-15)
  int freq;       // Frequency in Hz
  int resolution; // PWM resolution (8-16 bits)

  Led(int pin);
  int init();

  void enable_led(bool);
  /*void ledcWrite(int, int);
   */
};

#endif // LED_H

#endif
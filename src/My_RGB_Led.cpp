#include "My_RGB_Led.h"


My_RGB_Led::My_RGB_Led(int pin) {
    this->led_duty = 0;
    led = Adafruit_NeoPixel(1, pin, NEO_GRB + NEO_KHZ800);
};


int My_RGB_Led::init() {
   led.begin();  // Initialize the WS2812 LED
    led.show();   // Turn off the LED immediately
    Serial.println("WS2812 LED initialized");
    return 0;
}

void My_RGB_Led::enable_led(bool en) {
    if (en) {
        setColor(255, 255, 255);  // Set to white color for example
    } else {
        setColor(0, 0, 0);  // Turn off the LED
    }
    led.setBrightness(led_duty);
    led.show();
    log_i("Set LED enabled state to %d", en);
}


void My_RGB_Led::setColor(uint8_t r, uint8_t g, uint8_t b) {
    led.setPixelColor(0, led.Color(r, g, b));
    led.show();
    log_i("Set LED color to R:%d, G:%d, B:%d", r, g, b);
}


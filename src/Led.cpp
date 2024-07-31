#include "config.h"
#if CONFIG_LED_ILLUMINATOR_ENABLED
#include "Led.h"

Led::Led(int pin) {
    this->pin = pin;
    this->led_duty = 0;
    this->ledChannel = LED_CHANNEL;     // Choose a PWM channel (0-15)
    this->freq = LED_FREQ;              // Frequency in Hz
    this->resolution = LED_RESOLUTION;  // PWM resolution (8-16 bits)
}

int Led::init() {
    if (ledcSetup(this->ledChannel, this->freq, this->resolution) == 0) {
        return -1;  // Return error if setup fails
    }

    ledcAttachPin(this->pin, this->ledChannel);  // Attach the pin to the PWM channel
    Serial.println("ledcAttachPin succeeded");

    return 0;
}

void Led::enable_led(bool en) {  // Turn LED On or Off
    int duty = en ? this->led_duty : 0;
    if (en && (this->led_duty > CONFIG_LED_MAX_INTENSITY)) {
        duty = CONFIG_LED_MAX_INTENSITY;
    }

    ledcWrite(this->ledChannel, duty);

    log_i("Set LED intensity to %d", duty);
}
#endif
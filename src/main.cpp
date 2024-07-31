
//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//
//            You must select partition scheme from the board menu that has at least 3MB APP space.
//            Face Recognition is DISABLED for ESP32 and ESP32-S2, because it takes up from 15
//            seconds to process single frame. Face Detection is ENABLED if PSRAM is enabled as well
#include <Arduino.h>
#include "My_wifi.h"
#include "MyCamera.h"


My_wifi myWifi;
MyCamera myCamera;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  myWifi.init();
  myCamera.init(true);
  myCamera.startCameraServer();
  Serial.println("new and improve");
}

void loop() {
  // Do nothing. Everything is done in another task by the web server
  delay(10000);
}

#if  false
#ifndef CAPTURE_HANDLER_H 
#define CAPTURE_HANDLER_H
//#include "app_httpd_struct.h"
#include "camera_index.h"
#include "camera_pins.h"
#include "config.h"
#include "driver/ledc.h"  // Include this header for LED PWM functions
#include "esp32-hal-ledc.h"
#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "fb_gfx.h"
#include "img_converters.h"
#include "sdkconfig.h"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#endif
#if CONFIG_LED_ILLUMINATOR_ENABLED
#include "Led.h"
#endif

#if CONFIG_ESP_FACE_DETECT_ENABLED
#include "MyFaceDetection.h"
#endif



class Capture_handler {
public:
  static esp_err_t capture_handler(httpd_req_t *req);
    // Other methods and members of MyCamera
};



class Capture_handler {
       public:

#if CONFIG_ESP_FACE_DETECT_ENABLED
    static FaceDetection faceDetection;
#endif
    Capture_handler();
#if CONFIG_LED_ILLUMINATOR_ENABLED
    Capture_handler(Led *led);
    static Led *led;
#endif

    static esp_err_t captureFrame(camera_fb_t *&fb);
    static esp_err_t sendResponse(httpd_req_t *req, camera_fb_t *fb, int64_t fr_start);
    static void setResponseHeaders(httpd_req_t *req, camera_fb_t *fb);
   // static esp_err_t processFaceDetection(httpd_req_t *req, camera_fb_t *fb, int64_t fr_start);
    static size_t jpg_encode_stream(void *arg, size_t index, const void *data, size_t len);

    static esp_err_t capture_handler(httpd_req_t *req);
};
#endif
#endif
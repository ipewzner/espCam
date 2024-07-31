#if false
#ifndef STREAM_HANDLER_H
#define STREAM_HANDLER_H
// #include "app_httpd_struct.h"
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

class Stream_handler {
   public:
    static const char *_STREAM_CONTENT_TYPE;
    static const char *_STREAM_BOUNDARY;
    static const char *_STREAM_PART;

    // ra_filter_t ra_filter;


    static esp_err_t setResponseHeaders(httpd_req_t *req);

    static camera_fb_t *captureFrame();

    static esp_err_t handleCaptureFailure();

    static void processFrame(camera_fb_t *fb, uint8_t *&_jpg_buf, size_t &_jpg_buf_len, struct timeval &_timestamp, esp_err_t &res);

    static esp_err_t sendJpegFrame(httpd_req_t *req, size_t _jpg_buf_len, struct timeval _timestamp, uint8_t *_jpg_buf);

    static void cleanupFrameBuffer(camera_fb_t *fb, uint8_t *&_jpg_buf);

    static void logFrameDetails(int64_t &last_frame, size_t _jpg_buf_len);


#if CONFIG_ESP_FACE_DETECT_ENABLED
    static MyFaceDetection faceDetection;
#endif
    static esp_err_t stream_handler(httpd_req_t *req);


};
#endif
#endif
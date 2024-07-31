#ifndef FACE_DETECTION_H
#define FACE_DETECTION_H
#include "config.h"

#if CONFIG_ESP_FACE_DETECT_ENABLED
#include <vector>

// #include "app_httpd_struct.h"
#include "camera_index.h"
#include "camera_pins.h"
#include "driver/ledc.h"  // Include this header for LED PWM functions
#include "esp32-hal-ledc.h"
#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "fb_gfx.h"
#include "human_face_detect_mnp01.hpp"
#include "human_face_detect_msr01.hpp"
#include "img_converters.h"
#include "sdkconfig.h"

#if CONFIG_ESP_FACE_RECOGNITION_ENABLED
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#include "face_recognition_112_v1_s16.hpp"
#include "face_recognition_112_v1_s8.hpp"
#include "face_recognition_tool.hpp"
#pragma GCC diagnostic error "-Wformat"
#pragma GCC diagnostic warning "-Wstrict-aliasing"
#endif

class MyFaceDetection {
   public:
    static int face_id;

    static int8_t detection_enabled;

#if TWO_STAGE
    static HumanFaceDetectMSR01 s1;  //(0.1F, 0.5F, 10, 0.2F);
    static HumanFaceDetectMNP01 s2;  //(0.5F, 0.3F, 5);
#else
    static HumanFaceDetectMSR01 s1;  //(0.3F, 0.5F, 10, 0.2F);
#endif

#if CONFIG_ESP_FACE_RECOGNITION_ENABLED
    static int8_t recognition_enabled;
    static int8_t is_enrolling;

#if QUANT_TYPE
    // S16 model
    static FaceRecognition112V1S16 recognizer;
#else
    // S8 model
    static FaceRecognition112V1S8 recognizer;
#endif
#endif

    static void initFaceDetection();
    static void processFaceDetection(camera_fb_t *fb, uint8_t *&_jpg_buf, size_t &_jpg_buf_len, esp_err_t &res);

    static void processRGB565(camera_fb_t *fb, uint8_t *&_jpg_buf, size_t &_jpg_buf_len, esp_err_t &res);
    static void processRGB888(camera_fb_t *fb, uint8_t *&_jpg_buf, size_t &_jpg_buf_len, esp_err_t &res);
    static void draw_face_boxes(fb_data_t *fb, std::list<dl::detect::result_t> *results);

#if CONFIG_ESP_FACE_RECOGNITION_ENABLED
    static void rgb_print(fb_data_t *fb, uint32_t color, const char *str);
    static int rgb_printf(fb_data_t *fb, uint32_t color, const char *format, ...);
    static int run_face_recognition(fb_data_t *fb, std::list<dl::detect::result_t> *results);
#endif
};

#endif
#endif
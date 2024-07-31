#ifndef MYCAMERA_H
#define MYCAMERA_H

// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "config.h"
#include "app_httpd_struct.h"
#if CONFIG_LED_ILLUMINATOR_ENABLED
#ifdef CAMERA_MODEL_ESP32S3_EYE
#include "My_RGB_Led.h"
#else
#include "Led.h"
#endif
#endif

#include "esp_camera.h"
#include "camera_pins.h"

#include "esp_http_server.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "sdkconfig.h"
#include "camera_index.h"

#include "esp32-hal-ledc.h"
#include "driver/ledc.h" // Include this header for LED PWM functions



#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#endif

#if CONFIG_ESP_FACE_DETECT_ENABLED

#include <vector>
#include "human_face_detect_msr01.hpp"
#include "human_face_detect_mnp01.hpp"

#if CONFIG_ESP_FACE_RECOGNITION_ENABLED
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#include "face_recognition_tool.hpp"
#include "face_recognition_112_v1_s16.hpp"
#include "face_recognition_112_v1_s8.hpp"
#pragma GCC diagnostic error "-Wformat"
#pragma GCC diagnostic warning "-Wstrict-aliasing"

#endif
#endif

class MyCamera
{

  
  static bool isStreaming;

// LED FLASH setup
#if CONFIG_LED_ILLUMINATOR_ENABLED
#ifdef CAMERA_MODEL_ESP32S3_EYE
  static My_RGB_Led *led;
#else
  static Led *led;
#endif
#endif

  static const char *_STREAM_CONTENT_TYPE;
  static const char *_STREAM_BOUNDARY;
  static const char *_STREAM_PART;

  httpd_handle_t stream_httpd = NULL;
  httpd_handle_t camera_httpd = NULL;

  ra_filter_t ra_filter;

#if CONFIG_ESP_FACE_DETECT_ENABLED

  static int8_t detection_enabled;

  // #if TWO_STAGE
  //  HumanFaceDetectMSR01 s1(0.1F, 0.5F, 10, 0.2F);
  //  HumanFaceDetectMNP01 s2(0.5F, 0.3F, 5);
  // #else
  //  HumanFaceDetectMSR01 s1(0.3F, 0.5F, 10, 0.2F);
  // #endif

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

#endif

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
  static int ra_filter_run(ra_filter_t *filter, int value);
#endif

#if CONFIG_ESP_FACE_DETECT_ENABLED
#if CONFIG_ESP_FACE_RECOGNITION_ENABLED
  static void rgb_print(fb_data_t *fb, uint32_t color, const char *str);
  static int rgb_printf(fb_data_t *fb, uint32_t color, const char *format, ...);
#endif
  static void draw_face_boxes(fb_data_t *fb, std::list<dl::detect::result_t> *results, int face_id);
#if CONFIG_ESP_FACE_RECOGNITION_ENABLED
  static int run_face_recognition(fb_data_t *fb, std::list<dl::detect::result_t> *results);
#endif
#endif

/*
#if CONFIG_LED_ILLUMINATOR_ENABLED
  static void enable_led(bool en);
#endif
*/

  camera_config_t camera_config;

  void config();

  //================
  // HTTPD handler
  //================
  static esp_err_t reg_handler(httpd_req_t *req);
  static esp_err_t bmp_handler(httpd_req_t *req);
  static esp_err_t cmd_handler(httpd_req_t *req);
  static esp_err_t pll_handler(httpd_req_t *req);
  static esp_err_t win_handler(httpd_req_t *req);
  static esp_err_t greg_handler(httpd_req_t *req);
  static esp_err_t xclk_handler(httpd_req_t *req);
  static esp_err_t index_handler(httpd_req_t *req);
  static esp_err_t status_handler(httpd_req_t *req);
  static esp_err_t stream_handler(httpd_req_t *req);
  static esp_err_t capture_handler(httpd_req_t *req);
  //================
  static esp_err_t parse_get(httpd_req_t *req, char **obuf);
  static int parse_get_var(char *buf, const char *key, int def);
  static int print_reg(char *p, sensor_t *s, uint16_t reg, uint32_t mask);
  static size_t jpg_encode_stream(void *arg, size_t index, const void *data, size_t len);

  ra_filter_t *ra_filter_init(ra_filter_t *filter, size_t sample_size);

  // Used by: start_server -> startCameraServer
  void register_uri_handlers(httpd_handle_t server, httpd_uri_t *uris, size_t num_uris);
  // Used by: startCameraServer
  void start_server(httpd_handle_t *server, httpd_config_t config, httpd_uri_t *uris, size_t num_uris, const char *server_type);

public:
  int init(bool wifiConnected);
  MyCamera(/* args */);
  ~MyCamera();
  void startCameraServer();
};

#endif // MYCAMERA_H

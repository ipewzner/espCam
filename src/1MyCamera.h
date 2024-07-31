#if false 
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
// #include "Capture_handler.h"
// #include "Stream_handler.h"
#include "app_httpd_struct.h"
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

class MyCamera {
   public:
    static MyFaceDetection faceDetection;

    static const char *_STREAM_CONTENT_TYPE;
    static const char *_STREAM_BOUNDARY;
    static const char *_STREAM_PART;

#if CONFIG_LED_ILLUMINATOR_ENABLED
    static Led *led;
#endif

    // static Stream_handler  stream_handler;
    // static Capture_handler capture_handler;

    httpd_handle_t stream_httpd = NULL;
    httpd_handle_t camera_httpd = NULL;
    ra_filter_t ra_filter;
    camera_config_t camera_config;
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
    static int ra_filter_run(ra_filter_t *filter, int value);
#endif

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

#endif  // MYCAMERA_H
#endif  
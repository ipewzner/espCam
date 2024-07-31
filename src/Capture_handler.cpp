/// old
#if defined false
#include "Capture_handler.h"

esp_err_t Capture_handler::capture_handler(httpd_req_t *req) {
    int64_t fr_start = esp_timer_get_time();
    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb) {
        Serial.printf("Camera capture failed\n");
        return httpd_resp_send_500(req);
    }

    uint8_t *jpg_buf = NULL;
    size_t jpg_buf_len = 0;
    esp_err_t res = ESP_OK;

#if CONFIG_ESP_FACE_RECOGNITION_ENABLED
    MyFaceDetection faceDetection;
    faceDetection.initFaceDetection();

    if (fb->format == PIXFORMAT_RGB565 && !MyFaceDetection::recognition_enabled) {
        faceDetection.processRGB565(fb, jpg_buf, jpg_buf_len, res);
    } else {
        faceDetection.processRGB888(fb, jpg_buf, jpg_buf_len, res);
    }
#endif

    if (res == ESP_OK) {
        int64_t fr_end = esp_timer_get_time();
        Serial.printf("JPG: %uB %ums\n", (uint32_t)(jpg_buf_len), (uint32_t)((fr_end - fr_start) / 1000));
        res = httpd_resp_send(req, (const char *)jpg_buf, jpg_buf_len);
    }

    if (fb) {
        esp_camera_fb_return(fb);
        fb = NULL;
    }

    if (jpg_buf) {
        free(jpg_buf);
        jpg_buf = NULL;
    }

    return res;
}



esp_err_t Capture_handler::capture_handler(httpd_req_t *req) {
    camera_fb_t *fb = NULL;
    int64_t fr_start = esp_timer_get_time();

    esp_err_t res = captureFrame(fb);
    if (res != ESP_OK) return res;

    setResponseHeaders(req, fb);

#if CONFIG_ESP_FACE_DETECT_ENABLED
    if (faceDetection.detection_enabled && fb->width <= 400) {
        return faceDetection.processFaceDetection(req, fb, fr_start);
    }
#endif

    return sendResponse(req, fb, fr_start);
}

esp_err_t Capture_handler::captureFrame(camera_fb_t *&fb) {
#if CONFIG_LED_ILLUMINATOR_ENABLED
    led->enable_led(true);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    fb = esp_camera_fb_get();
    led->enable_led(false);
#else
    fb = esp_camera_fb_get();
#endif

    if (!fb) {
        log_e("Camera capture failed");
        return ESP_FAIL;
    }

    return ESP_OK;
}

void Capture_handler::setResponseHeaders(httpd_req_t *req, camera_fb_t *fb) {
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    char ts[32];
    snprintf(ts, 32, "%lld.%06ld", (long long int)fb->timestamp.tv_sec, (long int)fb->timestamp.tv_usec);
    httpd_resp_set_hdr(req, "X-Timestamp", (const char *)ts);
}

size_t Capture_handler::jpg_encode_stream(void *arg, size_t index, const void *data, size_t len) {
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if (!index) {
        j->len = 0;
    }

    if (httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK) {
        return 0;
    }
    j->len += len;
    return len;
}

esp_err_t Capture_handler::sendResponse(httpd_req_t *req, camera_fb_t *fb, int64_t fr_start) {
    esp_err_t res;
    size_t fb_len = 0;

    if (fb->format == PIXFORMAT_JPEG) {
        fb_len = fb->len;
        res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
    } else {
        jpg_chunking_t jchunk = {req, 0};
        if (frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk)) {
            res = ESP_OK;
        } else {
            res = ESP_FAIL;
        }
        httpd_resp_send_chunk(req, NULL, 0);
        fb_len = jchunk.len;
    }

    esp_camera_fb_return(fb);
    int64_t fr_end = esp_timer_get_time();
    log_i("JPG: %uB %ums", (uint32_t)(fb_len), (uint32_t)((fr_end - fr_start) / 1000));

    return res;
}

/*
esp_err_t Capture_handler::processFaceDetection(httpd_req_t *req, camera_fb_t *fb, int64_t fr_start) {
    size_t out_len, out_width, out_height;
    uint8_t *out_buf;
    bool s;
    int face_id = 0;
    bool detected = false;

    jpg_chunking_t jchunk = {req, 0};

    if (fb->format == PIXFORMAT_RGB565) {
#if TWO_STAGE
        HumanFaceDetectMSR01 s1(0.1F, 0.5F, 10, 0.2F);
        HumanFaceDetectMNP01 s2(0.5F, 0.3F, 5);
        std::list<dl::detect::result_t> &candidates = s1.infer((uint16_t *)fb->buf, {(int)fb->height, (int)fb->width, 3});
        std::list<dl::detect::result_t> &results = s2.infer((uint16_t *)fb->buf, {(int)fb->height, (int)fb->width, 3}, candidates);
#else
        HumanFaceDetectMSR01 s1(0.3F, 0.5F, 10, 0.2F);
        std::list<dl::detect::result_t> &results = s1.infer((uint16_t *)fb->buf, {(int)fb->height, (int)fb->width, 3});
#endif
        if (results.size() > 0) {
            fb_data_t rfb;
            rfb.width = fb->width;
            rfb.height = fb->height;
            rfb.data = fb->buf;
            rfb.bytes_per_pixel = 2;
            rfb.format = FB_RGB565;
            detected = true;
            faceDetection.draw_face_boxes(&rfb, &results, face_id);
        }
        s = fmt2jpg_cb(fb->buf, fb->len, fb->width, fb->height, PIXFORMAT_RGB565, 90, jpg_encode_stream, &jchunk);
        esp_camera_fb_return(fb);
    } else {
        out_len = fb->width * fb->height * 3;
        out_width = fb->width;
        out_height = fb->height;
        out_buf = (uint8_t *)malloc(out_len);
        if (!out_buf) {
            log_e("out_buf malloc failed");
            return ESP_FAIL;
        }
        s = fmt2rgb888(fb->buf, fb->len, fb->format, out_buf);
        esp_camera_fb_return(fb);
        if (!s) {
            free(out_buf);
            log_e("To rgb888 failed");
            return ESP_FAIL;
        }

        fb_data_t rfb;
        rfb.width = out_width;
        rfb.height = out_height;
        rfb.data = out_buf;
        rfb.bytes_per_pixel = 3;
        rfb.format = FB_BGR888;

#if TWO_STAGE
        HumanFaceDetectMSR01 s1(0.1F, 0.5F, 10, 0.2F);
        HumanFaceDetectMNP01 s2(0.5F, 0.3F, 5);
        std::list<dl::detect::result_t> &candidates = s1.infer((uint8_t *)out_buf, {(int)out_height, (int)out_width, 3});
        std::list<dl::detect::result_t> &results = s2.infer((uint8_t *)out_buf, {(int)out_height, (int)out_width, 3}, candidates);
#else
        HumanFaceDetectMSR01 s1(0.3F, 0.5F, 10, 0.2F);
        std::list<dl::detect::result_t> &results = s1.infer((uint8_t *)out_buf, {(int)out_height, (int)out_width, 3});
#endif

        if (results.size() > 0) {
            detected = true;
#if CONFIG_ESP_FACE_RECOGNITION_ENABLED
            if (faceDetection.recognition_enabled) {
                face_id = faceDetection.run_face_recognition(&rfb, &results);
            }
#endif
            faceDetection.draw_face_boxes(&rfb, &results, face_id);
        }

        s = fmt2jpg_cb(out_buf, out_len, out_width, out_height, PIXFORMAT_RGB888, 90, jpg_encode_stream, &jchunk);
        free(out_buf);
    }

    if (!s) {
        log_e("JPEG compression failed");
        return ESP_FAIL;
    }

    int64_t fr_end = esp_timer_get_time();
    log_i("FACE: %uB %ums %s%d", (uint32_t)(jchunk.len), (uint32_t)((fr_end - fr_start) / 1000), detected ? "DETECTED " : "", face_id);

    return ESP_OK;
}

*/
#endif

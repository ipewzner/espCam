#include "config.h"

#if CONFIG_ESP_FACE_DETECT_ENABLED
#include "MyFaceDetection.h"
int MyFaceDetection::face_id = 0;

int8_t MyFaceDetection::detection_enabled = 0;

#if TWO_STAGE
HumanFaceDetectMSR01 MyFaceDetection::s1 = HumanFaceDetectMSR01(0.1F, 0.5F, 10, 0.2F);
HumanFaceDetectMNP01 MyFaceDetection::s2 = HumanFaceDetectMNP01(0.5F, 0.3F, 5);
#else
HumanFaceDetectMSR01 MyFaceDetection::s1 = HumanFaceDetectMSR01(0.3F, 0.5F, 10, 0.2F);
#endif

#if CONFIG_ESP_FACE_RECOGNITION_ENABLED
int8_t MyFaceDetection::recognition_enabled = 0;
int8_t MyFaceDetection::is_enrolling = 0;
#if QUANT_TYPE
FaceRecognition112V1S16 MyFaceDetection::recognizer;  // S16 model
#else
FaceRecognition112V1S8 MyFaceDetection::recognizer;  // S8 model
#endif
#endif

void MyFaceDetection::initFaceDetection() {
    /*
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
    bool detected = false;
    int64_t fr_ready = 0;
    int64_t fr_recognize = 0;
    int64_t fr_encode = 0;
    int64_t fr_face = 0;
    int64_t fr_start = 0;
#endif
   // int face_id = 0;
    size_t out_len = 0, out_width = 0, out_height = 0;
    uint8_t *out_buf = NULL;
    bool s = false;
#if TWO_STAGE
    HumanFaceDetectMSR01 s1(0.1F, 0.5F, 10, 0.2F);
    HumanFaceDetectMNP01 s2(0.5F, 0.3F, 5);
#else
    HumanFaceDetectMSR01 s1(0.3F, 0.5F, 10, 0.2F);
#endif
*/
}

void MyFaceDetection::processFaceDetection(camera_fb_t *fb, uint8_t *&_jpg_buf, size_t &_jpg_buf_len, esp_err_t &res) {
    if (fb->format == PIXFORMAT_RGB565
#if CONFIG_ESP_FACE_RECOGNITION_ENABLED
        && !recognition_enabled
#endif
    ) {
        processRGB565(fb, _jpg_buf, _jpg_buf_len, res);
    } else {
        processRGB888(fb, _jpg_buf, _jpg_buf_len, res);
    }
}

void MyFaceDetection::processRGB565(camera_fb_t *fb, uint8_t *&_jpg_buf, size_t &_jpg_buf_len, esp_err_t &res) {
    bool s = false;

#if TWO_STAGE
    // HumanFaceDetectMSR01 s1(0.1F, 0.5F, 10, 0.2F);
    //   HumanFaceDetectMNP01 s2(0.5F, 0.3F, 5);
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

        draw_face_boxes(&rfb, &results);
    }
    s = fmt2jpg(fb->buf, fb->len, fb->width, fb->height, PIXFORMAT_RGB565, 80, &_jpg_buf, &_jpg_buf_len);
    esp_camera_fb_return(fb);
    fb = NULL;
    if (!s) {
        log_e("fmt2jpg failed");
        res = ESP_FAIL;
    }
}

void MyFaceDetection::processRGB888(camera_fb_t *fb, uint8_t *&_jpg_buf, size_t &_jpg_buf_len, esp_err_t &res) {
    Serial.println("*01");
    size_t out_len = fb->width * fb->height * 3;
    size_t out_width = fb->width;
    size_t out_height = fb->height;
    Serial.println("*02");
    uint8_t *out_buf = (uint8_t *)malloc(out_len);

    Serial.println("*03");
    if (!out_buf) {
        log_e("out_buf malloc failed");
        Serial.println("*03 out_buf malloc failed");
        res = ESP_FAIL;
    } else {
        Serial.println("*03 else");
        Serial.println("*03 fb->len: " + String(fb->len));
        Serial.println("*03 fb->format: " + String(fb->format));
        bool s = fmt2rgb888(fb->buf, fb->len, fb->format, out_buf);
        esp_camera_fb_return(fb);
        fb = NULL;
        if (!s) {
            free(out_buf);
            Serial.println("*03 To rgb888 failed");
            log_e("To rgb888 failed");
            res = ESP_FAIL;
        } else {
            fb_data_t rfb;
            rfb.width = out_width;
            rfb.height = out_height;
            rfb.data = out_buf;
            rfb.bytes_per_pixel = 3;
            rfb.format = FB_BGR888;

#if TWO_STAGE
            //  HumanFaceDetectMSR01 s1(0.1F, 0.5F, 10, 0.2F);
            // HumanFaceDetectMNP01 s2(0.5F, 0.3F, 5);
            std::list<dl::detect::result_t> &candidates = s1.infer((uint8_t *)out_buf, {(int)out_height, (int)out_width, 3});
            std::list<dl::detect::result_t> &results = s2.infer((uint8_t *)out_buf, {(int)out_height, (int)out_width, 3}, candidates);
#else
            HumanFaceDetectMSR01 s1(0.3F, 0.5F, 10, 0.2F);

            std::list<dl::detect::result_t> &results = s1.infer((uint8_t *)out_buf, {(int)out_height, (int)out_width, 3});
#endif

            if (results.size() > 0) {
#if CONFIG_ESP_FACE_RECOGNITION_ENABLED
                if (recognition_enabled) {
                    face_id = run_face_recognition(&rfb, &results);
                }
#endif
                draw_face_boxes(&rfb, &results);
            }
            s = fmt2jpg(out_buf, out_len, out_width, out_height, PIXFORMAT_RGB888, 90, &_jpg_buf, &_jpg_buf_len);
            free(out_buf);
            if (!s) {
                log_e("fmt2jpg failed");
                Serial.println("*03 fmt2jpg failed");

                res = ESP_FAIL;
            }
        }
    }
}

void MyFaceDetection::draw_face_boxes(fb_data_t *fb, std::list<dl::detect::result_t> *results) {
    int x, y, w, h;
    uint32_t color = FACE_COLOR_YELLOW;

    if (face_id < 0) {
        color = FACE_COLOR_RED;
    } else if (face_id > 0) {
        color = FACE_COLOR_GREEN;
    }

    if (fb->bytes_per_pixel == 2) {
        color = ((color >> 16) & 0x001F) | ((color >> 3) & 0x07E0) | ((color << 8) & 0xF800);
    }
    // color = ((color >> 8) & 0xF800) | ((color >> 3) & 0x07E0) | (color & 0x001F);

    int i = 0;
    for (std::list<dl::detect::result_t>::iterator prediction = results->begin(); prediction != results->end(); prediction++, i++) {
        // rectangle box
        x = (int)prediction->box[0];
        y = (int)prediction->box[1];
        w = (int)prediction->box[2] - x + 1;
        h = (int)prediction->box[3] - y + 1;

        if ((x + w) > fb->width) {
            w = fb->width - x;
        }
        if ((y + h) > fb->height) {
            h = fb->height - y;
        }

        fb_gfx_drawFastHLine(fb, x, y, w, color);
        fb_gfx_drawFastHLine(fb, x, y + h - 1, w, color);
        fb_gfx_drawFastVLine(fb, x, y, h, color);
        fb_gfx_drawFastVLine(fb, x + w - 1, y, h, color);
#if TWO_STAGE
        // landmarks (left eye, mouth left, nose, right eye, mouth right)
        int x0, y0, j;
        for (j = 0; j < 10; j += 2) {
            x0 = (int)prediction->keypoint[j];
            y0 = (int)prediction->keypoint[j + 1];
            fb_gfx_fillRect(fb, x0, y0, 3, 3, color);
        }
#endif
    }
}

#if CONFIG_ESP_FACE_RECOGNITION_ENABLED
void MyFaceDetection::rgb_print(fb_data_t *fb, uint32_t color, const char *str) {
    fb_gfx_print(fb, (fb->width - (strlen(str) * 14)) / 2, 10, color, str);
}

int MyFaceDetection::rgb_printf(fb_data_t *fb, uint32_t color, const char *format, ...) {
    char loc_buf[64];
    char *temp = loc_buf;
    int len;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    len = vsnprintf(loc_buf, sizeof(loc_buf), format, arg);
    va_end(copy);
    if (len >= sizeof(loc_buf)) {
        temp = (char *)malloc(len + 1);
        if (temp == NULL) {
            return 0;
        }
    }
    vsnprintf(temp, len + 1, format, arg);
    va_end(arg);
    rgb_print(fb, color, temp);
    if (len > 64) {
        free(temp);
    }
    return len;
}

int MyFaceDetection::run_face_recognition(fb_data_t *fb, std::list<dl::detect::result_t> *results) {
    std::vector<int> landmarks = results->front().keypoint;
    int id = -1;

    Tensor<uint8_t> tensor;
    tensor.set_element((uint8_t *)fb->data).set_shape({fb->height, fb->width, 3}).set_auto_free(false);

    int enrolled_count = recognizer.get_enrolled_id_num();

    if (enrolled_count < FACE_ID_SAVE_NUMBER && is_enrolling) {
        id = recognizer.enroll_id(tensor, landmarks, "", true);
        log_i("Enrolled ID: %d", id);
        rgb_printf(fb, FACE_COLOR_CYAN, "ID[%u]", id);
    }

    face_info_t recognize = recognizer.recognize(tensor, landmarks);
    if (recognize.id >= 0) {
        rgb_printf(fb, FACE_COLOR_GREEN, "ID[%u]: %.2f", recognize.id, recognize.similarity);
    } else {
        rgb_print(fb, FACE_COLOR_RED, "Intruder Alert!");
    }
    return recognize.id;
}
#endif

#endif
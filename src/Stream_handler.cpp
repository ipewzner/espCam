#if false
#include "Stream_handler.h"

const char *Stream_handler::_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
const char *Stream_handler::_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
const char *Stream_handler::_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: %d.%06d\r\n\r\n";

esp_err_t Stream_handler::stream_handler(httpd_req_t *req) {
    camera_fb_t *fb = NULL;
    struct timeval _timestamp;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    char part_buf[128];

    faceDetection.initFaceDetection();
    int64_t last_frame = initializeLastFrame();

    res = setResponseHeaders(req);
    if (res != ESP_OK) {
        return res;
    }

    while (true) {
        fb = captureFrame();
        if (!fb) {
            res = handleCaptureFailure();
        } else {
            processFrame(fb, _jpg_buf, _jpg_buf_len, _timestamp, res);
        }

        if (res == ESP_OK) {
            res = sendChunk(req, _STREAM_BOUNDARY);
        }
        if (res == ESP_OK) {
            res = sendJpegFrame(req, _jpg_buf_len, _timestamp, _jpg_buf);
        }

        cleanupFrameBuffer(fb, _jpg_buf);

        if (res != ESP_OK) {
            log_e("Send frame failed");
            break;
        }
        logFrameDetails(last_frame, _jpg_buf_len);
    }

    return res;
}

int64_t Stream_handler::initializeLastFrame() {
    int64_t last_frame = 0;
    if (!last_frame) {
        last_frame = esp_timer_get_time();
    }
    return last_frame;
}

esp_err_t Stream_handler::setResponseHeaders(httpd_req_t *req) {
    esp_err_t res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) {
        return res;
    }
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "X-Framerate", "60");
    return res;
}

camera_fb_t *Stream_handler::captureFrame() {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        log_e("Camera capture failed");
    }
    return fb;
}

esp_err_t Stream_handler::handleCaptureFailure() {
    esp_err_t res = ESP_FAIL;
    return res;
}

void Stream_handler::processFrame(camera_fb_t *fb, uint8_t *&_jpg_buf, size_t &_jpg_buf_len, struct timeval &_timestamp, esp_err_t &res) {
    _timestamp.tv_sec = fb->timestamp.tv_sec;
    _timestamp.tv_usec = fb->timestamp.tv_usec;

#if CONFIG_ESP_FACE_DETECT_ENABLED
    if (!faceDetection.detection_enabled || fb->width > 400) {
#endif
        if (fb->format != PIXFORMAT_JPEG) {
            bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
            esp_camera_fb_return(fb);
            fb = NULL;
            if (!jpeg_converted) {
                log_e("JPEG compression failed");
                res = ESP_FAIL;
            }
        } else {
            _jpg_buf_len = fb->len;
            _jpg_buf = fb->buf;
        }
#if CONFIG_ESP_FACE_DETECT_ENABLED
    } else {
        faceDetection.processFaceDetection(fb, _jpg_buf, _jpg_buf_len, res);
    }
#endif
}

esp_err_t Stream_handler::sendChunk(httpd_req_t *req, const char *chunk) {
    esp_err_t res = httpd_resp_send_chunk(req, chunk, strlen(chunk));
    return res;
}

esp_err_t Stream_handler::sendJpegFrame(httpd_req_t *req, size_t _jpg_buf_len, struct timeval _timestamp, uint8_t *_jpg_buf) {
    esp_err_t res = ESP_OK;
    char part_buf[128];
    size_t hlen = snprintf((char *)part_buf, 128, _STREAM_PART, _jpg_buf_len, _timestamp.tv_sec, _timestamp.tv_usec);
    res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    if (res == ESP_OK) {
        res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    return res;
}

void Stream_handler::cleanupFrameBuffer(camera_fb_t *fb, uint8_t *&_jpg_buf) {
    if (fb) {
        esp_camera_fb_return(fb);
        fb = NULL;
        _jpg_buf = NULL;
    } else if (_jpg_buf) {
        free(_jpg_buf);
        _jpg_buf = NULL;
    }
}

#endif
#include <Arduino.h>

#include "esp_http_server.h"

typedef struct
{
    httpd_req_t *req;
    size_t len;
} jpg_chunking_t;

typedef struct
{
    size_t size;   // number of values used for filtering
    size_t index;  // current value index
    size_t count;  // value count
    int sum;
    int *values;  // array to be filled with values
} ra_filter_t;
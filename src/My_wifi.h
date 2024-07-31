#ifndef MY_WIFI_H 
#define MY_WIFI_H
#include <WiFi.h>
#include "config.h"

class My_wifi
{
public:
    const char *ssid;
    const char *password;
    My_wifi(/* args */);
    ~My_wifi();
    int init();
};

My_wifi::My_wifi(/* args */)
{
    this->ssid = WIFI_SSID;
    this->password = WIFI_PASSWORD;
}

My_wifi::~My_wifi()
{
}

// ===========================
// Enter your WiFi credentials
// ===========================

int My_wifi::init()
{

    WiFi.begin(ssid, password);
    WiFi.setSleep(false);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");

    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("' to connect");

    return 0;
}
#endif
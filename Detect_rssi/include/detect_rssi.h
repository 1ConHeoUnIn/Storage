//detect_rssi.h
#ifndef DETECT_RSSI_H
#define DETECT_RSSI_H
#include <Arduino.h>
#include <WiFi.h>

class detect_rssi
{
    private:
    const char* ssid;
    const char* password;

    public:
    detect_rssi();
    void begin();
    long get_rssi();
    void stop();
};

#endif

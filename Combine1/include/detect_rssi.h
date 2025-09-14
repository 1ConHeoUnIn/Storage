//detect_rssi.h
#ifndef DETECT_RSSI_H
#define DETECT_RSSI_H
#include <Arduino.h>
#include <WiFi.h>
#include "kalman_filter.h"


class detect_rssi
{
    private:
    const char* ssid;
    const char* password;
    KalmanFilter rssi_filter;

    public:
    detect_rssi();
    void begin();
    long get_rssi();
    void stop();
};

#endif

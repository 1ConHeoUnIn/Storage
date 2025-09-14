//dectect_rssi.cpp
#include "detect_rssi.h"

detect_rssi::detect_rssi()
{
    ssid = "hehe";
    password = "12345678";
}
void detect_rssi::begin()
{
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi");
}

long detect_rssi::get_rssi() {
    long rssi = WiFi.RSSI();
    Serial.print("RSSI: ");
    Serial.print(rssi);
    Serial.println(" dBm");
    return rssi;
}


void detect_rssi::stop()
{
    WiFi.disconnect();
}

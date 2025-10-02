// dectect_rssi.cpp
#include "detect_rssi.h"

detect_rssi::detect_rssi()
{
    ssid = "pa";
    password ="12345678";
    rssi_filter = KalmanFilter(0.0, 1.0, 1.0, 0.95); // x, p , r, a
}
void detect_rssi::begin()
{
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi");
    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 20)
    {
        delay(500);
        Serial.print(".");
        attempt++;
    }
    Serial.println("Connected to WiFi");
}

long detect_rssi::get_rssi()
{
    /*
    // filter
    long rssi_raw = WiFi.RSSI();

    
    long rssi = rssi_filter.update(rssi_raw);
    */
   // none filter
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

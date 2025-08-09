#include <Arduino.h>
#include "detect_rssi.h"

detect_rssi d;

void setup() {
Serial.begin(115200);
d.begin();
}

void loop() {
  d.get_rssi();
  delay(100);
}


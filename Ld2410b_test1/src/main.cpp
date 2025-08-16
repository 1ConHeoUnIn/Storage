#include <Arduino.h>
#include <ld2410.h>

#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 20
#define RADAR_TX_PIN 21

ld2410 radar;

uint32_t lastReading = 0;

void setup(void)
{
   MONITOR_SERIAL.begin(115200);
  RADAR_SERIAL.begin(256000, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);
  delay(500);

  MONITOR_SERIAL.println(F("LD2410 radar sensor initializing..."));
  if (radar.begin(RADAR_SERIAL)) {
    MONITOR_SERIAL.println(F("Radar connected."));
    MONITOR_SERIAL.print(F("Firmware version: "));
    MONITOR_SERIAL.print(radar.firmware_major_version);
    MONITOR_SERIAL.print('.');
    MONITOR_SERIAL.print(radar.firmware_minor_version);
    MONITOR_SERIAL.print('.');
    MONITOR_SERIAL.println(radar.firmware_bugfix_version, HEX);
  } else {
    MONITOR_SERIAL.println(F("Radar not connected."));
  }
}

void loop()
{
  radar.read();
  if(radar.isConnected() && millis() - lastReading > 1000)  //Report every 1000ms
  {
    lastReading = millis();
    if(radar.presenceDetected())
    {
      if(radar.stationaryTargetDetected())
      {
        Serial.print(F("Stationary target: "));
        Serial.print(radar.stationaryTargetDistance());
        Serial.print(F("cm energy:"));
        Serial.print(radar.stationaryTargetEnergy());
        Serial.print(' ');
      }
      if(radar.movingTargetDetected())
      {
        Serial.print(F("Moving target: "));
        Serial.print(radar.movingTargetDistance());
        Serial.print(F("cm energy:"));
        Serial.print(radar.movingTargetEnergy());
      }
      Serial.println();
    }
    else
    {
      Serial.println(F("No target"));
    }
  }
}
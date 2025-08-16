#include <Arduino.h>
#include <ld2410.h>

#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 20
#define RADAR_TX_PIN 21

ld2410 radar;

uint32_t lastReading = 0;

// Ngưỡng năng lượng để xác định người thực sự, ta sẽ điều chỉnh thông số này
const uint16_t MOVING_ENERGY_THRESHOLD = 100;
const uint16_t STATIONARY_ENERGY_THRESHOLD = 100;

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

  //check radar connect
  if(!radar.isConnected())
  {
    return;
  }

  //check time to avoid sensor read to fast
  if(millis() - lastReading < 1000)
  {
    return;
  
  }
  lastReading = millis();

MONITOR_SERIAL.print("detect energy: ");
MONITOR_SERIAL.print(radar.stationaryTargetDetected());

MONITOR_SERIAL.print("| station energy: ");
MONITOR_SERIAL.println(radar.stationaryTargetEnergy());

MONITOR_SERIAL.print("detect moving energy: ");
MONITOR_SERIAL.print(radar.movingTargetDetected());

MONITOR_SERIAL.print("| moving energy: ");
MONITOR_SERIAL.println(radar.movingTargetEnergy());



delay(200);


}
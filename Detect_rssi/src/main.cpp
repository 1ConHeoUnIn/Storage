#include <Arduino.h>
#include "detect_rssi.h"

detect_rssi d;
 long sum = 0;
 int count =0;
void setup() {
Serial.begin(115200);
d.begin();
}

void loop() {
  d.get_rssi();


int i = d.get_rssi();
sum += i;
count++;

if(count == 50)
{
  float average = sum/50.0;
  Serial.print("Average RSSI: ");
  Serial.print(average);
  Serial.println(" dBm");
  count = 0;
  sum = 0;
}
  delay(100);

}


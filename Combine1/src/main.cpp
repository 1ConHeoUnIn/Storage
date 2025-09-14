#include <Arduino.h>
#include "display.h"
#include "Compass.h"
#include "Motor_control.h"
#include "detect_rssi.h"

Compass compass;
oled_display oled;
motor_driver xe;
detect_rssi rssi;
int target = 0;
int current_heading;
int best_rssi = -999;
int current_rssi;
int driff;
void setup()
{
    Serial.begin(115200);

    PinManager::init_i2c();
    oled.begin();
    xe.begin();
    rssi.begin();
    oled.notice();
    delay(1000);
    compass.begin();

    xe.stop();
 
}

void loop()
{

current_heading = compass.get_heading();
current_rssi = rssi.get_rssi();


oled.print(current_heading,current_rssi,best_rssi,target,driff);
   

if(current_rssi > best_rssi)
{
best_rssi = current_rssi;
target = current_heading;
if(best_rssi == 0)
{ best_rssi = -999;}

}
driff = (target - current_heading+540)%360-180;
Serial.print("driff: ");
Serial.println(driff);


if(driff > 5)
{
    xe.turn_right(100);
}

else if(driff< -5)
{
    xe.turn_left(100);
}
else
{
    xe.move_forward(100);
}



delay(50);



}
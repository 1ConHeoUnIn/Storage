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

  void check_direction()
{
    if(rssi.get_rssi() >= best_rssi)
    {
    best_rssi = rssi.get_rssi();
    target = current_heading;

    // Chống reset nếu RSSI bằng 0
    if (best_rssi == 0)
    {
        best_rssi = -999;
    }
}

    Serial.println("Đã cập nhật hướng tốt nhất.");
}

void first_scan()
{
    int start_heading = compass.get_heading();
    int s =0;
    while(s <360)
    {
    xe.turn_left(300);
    delay(100);

    xe.stop();
    delay(100);

    current_heading = compass.get_heading();
    check_direction();
    
    s = (current_heading - start_heading + 360) % 360;
    }
    xe.stop();
    delay(100);
}

void left_scan()
{
    xe.turn_left(500);
    delay(2000);
    xe.stop();
    delay(100);

}

void right_scan()
{
    xe.turn_right(500);
    delay(2000);
    xe.stop();
    delay(100);
}





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
    delay(100);
    first_scan();
    
 
}

void loop()
{
    current_heading = compass.get_heading();
    current_rssi = rssi.get_rssi();
oled.print(current_heading,current_rssi,best_rssi,target,driff);


driff = (target - current_heading+540)%360-180;
Serial.print("driff: ");
Serial.println(driff);


if(driff > 5)
{
    xe.turn_right(900);
}

else if(driff< -5)
{
    xe.turn_left(900);
}
else
{
    xe.move_forward(900);
}



delay(50);



}
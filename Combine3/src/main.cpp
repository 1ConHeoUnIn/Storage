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
int f = 0;
int start_heading =0;

  void check_direction()
{
    int temp = current_rssi;
    if(temp >= best_rssi)
    {
    best_rssi = temp;
    target = current_heading;

    // Chống reset nếu RSSI bằng 0
   /*  if (best_rssi == 0)
    {
        best_rssi = -999;
    }
        */
}

    Serial.println("Đã cập nhật hướng tốt nhất.");
}

void first_scan()
{

    start_heading = compass.get_heading();
    int spin =0;
    while(spin <350)
    {

    xe.turn_left(700);
    delay(200);

    xe.stop();
    
    
    current_heading = compass.get_heading();
    current_rssi = rssi.get_rssi();
    oled.first_scan_view(current_heading,current_rssi,best_rssi,target,spin);

    check_direction();
    
    int step = (start_heading - current_heading +360 ) % 360 ;
    start_heading = current_heading;
    if (step > 0 && step < 45) { // lọc nhiễu, chỉ cộng bước hợp lý
        spin += step;
    }
    Serial.print("spin: ");
    Serial.println(spin);
    delay(1000);
    
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
   

 
}

void loop()
{

    if(f <1)
    {
        delay(1000);
    first_scan();
    f++;
    }

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

check_direction();

delay(50);



}
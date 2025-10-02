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
    if (best_rssi == 0)
    {
        best_rssi = -999;
    }
        
}


    Serial.println("Đã cập nhật hướng tốt nhất.");
}

void first_scan()
{

    start_heading = compass.get_heading();
    int spin =0;
    while(spin <200)
    {

    xe.turn_left(700);
    delay(500);

    xe.stop();
    
    
    current_heading = compass.get_heading();
    current_rssi = rssi.get_rssi();
    oled.first_scan_view(current_heading,current_rssi,best_rssi,target,spin);

    check_direction();
    
    int step = (start_heading - current_heading +360 ) % 360 ;
   
    if (step > 0 && step < 100) { // lọc nhiễu, chỉ cộng bước hợp lý
        spin += step;
         start_heading = current_heading;
    }
    else{
        xe.stop();
    Serial.println("Nhiễu từ trường, dừng lại để kiểm tra lại hướng...");
    
    // Lặp lại kiểm tra cho đến khi step hợp lý
    while (step >= 100 || step <= 0) {
        delay(500); // chờ ổn định từ trường
        current_heading = compass.get_heading();
        current_rssi = rssi.get_rssi();
        check_direction();
        oled.first_scan_view(current_heading, current_rssi, best_rssi, target, spin);

        step = (start_heading - current_heading + 360) % 360;
    }

    spin += step;
    start_heading = current_heading;
    }

    Serial.print("spin: ");
    Serial.println(spin);
    delay(1000);
    
    }
    xe.stop();
    delay(100);
}

void scan()
{
    xe.turn_left(1000);
    delay(500);
    xe.stop();
    check_direction();
    delay(100);
    xe.turn_right(1000);
    delay(1000);
     xe.stop();
    check_direction();
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

 if(current_rssi > -50 )
{
    xe.stop();
    
}

if(current_rssi < best_rssi )
{
    best_rssi = -999;

scan();
}


delay(50);
}
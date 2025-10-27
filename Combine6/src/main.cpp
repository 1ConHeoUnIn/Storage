#include <Arduino.h>
#include "display.h"
#include "Compass.h"
#include "Motor_control.h"
#include "detect_rssi.h"
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

Compass compass;
oled_display oled;
motor_driver xe;
detect_rssi rssi;
WiFiClientSecure secureClient;
PubSubClient client(secureClient);


    const char* my_ssid = rssi.get_ssid();
    const char* my_password = rssi.get_password();

    const char* mqtt_server = "032497695b644f6ab159c2c420aedb87.s1.eu.hivemq.cloud";

const int mqtt_port = 8883;
const char* mqtt_topic = "data";
const char* mqtt_user = "caixe";     // Thay bằng username MQTT thật
const char* mqtt_pass = "1Caixelamdoan";     // Thay bằng password MQTT thật

int target = 0;
int current_heading;
int best_rssi = -999;
int current_rssi;
int driff;
int f = 0;
int start_heading =0;

void reconnect() {
  while (!client.connected()) {
    Serial.print("Đang kết nối MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println("Thành công!");
    } else {
      Serial.print("Thất bại, mã lỗi: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

  void check_direction()
{
    int temp = current_rssi;
    if(temp > best_rssi)
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

    int spin =0;
    while(spin <200)
    {
     if (!client.connected()) {
    reconnect();
  }
  client.loop();

    start_heading = compass.get_heading();

    xe.turn_left(500);
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
              String payload = String(current_rssi);
   bool success = client.publish(mqtt_topic, payload.c_str());

if (success) {
  Serial.print("✅ Publish thành công: ");
  Serial.println(payload);
} else {
  Serial.println("❌ Publish thất bại!");
}

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
    xe.turn_left(400);
    delay(500);
    xe.stop();
    check_direction();
    delay(1000);
    xe.turn_right(400);
    delay(1000);
     xe.stop();
    check_direction();
    delay(1000);



}





void setup()
{
    Serial.begin(115200);
      secureClient.setInsecure(); // Bỏ qua xác thực chứng chỉ
  client.setServer(mqtt_server, mqtt_port);
  

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
   if (!client.connected()) {
    reconnect();
  }
  client.loop();
    if(f <1)
    {
        delay(1000);
    first_scan();
    f++;
    }
    

current_heading = compass.get_heading();
current_rssi = rssi.get_rssi();
check_direction();

     String payload = String(current_rssi);
     bool success = client.publish(mqtt_topic, payload.c_str());

if (success) {
  Serial.print("✅ Publish thành công: ");
  Serial.println(payload);
} else {
  Serial.println("❌ Publish thất bại!");
}


driff = (target - current_heading+540)%360-180;
Serial.print("driff: ");
Serial.println(driff);


if(driff > 5)
{
    xe.turn_right(400);

}

else if(driff< -5)
{
    xe.turn_left(400);

}
else
{
    xe.move_forward(400);

}



 if(current_rssi > -50 )
{
    xe.stop();
    
}

if((best_rssi -current_rssi) >7 )
{
     Serial.println("📉 RSSI giảm mạnh, quét lại...");
    best_rssi = -999;
    oled.print(current_heading,current_rssi,best_rssi,target,333); // 333 là hiển thị vào chế độ scan

scan();
}

 oled.print(current_heading, current_rssi, best_rssi, target, driff);
delay(50);
}
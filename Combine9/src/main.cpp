#include <Arduino.h>
#include "display.h"
#include "Compass.h"
#include "Motor_control.h"
#include "detect_rssi.h"
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

const int button =10;
volatile bool buttonPressed = false;
void IRAM_ATTR handleButtonInterrupt() {
  buttonPressed = true;
}


Compass compass;
oled_display oled;
motor_driver xe;
detect_rssi rssi;
WiFiClientSecure secureClient;
PubSubClient client(secureClient);

const char *my_ssid = rssi.get_ssid();
const char *my_password = rssi.get_password();

const char *mqtt_server = "032497695b644f6ab159c2c420aedb87.s1.eu.hivemq.cloud";

const int mqtt_port = 8883;
const char *mqtt_user = "caixe";         // Thay bằng username MQTT thật
const char *mqtt_pass = "1Caixelamdoan"; // Thay bằng password MQTT thật

const char *rssi_topic = "rssi";
const char *best_rssi_topic = "best rssi";
const char *compass_topic = "compass";
const char *best_heading_topic = "best heading";
const char *status_topic = "status";
const char *decline_topic ="decline";


int target = 0;
int current_heading;
int best_rssi = -999;
int current_rssi;
int driff;
int f = 0;
int start_heading = 0;
int decline;

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Đang kết nối MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass))
    {
      Serial.println("Thành công!");
    }
    else
    {
      Serial.print("Thất bại, mã lỗi: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}
bool publish_data(const char *topic, const String &payload)
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  bool success = client.publish(topic, payload.c_str());

  if (success)
  {
    Serial.print("✅ Publish thành công tới topic [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(payload);
  }
  else
  {
    Serial.print("❌ Publish thất bại tới topic [");
    Serial.print(topic);
    Serial.println("]");
  }

  return success;
}

void check_direction()
{
  int temp = current_rssi;
  if (temp > best_rssi)
  {
    best_rssi = temp;
    target = current_heading;

    // Chống reset nếu RSSI bằng 0
    if (best_rssi == 0)
    {
      best_rssi = -999;
    }
  publish_data(rssi_topic, String(current_rssi));
  publish_data(compass_topic, String(current_heading));
  publish_data(best_rssi_topic, String(best_rssi));
  publish_data(best_heading_topic, String(target));
  }

  Serial.println("Đã cập nhật hướng tốt nhất.");
}

void first_scan()
{
  publish_data(status_topic, "first_scan");
  int spin = 0;
 
  while (spin < 250)
  {
    if (!client.connected())
    {
      reconnect();
    }
    client.loop();

    start_heading = compass.get_heading();

    xe.turn_left(500);
    delay(500);

    xe.stop();

    current_heading = compass.get_heading();
    current_rssi = rssi.get_rssi();
    oled.first_scan_view(current_heading, current_rssi, best_rssi, target, spin);

    check_direction();


    int step = (start_heading - current_heading + 360) % 360;

    if (step > 0 && step < 100)
    { // lọc nhiễu, chỉ cộng bước hợp lý
      spin += step;
      start_heading = current_heading;
    }
    else
    {
      xe.stop();
      Serial.println("Nhiễu từ trường, dừng lại để kiểm tra lại hướng...");

      // Lặp lại kiểm tra cho đến khi step hợp lý
      while (step >= 100 || step <= 0)
      {
        delay(500); // chờ ổn định từ trường
        current_heading = compass.get_heading();
        current_rssi = rssi.get_rssi();
        check_direction();
        oled.first_scan_view(current_heading, current_rssi, best_rssi, target, spin);
        publish_data(rssi_topic, String(current_rssi));
        publish_data(compass_topic, String(current_heading));
        publish_data(best_rssi_topic, String(best_rssi));
        publish_data(best_heading_topic, String(target));

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
  publish_data(status_topic, "scan right");
  xe.turn_right(400);
  delay(1000);
  xe.stop();
  check_direction();
  delay(2000);
  publish_data(status_topic, "scan left");
  xe.turn_left(500);
  delay(500);
  xe.stop();
  check_direction();
  delay(2000);

}

void setup()
{

  Serial.begin(115200);
  secureClient.setInsecure(); // Bỏ qua xác thực chứng chỉ
  client.setServer(mqtt_server, mqtt_port);

    pinMode(button, INPUT_PULLUP); // Kích hoạt điện trở kéo lên
attachInterrupt(digitalPinToInterrupt(button), handleButtonInterrupt, FALLING);


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
  if (buttonPressed) {
  buttonPressed = false;

  Serial.println("🔘 Nút được nhấn - Lùi và rẽ trái");
  
  publish_data(status_topic, "colliding");
  xe.move_backward(400); 
  delay(2000);
  scan();
  xe.stop();
}

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  if (f < 1)
  {
    delay(1000);
    first_scan();
    f++;
  }

  current_heading = compass.get_heading();
  current_rssi = rssi.get_rssi();
  //  check_direction();
  publish_data(status_topic, "go");
  publish_data(rssi_topic, String(current_rssi));
  publish_data(compass_topic, String(current_heading));
  publish_data(best_rssi_topic, String(best_rssi));
  publish_data(best_heading_topic, String(target));

  driff = (target - current_heading + 540) % 360 - 180;
  Serial.print("driff: ");
  Serial.println(driff);


  if (driff > 5)
  {
    xe.turn_right(400);
  }

  else if (driff < -5)
  {
    xe.turn_left(400);
  }
  else
  {
    xe.move_forward(400);
  }

  if (current_rssi > -55)
  {
    xe.stop();
  }
  decline = best_rssi - current_rssi;
  publish_data(decline_topic, String(decline));
  if (decline > 7)
  {
    
    Serial.println("📉 RSSI giảm mạnh, quét lại...");
    best_rssi = -999;
    oled.print(current_heading, current_rssi, best_rssi, target, 333); // 333 là hiển thị vào chế độ scan
     
    scan();
  }

  oled.print(current_heading, current_rssi, best_rssi, target, driff);
  delay(50);
}
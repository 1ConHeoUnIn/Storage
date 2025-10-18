#include <Arduino.h>
#include "detect_rssi.h"
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

WiFiClientSecure secureClient;
PubSubClient client(secureClient);

const char* mqtt_server = "032497695b644f6ab159c2c420aedb87.s1.eu.hivemq.cloud";

const int mqtt_port = 8883;
const char* mqtt_topic = "rssi";
const char* mqtt_user = "caixe";     // Thay bằng username MQTT thật
const char* mqtt_pass = "1Caixelamdoan";  

detect_rssi d;
 long sum = 0;
 int count =0;
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

void setup() {
Serial.begin(115200);
      secureClient.setInsecure(); // Bỏ qua xác thực chứng chỉ
  client.setServer(mqtt_server, mqtt_port);
d.begin();
}

void loop() {
     if (!client.connected()) {
    reconnect();
  }
  client.loop();

  d.get_rssi();


int i = d.get_rssi();
sum += i;
count++;
float average;
if(count == 50)
{
  average = sum/50.0;
  Serial.print("Average RSSI: ");
  Serial.print(average);
  Serial.println(" dBm");
  count = 0;
  sum = 0;
   String payload = String(average);
     bool success = client.publish(mqtt_topic, payload.c_str());
     if (success) {
  Serial.print("✅ Publish thành công: ");
  Serial.println(payload);
} else {
  Serial.println("❌ Publish thất bại!");
}
}
    


  delay(100);

}


#include <Arduino.h>
#include <MyLD2410.h>

// Khởi tạo UART1 với chân RX=20, TX=21
HardwareSerial SensorSerial(1);
MyLD2410 sensor(SensorSerial, true);  // Bật debug nếu cần

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Khởi tạo UART1
  SensorSerial.begin(256000, SERIAL_8N1, 20, 21);  // LD2410 sử dụng baud 256000

  Serial.println("Đang khởi động cảm biến LD2410B...");

  if (sensor.begin()) {
    Serial.println("Cảm biến đã sẵn sàng!");
  } else {
    Serial.println("Không thể kết nối với cảm biến.");
  }

  // Yêu cầu chế độ nâng cao nếu cần
  sensor.configMode(true);
  sensor.enhancedMode(true);
  sensor.configMode(false);
}

void loop() {
  MyLD2410::Response response = sensor.check();

  if (response == MyLD2410::DATA) {
    if (sensor.presenceDetected()) {
      Serial.println("Phát hiện có người!");
      Serial.print("Khoảng cách: ");
      Serial.print(sensor.detectedDistance());
      Serial.println(" cm");

      Serial.print("Tín hiệu chuyển động: ");
      Serial.println(sensor.movingTargetSignal());

      Serial.print("Tín hiệu đứng yên: ");
      Serial.println(sensor.stationaryTargetSignal());
    } else {
      Serial.println("Không có người.");
    }
  }

  delay(200);
}

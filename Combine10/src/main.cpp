#include <Arduino.h>
#include "display.h"
#include "Compass.h"
#include "Motor_control.h"
#include "detect_rssi.h"
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// --- PIN CONFIG ---
const int button = 10;
volatile bool buttonPressed = false;

// --- OBJECTS ---
Compass compass;
oled_display oled;
motor_driver xe;
detect_rssi rssi;
WiFiClientSecure secureClient;
PubSubClient client(secureClient);

// --- MQTT CONFIG ---
const char *mqtt_server = "032497695b644f6ab159c2c420aedb87.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char *mqtt_user = "caixe";
const char *mqtt_pass = "1Caixelamdoan";

// --- MQTT TOPICS ---
const char *rssi_topic = "rssi";
const char *best_rssi_topic = "best rssi";
const char *compass_topic = "compass";
const char *best_heading_topic = "best heading";
const char *status_topic = "status";
const char *decline_topic = "decline";

// --- DEBUG TOPICS ---
const char *log_topic = "log";           
const char *smooth_rssi_topic = "smooth"; 
const char *driff_topic = "driff";       

// --- GLOBAL VARIABLES ---
int target = 0;
int current_heading;
int best_rssi = -120; 
int current_rssi;
int driff;
bool has_first_scan = false; 
int decline;
int start_heading = 0; // Biến lưu góc bắt đầu cho First Scan

// --- BIẾN BỘ LỌC ---
float smooth_rssi_val = -120.0; 
int noise_strike_count = 0; // Đếm số lần tín hiệu bị sốc liên tiếp

// --- INTERRUPT ---
void IRAM_ATTR handleButtonInterrupt() {
  buttonPressed = true;
}

// --- HELPER FUNCTIONS ---

void log_remote(String msg) {
  Serial.println(msg); 
  if (client.connected()) {
    client.publish(log_topic, msg.c_str());
  }
}

// --- HÀM LỌC NHIỄU THÔNG MINH (SMART FILTER) ---
int get_smooth_rssi() {
  int raw_val = rssi.get_rssi(); 
  
  // 1. Lọc giá trị rác (0 hoặc quá lớn do lỗi)
  if (raw_val == 0 || raw_val > -10) {
      return (int)smooth_rssi_val; 
  }

  // Lần đầu chạy
  if (smooth_rssi_val < -119.0) {
    smooth_rssi_val = raw_val;
    return raw_val;
  }

  // 2. Kiểm tra độ lệch (Threshold Check)
  int diff = abs(raw_val - (int)smooth_rssi_val);
  
  // Nếu lệch quá 15 đơn vị -> Nghi ngờ nhiễu
  if (diff > 15) {
      noise_strike_count++; 
      // Nếu chưa đủ 3 lần liên tiếp -> BỎ QUA, dùng giá trị cũ
      if (noise_strike_count < 3) {
          return (int)smooth_rssi_val; 
      }
      // Nếu đã 3 lần -> Chấp nhận sự thật, reset đếm
  } else {
      noise_strike_count = 0; // Tín hiệu ổn định
  }
  
  // 3. Trung bình động (EMA) - Hệ số 0.2/0.8 để mượt
  smooth_rssi_val = (raw_val * 0.2) + (smooth_rssi_val * 0.8);
  
  return (int)smooth_rssi_val; 
}

void keep_connection(unsigned long duration) {
  unsigned long start = millis();
  while (millis() - start < duration) {
    if (client.connected()) {
      client.loop();
    }
    if (buttonPressed) break; 
  }
}

void reconnect() {
  if (!client.connected()) {
    Serial.print("Connecting MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println("Connected!");
      client.publish(status_topic, "Device Online");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      delay(2000); 
    }
  }
}

bool publish_data(const char *topic, const String &payload) {
  if (!client.connected()) reconnect();
  client.loop();
  return client.publish(topic, payload.c_str());
}

void update_status() {
  publish_data(rssi_topic, String(current_rssi));
  publish_data(smooth_rssi_topic, String(get_smooth_rssi())); 
  publish_data(compass_topic, String(current_heading));
  publish_data(best_rssi_topic, String(best_rssi));
  publish_data(best_heading_topic, String(target));
  publish_data(driff_topic, String(driff));
}

// --- ACTION FUNCTIONS ---

void scan() {
  log_remote("👁️ Scanning...");
  publish_data(status_topic, "scanning");
  
  // Reset bộ lọc để lấy giá trị thực tế tại chỗ
  smooth_rssi_val = -120.0;
  noise_strike_count = 0;

  int local_max_rssi = -120; 
  int local_best_head = current_heading;

  // Quét Phải
  xe.turn_right(400);
  unsigned long start = millis();
  while (millis() - start < 1000) { 
    client.loop();
    int r = get_smooth_rssi(); 
    int h = compass.get_heading();
    if (r > local_max_rssi) {
      local_max_rssi = r;
      local_best_head = h;
    }
  }
  xe.stop();
  keep_connection(300);

  // Quét Trái
  xe.turn_left(400);
  start = millis();
  while (millis() - start < 2000) { 
    client.loop();
    int r = get_smooth_rssi(); 
    int h = compass.get_heading();
    if (r > local_max_rssi) {
      local_max_rssi = r;
      local_best_head = h;
    }
  }
  xe.stop();

  best_rssi = local_max_rssi; 
  target = local_best_head;
  
  log_remote("✅ Scan Done. New Best: " + String(best_rssi));
  publish_data(status_topic, "scan done");
  
  keep_connection(500);
}

void execute_u_turn() {
  log_remote("↩️ U-TURN EXECUTE!");
  publish_data(status_topic, "u-turn");
  
  int start_h = compass.get_heading();
  int target_u = (start_h + 180) % 360; 
  
  xe.turn_left(450); 
  
  unsigned long start_time = millis();
  while(millis() - start_time < 3000) { 
    client.loop(); 
    int current_h = compass.get_heading();
    int diff = abs(current_h - target_u);
    if(diff < 20) break; 
  }
  xe.stop();
  
  xe.move_forward(450);
  keep_connection(1500); 
  xe.stop();
  
  // Reset bộ lọc
  best_rssi = -120; 
  smooth_rssi_val = -120.0;
  noise_strike_count = 0;
}

void first_scan() {
  log_remote("🏁 FIRST SCAN START");
  publish_data(status_topic, "first_scan");
  int spin = 0;
  
  best_rssi = -120; 
  smooth_rssi_val = -120.0;
  target = compass.get_heading();

  // Giai đoạn 1: Quay và quét
  while (spin < 340) { 
    if (!client.connected()) reconnect();
    client.loop();

    start_heading = compass.get_heading();
    xe.turn_left(500);
    keep_connection(300); 
    xe.stop();
    keep_connection(300); 

    current_heading = compass.get_heading();
    current_rssi = get_smooth_rssi(); 

    if (current_rssi > best_rssi) {
      best_rssi = current_rssi;
      target = current_heading;
    }
    
    oled.first_scan_view(current_heading, current_rssi, best_rssi, target, spin);
    update_status();

    int step = (start_heading - current_heading + 360) % 360;
    if (step > 0 && step < 100) {
      spin += step;
    } else {
       xe.stop();
       keep_connection(500); 
    }
  }
  xe.stop();
  log_remote("🏁 Scan Done. Aligning...");

  // Giai đoạn 2: Quay về hướng tốt nhất (FIX LỖI LOOP SỚM)
  unsigned long align_start = millis();
  while (millis() - align_start < 5000) { 
      if (!client.connected()) reconnect();
      client.loop();

      int h = compass.get_heading();
      int diff = (target - h + 540) % 360 - 180;

      if (abs(diff) < 10) break; 

      if (diff > 0) xe.turn_right(450);
      else xe.turn_left(450);
      
      delay(50); 
  }
  xe.stop();
  
  // Đọc lại RSSI thực tế tại hướng chuẩn để Loop không bị sốc
  keep_connection(1000);
  smooth_rssi_val = -120; 
  best_rssi = get_smooth_rssi(); 
  
  log_remote("✅ Ready! Start Loop.");
  publish_data(status_topic, "first_scan_completed");
}

// --- SETUP ---
void setup() {
  Serial.begin(115200);
  secureClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);

  pinMode(button, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button), handleButtonInterrupt, FALLING);

  PinManager::init_i2c();
  oled.begin();
  xe.begin();
  rssi.begin();
  oled.notice();
  
  keep_connection(1000);
  compass.begin();
  xe.stop();
}

// --- LOOP ---
void loop() {
  // 1. VA CHẠM
  if (buttonPressed) {
    buttonPressed = false;
    log_remote("💥 COLLISION!");
    publish_data(status_topic, "colliding");
    
    xe.stop();
    delay(100);
    xe.move_backward(500); 
    keep_connection(1000); 
    xe.turn_back_right(500); 
    keep_connection(1500);
    
    // Reset hoàn toàn
    smooth_rssi_val = -120.0;
    best_rssi = -120;
    scan(); 
  }

  if (!client.connected()) reconnect();
  client.loop();

  // 2. STARTUP
  if (!has_first_scan) {
    keep_connection(1000);
    first_scan();
    has_first_scan = true;
  }

  // 3. SENSORS
  current_heading = compass.get_heading();
  current_rssi = get_smooth_rssi(); 
  update_status();

  // 4. UPDATE BEST (PASSIVE)
  if (current_rssi > best_rssi) {
    best_rssi = current_rssi;
  }

  // 5. DECLINE CHECK
  decline = best_rssi - current_rssi;
  publish_data(decline_topic, String(decline));

  // 6. LOGIC LẠC ĐƯỜNG
  if (decline > 5) { // Ngưỡng 5dBm
      log_remote("📉 Drop: " + String(decline) + "dB");
      xe.stop();
      
      int old_best = best_rssi; 
      scan(); 
      
      int gap = old_best - best_rssi;
      bool is_bad_drop = (old_best > -90) && (gap > 6);
      
      // Safe Zone (> -55)
      if (old_best > -55) {
         log_remote("🛡️ Safe Zone. Ignore drop.");
         is_bad_drop = false; 
         best_rssi = current_rssi; 
      }

      if (is_bad_drop) {
         log_remote("⚠️ Bad Drop! Breakthrough...");
         
         xe.move_forward(450);
         keep_connection(1000); 
         xe.stop();
         
         int check_again = get_smooth_rssi();
         
         if (check_again >= old_best - 5) {
              log_remote("🎉 Success! Continue.");
              best_rssi = check_again;
         } 
         else {
              log_remote("❌ Failed. U-TURN.");
              execute_u_turn();
              // Reset sau U-Turn
              smooth_rssi_val = -120;
              scan(); 
         }
      } else {
          log_remote("ℹ️ Drop OK.");
      }
     decline = 0; 
  }

  // 7. NAVIGATE
  driff = (target - current_heading + 540) % 360 - 180;
  
  if (current_rssi > -55) { 
    xe.stop();
    publish_data(status_topic, "Arrived Target!");
  } 
  else {
    if (driff > 8) xe.turn_right(400);
    else if (driff < -8) xe.turn_left(400);
    else {
      xe.move_forward(450);
      publish_data(status_topic, "go forward");
    }
  }

  oled.print(current_heading, current_rssi, best_rssi, target, driff);
  keep_connection(50); 
}
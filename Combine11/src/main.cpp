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

// --- TOPICS ---
const char *rssi_topic = "rssi";
const char *best_rssi_topic = "best rssi";
const char *compass_topic = "compass";
const char *best_heading_topic = "best heading";
const char *status_topic = "status";
const char *decline_topic = "decline";
const char *driff_topic = "driff";
const char *log_topic = "debug/log";           
const char *smooth_rssi_topic = "debug/smooth";

// --- GLOBAL VARIABLES ---
int target = 0;
int current_heading;
int current_rssi;
int best_rssi = -120; 
int decline = 0;
int driff = 0;
bool has_first_scan = false; 
float smooth_rssi_val = -120.0; 
unsigned long last_mqtt_pub = 0;

// --- BIẾN LỌC NHIỄU ---
int bad_signal_counter = 0;       

// --- INTERRUPT ---
void IRAM_ATTR handleButtonInterrupt() {
  buttonPressed = true;
}

// --- HELPER FUNCTIONS ---
void log_remote(String msg) {
  Serial.println(msg); 
  if (client.connected()) client.publish(log_topic, msg.c_str());
}

int get_smooth_rssi() {
  int raw_val = rssi.get_rssi(); 
  if (raw_val == 0 || raw_val > -10) return (int)smooth_rssi_val;

  if (smooth_rssi_val < -119.0) {
    smooth_rssi_val = raw_val;
    return raw_val;
  }
  smooth_rssi_val = (raw_val * 0.2) + (smooth_rssi_val * 0.8);
  return (int)smooth_rssi_val; 
}

void reconnect() {
  if (!client.connected()) {
    Serial.print("Connecting MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println("Connected!");
    } else {
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
  if (millis() - last_mqtt_pub < 100) return; 
  publish_data(rssi_topic, String(current_rssi));
  publish_data(smooth_rssi_topic, String(get_smooth_rssi())); 
  publish_data(compass_topic, String(current_heading));
  publish_data(best_rssi_topic, String(best_rssi));
  publish_data(best_heading_topic, String(target));
  publish_data(decline_topic, String(decline));
  publish_data(driff_topic, String(driff)); 
  last_mqtt_pub = millis();
}

// --- HÀM GIỮ KẾT NỐI + CẬP NHẬT OLED ---
// Hàm này được dùng ở khắp mọi nơi (khi chờ, khi quay, khi đo)
// Nên thêm oled.print vào đây sẽ giúp màn hình luôn sống
void keep_connection_active(unsigned long duration) {
  unsigned long start = millis();
  while (millis() - start < duration) {
    if (client.connected()) client.loop();
    if (buttonPressed) break; 
    
    // Cập nhật số liệu nền
    current_heading = compass.get_heading();
    current_rssi = get_smooth_rssi();
    driff = (target - current_heading + 540) % 360 - 180;
    
    update_status(); // Gửi MQTT
    
    // CẬP NHẬT OLED (Fix lỗi treo màn hình)
    oled.print(current_heading, current_rssi, best_rssi, target, driff);
  }
}

// --- ĐO KỸ "TOP-TIER" ---
int get_best_rssi_samples(int samples) {
  int buffer[samples];
  int valid_count = 0;

  for(int i=0; i<samples; i++) {
     int r = rssi.get_rssi();
     if(r < 0 && r > -120) {
         buffer[valid_count] = r;
         valid_count++;
     }
     
     // Trong lúc đo vẫn cập nhật màn hình
     current_heading = compass.get_heading();
     driff = (target - current_heading + 540) % 360 - 180;
     update_status(); 
     oled.print(current_heading, r, best_rssi, target, driff); // Hiện RSSI tức thời
     
     delay(15); 
  }

  if(valid_count == 0) return -100;

  for (int i = 0; i < valid_count - 1; i++) {
    for (int j = 0; j < valid_count - i - 1; j++) {
      if (buffer[j] < buffer[j + 1]) {
        int temp = buffer[j];
        buffer[j] = buffer[j + 1];
        buffer[j + 1] = temp;
      }
    }
  }

  int top_n = valid_count / 3; 
  if (top_n < 1) top_n = 1; 

  long sum_top = 0;
  for (int k = 0; k < top_n; k++) {
      sum_top += buffer[k];
  }

  return sum_top / top_n;
}

void turn_to_absolute_angle(int target_angle) {
  unsigned long start_turn = millis();
  while (millis() - start_turn < 4000) { 
      if (buttonPressed) return;
      
      int h = compass.get_heading();
      int diff = (target_angle - h + 540) % 360 - 180;
      
      current_heading = h;
      driff = (target - h + 540) % 360 - 180; 
      update_status(); 
      oled.print(current_heading, current_rssi, best_rssi, target, driff); // Update OLED

      if (abs(diff) < 5) { xe.stop(); return; }
      
      int spd = 450;
      if (abs(diff) < 30) spd = 350; 
      
      if (diff > 0) xe.turn_right(spd); else xe.turn_left(spd);
      delay(20); 
  }
  xe.stop(); 
}

void execute_u_turn() {
  log_remote("U-TURN EXECUTE!");
  publish_data(status_topic, "u-turn");
  
  int start_h = compass.get_heading();
  int target_u = (start_h + 180) % 360; 
  
  xe.turn_left(450); 
  unsigned long start_time = millis();
  while(millis() - start_time < 3000) { 
    client.loop(); 
    if (buttonPressed) { xe.stop(); return; }
    
    current_heading = compass.get_heading();
    update_status();
    oled.print(current_heading, current_rssi, best_rssi, target, 999); // 999 báo hiệu đang quay

    if(abs(current_heading - target_u) < 20) break; 
  }
  xe.stop();
  
  xe.move_forward(450);
  keep_connection_active(1000); 
  xe.stop();
}

bool scan_greedy(int old_best_val) {
  log_remote("Greedy Scan (Top-Tier)...");
  publish_data(status_topic, "scanning");
  xe.stop();
  keep_connection_active(500); 

  int center_heading = compass.get_heading();
  int center_rssi = get_best_rssi_samples(20); 
  
  int best_local_rssi = center_rssi;
  int best_local_heading = center_heading;
  int offset = 60; 

  // Check Phải
  int right_angle = (center_heading + offset) % 360;
  turn_to_absolute_angle(right_angle); 
  xe.stop();
  keep_connection_active(500); 
  int right_rssi = get_best_rssi_samples(20);

  if (right_rssi > center_rssi + 3) { 
      log_remote("Right is better!");
      best_local_rssi = right_rssi;
      best_local_heading = right_angle;
  } 
  else {
      // Check Trái
      int left_angle = (center_heading - offset + 360) % 360;
      turn_to_absolute_angle(left_angle);
      xe.stop();
      keep_connection_active(500);
      int left_rssi = get_best_rssi_samples(20);

      if (left_rssi > right_rssi && left_rssi > center_rssi + 3) {
          best_local_rssi = left_rssi;
          best_local_heading = left_angle;
      } else if (right_rssi > center_rssi + 3) {
          best_local_rssi = right_rssi;
          best_local_heading = right_angle;
      }
  }

  int gap = old_best_val - best_local_rssi;
  
  if (old_best_val > -90 && gap > 8) {
      log_remote("Weak Signal. Rejecting.");
      return false; 
  }

  best_rssi = best_local_rssi;
  target = best_local_heading;
  
  log_remote("New Target: " + String(target) + " (RSSI: " + String(best_rssi) + ")");
  
  turn_to_absolute_angle(target);
  return true; 
}

// --- FIRST SCAN 8 HƯỚNG (ĐÃ THÊM OLED VIEW) ---
void scan_8_points() {
  log_remote("FIRST SCAN (8-Point / Top-Tier)");
  publish_data(status_topic, "first scan 8pts");
  xe.stop();
  
  int points[8] = {0, 45, 90, 135, 180, 225, 270, 315};
  
  // Reset để cập nhật ngay
  best_rssi = -120; 
  target = current_heading;

  for (int i = 0; i < 8; i++) {
      if (buttonPressed) return;
      
      log_remote("Check: " + String(points[i]));
      turn_to_absolute_angle(points[i]); 
      xe.stop();
      keep_connection_active(1500); // Ổn định 1.5s
      
      int val = get_best_rssi_samples(20);
      
      log_remote("Angle " + String(points[i]) + ": " + String(val));
      
      if (val > best_rssi) {
          best_rssi = val;
          target = points[i];
          update_status();
          log_remote("New Best Found: " + String(best_rssi));
      }
      
      // CẬP NHẬT OLED GIAO DIỆN FIRST SCAN
      // Hiển thị góc đang quét (points[i]) và giá trị đo được (val)
      oled.first_scan_view(points[i], val, best_rssi, target, i * 45); 
  }
  
  log_remote("COMPLETE. Winner: " + String(target));
  turn_to_absolute_angle(target);
  keep_connection_active(500);
}

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
  keep_connection_active(1000);
  compass.begin();
  xe.stop();
}

void loop() {
  // 1. VA CHẠM
  if (buttonPressed) {
    buttonPressed = false;
    log_remote("COLLISION!");
    xe.stop(); delay(100);
    xe.move_backward(500); keep_connection_active(1000);
    xe.turn_back_right(500); keep_connection_active(1000);
    best_rssi = -120; 
    scan_8_points(); 
  }

  if (!client.connected()) reconnect();
  client.loop();

  if (!has_first_scan) {
    scan_8_points(); 
    has_first_scan = true;
  }

  current_heading = compass.get_heading();
  current_rssi = get_smooth_rssi(); 
  driff = (target - current_heading + 540) % 360 - 180;
  
  // Update status và OLED liên tục
  update_status();
  oled.print(current_heading, current_rssi, best_rssi, target, driff);

  if (current_rssi > best_rssi + 2) {
    best_rssi = current_rssi;
    bad_signal_counter = 0; 
  }

  decline = best_rssi - current_rssi;
  publish_data(decline_topic, String(decline));

  // 3. LOGIC KÍCH HOẠT SCAN
  if (decline > 5) { 
      bad_signal_counter++; 
      
      if (bad_signal_counter > 8) { // Ngưỡng kiên nhẫn 8 lần
          log_remote("Confirmed Drop: " + String(decline) + "dB. Scanning...");
          xe.stop();
          
          int old_best_val = best_rssi;
          bool scan_success = scan_greedy(old_best_val);
             
          if (!scan_success) {
             execute_u_turn();
             best_rssi = -120;
             scan_greedy(-120); 
          }
             
          smooth_rssi_val = get_best_rssi_samples(10); 
          current_rssi = (int)smooth_rssi_val;
          bad_signal_counter = 0; 
      }
  } else {
      bad_signal_counter = 0;
  }

  // 4. ĐIỀU HƯỚNG
  if (current_rssi > -45) { 
    xe.stop();
    publish_data(status_topic, "Arrived Target!");
  } 
  else {
    int deadband = 15; 
    int turn_spd = 350; 

    if (driff > deadband) { 
      xe.turn_right(turn_spd);
      keep_connection_active(100); 
      xe.move_forward(450);
    } 
    else if (driff < -deadband) {
      xe.turn_left(turn_spd);
      keep_connection_active(100); 
      xe.move_forward(450);
    } 
    else {
      xe.move_forward(450);
      publish_data(status_topic, "go forward");
    }
  }
  
  keep_connection_active(20); 
}
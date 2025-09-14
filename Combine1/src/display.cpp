//display.cpp
#include "display.h"
#include <math.h>

oled_display::oled_display()
    : u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE) {}



void oled_display::begin() {
    u8g2.begin();
    u8g2.setFont(u8g2_font_6x12_tr);
}

void oled_display::clear() {
    u8g2.clearBuffer();
    u8g2.sendBuffer();
}

void oled_display::print(int heading, int rssi ,int best_rssi, int best_heading,int driff)
{

    draw_compass(heading);
    show_rssi(rssi);
    show_best_rssi(best_rssi);
    show_best_heading(best_heading);
    direction(driff);
    u8g2.sendBuffer();
}




void oled_display::draw_compass(int heading) {
    const int cx = 100;  // Tâm màn hình OLED
    const int cy = 32;
    const int r = 20;   // Bán kính vòng tròn

    // Xóa màn hình
    u8g2.clearBuffer();

     char buffer[10];
    snprintf(buffer, sizeof(buffer),"%d", heading);
    u8g2.drawStr(cx-7, cy + 4, buffer);

    // Các ký hiệu và góc gốc
    const char* labels[4] = {"N", "E", "S", "W"};
    const int base_angles[4] = {0, 90, 180, 270};

    for (int i = 0; i < 4; i++) {
        float angle_deg = base_angles[i] - heading;
        float angle_rad = angle_deg * PI / 180.0;

        int x = cx + r * cos(angle_rad);
        int y = cy + r * sin(angle_rad);

        // Điều chỉnh vị trí chữ cho đẹp
        u8g2.drawStr(x - 3, y + 4, labels[i]);

        
    }


}



void oled_display::show_rssi(int rssi)
{
    //u8g2.clearBuffer();
    char buffer[10];
    snprintf(buffer, sizeof(buffer),"R: %d", rssi);
    u8g2.drawStr(5, 10, buffer);

}

void oled_display::show_best_rssi(int best_rssi)
{
    //u8g2.clearBuffer();
    char buffer[10];
    snprintf(buffer, sizeof(buffer),"b: %d", best_rssi);
    u8g2.drawStr(5, 25, buffer);
    
}

void oled_display::show_best_heading(int best_heading)
{
    //u8g2.clearBuffer();
    char buffer[10];
    snprintf(buffer, sizeof(buffer),"a: %d", best_heading);
    u8g2.drawStr(5, 40, buffer);
    
}

void oled_display::direction(int driff)
{
  
    char buffer[10];
    snprintf(buffer, sizeof(buffer),"%d", driff);
    u8g2.drawStr(5, 55, buffer);
    u8g2.sendBuffer();
}

void oled_display::notice()
{
    u8g2.clearBuffer();
     char buffer[20];
    snprintf(buffer, sizeof(buffer), "Spin to calibrate");
    u8g2.drawStr(5, 10, buffer);
  u8g2.sendBuffer();

}


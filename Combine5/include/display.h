//display.h
#ifndef DISPLAY_H
#define DISPLAY_H

#include <U8g2lib.h>
#include <Arduino.h>
#include "pin_manager.h"

class oled_display {
private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;


    

public:
    oled_display();
    void begin();
    void clear();
    void draw_compass(int heading);

    void show_rssi(int rssi);
    void show_best_rssi(int best_rssi);
    void show_best_heading(int best_heading);
    void direction(int driff);
    void notice();
    void print(int heading, int rssi,int best_rssi, int best_heading,int driff);
    void first_scan_view(int heading, int rssi, int best_rssi, int best_heading,int spin);
    void show_spin(int spin);



    
};
#endif

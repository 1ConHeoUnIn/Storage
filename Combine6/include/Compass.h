//compass.h
#ifndef COMPASS_H
#define COMPASS_H

#include <Arduino.h>
#include <QMC5883LCompass.h>
#include <Preferences.h>
#include "pin_manager.h"


class Compass
{
private:
    QMC5883LCompass qmc5883l;
    Preferences memory;


public:
    float off_x, off_y, off_z;
    float scale_x, scale_y, scale_z;

    Compass();
    void begin();
    void start_calibrate();
    void save_data();
    void load_data();
    void check_calibrate();
    int get_heading();
    const char *store = "MyCompass_data"; // namespace Flash
    const char *key_used = "useCount";
    const char *key_flag = "calibrated";
};

#endif

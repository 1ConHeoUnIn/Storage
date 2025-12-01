//compass.cpp
#include "Compass.h"

Compass::Compass() {}
void Compass::begin()
{

    qmc5883l.init();    
    qmc5883l.setSmoothing(10, false);
    check_calibrate();
    qmc5883l.setCalibrationOffsets(off_x, off_y, off_z);
    qmc5883l.setCalibrationScales(scale_x, scale_y, scale_z);
}

void Compass::start_calibrate()
{
    qmc5883l.calibrate();
    off_x = qmc5883l.getCalibrationOffset(0);
    off_y = qmc5883l.getCalibrationOffset(1);
    off_z = qmc5883l.getCalibrationOffset(2);
    scale_x = qmc5883l.getCalibrationScale(0);
    scale_y = qmc5883l.getCalibrationScale(1);
    scale_z = qmc5883l.getCalibrationScale(2);
}

void Compass::save_data()
{
    memory.begin(store, false);
    memory.putFloat("off0", off_x);
    memory.putFloat("off1", off_y);
    memory.putFloat("off2", off_z);
    memory.putFloat("scale0", scale_x);
    memory.putFloat("scale1", scale_y);
    memory.putFloat("scale2", scale_z);
    memory.end();
    Serial.println("Success save calibrate ");
}

void Compass::load_data()
{
    memory.begin(store, true);
    off_x = memory.getFloat("off0");
    off_y = memory.getFloat("off1");
    off_z = memory.getFloat("off2");
    scale_x = memory.getFloat("scale0");
    scale_y = memory.getFloat("scale1");
    scale_z = memory.getFloat("scale2");
    memory.end();
}
void Compass::check_calibrate()
{
    memory.begin(store, false);
    bool isCalibrated = memory.getBool(key_flag, false);
    int useCount = memory.getInt(key_used, 0);

    if (!isCalibrated || useCount >= 3)
    {
        Serial.println("please move and wait to calibrate");
        start_calibrate();
        save_data();
        memory.putBool(key_flag, true);
        memory.putInt(key_used, 1);
        Serial.println("Finishing calibrate");
    }
    else
    {
        load_data();
        memory.putInt(key_used, useCount + 1);
        Serial.printf("Number of use: %d\n", useCount + 1);
    }
    memory.end();
}
int Compass::get_heading()
{
    qmc5883l.read();
    int azimuth = qmc5883l.getAzimuth();
    if (azimuth < 0)
    {
        azimuth += 360; // Chuyển đổi góc âm sang khoảng 180-359
    }
    Serial.print(" Azimuth : ");
    Serial.println(azimuth);
    return azimuth;
}

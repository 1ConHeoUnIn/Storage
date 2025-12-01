#include "Motor_control.h"

motor_driver::motor_driver(){}

void motor_driver::begin()
{
    // Gán kênh PWM cho từng chân
    ledcSetup(0, 10000, 9); // kênh 0, tần số 10kHz, độ phân giải 9bit
    ledcSetup(1, 10000, 9);
    ledcSetup(2, 10000, 9);
    ledcSetup(3, 10000, 9);

    ledcAttachPin(PinManager::in1, 0);
    ledcAttachPin(PinManager::in2, 1);
    ledcAttachPin(PinManager::in3, 2);
    ledcAttachPin(PinManager::in4, 3);

    stop(); // đảm bảo mức 0
}

void motor_driver::move_forward(int speed) {
    ledcWrite(0, speed); //A+
    ledcWrite(1, 0);
    ledcWrite(2, speed); //B+
    ledcWrite(3, 0);
}

void motor_driver::move_backward(int speed) {
    ledcWrite(0, 0);
    ledcWrite(1, speed);
    ledcWrite(2, 0);
    ledcWrite(3, speed);
}

void motor_driver::turn_left(int speed) {
    ledcWrite(0, 0);
    ledcWrite(1, 0);
    ledcWrite(2, speed); //B+
    ledcWrite(3, 0);
  
}

void motor_driver::turn_right(int speed) {
    ledcWrite(0, speed); //A+
    ledcWrite(1, 0);
    ledcWrite(2, 0);
    ledcWrite(3, 0);
     
}

void motor_driver::stop() {
    ledcWrite(0, 0);
    ledcWrite(1, 0);
    ledcWrite(2, 0);
    ledcWrite(3, 0);
}

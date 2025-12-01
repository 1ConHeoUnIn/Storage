#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <Arduino.h>
#include "pin_manager.h"

class motor_driver
{
    public:
        motor_driver();
        void begin();
        void move_forward(int speed);
        void move_backward(int speed);
        void turn_left(int speed);
        void turn_right(int speed);
        void stop();

};
#endif

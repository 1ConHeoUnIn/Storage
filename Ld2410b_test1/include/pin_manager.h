#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H
#include<Wire.h>
#include <HardwareSerial.h>

//  Cấu hình chân dùng chung
struct PinManager {
   //ld2410b
   static const int UART_TX = 21;
   static const int UART_RX = 20;

  static void init_uart() {
        Serial1.begin(115200, SERIAL_8N1, UART_RX, UART_TX);
    }
   
};

#endif

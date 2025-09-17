#ifndef UART_H
#define UART_H

#include <Arduino.h>

void uart_init(unsigned long baudRate = 115200);
void uart_loop();
void processUartCommand(String command);


#endif // UART_H

#ifndef ESP32SERVO_H
#define ESP32SERVO_H

#include <Arduino.h>

class Servo {
public:
    Servo();
    void attach(int pin);  // Подключение сервопривода к пину
    void write(int angle); // Установка угла поворота (0-180 градусов)
    void detach();         // Отключение сервопривода

private:
    int _pin;              // Пин, к которому подключен сервопривод
    bool _attached;        // Флаг подключения
};

// Инициализация библиотеки
void init();

#endif
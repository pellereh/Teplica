#include "ESP32Servo.h"
#include <esp_timer.h>

// Глобальные переменные для управления PWM
static const int PWM_FREQ = 50;  // Частота PWM (50 Hz для сервоприводов)
static const int MIN_PULSE_WIDTH = 500;  // Минимальная ширина импульса (микросекунды)
static const int MAX_PULSE_WIDTH = 2500; // Максимальная ширина импульса (микросекунды)

// Преобразование угла в ширину импульса
int mapAngleToPulseWidth(int angle) {
    return MIN_PULSE_WIDTH + (angle * (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) / 180);
}

// Инициализация библиотеки
void init() {
    // Ничего не делаем, так как ESP32 автоматически поддерживает PWM
}

// Конструктор класса Servo
Servo::Servo() : _pin(-1), _attached(false) {}

// Подключение сервопривода к пину
void Servo::attach(int pin) {
    if (_attached) {
        detach();  // Если уже подключён, отключаем предыдущий пин
    }

    _pin = pin;
    _attached = true;

    // Настройка PWM для указанного пина
    ledcSetup(0, PWM_FREQ, 16);  // Канал 0, частота 50 Hz, разрешение 16 бит
    ledcAttachPin(_pin, 0);      // Привязываем пин к каналу 0
}

// Установка угла поворота
void Servo::write(int angle) {
    if (!_attached || angle < 0 || angle > 180) {
        return;  // Выходим, если не подключены или угол вне диапазона
    }

    int pulseWidth = mapAngleToPulseWidth(angle);  // Преобразуем угол в ширину импульса
    float dutyCycle = (pulseWidth / 1000000.0) * PWM_FREQ * 100;  // Рассчитываем коэффициент заполнения

    // Устанавливаем значение duty cycle для канала 0
    ledcWriteTone(0, PWM_FREQ);  // Устанавливаем частоту
    ledcWrite(0, dutyCycle);     // Устанавливаем duty cycle
}

// Отключение сервопривода
void Servo::detach() {
    if (_attached) {
        ledcDetachPin(_pin);  // Отключаем PWM от пина
        _attached = false;
    }
}
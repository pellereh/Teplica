#include <WiFi.h>
#include <ESP32Servo.h>  // Подключение нашей библиотеки

// Настройка пина для сервопривода
const int servoPin = 19;  // Пин, к которому подключен сервопривод

// Объект для управления сервоприводом
Servo servoWindow;

void setup() {
    Serial.begin(115200);  // Инициализация последовательного порта
    Serial.println("Starting Servo Test...");

    // Инициализация библиотеки ESP32Servo
    init();

    // Подключение сервопривода к указанному пину
    servoWindow.attach(servoPin);

    // Установка начального угла в 0 градусов
    servoWindow.write(0);
    Serial.println("Servo initialized at 0 degrees.");
}

void loop() {
    // Плавное открытие форточки (0 → 90 градусов)
    for (int angle = 0; angle <= 90; angle++) {
        servoWindow.write(angle);  // Устанавливаем угол
        Serial.print("Moving to angle: ");
        Serial.println(angle);
        delay(15);  // Задержка для плавности движения
    }

    // Плавное закрытие форточки (90 → 0 градусов)
    for (int angle = 90; angle >= 0; angle--) {
        servoWindow.write(angle);  // Устанавливаем угол
        Serial.print("Moving to angle: ");
        Serial.println(angle);
        delay(15);  // Задержка для плавности движения
    }
}
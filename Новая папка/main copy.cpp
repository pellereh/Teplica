#include <WiFi.h>
#include <ESP32Servo.h>
#include "page.h" // Подключение файла с HTML-страницей

// Настройки Wi-Fi
const char* ssid = "YOUR_WIFI_SSID";       // Название вашей сети
const char* password = "YOUR_WIFI_PASSWORD";  // Пароль вашей сети

// Номер порта для сервера
WiFiServer server(80);

// Состояние выходов
bool pumpState = false;  // Состояние насоса
bool windState = false;  // Состояние вентилятора
bool windowState = false;  // Состояние форточки

// Номера выводов
const int pumpPin = 17;  // Пин для насоса
const int windPin = 16;  // Пин для вентилятора
const int windowPin = 19;  // Пин для сервопривода (форточка)

// Объект для управления сервоприводом
Servo servoWindow;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting...");

    // Инициализация библиотеки ESP32Servo
    ESP32Servo::init();

    // Настройка GPIO как выходы
    pinMode(pumpPin, OUTPUT);
    pinMode(windPin, OUTPUT);

    // Устанавливаем GPIO в LOW
    digitalWrite(pumpPin, LOW);
    digitalWrite(windPin, LOW);

    // Подключение к Wi-Fi
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    int connectionAttempts = 0;
    const int maxConnectionAttempts = 20; // Максимум 10 секунд ожидания

    while (WiFi.status() != WL_CONNECTED && connectionAttempts < maxConnectionAttempts) {
        delay(500);
        Serial.print(".");
        connectionAttempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected.");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("");
        Serial.println("Failed to connect to WiFi.");
        return; // Выходим из setup(), если подключение не удалось
    }

    // Инициализация сервопривода
    servoWindow.attach(windowPin);  // Подключение сервопривода к пину 18
    servoWindow.write(0);           // Закрываем форточку по умолчанию

    // Запуск сервера
    server.begin();
}

void loop() {
    WiFiClient client = server.available();
    if (client) {
        Serial.println("New Client.");
        String currentLine = "";
        String request = "";

        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                Serial.write(c);
                request += c;

                if (c == '\n') {
                    if (currentLine.length() == 0) {
                        handleRequest(client, request);
                        request = "";
                        break;
                    } else {
                        currentLine = "";
                    }
                } else if (c != '\r') {
                    currentLine += c;
                }
            }
        }

        client.stop();
        Serial.println("Client disconnected.");
    }
}

void handleRequest(WiFiClient& client, const String& request) {
    // Обработка команд управления GPIO
    if (request.indexOf("/pump/on") != -1) {
        digitalWrite(pumpPin, HIGH);
        pumpState = true;
        Serial.println("Насос ВКЛ");
    } else if (request.indexOf("/pump/off") != -1) {
        digitalWrite(pumpPin, LOW);
        pumpState = false;
        Serial.println("Насос ВЫКЛ");
    } else if (request.indexOf("/wind/on") != -1) {
        digitalWrite(windPin, HIGH);
        windState = true;
        Serial.println("Вентилятор ВКЛ");
    } else if (request.indexOf("/wind/off") != -1) {
        digitalWrite(windPin, LOW);
        windState = false;
        Serial.println("Вентилятор ВЫКЛ");
    } else if (request.indexOf("/window/open") != -1) {
        servoWindow.write(90);  // Открываем форточку
        windowState = true;
        Serial.println("Форточка ОТКРЫТА");
    } else if (request.indexOf("/window/close") != -1) {
        servoWindow.write(0);  // Закрываем форточку
        windowState = false;
        Serial.println("Форточка ЗАКРЫТА");
    }

    // Отправка HTTP-ответа
    sendResponse(client);
}

void sendResponse(WiFiClient& client) {
    // Отправляем HTTP-заголовки
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();

    // Чтение HTML-страницы из page.h
    String page = PAGE;

    // Замена плейсхолдеров на актуальные значения состояний
    page.replace("%WINDOW_STATE%", windowState ? "checked" : "");
    page.replace("%PUMP_STATE%", pumpState ? "checked" : "");
    page.replace("%WIND_STATE%", windState ? "checked" : "");

    // Отправляем страницу клиенту
    client.print(page);
}
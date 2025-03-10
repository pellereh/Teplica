#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>
#include <FastLED.h>
#include <Adafruit_BME280.h> // Подключение библиотеки FastLED
#include <Wire.h>
#include <BH1750.h>
#include "page.h" // HTML-страницей

// Прототип функции FillSolidColor
void FillSolidColor(uint32_t c);

// Настройки точки доступа
const char* ap_ssid = "ESP32_AP";
const char* ap_password = "12345678";

// Состояние выходов
bool pumpState = false;
bool windState = false;
bool windowState = false;
bool lightState = false; // Добавлено состояние освещения

// Номера выводов
const int pumpPin = 17;
const int windPin = 16;
const int windowPin = 19;
const int lightPin = 18;

// Объект для управления сервоприводом
Servo servoWindow;

// Настройки FastLED
#define LED_PIN     18
#define LED_COUNT   64
#define LED_TYPE    SK6812
CRGB leds[LED_COUNT];

// Инициализация датчиков
BH1750 lightSensor; // Датчик освещенности
Adafruit_BME280 bme; // Датчик температуры, влажности и давления

// Асинхронный веб-сервер
AsyncWebServer server(80);

// Функция для преобразования шестнадцатеричного цвета в RGB с учетом GRB
uint32_t HexToRGB(String hexColor) {
    if (hexColor.startsWith("#")) {
        hexColor = hexColor.substring(1); // Убираем символ #
    }
    uint32_t red = strtol(&hexColor[0], NULL, 16);   // Красный компонент
    uint32_t green = strtol(&hexColor[2], NULL, 16); // Зеленый компонент
    uint32_t blue = strtol(&hexColor[4], NULL, 16);  // Синий компонент

    // Для FastLED нужно поменять порядок на GRB
    return (green << 16) | (red << 8) | blue;
}

// Определение функции FillSolidColor
void FillSolidColor(uint32_t c) {
    for (int i = 0; i < LED_COUNT; i++) {
        leds[i] = c;
    }
    FastLED.show();
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting...");

    // Инициализация I2C
    Wire.begin(21, 22); // GP21 - SDA, GP22 - SCL

    // Инициализация датчиков
    if (!lightSensor.begin()) {
        Serial.println("Ошибка инициализации BH1750");
    } else {
        Serial.println("BH1750 инициализирован");
    }
    if (!bme.begin(0x77)) { 
        Serial.println("Ошибка инициализации BME280");
    } else {
        Serial.println("BME280 инициализирован");
    }

    // Настройка GPIO как выходы
    pinMode(pumpPin, OUTPUT);
    pinMode(windPin, OUTPUT);
    pinMode(lightPin, OUTPUT); // Настройка пина освещения
    digitalWrite(pumpPin, LOW);
    digitalWrite(windPin, LOW);
    digitalWrite(lightPin, LOW); // По умолчанию свет выключен

    // Создание точки доступа
    if (WiFi.softAP(ap_ssid, ap_password)) {
        Serial.println("Access Point started.");
        Serial.print("Connect to SSID: ");
        Serial.println(ap_ssid);
        Serial.print("Password: ");
        Serial.println(ap_password);
        Serial.println("IP address: 192.168.4.1");
    } else {
        Serial.println("Failed to create Access Point.");
        return;
    }

    // Инициализация сервопривода
    servoWindow.attach(windowPin);
    servoWindow.write(0);

    // Инициализация FastLED
    FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(leds, LED_COUNT);
    FastLED.setBrightness(50);
    FillSolidColor(0xFFFFFF); // Теперь эта строка работает корректно

    // Настройка маршрутов сервера
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String page = PAGE;

        // Получение данных с датчиков
        float temperature = bme.readTemperature();
        float humidity = bme.readHumidity();
        float pressure = bme.readPressure() / 100.0F;
        float lux = lightSensor.readLightLevel();

        // Замена плейсхолдеров на актуальные значения
        page.replace("%TEMPERATURE%", String(temperature, 1));
        page.replace("%HUMIDITY%", String(humidity, 1));
        page.replace("%PRESSURE%", String(pressure, 1));
        page.replace("%LUX%", String(lux, 1));
        page.replace("%WINDOW_STATE%", windowState ? "checked" : "");
        page.replace("%PUMP_STATE%", pumpState ? "checked" : "");
        page.replace("%WIND_STATE%", windState ? "checked" : "");
        page.replace("%LIGHT_STATE%", lightState ? "checked" : ""); // Добавлено состояние света
        page.replace("%BRIGHTNESS%", String(FastLED.getBrightness()));

        request->send(200, "text/html", page);
    });

    server.on("/sensor/data", HTTP_GET, [](AsyncWebServerRequest *request) {
        float temperature = bme.readTemperature();
        float humidity = bme.readHumidity();
        float pressure = bme.readPressure() / 100.0F;
        float lux = lightSensor.readLightLevel();

        String jsonResponse = "{";
        jsonResponse += "\"temperature\":" + String(temperature, 1) + ",";
        jsonResponse += "\"humidity\":" + String(humidity, 1) + ",";
        jsonResponse += "\"pressure\":" + String(pressure, 1) + ",";
        jsonResponse += "\"lux\":" + String(lux, 1);
        jsonResponse += "}";

        request->send(200, "application/json", jsonResponse);
    });

    server.on("/pump/on", HTTP_GET, [](AsyncWebServerRequest *request) {
        digitalWrite(pumpPin, HIGH);
        pumpState = true;
        Serial.println("Насос ВКЛ");
        request->send(200, "text/plain", "OK");
    });

    server.on("/pump/off", HTTP_GET, [](AsyncWebServerRequest *request) {
        digitalWrite(pumpPin, LOW);
        pumpState = false;
        Serial.println("Насос ВЫКЛ");
        request->send(200, "text/plain", "OK");
    });

    server.on("/wind/on", HTTP_GET, [](AsyncWebServerRequest *request) {
        digitalWrite(windPin, HIGH);
        windState = true;
        Serial.println("Вентилятор ВКЛ");
        request->send(200, "text/plain", "OK");
    });

    server.on("/wind/off", HTTP_GET, [](AsyncWebServerRequest *request) {
        digitalWrite(windPin, LOW);
        windState = false;
        Serial.println("Вентилятор ВЫКЛ");
        request->send(200, "text/plain", "OK");
    });

    server.on("/window/open", HTTP_GET, [](AsyncWebServerRequest *request) {
        servoWindow.write(90);  // Открываем форточку
        windowState = true;
        Serial.println("Форточка ОТКРЫТА");
        request->send(200, "text/plain", "OK");
    });

    server.on("/window/close", HTTP_GET, [](AsyncWebServerRequest *request) {
        servoWindow.write(0);  // Закрываем форточку
        windowState = false;
        Serial.println("Форточка ЗАКРЫТА");
        request->send(200, "text/plain", "OK");
    });

    server.on("/light/on", HTTP_GET, [](AsyncWebServerRequest *request) {
        digitalWrite(lightPin, HIGH); // Включаем свет
        lightState = true;
        FastLED.setBrightness(255); // Максимальная яркость
        FastLED.show();
        Serial.println("Свет ВКЛ");
        request->send(200, "text/plain", "OK");
    });

    server.on("/light/off", HTTP_GET, [](AsyncWebServerRequest *request) {
        digitalWrite(lightPin, LOW); // Выключаем свет
        lightState = false;
        FastLED.setBrightness(0); // Минимальная яркость
        FastLED.show();
        Serial.println("Свет ВЫКЛ");
        request->send(200, "text/plain", "OK");
    });

    server.on("/light/brightness/", HTTP_GET, [=](AsyncWebServerRequest *request) {
        if (request->hasParam("value")) {
            AsyncWebParameter* p = request->getParam("value");
            int newBrightness = p->value().toInt();
            int constrainedBrightness = constrain(newBrightness, 0, 100);
            FastLED.setBrightness(map(constrainedBrightness, 0, 100, 0, 255));
            FastLED.show();
            Serial.print("Яркость: ");
            Serial.println(constrainedBrightness);
        }
        request->send(200, "text/plain", "OK");
    });

    server.begin();
}

void loop() {
    // Ничего не делать в основном цикле
}
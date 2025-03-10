#ifndef PAGE_H
#define PAGE_H

const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Умная теплица</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #2b19a4;
            color: #ffffff;
            margin: 0;
            padding: 0;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
            display: flex;
            flex-wrap: wrap;
            justify-content: space-between;
        }
        .block-thin-tab {
            background-color: rgb(29, 134, 146);
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            margin: 10px;
            padding: 15px;
            width: calc(33.333% - 20px);
            box-sizing: border-box;
        }
        .switch input {
            display: none;
        }
        .switch .slider {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
            background-color: #ccc;
            border-radius: 34px;
            transition: background-color 0.3s;
            cursor: pointer;
        }
        .switch .slider:before {
            content: '';
            position: absolute;
            left: 4px;
            top: 4px;
            width: 26px;
            height: 26px;
            background-color: white;
            border-radius: 50%;
            transition: left 0.3s;
        }
        .switch input:checked + .slider {
            background-color: #27ae60;
        }
        .switch input:checked + .slider:before {
            left: 30px;
        }
        .light-slider {
            width: 100%;
            margin-top: 10px;
        }
        .light-value {
            text-align: center;
            margin-top: 5px;
        }
        .button-container {
            display: flex;
            justify-content: space-around;
            margin-top: 10px;
        }
        .control-button {
            padding: 10px 20px;
            background-color: #27ae60;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            transition: background-color 0.3s;
        }
        .control-button:hover {
            background-color: #218c4e;
        }
        .sensor-data {
            font-size: 18px;
            margin-top: 10px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Умная теплица</h1>

        <!-- Форточка -->
        <div class="block-thin-tab">
            <div class="switch-container">
                <label for="window-switch">Форточка:</label>
                <label class="switch">
                    <input type="checkbox" id="window-switch" %WINDOW_STATE%>
                    <span class="slider round"></span>
                </label>
            </div>
        </div>

        <!-- Насос -->
        <div class="block-thin-tab">
            <div class="switch-container">
                <label for="pump-switch">Насос:</label>
                <label class="switch">
                    <input type="checkbox" id="pump-switch" %PUMP_STATE%>
                    <span class="slider round"></span>
                </label>
            </div>
        </div>

        <!-- Вентилятор -->
        <div class="block-thin-tab">
            <div class="switch-container">
                <label for="wind-switch">Вентилятор:</label>
                <label class="switch">
                    <input type="checkbox" id="wind-switch" %WIND_STATE%>
                    <span class="slider round"></span>
                </label>
            </div>
        </div>

        <!-- Освещение -->
        <div class="block-thin-tab">
            <div class="switch-container">
                <label for="light-switch">Освещение:</label>
                <label class="switch">
                    <input type="checkbox" id="light-switch" %LIGHT_STATE%>
                    <span class="slider round"></span>
                </label>
            </div>
            <div class="light-container">
                <label for="light-slider">Яркость:</label>
                <input type="range" id="light-slider" min="0" max="100" value="%BRIGHTNESS%" class="light-slider">
                <div class="light-value" id="light-value">%BRIGHTNESS%%</div>
            </div>
        </div>

        <!-- Датчик температуры и влажности (BME280) -->
        <div class="block-thin-tab">
            <h3>Температура и влажность</h3>
            <div class="sensor-data">
                Температура: <span id="temperature">%TEMPERATURE% °C</span><br>
                Влажность: <span id="humidity">%HUMIDITY% %</span><br>
                Давление: <span id="pressure">%PRESSURE% hPa</span>
            </div>
        </div>

        <!-- Датчик освещенности (BH1750) -->
        <div class="block-thin-tab">
            <h3>Освещенность</h3>
            <div class="sensor-data">
                Уровень света: <span id="lux">%LUX% лк</span>
            </div>
        </div>
    </div>

    <script>
        // Функция для обновления данных с датчиков
        function updateSensorData() {
            fetch('/sensor/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('temperature').textContent = data.temperature + ' °C';
                    document.getElementById('humidity').textContent = data.humidity + ' %';
                    document.getElementById('pressure').textContent = data.pressure + ' hPa';
                    document.getElementById('lux').textContent = data.lux + ' лк';
                })
                .catch(error => console.error('Ошибка при получении данных:', error));
        }

        // Обновление данных каждые 2 секунды
        setInterval(updateSensorData, 2000);

        // Управление форточкой
        const windowSwitch = document.getElementById('window-switch');
        windowSwitch.addEventListener('change', function() {
            fetch(`/window/${this.checked ? 'open' : 'close'}`);
        });

        // Управление насосом
        const pumpSwitch = document.getElementById('pump-switch');
        pumpSwitch.addEventListener('change', function() {
            fetch(`/pump/${this.checked ? 'on' : 'off'}`);
        });

        // Управление вентилятором
        const windSwitch = document.getElementById('wind-switch');
        windSwitch.addEventListener('change', function() {
            fetch(`/wind/${this.checked ? 'on' : 'off'}`);
        });

        // Управление освещением через слайдер
        const lightSlider = document.getElementById('light-slider');
        const lightValue = document.getElementById('light-value');

        lightSlider.addEventListener('input', function() {
            lightValue.textContent = `${this.value}%`;
            fetch(`/light/brightness/?value=${this.value}`);
        });

        // Управление освещением через переключатель
        const lightSwitch = document.getElementById('light-switch');
        lightSwitch.addEventListener('change', function() {
            fetch(`/light/${this.checked ? 'on' : 'off'}`);
        });
    </script>
</body>
</html>
)rawliteral";

#endif // PAGE_H
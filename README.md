# Hardware

* 

```mermaid
graph TD
    ESP32[ESP32 C3] -->|I2C: GPIO 5/6 Vcc (3v) GND| OLED[0.96 Inch OLED Display Module I2C]
    ESP32 -->|ADC1: GPIO 34| ECG[Modulo ECG AD8232]
    ESP32 -->|UART2 RX: GPIO 16| EEG[TGAM EEG Sensor]
    ESP32 -->|GPIO 12, 13, 14, 27| Bottoni[4x Pulsanti Menu]
    ESP32 -->|Wi-Fi / MQTT| HA[Home Assistant]

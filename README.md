# neuroHeart32 🧠❤️

`neuroHeart32` is an open-source, ESP32-powered biofeedback hub that integrates real-time Brainwave (EEG) and Heart Rate (ECG) tracking into your smart home ecosystem via MQTT and Home Assistant.

Designed with custom 3D-printed enclosures and lightweight data protocols, this project aims to bridge the gap between personal neuroscience, biometric monitoring, and home automation.

---

## 🚀 Features

* **Dual-Biometric Tracking:** Simultaneous processing of EEG (brainwaves) and ECG (heart activity).
* **Local OLED Display:** Real-time visual feedback on a 0.96" SSD1306 display.
* **Smart Home Integration:** Native MQTT data broadcasting, designed to auto-discover or easily map into **Home Assistant**.
* **4-Button UI:** Simple physical interface to navigate menus, change views, or trigger calibration.
* **3D-Printed Enclosure:** Compact and portable design.

---

## 🛠️ Hardware Components & Bill of Materials (BOM)

> ⚠️ **Affiliate Disclosure:** The links below are affiliate links. If you purchase components through them, I will earn a small commission at no extra cost to you. This helps support the development and maintenance of the `neuroHeart32` project! Thank you!

| Component | Description | Quality/Source Note | Buy Link |
| :--- | :--- | :--- | :--- |
| **ESP32 DevKit** | 38-Pin Development Board (ESP-WROOM-32) | Choose the version with pins pre-soldered if preferred. | [AliExpress]() |
| **TGAM EEG Kit** | NeuroSky-compatible Brainwave Sensor Module | Includes forehead and ear-clip electrodes. | [AliExpress]() |
| **AD8232 ECG** | Heart Rate Monitor Module Kit | Comes with standard 3-lead cables and disposable pads. | [AliExpress]() |
| **0.96" OLED Display** | I2C SSD1306 Serial Screen (128x64, White) | Ensure it is the 4-pin I2C version (not SPI). | [AliExpress]() |
| **Push Buttons (x4)** | 12x12mm Tactile Switches (or preferred size) | Used for menu navigation. | [AliExpress]() |

---

## 📐 Wiring & Pinout Configuration

Due to the sensitive nature of microvolt biometric signals ($\mu V$), keeping wires short and ground loops isolated is key. 

Below is the confirmed wiring map for `neuroHeart32`. 

### 🖥️ Display (I2C)
* **VCC** ➡️ 3.3V
* **GND** ➡️ GND
* **SDA** ➡️ **GPIO 5** *(Software I2C)*
* **SCL** ➡️ **GPIO 6** *(Software I2C / Confirmed stable on local devboard)*

### 🫀 Heart Rate Monitor (AD8232 ECG)
* **VCC** ➡️ 3.3V
* **GND** ➡️ GND
* **OUTPUT** ➡️ **GPIO 34** *(Analog Input - ADC1)*
* **LO-** ➡️ **GPIO 25** *(Digital Input)*
* **LO+** ➡️ **GPIO 26** *(Digital Input)*

### 🧠 Brainwave Sensor (TGAM EEG)
* **VCC** ➡️ 3.3V
* **GND** ➡️ GND
* **TX (Module)** ➡️ **GPIO 16** *(ESP32 RX2)*

### 🎛️ Navigation Buttons
All buttons use the internal ESP32 `INPUT_PULLUP` resistors. Wire one side to the GPIO and the other side to GND.
* **Button 1 (Up/Next)** ➡️ **GPIO 13**
* **Button 2 (Down/Prev)** ➡️ **GPIO 12**
* **Button 3 (Select)** ➡️ **GPIO 14**
* **Button 4 (Back/Reset)** ➡️ **GPIO 27**

---

## ⚡ Critical Hardware & Power Notes

1. **Power Isolation (Very Important):** Biometric sensors are highly susceptible to 50Hz/60Hz AC grid noise. If you power the ESP32 via a USB cable connected to a wall charger or a PC plugged into the mains, your ECG/EEG graphs will likely look noisy. **For clean medical-grade data trats, always power the device via battery** (e.g., LiPo cell with a proper 3.3V regulator).
2. **I2C Remapping:** This project overrides the default hardware I2C pins of the ESP32 to use GPIO 5 and 6. Ensure your firmware initialization includes `Wire.begin(5, 6);`.
3. **ADC Restrictions:** The AD8232 output must remain on **ADC1** (Pins 32-39). Do not remap it to ADC2, as ADC2 is programmatically disabled when the ESP32 Wi-Fi stack is active.

---

## 📂 Repository Structure

* `/firmware` : Source code for the ESP32 (Arduino IDE / PlatformIO).
* `/hardware` : Schematic diagrams and wiring guides.
* `/3d-models` : `.STL` files for printing the enclosure.

---

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.
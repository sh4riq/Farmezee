# Farmezee
This is the code for my 2025 Hackathon project. It's a complete IoT solution for smart farming that uses two ESP32s to communicate over LoRa. It monitors weather and soil conditions and sends data to a Blynk dashboard.
---

## 📋 Features

* **Long-Range Communication:** Uses LoRa (868.5 MHz) for a reliable, long-distance link between the sensor station and the base station.
* **Full Weather Monitoring:** The transmitter node reads:
    * Temperature (BME280)
    * Humidity (BME280)
    * Barometric Pressure (BME280)
    * Soil Moisture
    * Rain Detection
    * Air Quality (MQ-series sensor)
* **Smart Base Station:** The receiver node:
    * Connects to WiFi to upload all data to the **Blynk Cloud**.
    * Displays all data on a 20x4 I2C LCD.
    * Allows user to select a crop (Rice, Wheat, Maize) via push buttons.
    * Provides **crop-specific alerts** (buzzer and Blynk notification) if soil is too dry for the selected crop.
    * Provides rain alerts.
* **Blynk Dashboard:** A mobile and web dashboard shows all sensor data in real-time and stores historical data.

---

## 🛠️ Hardware

### Transmitter Station
* ESP32
* LoRa Module (AT-command based, e.g., Reyax RYLR896)
* BME280 (I2C)
* Soil Moisture Sensor (Analog)
* Rain Sensor (Digital)
* MQ-series Gas Sensor (Analog)
* Power source (e.g., battery pack)

### Receiver Station
* ESP32
* LoRa Module (AT-command based)
* 20x4 I2C LCD Display
* 4x Push Buttons (for Rice, Wheat, Maize, Manual)
* Buzzer
* Resistors

---

## 💻 Software & Libraries

* [Arduino IDE](https://www.arduino.cc/en/software)
* [Blynk Library](https://github.com/blynkkk/blynk-library)
* [Adafruit BME280 Library](https://github.com/adafruit/Adafruit_BME280_Library)
* [LiquidCrystal_I2C Library](https://github.com/johnrickman/LiquidCrystal_I2C)

---

## ⚙️ How to Use

1.  **Blynk Setup:**
    * Create a new Blynk project.
    * In the receiver code, update these lines with your credentials:
        ```cpp
        #define BLYNK_TEMPLATE_ID "..."
        #define BLYNK_TEMPLATE_NAME "..."
        #define BLYNK_AUTH_TOKEN "..."
        char ssid[] = "...";
        char pass[] = "...";
        ```
    * Set up Datastreams in Blynk for V0-V6.

2.  **Transmitter:** Upload the `base_transceiver.ino` code to the first ESP32.
3.  **Receiver:** Upload the `receiver_station.ino` code to the second ESP32.
4.  **Run:** Power on the transmitter, then the receiver. The receiver will connect to WiFi and start sending data to Blynk.

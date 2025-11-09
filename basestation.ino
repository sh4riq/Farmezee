#include <Wire.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

// LoRa Serial
HardwareSerial LoRaSerial(2); // UART2
#define LORA_RX 16 // ESP32 RX2 <- LoRa TX
#define LORA_TX 17 // ESP32 TX2 -> LoRa RX

// Sensors
#define SOIL_PIN 34
#define RAIN_PIN 32
#define MQ_PIN 36
#define PUMP_PIN 25

Adafruit_BME280 bme;
unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(SOIL_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT);
  pinMode(MQ_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  // Init LoRa module
  LoRaSerial.begin(115200, SERIAL_8N1, LORA_RX, LORA_TX);
  delay(2000);
  Serial.println("Initializing LoRa...");

  // --- CONFIGURE LORA MODULE ---
  LoRaSerial.println("AT+ADDRESS=1");      // Base address = 1
  delay(500);
  LoRaSerial.println("AT+NETWORKID=5");    // Network ID = 5
  delay(500);
  LoRaSerial.println("AT+BAND=868500000"); // Frequency = 868.5 MHz
  delay(500);
  LoRaSerial.println("AT+PARAMETER=12,7,1,4"); // (SF12,BW125,CR4/5,PL=4)
  delay(500);
  Serial.println("LoRa Configured.");

  // --- BME280 init ---
  if (!bme.begin(0x76)) {
    Serial.println("BME280 not found!");
  } else {
    Serial.println("BME280 ready.");
  }

  Serial.println("Base Station Started...");
}

void loop() {
  unsigned long now = millis();
  if (now - lastSend >= 5000) { // every 5s
    lastSend = now;

    float temp = bme.readTemperature();
    float hum = bme.readHumidity();
    float pres = bme.readPressure() / 100.0F;
    int soil = analogRead(SOIL_PIN);
    int rain = digitalRead(RAIN_PIN);
    int mq = analogRead(MQ_PIN);

    Serial.printf("Temp:%.1fC Hum:%.1f%% Pres:%.1fhPa Soil:%d Rain:%d MQ:%d\n",
                  temp, hum, pres, soil, rain, mq);

    // --- Send Data to Receiver (Address 2) ---
    String payload = String(temp, 1) + "," + String(hum, 1) + "," + String(pres, 1) + "," +
                     String(soil) + "," + String(rain) + "," + String(mq);
    String cmd = "AT+SEND=2," + String(payload.length()) + "," + payload;
    LoRaSerial.println(cmd);
  }
}

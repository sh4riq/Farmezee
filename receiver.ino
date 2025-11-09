#define BLYNK_TEMPLATE_ID "TMPL3OfdF-Orn"
#define BLYNK_TEMPLATE_NAME "SenseCAP S2120 Weather Station"
#define BLYNK_AUTH_TOKEN "Hi9jQDc0zjSHaUzalyiOweLHoLqrmjjN"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>


// WiFi credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "iPhone XIV";
char pass[] = "aaa1234y";

// --- LORA PINS ---
#define LORA_RX 16
#define LORA_TX 17
HardwareSerial LoRaSerial(2);

// --- LCD ---
LiquidCrystal_I2C lcd(0x27, 20, 4);

// --- COMPONENT PINS ---
#define BUZZER 27
#define SW_RICE 25
#define SW_WHEAT 26
#define SW_MAIZE 33
#define SW_MANUAL 14

// --- CROP & SENSOR THRESHOLDS ---
#define RICE_SOIL_DRY 2500
#define WHEAT_SOIL_DRY 2000
#define MAIZE_SOIL_DRY 2200
#define RAIN_DETECT_VALUE 0

// --- GLOBAL VARIABLES ---
int selectedCropMode = 0; // 0=Manual, 1=Rice, 2=Wheat, 3=Maize
String currentModeStr = "Manual";
int currentSoilThreshold = 0;

// --- Function to check button presses ---
void checkButtons() {
  bool buttonPressed = false;
  String newModeStr = currentModeStr;
  int newMode = selectedCropMode;

  if (digitalRead(SW_MANUAL) == LOW) {
    newMode = 0; newModeStr = "Manual"; buttonPressed = true;
  } else if (digitalRead(SW_RICE) == LOW) {
    newMode = 1; newModeStr = "Rice"; buttonPressed = true;
  } else if (digitalRead(SW_WHEAT) == LOW) {
    newMode = 2; newModeStr = "Wheat"; buttonPressed = true;
  } else if (digitalRead(SW_MAIZE) == LOW) {
    newMode = 3; newModeStr = "Maize"; buttonPressed = true;
  }

  if (buttonPressed && newMode != selectedCropMode) {
    selectedCropMode = newMode;
    currentModeStr = newModeStr;
    currentSoilThreshold = 0;

    Serial.println("Mode changed to: " + currentModeStr);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(currentModeStr + " Selected");

    if (selectedCropMode == 1) currentSoilThreshold = RICE_SOIL_DRY;
    else if (selectedCropMode == 2) currentSoilThreshold = WHEAT_SOIL_DRY;
    else if (selectedCropMode == 3) currentSoilThreshold = MAIZE_SOIL_DRY;

    if (selectedCropMode > 0) {
      lcd.setCursor(0, 1);
      lcd.print("Req. Soil >");
      lcd.print(currentSoilThreshold);
    } else {
      lcd.setCursor(0, 1);
      lcd.print("Manual Irrigation");
    }

    Blynk.virtualWrite(V6, currentModeStr);

    delay(2000);
    lcd.clear();
    lcd.print("Waiting for data...");
    lcd.setCursor(10, 2);
    lcd.print("M:");
    lcd.print(currentModeStr.substring(0, 6));
  }
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Receiver Started");

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  pinMode(SW_RICE, INPUT_PULLUP);
  pinMode(SW_WHEAT, INPUT_PULLUP);
  pinMode(SW_MAIZE, INPUT_PULLUP);
  pinMode(SW_MANUAL, INPUT_PULLUP);

  // --- Connect to WiFi and Blynk ---
  lcd.setCursor(0, 1);
  lcd.print("Connecting WiFi..");
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected.");
  Blynk.config(auth);
  Blynk.connect();

  LoRaSerial.begin(115200, SERIAL_8N1, LORA_RX, LORA_TX);
  delay(2000);
  Serial.println("Configuring LoRa Receiver...");

  LoRaSerial.println("AT+ADDRESS=2");
  delay(500);
  LoRaSerial.println("AT+NETWORKID=5");
  delay(500);
  LoRaSerial.println("AT+BAND=868500000");
  delay(500);
  LoRaSerial.println("AT+PARAMETER=12,7,1,4");
  delay(500);
  Serial.println("LoRa Receiver Ready...");

  lcd.clear();
  lcd.print("Waiting for data...");
  lcd.setCursor(10, 2);
  lcd.print("M:");
  lcd.print(currentModeStr);
}

void loop() {
  Blynk.run();
  checkButtons();

  if (LoRaSerial.available()) {
    String data = LoRaSerial.readStringUntil('\n');
    data.trim();

    if (data.startsWith("+RCV")) {
      int firstComma = data.indexOf(',');
      int secondComma = data.indexOf(',', firstComma + 1);
      String payload = data.substring(secondComma + 1);

      float temp, hum, pres;
      int soil, rain, mq;
      sscanf(payload.c_str(), "%f,%f,%f,%d,%d,%d", &temp, &hum, &pres, &soil, &rain, &mq);

      Serial.println("Data received:");
      Serial.println(payload);

      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("T:"); lcd.print(temp, 1); lcd.print("C");
      lcd.setCursor(10, 0); lcd.print("P:"); lcd.print(pres, 0); lcd.print("hPa");
      lcd.setCursor(0, 1); lcd.print("H:"); lcd.print(hum, 1); lcd.print("%");
      lcd.setCursor(10, 1); lcd.print("R:");
      lcd.print((rain == RAIN_DETECT_VALUE) ? "Yes" : "No ");
      lcd.setCursor(0, 2); lcd.print("S:"); lcd.print(soil);
      lcd.setCursor(10, 2); lcd.print("M:"); lcd.print(currentModeStr.substring(0, 3));
      lcd.setCursor(0, 3); lcd.print("MQ:"); lcd.print(mq);
      if (selectedCropMode > 0) {
        lcd.setCursor(10, 3);
        lcd.print("Req:>");
        lcd.print(currentSoilThreshold);
      }

      // --- Send to Blynk ---
      Blynk.virtualWrite(V0, temp);
      Blynk.virtualWrite(V1, hum);
      Blynk.virtualWrite(V2, pres);
      Blynk.virtualWrite(V3, soil);
      Blynk.virtualWrite(V4, rain);
      Blynk.virtualWrite(V5, mq);
      Blynk.virtualWrite(V6, currentModeStr);

      // --- Alerts ---
      if (rain == RAIN_DETECT_VALUE) {
        Serial.println("RAIN DETECTED!");
        Blynk.logEvent("rain_alert", "It is raining!");
        digitalWrite(BUZZER, HIGH);
        delay(500);
        digitalWrite(BUZZER, LOW);
      }

      if (selectedCropMode > 0 && soil > currentSoilThreshold) {
        Serial.println("SOIL IS DRY!");
        String alertMsg = "Soil dry for " + currentModeStr;
        Blynk.logEvent("soil_dry", alertMsg.c_str());
        digitalWrite(BUZZER, HIGH); delay(100); digitalWrite(BUZZER, LOW);
        delay(100);
        digitalWrite(BUZZER, HIGH); delay(100); digitalWrite(BUZZER, LOW);
      }
    }
  }
}

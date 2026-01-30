#include <ESP8266WiFi.h>
#include <time.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// -------- WIFI --------
const char* ssid = "wifi name";
const char* password = "wifi password";

// -------- TIME --------
const long gmtOffset_sec = 6 * 3600;   // Bangladesh GMT +6
const int daylightOffset_sec = 0;

// -------- PINS --------
#define SERVO_PIN D6
#define FLAME_PIN D7
#define BUZZER_PIN D8
#define FLAME_LED_PIN D4   // optional LED for flame indication

// -------- OBJECTS --------
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo feederServo;

// -------- TIMERS --------
unsigned long lastFeedTime = 0;
const unsigned long feedInterval = 10000;   // 🔬 Prototype: 1 minute
bool feederState = false;                   // ON/OFF tracker

void setup() {
  Serial.begin(115200);

  pinMode(FLAME_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FLAME_LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(FLAME_LED_PIN, LOW);

  feederServo.attach(SERVO_PIN);
  feederServo.write(0);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  // WiFi connection for time sync only
  lcd.setCursor(0,0);
  lcd.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.print(".");
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("WiFi Connected");

  // Sync time
  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
  delay(2000);
}

void loop() {
  unsigned long currentMillis = millis();

  // ---------- TIME ----------
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeStr[17];
    strftime(timeStr, sizeof(timeStr), "%I:%M:%S %p", &timeinfo);
    lcd.setCursor(0,0);
    lcd.print("Time:");
    lcd.print(timeStr);
  }

  // ---------- FLAME SENSOR ----------
  int flame = digitalRead(FLAME_PIN);
  if (flame == LOW) { // flame detected
    // Emergency buzzer: rapid beep pattern
    for (int i = 0; i < 5; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(FLAME_LED_PIN, HIGH);
      delay(100);
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(FLAME_LED_PIN, LOW);
      delay(100);
    }
    lcd.setCursor(0,1);
    lcd.print("!!! FIRE ALERT ");
  } else {
    lcd.setCursor(0,1);
    if (feederState) {
      lcd.print("Feeder ON       ");
    } else {
      lcd.print("Feeder Ready    ");
    }
  }

  // ---------- AUTO FEED (PROTOTYPE) ----------
  if (currentMillis - lastFeedTime > feedInterval) {
    lastFeedTime = currentMillis;
    feederState = true;
    feederServo.write(90);   // Open feeder
    delay(1000);              // Keep servo open
    feederServo.write(0);    // Close feeder
    feederState = false;     // Update feeder state
  }

  delay(500);
}

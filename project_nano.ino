#include "DHT.h"

// -------- RELAY PINS --------
#define FAN_RELAY_PIN 2        // Relay 1 for fan
#define CONVEYOR_RELAY_PIN 3   // Relay 2 for conveyor belt

// -------- DHT SENSOR --------
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// -------- TIMERS --------
unsigned long lastConveyorChange = 0;
bool conveyorState = false;   // OFF initially

const unsigned long conveyorOnTime = 15000;   // 2s ON
const unsigned long conveyorOffTime = 2000; // 15s OFF

void setup() {
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(CONVEYOR_RELAY_PIN, OUTPUT);

  digitalWrite(FAN_RELAY_PIN, LOW);        // Fan OFF
  digitalWrite(CONVEYOR_RELAY_PIN, LOW);   // Conveyor OFF

  dht.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  // ---------- FAN CONTROL ----------
  float temp = dht.readTemperature();
  if (!isnan(temp)) {
    if (temp < 29.0) {
      digitalWrite(FAN_RELAY_PIN, HIGH);   // Fan ON
    } else {
      digitalWrite(FAN_RELAY_PIN, LOW);    // Fan OFF
    }
  }

  // ---------- CONVEYOR CONTROL ----------
  if (conveyorState) { // Conveyor ON
    if (currentMillis - lastConveyorChange >= conveyorOnTime) {
      digitalWrite(CONVEYOR_RELAY_PIN, LOW);
      conveyorState = false;
      lastConveyorChange = currentMillis;
    }
  } else { // Conveyor OFF
    if (currentMillis - lastConveyorChange >= conveyorOffTime) {
      digitalWrite(CONVEYOR_RELAY_PIN, HIGH);
      conveyorState = true;
      lastConveyorChange = currentMillis;
    }
  }
}

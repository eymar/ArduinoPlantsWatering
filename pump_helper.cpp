#include "pump_helper.h"
#include "config.h"
#include <Arduino.h>

void setupPumpPins() {
  pinMode(WATER_CHECK_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(WATER_CHECK_ENABLE_PIN, OUTPUT);
}

bool isWaterTankNotEmpty() {
  digitalWrite(WATER_CHECK_ENABLE_PIN, HIGH);
  delay(50);
  
  bool hasWater = digitalRead(WATER_CHECK_PIN) == HIGH; // pulld-down resistor
  digitalWrite(WATER_CHECK_ENABLE_PIN, LOW);
  
  return hasWater;
}

void turnOnOffThePump(bool turnOn) {
  if (turnOn) {
    digitalWrite(PUMP_PIN, HIGH);
  } else {
    digitalWrite(PUMP_PIN, LOW);
  }
}


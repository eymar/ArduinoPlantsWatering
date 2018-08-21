#include "config.h"

#include <Time.h>
#include <TimeLib.h> // https://github.com/PaulStoffregen/Time

#include <Arduino.h>
#include "time_helper.h"

#include <stdio.h>
#include <DS1302.h>

const int kCePin   = 9;  // Chip Enable
const int kIoPin   = 8;  // Input/Output
const int kSclkPin = 7;  // Serial Clock


// Create a DS1302 object.
DS1302 rtc(kCePin, kIoPin, kSclkPin);

unsigned long currentTimeSeconds() {
  Time t = rtc.time();
  setTime(t.hr, t.min, t.sec, t.date, t.mon, t.yr);

  #ifdef DEBUG
  Serial.println("\nYear " + String(t.yr));
  Serial.println("\nMonth " + String(t.mon));
  Serial.println("\nDay " + String(t.date));
  Serial.println("\nHour " + String(t.hr));
  Serial.println("\nMinutes  " + String(t.min));
  Serial.println("\nSeconds  " + String(t.sec));
  #endif
  
  return (unsigned long) now();
}

byte setCurrentTimeInSeconds(unsigned long timestampSeconds) {
  /* Conversion to time_t as localtime() expects a time_t* */
  time_t epoch_time_as_time_t = timestampSeconds;

  int tyear = year(epoch_time_as_time_t);
  int tmonth = month(epoch_time_as_time_t);
  int tday = day(epoch_time_as_time_t);
  int thour = hour(epoch_time_as_time_t);
  int tminute = minute(epoch_time_as_time_t);
  int tseconds = second(epoch_time_as_time_t);

  #ifdef DEBUG
  Serial.println("\nYear " + String(tyear));
  Serial.println("\nMonth " + String(tmonth));
  Serial.println("\nDay " + String(tday));
  Serial.println("\nHour " + String(thour));
  Serial.println("\nMinutes  " + String(tminute));
  Serial.println("\nSeconds  " + String(tseconds));
  #endif

  rtc.writeProtect(false);
  rtc.halt(false);
  
  Time result(tyear, tmonth, tday, thour, tminute, tseconds, Time::kSunday); 
  rtc.time(result);
  
  return 1;
}


#ifndef NUM_H
#define NUM_H

#include <EEPROM.h>
#include <Arduino.h>
#include "config.h"

#define START_SCRIPTS_ADDRESS 3
#define SCRIPTS_COUNT 3

#define SCRIPT_BYTES_LEN 16

// 60, 15, 0, 555555, 0, true
// 00 00 00 3C | 00 0F | 00 00 00 00 | FF 08 7A 23 | 00 | 01
// 0x07 00  0000003C000F00000000FF087A230001

struct WateringProgram {
  unsigned long intervalSeconds;
  unsigned short durationSeconds; // 0 - 1200
  
  unsigned long lastWateringTime;
  unsigned long nextWateringTime;
  
  byte purposeId;   // общий(0) или конкретный клапан (1 - n)
  bool enabled;
};

extern WateringProgram wateringScripts[SCRIPTS_COUNT];

void initializeWateringScripts();
void saveScript(int pos, WateringProgram script);
byte editWateringProgram(WateringProgram *p, unsigned long intervalSeconds, unsigned long nextWateringTime,
      unsigned short durationSeconds, byte purposeId, bool enabled);
bool needToExecute(WateringProgram * script);
byte tryToExecuteProgram(WateringProgram * script);

void printScript(WateringProgram script);
void readWateringScripts(WateringProgram buf[]);
bool checkIsInitialized();
void initializeEEPROM();

void cpyWateringProgramToBuf(WateringProgram p, byte buf[]);
WateringProgram convertBufToWateringProgram(byte buf[], WateringProgram p);
void cpyAllScriptsToBuf(byte buf[]);

#endif 

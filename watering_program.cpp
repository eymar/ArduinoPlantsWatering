#include "watering_program.h"
#include "time_helper.h"
#include "pump_helper.h"

#include "convertors.h"

WateringProgram wateringScripts[SCRIPTS_COUNT];

void readWateringScripts(WateringProgram buf[]) {
  int startAddress = START_SCRIPTS_ADDRESS;

  int structureSize = sizeof(WateringProgram);
  for (int i = 0; i < SCRIPTS_COUNT; i++) {
    EEPROM.get(startAddress, buf[i]);
    startAddress += structureSize;
  }
}

void initializeWateringScripts() {
  if (!checkIsInitialized()) {
    initializeEEPROM();
  }
  
  readWateringScripts(wateringScripts);
}

void saveScript(int pos, WateringProgram script) {
  int address = START_SCRIPTS_ADDRESS + pos * sizeof(WateringProgram);
  EEPROM.put(address, script);
}

byte editWateringProgram(WateringProgram *p, unsigned long intervalSeconds, unsigned long nextWateringTime,
      unsigned short durationSeconds, byte purposeId, bool enabled) {
        if (intervalSeconds < MIN_INTERVAL_SECONDS or intervalSeconds < durationSeconds) {
          return 2; // invalid interval
        }
        
        if (durationSeconds > MAX_WATERING_DURATION) {
          return 3; // invalid duration
        }
        
        if (purposeId > SCRIPTS_COUNT - 1) {
          return 4; // invalid pump id
        }

        if (nextWateringTime < currentTimeSeconds()) {
          return 5; // next watering time can't be in the past
        }
        
        p->intervalSeconds = intervalSeconds;
        p->durationSeconds = durationSeconds;

        p->purposeId = purposeId;
        p->enabled = enabled;

        p->lastWateringTime = 0;
        p->nextWateringTime = nextWateringTime;
        
        return 1;
}

bool needToExecute(WateringProgram script) {
  if (!script.enabled || script.durationSeconds < 1) {
    return false;
  }

  return currentTimeSeconds() > script.nextWateringTime;
}

byte tryToExecuteProgram(WateringProgram * script) {
  if (!needToExecute(*script)) {
    return -2; // Этот скрипт выключен либо его время еще не пришло
  }
  
  if (!isWaterTankNotEmpty()) {
    return -1; // Не может полить, так как нет воды в баке
  }

  turnOnOffThePump(true);
  bool runOutOfWater = false;
  
  int start = currentTimeSeconds();
  int t = start;
  while (t - start < script -> durationSeconds) {
      delay(1000);
      runOutOfWater = !isWaterTankNotEmpty();
      
      if (runOutOfWater) {
        break;
      }

      t = currentTimeSeconds();
  }
  
  turnOnOffThePump(false);
  script->lastWateringTime = start;
  script->nextWateringTime = script->nextWateringTime + script->intervalSeconds;

  if (runOutOfWater) {
    return 0; // Полив начался, но был прерван так как закончилась вода в емкости
  }

  return 1; // Полив успешно выполнен
}

void printScript(WateringProgram script) {
  char buf[50];
  snprintf(buf, sizeof(buf), "%lu %hu %d %lu %lu %d",
    script.intervalSeconds, script.durationSeconds, script.purposeId, script.lastWateringTime, script.nextWateringTime, script.enabled);
  Serial.print("Script str: ");
  Serial.println(buf);
}

bool checkIsInitialized() {
  bool isInitialized = true;
  
  for (int i = 0; i < 3; i++) {
    byte header = EEPROM.read(i);
    isInitialized = (header == 0xFF);
    if (!isInitialized) break;
  }

  return isInitialized;
}

void initializeEEPROM() {
  // Write 0xFFFFFF header to memory (determines that memory is initialized)
  for (int i = 0; i < START_SCRIPTS_ADDRESS; i++) {
      EEPROM.write(i, 0xFF);
    }

    int startAddress = START_SCRIPTS_ADDRESS;
    
    WateringProgram empty = {
      0, 0, 0, 0, 0, false
    };

    int sizeOfStructure = sizeof(WateringProgram);
    
    // Write empty scripts to memory
    for (int i = 0; i < SCRIPTS_COUNT; i++) {
         EEPROM.put(startAddress, empty);
         startAddress += sizeOfStructure;
    }
}

void cpyWateringProgramToBuf(WateringProgram p, byte buf[]) {
  writeULongToBuf(buf, p.intervalSeconds);
  writeUShortToBuf(buf + 4, p.durationSeconds);
  writeULongToBuf(buf + 6, p.lastWateringTime);
  writeULongToBuf(buf + 10, p.nextWateringTime);
  buf[14] = p.purposeId;
  buf[15] = (byte) p.enabled; 
}

WateringProgram convertBufToWateringProgram(byte buf[], WateringProgram p) {    
  p.intervalSeconds = uLongFromBuf(buf);
  p.durationSeconds = uShortFromBuf(buf+4);
  p.lastWateringTime = uLongFromBuf(buf+6);
  p.nextWateringTime = uLongFromBuf(buf+10);
  p.purposeId = buf[14];
  p.enabled = (bool) buf[15];
  return p;
}

void cpyAllScriptsToBuf(byte buf[]) {
  byte offset = 0;
  for (int i = 0; i < SCRIPTS_COUNT; i++) {
    cpyWateringProgramToBuf(wateringScripts[i], buf + offset);
    offset += SCRIPT_BYTES_LEN;
  }
}


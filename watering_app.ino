#include "config.h"
#include "watering_program.h"
#include "pump_helper.h"
#include "time_helper.h"
#include "convertors.h"

bool isWaterTankEmpty = false;

WateringProgram oneTimeScript = {0, 0, 0, 0, 0, false};

byte responseBuf[64];

void pepareResponseBuf() {
  for (int i = 0; i < 64; i++) {
    responseBuf[i] = 0x00;
  }
}

void setup() {    
  setupPumpPins();
  
  Serial.begin(9600);
  Serial.setTimeout(1000);
  while (!Serial) {
    ;
  }

  initializeWateringScripts();
  pinMode(13, OUTPUT);
}


void loop() {
  if (Serial.available()) {
    digitalWrite(13, HIGH);
    byte buf[20];
    for (int i = 0; i < 20; i++) {
      buf[i] = 0;
    }
    Serial.readBytes(buf, 20); 
    processUserCommand(buf);
  } else {
    digitalWrite(13, LOW);
  }
  
  tryToExecuteOneTimeScript();
  
//  for (int i = 0; i < SCRIPTS_COUNT; i++) {
//    WateringProgram * p = &(wateringScripts[i]);
//    
//    if (needToExecute(p)) {
//      byte result = tryToExecuteProgram(p);
//      isWaterTankEmpty = result == -1 or result == 0;
//    }
//  }
}

void tryToExecuteOneTimeScript() {
  if (needToExecute(&oneTimeScript)) {
    byte result = tryToExecuteProgram(&oneTimeScript);
    isWaterTankEmpty = result == -1 or result == 0;
    oneTimeScript.enabled = false;
    oneTimeScript.durationSeconds = 0;
  }
}

const byte CMD_SET_CURRENT_TIME = 0x05;
const byte CMD_GET_CURRENT_TIME = 0x06;
const byte CMD_SET_PROGRAM = 0x07;
const byte CMD_GET_PROGRAMS = 0x08;
const byte CMD_SET_TEST_PROGRAM = 0x09;
const byte CMD_GET_WATER_STATUS = 0x0A;

void sendCommandResult(byte commandResult[]) {
  int len = 64;
  Serial.write(commandResult, len);  
//  for (int i = 0; i < len; i++) {
//    Serial.print(String(commandResult[i], HEX) + " "); 
//  }
}

void processUserCommand(byte command[]) {
  byte sizeOfCommand = sizeof(command);
  if (sizeOfCommand == 0) {
    return;
  }
  
  byte cmd = command[0];
  switch (cmd) {
    case CMD_SET_CURRENT_TIME:
      #ifdef DEBUG
      Serial.println("\nSet c time");
      #endif
      
      performCmdSetCurrentTime(command);
      break;

     case CMD_GET_CURRENT_TIME:
      #ifdef DEBUG
      Serial.println("\nGet c time");
      #endif
      
      performCmdGetCurrentTime();
      break;

     case CMD_SET_PROGRAM:
      #ifdef DEBUG
      Serial.println("\nSet program " + String(command[1]));
      #endif
      performCmdSetProgram(command);
      break;

     case CMD_GET_PROGRAMS:
      performCmdGetPrograms();
      break;

     case CMD_SET_TEST_PROGRAM:
      performCmdSetTestProrgram(command);
      break;

     case CMD_GET_WATER_STATUS:
      pepareResponseBuf();
      responseBuf[0] = CMD_GET_WATER_STATUS;
      responseBuf[1] = isWaterTankNotEmpty();
      sendCommandResult(responseBuf);
      break;
  }
}

void performCmdSetProgram(byte data[]) {
  byte num = data[1]; // max = 2 (0, 1, 2)
  WateringProgram p = {0, 0, 0, 0, 0, false};
  
  p = convertBufToWateringProgram(data + 2, p);
  #ifdef DEBUG
  printScript(p);
  #endif

  int res = editWateringProgram(&(wateringScripts[num]), p.intervalSeconds, p.nextWateringTime, p.durationSeconds, p.purposeId, p.enabled);
  saveScript(num, wateringScripts[num]);

  pepareResponseBuf();
  responseBuf[0] = CMD_SET_PROGRAM;
  responseBuf[1] = res;
  sendCommandResult(responseBuf);
}

void performCmdSetTestProrgram(byte data[]) {
  WateringProgram p = {0, 0, 0, 0, 0, false};

  oneTimeScript = convertBufToWateringProgram(data + 1, p);
  #ifdef DEBUG
  printScript(oneTimeScript);
  #endif

  pepareResponseBuf();
  responseBuf[0] = CMD_SET_TEST_PROGRAM;
  responseBuf[1] = oneTimeScript.enabled and oneTimeScript.durationSeconds > 0;
  sendCommandResult(responseBuf);
}

void performCmdGetPrograms() {
  pepareResponseBuf();
  responseBuf[0] = CMD_GET_PROGRAMS;
  responseBuf[1] = 1;
  responseBuf[2] = SCRIPTS_COUNT;

  #ifdef DEBUG
  for (int i = 0; i < SCRIPTS_COUNT; i++) {
    printScript(wateringScripts[i]);
  }
  #endif

  cpyAllScriptsToBuf(responseBuf + 3);

  sendCommandResult(responseBuf);
}

void performCmdSetCurrentTime(byte data[]) {
  unsigned long t = uLongFromBuf(data + 1);
  
  #ifdef DEBUG
  Serial.println("\nInput: " + String(t));
  #endif

  setCurrentTimeInSeconds(t);
  
  pepareResponseBuf();
  responseBuf[0] = CMD_SET_CURRENT_TIME;
  responseBuf[1] = 1;
  sendCommandResult(responseBuf);
}

void performCmdGetCurrentTime() {
  unsigned long t = currentTimeSeconds();
  pepareResponseBuf();
  
  #ifdef DEBUG
  Serial.println("\nTime: " + String(t));
  #endif
 
  responseBuf[0] = CMD_GET_CURRENT_TIME;
  responseBuf[1] = 1;
  writeULongToBuf(responseBuf + 2, t);
  
  sendCommandResult(responseBuf);
}


//#define debug

#define buttonTime 500
#define scrollTime 20

#define debounceTime 20
#define dmxFailTime 500

unsigned long lastPacket = dmxFailTime;

#ifndef debug
#include "DMXSerial.h"        // norādam izmantojamo bibliotēku
#endif

#include <EEPROM.h>
#include "Button.h"
#include "MCP_Display.h"
#include "reverse_cie1931.h"

unsigned long scrollNext;
uint16_t dmxaddr = 1;
uint16_t trueDmxAddr = 1;
uint8_t modeSet = 0;

uint8_t preHeat;
uint8_t truePreHeat;

Button modeBut = Button(3, false, true, debounceTime);
Button upBut = Button(2, false, true, debounceTime);
Button downBut = Button(1, false, true, debounceTime);
Button enterBut = Button(0, false, true, debounceTime);

void setup() {
#ifdef debug
  Serial.begin(9600);
  Serial.println("BOOT");
#endif

#ifndef debug
  DMXSerial.init(DMXReceiver);            // pasakam, ka no bibliotekas izmantosim RECIEVER
#endif

  setupDipslay();
  EEPROM.get(0, trueDmxAddr);
  trueDmxAddr = constrain(trueDmxAddr, 1, 512);

  EEPROM.get(2, truePreHeat);
  //truePreheat = constrain(truePreHeat, 0, 255);
}

uint8_t rawButtons;
uint8_t dmxOk;
int value;
void loop() {
#ifndef debug
  lastPacket = DMXSerial.noDataSince();
#endif

  dmxOk = lastPacket < dmxFailTime ? true : false;

  setLeds();
  rawButtons = doDigit(value, dmxOk, modeSet);

  modeBut.read(rawButtons);
  upBut.read(rawButtons);
  downBut.read(rawButtons);
  enterBut.read(rawButtons);

  if (modeBut.wasPressed()) {
    switch (modeSet) {
      case 0:
        modeSet=1;        
        break;        
      case 1:
        modeSet=2;
        value=preHeat;
        break;
      case 2:
        modeSet=0;
        value=trueDmxAddr;
        break;
      default:
        break;
    }
    //if (modeSet) modeSet = false;
    //else modeSet = true;
  }

  if (modeSet==0){
    value=trueDmxAddr;
  }
  if (modeSet==1) {
    value=dmxaddr;
    if ( downBut.pressedFor(buttonTime) && (millis() >= scrollNext)) {
      dmxaddr--;
      //delay(buttonTime);
      scrollNext = millis() + scrollTime;
    }
    if (upBut.pressedFor(buttonTime) && (millis() >= scrollNext)) {
      dmxaddr++;
      //delay(buttonTime);
      scrollNext = millis() + scrollTime;
    }

    if (downBut.wasPressed()) dmxaddr--;
    if (upBut.wasPressed()) dmxaddr++;

    if (enterBut.wasPressed()) {
      trueDmxAddr = dmxaddr;
      modeSet = 0;
      EEPROM.put(0, trueDmxAddr);
    }
  } else {
    dmxaddr = trueDmxAddr;
  }

  if (dmxaddr < 1) dmxaddr = 512;
  if (dmxaddr > 512) dmxaddr = 1;

  if (modeSet==2){
    value=preHeat;
    if ( downBut.pressedFor(buttonTime) && (millis() >= scrollNext)) {
      preHeat--;
      //delay(buttonTime);
      scrollNext = millis() + scrollTime;
    }
    if (upBut.pressedFor(buttonTime) && (millis() >= scrollNext)) {
      preHeat++;
      //delay(buttonTime);
      scrollNext = millis() + scrollTime;
    }

    if (downBut.wasPressed()) preHeat--;
    if (upBut.wasPressed()) preHeat++;

    if (enterBut.wasPressed()) {
      truePreHeat = preHeat;
      modeSet = 0;
      EEPROM.put(2, truePreHeat);
    }
  } else {
    preHeat = truePreHeat;
  }
    
  //if (preHeat < 0) preHeat = 255;
  //if (preHeat > 255) preHeat = 0;

}

#ifdef debug
int dir = 1;
int fade = 0;
#endif

uint8_t ledState[] = {255, 255, 255, 255};
void setLeds() {
  uint8_t ledTmp;

#ifdef debug
  if (fade >= 255) dir = -1;
  if (fade <= 0) dir = 1;
  fade += dir;
#endif

  for (uint8_t i = 0; i < 4; i++) {
    if ((trueDmxAddr + i) <= 512) {

#ifndef debug
      ledTmp = DMXSerial.read(trueDmxAddr + i);
      if (ledTmp<truePreHeat) ledTmp=truePreHeat;
#endif

#ifdef debug
      ledTmp = fade;
      //delay(2);
#endif

    }
    if (ledTmp != ledState[i]) {
      ledState[i] = ledTmp;
      analogWrite(ledPin[i], cie[ledTmp]);
    }
  }

}


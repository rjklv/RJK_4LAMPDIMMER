
//#define debug

#define whiteMax 230          //maximum intensity for ledPin4 (white channel)

#define buttonTime 500        //timount for scroll start in milliseconds
#define scrollTime 20         //scroll speed in milliseconds (some magic used)
#define blankTime 30 * 1000   //display blank time in milliseconds

#define debounceTime 30       //button bouncing timeout in milliseconds
#define dmxFailTime 500       //timeout for DXM failure detect in milliseconds

// avrdude command
// avrdude -p m328p -c arduino -P com22 -b 19200 -U lfuse:w:0xFF:m -U hfuse:w:0xD7:m -U efuse:w:0xFD:m
// avrdude -p m328p -c arduino -P com22 -b 19200 -U flash:w:firmware.hex:m

// LED PWM output pins
#define ledPin0 10
#define ledPin1 9
#define ledPin2 6
#define ledPin3 5

// button input pins
#define modePin A1
#define upPin A0
#define downPin 12
#define enterPin 11

unsigned long lastPacket = dmxFailTime;

#include <DMXSerial.h>
#include <EEPROM.h>
#include "Button.h"
#include "SCT_Display.h"
#include "cie1931.h"

const uint8_t ledPin[] = {ledPin0, ledPin1, ledPin2, ledPin3};    //led assing magic
uint8_t ledState[] = {0, 0, 0, 0};    //led state on startup

unsigned long scrollNext;
unsigned long blankNext;

uint16_t dmxaddr = 1;
uint16_t trueDmxAddr = 1;
uint8_t modeSet = 0;

uint8_t preHeat;
uint8_t truePreHeat;

Button modeBut = Button(modePin, true, true, debounceTime);
Button upBut = Button(upPin, true, true, debounceTime);
Button downBut = Button(downPin, true, true, debounceTime);
Button enterBut = Button(enterPin, true, true, debounceTime);

void setup() {
  DMXSerial.init(DMXReceiver);            // pasakam, ka no bibliotekas izmantosim RECIEVER
  setupDisplay();
  EEPROM.get(0, trueDmxAddr);
  trueDmxAddr = constrain(trueDmxAddr, 1, 512);

  EEPROM.get(2, truePreHeat);
  //generateCIE1931(truePreHeat);
  //generateCIE1931(0);
  blankNext = millis() + blankTime;
}

uint8_t rawButtons;
uint8_t dmxOk;
int value;

void loop() {

  lastPacket = DMXSerial.noDataSince();
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
        modeSet = 1;
        break;
      case 1:
        modeSet = 0;
        value = trueDmxAddr;
        break;
      default:
        break;
    }
  }

  if (modeSet == 0) {
    if (millis() >= blankNext) {
      value = 1000;
    } else {
      value = trueDmxAddr;
    }
  } else {
    blankNext = millis() + blankTime;
  }

  if (modeSet == 1) {
    value = dmxaddr;
    if ( downBut.pressedFor(buttonTime) && (millis() >= scrollNext)) {
      dmxaddr--;
      scrollNext = millis() + scrollTime;
    }
    if (upBut.pressedFor(buttonTime) && (millis() >= scrollNext)) {
      dmxaddr++;
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
}

uint8_t ledTmp;

void setLeds() {
  for (uint8_t i = 0; i < 4; i++) {
    if ((trueDmxAddr + i) <= 512) {
      ledTmp = DMXSerial.read(trueDmxAddr + i);      
    }
    if (ledTmp != ledState[i]) {      
      if (i==3){
        ledTmp=map(ledTmp,0,255,0,whiteMax);
      }
      ledState[i] = ledTmp;
      analogWrite(ledPin[i], cieTable(ledTmp));

    }
  }
}


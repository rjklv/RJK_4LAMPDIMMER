//#define debug

#define whiteMax 255          //maximum intensity for ledPin4 (white channel)

#define blankTime 30 * 1000   //display blank time in milliseconds
#define buttonTime 500        //timount for scroll start in milliseconds
#define scrollTime 20         //scroll speed in milliseconds (some magic used)

#define debounceTime 30       //button bouncing timeout in milliseconds
#define dmxFailTime 500       //timeout for DXM failure detect in milliseconds

//#define pwmFreq 200           // Default is 200Hz, supports 24Hz to 1526Hz
// avrdude command
// avrdude -p m328p -c arduino -P com3 -b 19200 -U lfuse:w:0xC7:m -U hfuse:w:0xD3:m -U efuse:w:0xFD:m
// avrdude -p m328p -c arduino -P com3 -b 19200 -U flash:w:firmware.hex


// LED PWM output pins
#define ledPin0 10
#define ledPin1 9
#define ledPin2 8
#define ledPin3 7
#define ledPin4 6
#define ledPin5 5

// button input pins for production version
#define modePin1 11
#define upPin2 12
#define downPin3 A0
#define enterPin4 A1

// button input pins for 1st test version
//#define modePin1 A1
//#define upPin2 A0
//#define downPin3 12
//#define enterPin4 11

unsigned long lastPacket = dmxFailTime;

#include "DMXSerial.h"
#include <EEPROM.h>
#include "Button.h"
#include "SCT_Display.h"

const uint8_t ledPin[] = {ledPin0, ledPin1, ledPin2, ledPin3, ledPin4, ledPin5};    //led assing magic

//uint8_t ledState[] = {0, 0, 0, 0};    //led state on startup

unsigned long scrollNext;
unsigned long blankNext;

uint16_t dmxaddr = 1;
uint16_t trueDmxAddr = 1;
//uint8_t dmxMode = 0;
uint8_t blankScreen=false;

uint8_t dig3=10,dig2=10,dig1=10,dig0=10;

uint8_t testState[]={0,0,0,0,0,0};

Button modeBut = Button(modePin1, true, true, debounceTime);
Button upBut = Button(upPin2, true, true, debounceTime);
Button downBut = Button(downPin3, true, true, debounceTime);
Button enterBut = Button(enterPin4, true, true, debounceTime);

void numberToDigits(int indata){
    int tmp = indata - (indata % 100);
    dig2 = tmp / 100;
    tmp = indata - (indata % 10) - (dig2 * 100);
    dig1 = tmp / 10;
    dig0 = indata - (dig2 * 100) - (dig1 * 10);  
}
void setup() {
  for (uint8_t ii=0;ii<6;ii++){
    pinMode(ledPin[ii],OUTPUT);
  }
  DMXSerial.init(DMXReceiver);            // pasakam, ka no bibliotekas izmantosim RECIEVER
  setupDisplay();
  EEPROM.get(0, trueDmxAddr);
  trueDmxAddr = constrain(trueDmxAddr, 1, 512);
  
  blankNext = millis() + blankTime;
}


enum _menuState{
  work=0,
    menuAddress=1,
      menuAddressSet=2,
    menuTest=3,
      menuTest0=4,
      menuTest1=5,
      menuTest2=6,
      menuTest3=7,
      menuTest4=8,
      menuTest5=9,
    menuExit=10,
  sleep=11,
};

uint8_t menuState=work;

uint8_t tmpColor;

uint8_t menuStateMachine(void){
  switch (menuState){
    case sleep:
      dig0=10;
      dig1=10;
      dig2=10;
      dig3=10;
    case work:
      if (blankScreen==true){
        return;
      }
      if (modeBut.wasPressed()) {
        menuState=menuAddress;      
        return;
      }
      if (enterBut.wasPressed()) {
        menuState=work;
        return;
      }
//      if (dmxMode){
        dig3=10;
        numberToDigits(trueDmxAddr);        
      //} //else {
        //dig3='Q'-54;
        //dig2='Q'-54;
        //dig1='Q'-54;
        //dig0='Q'-54;        
      //}
      return;
//***************************************************************************            
    case menuAddress:
      if (modeBut.wasPressed()) {
        menuState=work;
        return;
      }
      if (enterBut.wasPressed()) {
        menuState=menuAddressSet;
        dmxaddr=trueDmxAddr;
        return;
      }
      if (upBut.wasPressed()) {
        menuState=menuExit;
        return;
      }
      if (downBut.wasPressed()) {
        menuState=menuTest;
        return;
      }

      dig3='A'-54;
      dig2='D'-54;
      dig1='D'-54;
      dig0='R'-54;
      return;
//***************************************************************************      
    case menuAddressSet:
      if (modeBut.wasPressed()) {
        menuState=work;
        return;
      }
      if (enterBut.wasPressed()) {
        menuState=menuAddress;
        trueDmxAddr = dmxaddr;
        EEPROM.put(0, trueDmxAddr);
        return;
      }                
      if (upBut.wasPressed()) {
        // dmx addr increment
        dmxaddr++;
        if (dmxaddr < 1) dmxaddr = 512;
        if (dmxaddr > 512) dmxaddr = 1;
        dig3='A'-54;
        numberToDigits(dmxaddr);
        return;
      }
      if (downBut.wasPressed()) {
        // dmx addr decrement
        dmxaddr--;
        if (dmxaddr < 1) dmxaddr = 512;
        if (dmxaddr > 512) dmxaddr = 1;
        dig3='A'-54;
        numberToDigits(dmxaddr);
        return;
      }
    if ( upBut.pressedFor(buttonTime) && (millis() >= scrollNext)) {
      dmxaddr++;
      scrollNext = millis() + scrollTime;
      if (dmxaddr < 1) dmxaddr = 512;
      if (dmxaddr > 512) dmxaddr = 1;
      dig3='A'-54;
      numberToDigits(dmxaddr);
    }

    if ( downBut.pressedFor(buttonTime) && (millis() >= scrollNext)) {
      dmxaddr--;
      scrollNext = millis() + scrollTime;
      if (dmxaddr < 1) dmxaddr = 512;
      if (dmxaddr > 512) dmxaddr = 1;
      dig3='A'-54;
      numberToDigits(dmxaddr);

    }

      dig3='A'-54;
      numberToDigits(dmxaddr);
      return;
//***************************************************************************      
    case menuTest:
      if (modeBut.wasPressed()) {
        menuState=work;
        return;
      }
      if (enterBut.wasPressed()) {
        menuState=menuTest0;
        tmpColor=testState[0];
        break;
      }
      if (upBut.wasPressed()) {
        menuState=menuAddress;
        return;
      }
      if (downBut.wasPressed()) {
        menuState=menuExit;
        return;
      }

      dig3='T'-54;
      dig2='E'-54;
      dig1='S'-54;
      dig0='T'-54;
      return;

//***************************************************************************      
    case menuTest0:
      if (modeBut.wasPressed()) {
        menuState=work;
        return;
      }
      if (enterBut.wasPressed()) {
        menuState=menuTest1;
        testState[0]= tmpColor;
        tmpColor=testState[1];       
        return;
      }                
      if (upBut.wasPressed()) {
        if (tmpColor) {
          tmpColor=0;
        } else {
          tmpColor=1;
        }
        testState[0]= tmpColor;
        dig3='R'-54;
        dig2=1;
        dig0=tmpColor;
        return;
      }
      if (downBut.wasPressed()) {
        if (tmpColor) {
          tmpColor=0;
        } else {
          tmpColor=1;
        }
        testState[0]= tmpColor;
        dig3='R'-54;
        dig2=1;
        dig0=tmpColor;
        return;
      }
      dig3='R'-54;
      dig2=1;
      dig1=10;
      dig0=tmpColor;
      return;

//***************************************************************************      
    case menuTest1:
      if (modeBut.wasPressed()) {
        menuState=work;
        return;
      }
      if (enterBut.wasPressed()) {
        menuState=menuTest2;
        testState[1]= tmpColor;
        tmpColor=testState[2];       
        return;
      }                
      if (upBut.wasPressed()) {
        if (tmpColor) {
          tmpColor=0;
        } else {
          tmpColor=1;
        }
        testState[1]= tmpColor;
        dig3='R'-54;
        dig2=2;
        dig0=tmpColor;
        return;
      }
      if (downBut.wasPressed()) {
        if (tmpColor) {
          tmpColor=0;
        } else {
          tmpColor=1;
        }
        testState[1]= tmpColor;
        dig3='R'-54;
        dig2=2;
        dig0=tmpColor;
        return;
      }

      dig3='R'-54;
      dig2=2;
      dig1=10;
      dig0=tmpColor;
      return;

//***************************************************************************      
    case menuTest2:
      if (modeBut.wasPressed()) {
        menuState=work;
        return;
      }
      if (enterBut.wasPressed()) {
        menuState=menuTest3;
        testState[2]= tmpColor;
        tmpColor=testState[3];       
        return;
      }                
      if (upBut.wasPressed()) {
        if (tmpColor) {
          tmpColor=0;
        } else {
          tmpColor=1;
        }
        testState[2]= tmpColor;
        dig3='R'-54;
        dig2=3;
        dig0=tmpColor;
        return;
      }
      if (downBut.wasPressed()) {
        if (tmpColor) {
          tmpColor=0;
        } else {
          tmpColor=1;
        }
        testState[2]= tmpColor;
        dig3='R'-54;
        dig2=3;
        dig0=tmpColor;
        return;
      }
      
      dig3='R'-54;
      dig2=3;
      dig1=10;
      dig0=tmpColor;
      return;


//***************************************************************************      
    case menuTest3:
      if (modeBut.wasPressed()) {
        menuState=work;
        return;
      }
      if (enterBut.wasPressed()) {
        menuState=menuTest4;
        testState[3]= tmpColor;
        tmpColor=testState[4];       
        return;
      }                
      if (upBut.wasPressed()) {
        if (tmpColor) {
          tmpColor=0;
        } else {
          tmpColor=1;
        }
        testState[3]= tmpColor;
        dig3='R'-54;
        dig2=3;
        dig0=tmpColor;
        return;
      }
      if (downBut.wasPressed()) {
        if (tmpColor) {
          tmpColor=0;
        } else {
          tmpColor=1;
        }
        testState[3]= tmpColor;
        dig3='R'-54;
        dig2=3;
        dig0=tmpColor;
        return;
      }
      
      dig3='R'-54;
      dig2=4;
      dig1=10;
      dig0=tmpColor;
      return;


//*************************************************************************** 
    case menuTest4:
      if (modeBut.wasPressed()) {
        menuState=work;
        return;
      }
      if (enterBut.wasPressed()) {
        menuState=menuTest5;
        testState[4]= tmpColor;
        tmpColor=testState[5];       
        return;
      }                
      if (upBut.wasPressed()) {
        if (tmpColor) {
          tmpColor=0;
        } else {
          tmpColor=1;
        }
        testState[4]= tmpColor;
        dig3='R'-54;
        dig2=3;
        dig0=tmpColor;
        return;
      }
      if (downBut.wasPressed()) {
        if (tmpColor) {
          tmpColor=0;
        } else {
          tmpColor=1;
        }
        testState[4]= tmpColor;
        dig3='R'-54;
        dig2=3;
        dig0=tmpColor;
        return;
      }
      
      dig3='R'-54;
      dig2=5;
      dig1=10;
      dig0=tmpColor;
      return;


//*************************************************************************** 
    case menuTest5:
      if (modeBut.wasPressed()) {
        menuState=work;
        return;
      }
      if (enterBut.wasPressed()) {
        menuState=menuTest;
        testState[5]= tmpColor;       
        return;
      }                
      if (upBut.wasPressed()) {
        if (tmpColor) {
          tmpColor=0;
        } else {
          tmpColor=1;
        }
        testState[5]= tmpColor;
        dig3='R'-54;
        dig2=6;
        dig0=tmpColor;
        return;
      }
      if (downBut.wasPressed()) {
        //tmpColor;
        if (tmpColor) {
          tmpColor=0;
        } else {
          tmpColor=1;
        }
        testState[5]= tmpColor;
        dig3='R'-54;
        dig2=6;
        dig0=tmpColor;
        return;
      }

      dig3='R'-54;
      dig2=6;
      dig1=10;
      dig0=tmpColor;
      return;


//***************************************************************************
    case menuExit:
      if (modeBut.wasPressed()) {
        menuState=work;
        return;
      }
      if (enterBut.wasPressed()) {
        menuState=work;
        return;
      }
      if (upBut.wasPressed()) {
        menuState=menuTest;
        return;
      }
      if (downBut.wasPressed()) {
        menuState=menuAddress;
        return;
      }

      dig3='E'-54;
      dig2='X'-54;
      dig1='I'-54;
      dig0='T'-54;      
      return;
    default:
      return;  
  }
  
}
uint8_t rawButtons;
uint8_t dmxOk;
int value;

void loop() {

  lastPacket = DMXSerial.noDataSince();
  dmxOk = lastPacket < dmxFailTime ? true : false;

  setLeds();
  //rawButtons = doDigit(value, dmxOk, modeSet);
  
  //rawButtons = doDigit('E'-54,'X'-54,'I'-54,'T'-54,dmxOk);

  //rawButtons = doDigit('W'-54,2,5,5,dmxOk);
  rawButtons = doDigit(dig3,dig2,dig1,dig0,dmxOk);

  modeBut.read(rawButtons);
  upBut.read(rawButtons);
  downBut.read(rawButtons);
  enterBut.read(rawButtons);

  menuStateMachine();
  
  if ( ( menuState!=sleep ) && ( enterBut.isChanged() || downBut.isChanged() || upBut.isChanged() ) ) {
    //menuState=work;
    blankNext = millis() + blankTime;
    blankScreen=false;
  }
  
  if ( modeBut.isChanged()) {
    if ( menuState==sleep ) {
      menuState=work;
    }
    blankNext = millis() + blankTime;
    blankScreen=false;
  } else {    
    if (millis() >= blankNext) {
      //doBlankScreen
      menuState=sleep;
      blankScreen=true;
      dig0=10;
      dig1=10;
      dig2=10;
      dig3=10;
    }
  }
}

uint8_t ledTmp;

void setLeds() {

    for (uint8_t i = 0; i < 6; i++) {
        if ((trueDmxAddr + i) <= 512) {
          ledTmp = DMXSerial.read(trueDmxAddr + i);
        }
        if (testState[i]){
          ledTmp=255;
        }
      if (ledTmp > 127) {
        digitalWrite(ledPin[i],HIGH);
      } else {
        digitalWrite(ledPin[i],LOW);
      }
    } 
}


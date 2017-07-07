//#define debug

#define whiteMax 230          //maximum intensity for ledPin4 (white channel)

#define blankTime 30 * 1000   //display blank time in milliseconds
#define buttonTime 500        //timount for scroll start in milliseconds
#define scrollTime 20         //scroll speed in milliseconds (some magic used)

#define debounceTime 30       //button bouncing timeout in milliseconds
#define dmxFailTime 500       //timeout for DXM failure detect in milliseconds

// avrdude command
// avrdude -p m328p -c arduino -P com3 -b 19200 -U lfuse:w:0xC7:m -U hfuse:w:0xD3:m -U efuse:w:0xFD:m
// avrdude -p m328p -c arduino -P com3 -b 19200 -U flash:w:firmware.hex


// LED PWM output pins
#define ledPin0 10
#define ledPin1 9
#define ledPin2 6
#define ledPin3 5

// button input pins
#define modePin1 11
#define upPin2 12
#define downPin3 A0
#define enterPin4 A1

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
uint8_t dmxMode = 0;
uint8_t blankScreen=false;

uint8_t dig3=10,dig2=10,dig1=10,dig0=10;

uint8_t standaloneColor[]={0,0,0,0};

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
  DMXSerial.init(DMXReceiver);            // pasakam, ka no bibliotekas izmantosim RECIEVER
  setupDisplay();
  EEPROM.get(0, trueDmxAddr);
  trueDmxAddr = constrain(trueDmxAddr, 1, 512);

  EEPROM.get(2, dmxMode);

 for (uint8_t ii=0;ii<4;ii++){
  EEPROM.get(ii+3, standaloneColor[ii]); 
 }
  //generateCIE1931(truePreHeat);
  //generateCIE1931(0);
  blankNext = millis() + blankTime;
}


enum _menuState{
  work=0,
    menuAddress=1,
      menuAddressSet=2,
    menuMode=3,
      modeDMX=4,
      modeStandAlone=5,
    menuColor=6,
      menuColorRed=7,
      menuColorGreen=8,
      menuColorBlue=9,
      menuColorWhite=10,
    menuExit=11,
  sleep=12,
};

uint8_t menuState=work;

uint8_t tmpColor;

uint8_t menuStateMachine(void){
  switch (menuState){
    case sleep:
      if (modeBut.wasPressed()) {
        menuState=menuAddress;      
        return;
      }
      if (enterBut.wasPressed()) {
        menuState=menuAddress;      
        return;
      }
      if (upBut.wasPressed()) {
        menuState=menuAddress;      
        return;
      }
      if (downBut.wasPressed()) {
        menuState=menuAddress;      
        return;
      }
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
      if (dmxMode){
        dig3=10;
        numberToDigits(trueDmxAddr);        
      } else {
        dig3='Q'-54;
        dig2='Q'-54;
        dig1='Q'-54;
        dig0='Q'-54;        
      }
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
        menuState=menuMode;
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
    case menuMode:
      if (modeBut.wasPressed()) {
        menuState=work;
        return;
      }
      if (enterBut.wasPressed()) {
        if (dmxMode){
          menuState=modeDMX; 
        } else {
          menuState=modeStandAlone; 
        }        
        //tmpDmxMode=dmxMode;
        return;
      }
      if (upBut.wasPressed()) {
        menuState=menuAddress;
        return;
      }
      if (downBut.wasPressed()) {
        menuState=menuColor;
        return;
      }    
      dig3='M'-54;
      dig2='O'-54;
      dig1='D'-54;
      dig0='E'-54;
      return;
//***************************************************************************
    case modeDMX:
      if (modeBut.wasPressed()) {
        menuState=work;
        return;
      }

      if (enterBut.wasPressed()) {
        menuState=menuMode;
        dmxMode=1;
        EEPROM.put(2, dmxMode);
        return;
      }
      if (upBut.wasPressed()) {
        menuState=modeStandAlone;
        return;
      }
      if (downBut.wasPressed()) {
        menuState=modeStandAlone;
        return;
      }    

      dig3='D'-54;
      dig2='A'-54;
      dig1='T'-54;
      dig0='A'-54;
      return;
//***************************************************************************
    case modeStandAlone:
      if (modeBut.wasPressed()) {
        menuState=work;
        return;
      }

      if (enterBut.wasPressed()) {
        menuState=menuMode;
        dmxMode=0;
        EEPROM.put(2, dmxMode);
        return;
      }


      if (upBut.wasPressed()) {
        menuState=modeDMX;
        return;
      }
      if (downBut.wasPressed()) {
        menuState=modeDMX;
        return;
      }    

      dig3='S'-54;
      dig2='T'-54;
      dig1='L'-54;
      dig0='N'-54;
      return;
    
//***************************************************************************      
    case menuColor:
      if (modeBut.wasPressed()) {
        menuState=work;
        return;
      }
      if (enterBut.wasPressed()) {
        menuState=menuColorRed;
        tmpColor=standaloneColor[0];
        break;
      }
      if (upBut.wasPressed()) {
        menuState=menuMode;
        return;
      }
      if (downBut.wasPressed()) {
        menuState=menuExit;
        return;
      }

      dig3='C'-54;
      dig2='O'-54;
      dig1='L'-54;
      dig0=10;
      return;

//***************************************************************************      
    case menuColorRed:
      if (modeBut.wasPressed()) {
        menuState=work;
        return;
      }
      if (enterBut.wasPressed()) {
        menuState=menuColorGreen;
        standaloneColor[0]= tmpColor;
        EEPROM.put(3, tmpColor);
        tmpColor=standaloneColor[1];       
        return;
      }                
      if (upBut.wasPressed()) {
        tmpColor++;
        standaloneColor[0]= tmpColor;
        dig3='R'-54;
        numberToDigits(tmpColor);
        return;
      }
      if (downBut.wasPressed()) {
        tmpColor--;
        standaloneColor[0]= tmpColor;
        dig3='R'-54;
        numberToDigits(tmpColor);
        return;
      }

    if ( upBut.pressedFor(buttonTime) && (millis() >= scrollNext)) {
      tmpColor++;
      scrollNext = millis() + scrollTime;
      standaloneColor[0]= tmpColor;
      dig3='R'-54;
      numberToDigits(dmxaddr);
    }
    if ( downBut.pressedFor(buttonTime) && (millis() >= scrollNext)) {
      tmpColor--;
      scrollNext = millis() + scrollTime;
      standaloneColor[0]= tmpColor;
      dig3='R'-54;
      numberToDigits(dmxaddr);
    }

      dig3='R'-54;
      numberToDigits(tmpColor);
      return;

//***************************************************************************      
    case menuColorGreen:
      if (modeBut.wasPressed()) {
        menuState=work;
        return;
      }
      if (enterBut.wasPressed()) {
        menuState=menuColorBlue;
        standaloneColor[1]= tmpColor;
        EEPROM.put(4, tmpColor);
        tmpColor=standaloneColor[2];       
        return;
      }                
      if (upBut.wasPressed()) {
        tmpColor++;
        standaloneColor[1]= tmpColor;
        dig3='G'-54;
        numberToDigits(tmpColor);
        return;
      }
      if (downBut.wasPressed()) {
        tmpColor--;
        standaloneColor[1]= tmpColor;
        dig3='G'-54;
        numberToDigits(tmpColor);
        return;
      }

    if ( upBut.pressedFor(buttonTime) && (millis() >= scrollNext)) {
      tmpColor++;
      scrollNext = millis() + scrollTime;
      standaloneColor[1]= tmpColor;
      dig3='G'-54;
      numberToDigits(dmxaddr);
    }
    if ( downBut.pressedFor(buttonTime) && (millis() >= scrollNext)) {
      tmpColor--;
      scrollNext = millis() + scrollTime;
      standaloneColor[1]= tmpColor;
      dig3='G'-54;
      numberToDigits(dmxaddr);
    }

      dig3='G'-54;
      numberToDigits(tmpColor);
      return;

//***************************************************************************      
    case menuColorBlue:
      if (modeBut.wasPressed()) {
        menuState=work;
        return;
      }
      if (enterBut.wasPressed()) {
        menuState=menuColorWhite;
        standaloneColor[2]= tmpColor;
        EEPROM.put(5, tmpColor);
        tmpColor=standaloneColor[3];       
        return;
      }                
      if (upBut.wasPressed()) {
        tmpColor++;
        standaloneColor[2]= tmpColor;
        dig3='B'-54;
        numberToDigits(tmpColor);
        return;
      }
      if (downBut.wasPressed()) {
        tmpColor--;
        standaloneColor[2]= tmpColor;
        dig3='B'-54;
        numberToDigits(tmpColor);
        return;
      }

    if ( upBut.pressedFor(buttonTime) && (millis() >= scrollNext)) {
      tmpColor++;
      scrollNext = millis() + scrollTime;
      standaloneColor[2]= tmpColor;
      dig3='B'-54;
      numberToDigits(dmxaddr);
    }
    if ( downBut.pressedFor(buttonTime) && (millis() >= scrollNext)) {
      tmpColor--;
      scrollNext = millis() + scrollTime;
      standaloneColor[2]= tmpColor;
      dig3='B'-54;
      numberToDigits(dmxaddr);
    }

      dig3='B'-54;
      numberToDigits(tmpColor);
      return;


//***************************************************************************      
    case menuColorWhite:
      if (modeBut.wasPressed()) {
        menuState=work;
        return;
      }
      if (enterBut.wasPressed()) {
        menuState=menuColor;
        standaloneColor[3]= tmpColor;
        EEPROM.put(6, tmpColor);
        //tmpColor=standaloneColor[3];       
        return;
      }                
      if (upBut.wasPressed()) {
        tmpColor++;
        standaloneColor[3]= tmpColor;
        dig3='W'-54;
        numberToDigits(tmpColor);
        return;
      }
      if (downBut.wasPressed()) {
        tmpColor--;
        standaloneColor[3]= tmpColor;
        dig3='W'-54;
        numberToDigits(tmpColor);
        return;
      }

    if ( upBut.pressedFor(buttonTime) && (millis() >= scrollNext)) {
      tmpColor++;
      scrollNext = millis() + scrollTime;
      standaloneColor[3]= tmpColor;
      dig3='W'-54;
      numberToDigits(dmxaddr);
    }
    if ( downBut.pressedFor(buttonTime) && (millis() >= scrollNext)) {
      tmpColor--;
      scrollNext = millis() + scrollTime;
      standaloneColor[3]= tmpColor;
      dig3='W'-54;
      numberToDigits(dmxaddr);
    }

      dig3='W'-54;
      numberToDigits(tmpColor);
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
        menuState=menuColor;
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
  

  if ( modeBut.isChanged() || enterBut.isChanged() || downBut.isChanged() || upBut.isChanged() ) {
    if ( menuState==sleep ) {
      menuState=work;
    }
    blankNext = millis() + blankTime;
    blankScreen=false;
    //menuState=work;
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

    for (uint8_t i = 0; i < 4; i++) {
      if (dmxMode){
        if ((trueDmxAddr + i) <= 512) {
          ledTmp = DMXSerial.read(trueDmxAddr + i);      
        }
      } else {
        ledTmp=standaloneColor[i];
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


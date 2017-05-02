#include "MCP_Display.h"

#include <Wire.h>

#include "Adafruit_MCP23017.h"
Adafruit_MCP23017 mcp;

int8_t digitArr[3];                     // address output

uint16_t tmpPins;

uint8_t doDigit(int dmxaddr, uint8_t dp, uint8_t mode) {
  static uint8_t digit;
  uint8_t i;
  uint8_t segChar;
  static uint8_t prewDigit;

  int tmp = dmxaddr - (dmxaddr % 100);
  digitArr[2] = tmp / 100;
  tmp = dmxaddr - (dmxaddr % 10) - (digitArr[2] * 100);
  digitArr[1] = tmp / 10;
  digitArr[0] = dmxaddr - (digitArr[2] * 100) - (digitArr[1] * 10);


  //gashenie
  //mcp.digitalWrite(digitPins[prewDigit],LOW);
  bitClear(tmpPins, digitPins[prewDigit]);
  mcp.writeGPIOAB(tmpPins);

  if (digit == 3) {
    if (mode) segChar = charGen[10];
    else segChar = 0;
  }
  else {
    segChar = charGen[digitArr[digit]];
    //segChar=charGen[number];
    //a=pow(10,digit+1);
  }

  //check for decimal point
  if (dp != 0) bitSet(segChar, 7);

  // set segments On
  //for(i=0;i<8;i++) mcp.digitalWrite(segPins[i],bitRead(segChar,i));
  for (i = 0; i < 8; i++) bitWrite(tmpPins, segPins[i], bitRead(segChar, i));

  //set digit
  //mcp.digitalWrite(digitPins[digit],HIGH);
  bitSet(tmpPins, digitPins[digit]);

  //set all pins
  mcp.writeGPIOAB(tmpPins);

  prewDigit = digit;
  digit++;
  if (digit > 3) digit = 0;
  delay(1);

  return mcp.readGPIO(0) >>  4;  //read buttons
}
void setupDipslay(void) {
  uint8_t i;
  mcp.begin();
  Wire.setClock(400000);
  // set MCP23017 pins GPA0-GPA3 to output
  for (i = 0; i < 4; i++) {
    mcp.pinMode(i, OUTPUT);
    mcp.digitalWrite(i, LOW);
    bitClear(tmpPins, i);
  }
  // set MCP23017 pins GPA4-GPA7 to input for buttons
  for (i = 4; i < 8; i++) {
    mcp.pinMode(i, INPUT);
    mcp.pullUp(i, 1);
  }
  // set MCP23017 pins GPB0-GPB7 to output
  for (i = 8; i < 16; i++) {
    mcp.pinMode(i, OUTPUT);
    mcp.digitalWrite(i, LOW);
    bitClear(tmpPins, i);
  }

}

#include "SCT_Display.h"

int8_t digitArr[4];                     // address output

void sendBit(uint8_t bit) {
  digitalWrite(sdiPin, bit);
  digitalWrite(clkPin, HIGH);
  digitalWrite(clkPin, LOW);
}

uint8_t i;
unsigned long segArr;
int tmp;

uint8_t doDigit(int dmxaddr, uint8_t dp, uint8_t mode) {  

  if (dmxaddr>=1000){
    for (i = 0; i < 3; i++) {
      digitArr[i] = 0;
      bitSet(digitArr[0], 7);
    }
  } else {
    tmp = dmxaddr - (dmxaddr % 100);
    digitArr[2] = tmp / 100;
    tmp = dmxaddr - (dmxaddr % 10) - (digitArr[2] * 100);
    digitArr[1] = tmp / 10;
    digitArr[0] = dmxaddr - (digitArr[2] * 100) - (digitArr[1] * 10);
  
    for (i = 0; i < 3; i++) {
      digitArr[i] = charGen[digitArr[i]];    
      //if (dp != 0) bitSet(digitArr[i], 7);
      //else bitClear(digitArr[i], 7);
    }
  }
  
  // set segment 3 (left) according to mode
  if (mode == 1) digitArr[3] = charGen[11];
  else if (mode == 2) digitArr[3] = charGen[12];
  else digitArr[3] = 0;

  // set segment 3 (left) decimal point
  if (dp != 0) bitSet(digitArr[3], 7);
  else bitClear(digitArr[3], 7);

  // set segments
  for (i = 0; i < 4; i++) {
    for (uint8_t j = 0; j < 8; j++) {
      if (bitRead(digitArr[i], j)) {
        bitSet(segArr, segPins[i][j]);
      } else {
        bitClear(segArr, segPins[i][j]);
      }
    }
  }

  digitalWrite(laPin, LOW); //latch low, activeate shift register
  for (i = 0; i < 32; i++) {
    if (bitRead(segArr, i)) {
      sendBit(HIGH);
    } else {
      sendBit(LOW);
    }
  }
  digitalWrite(laPin, HIGH); //latch high, copy new data from shift register to display
  digitalWrite(laPin, LOW); //latch low, activeate shift register
}

void setupDisplay(void) {
  pinMode(laPin, OUTPUT);
  digitalWrite(laPin, LOW);

  pinMode(clkPin, OUTPUT);
  digitalWrite(clkPin, LOW);

  pinMode(sdiPin, OUTPUT);
  digitalWrite(sdiPin, LOW);
}

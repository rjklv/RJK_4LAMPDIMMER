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

uint8_t doDigit(uint8_t dig3, uint8_t dig2, uint8_t dig1, uint8_t dig0, uint8_t dp) {  

  digitArr[0]=dig0;
  digitArr[1]=dig1;
  digitArr[2]=dig2;
  digitArr[3]=dig3;
  
  for (i = 0; i < 4; i++) {
    digitArr[i] = charGen[digitArr[i]];    
  }

  //set decimal point 0 (right) if display in sleep mode (sending blank characetrs
  //if ( (digitArr[0]==10) && (digitArr[1]==10) && (digitArr[2]==10) && (digitArr[3]==10) ){
    bitSet(digitArr[0], 7);
  //} else {
    //bitClear(digitArr[0], 7);
  //}
  
  // set segment 3 (left) decimal point if DMX present
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
  sendToSCT();
}

void sendToSCT(void){
    //send to SCT chip via hardware pins
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

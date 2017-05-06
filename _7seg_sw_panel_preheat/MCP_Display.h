#include <Arduino.h>

#define ledPin0 10
#define ledPin1 9
#define ledPin2 6
#define ledPin3 5

#define segPinA 14
#define segPinB 12
#define segPinC 2
#define segPinD 0
#define segPinE 15
#define segPinF 13
#define segPinG 3
#define segPinDP 1

#define digPin0 11
#define digPin1 8
#define digPin2 9
#define digPin3 10

#define swPin0 4
#define swPin1 5
#define swPin2 6
#define swPin3 7

#define refreshRate 1000

#define segZero   0x7E
#define segOne    0x30
#define segTwo    0x6D
#define segThree  0x79
#define segFour   0x33
#define segFive   0x5B
#define segSix    0x5F
#define segSeven  0x70
#define segEight  0x7F
#define segNine   0x7B
#define segLetA   0x77
#define segLetP   0x67

const uint8_t charGen[] = {segZero, segOne, segTwo, segThree, segFour, segFive, segSix, segSeven, segEight, segNine, segLetA, segLetP};
const uint8_t segPins[] = {segPinG, segPinF, segPinE, segPinD, segPinC, segPinB, segPinA, segPinDP};
const uint8_t digitPins[] = {digPin0, digPin1, digPin2, digPin3};
const uint8_t ledPin[] = {ledPin0, ledPin1, ledPin2, ledPin3};



void setupDipslay(void);
uint8_t doDigit(int dmxaddr, uint8_t dp, uint8_t mode);

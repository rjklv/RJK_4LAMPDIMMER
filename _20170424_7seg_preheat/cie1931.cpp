#include "cie1931.h"

uint8_t cie[256];

uint8_t cieTable(uint8_t index) {
  return cie[index];
}

void generateCIE1931(uint8_t preheat) {
  float xL;
  float cieTemp;
  float outScale = (float)(maxOut - preheat);

  for (int i = 0; i <= 255; i++) {
    xL = (float)i * xLmul;
    if (xL <= 8) {
      cieTemp = xL / 902.3f;
    } else {
      cieTemp = pow((xL + 16.0f) / 116.0f, 3);
    }
    cie[i] = (uint8_t)round(cieTemp * outScale) + preheat;
  }
}


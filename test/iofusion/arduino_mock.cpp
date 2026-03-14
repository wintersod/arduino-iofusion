#include "Arduino.h"

unsigned long mockMillis = 0;
uint8_t mockPinModes[64] = {0};
uint8_t mockPortIn[8] = {0};
uint8_t mockPortOut[8] = {0};
int mockAnalogValues[16] = {0};
uint8_t TCCR1A = 0;
uint8_t TCCR1B = 0;
uint8_t TCCR2A = 0;
uint8_t TCCR2B = 0;
uint8_t TIMSK2 = 0;
uint8_t OCR2A = 0;
uint16_t OCR1A = 0;
uint16_t OCR1B = 0;
uint16_t ICR1 = 0;
uint16_t TCNT1 = 0;
MockSerial Serial;

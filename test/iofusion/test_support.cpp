#include "test_support.h"

#include "Arduino.h"

namespace {
  void clearPorts() {
    for (uint8_t i = 0; i < 8; ++i) {
      mockPortIn[i] = 0;
      mockPortOut[i] = 0;
    }
  }
}

void resetMockState() {
  mockMillis = 0;
  for (int i = 0; i < 16; ++i) mockAnalogValues[i] = 0;
  for (int i = 0; i < 64; ++i) mockPinModes[i] = 0;
  clearPorts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR2A = 0;
  TCCR2B = 0;
  TIMSK2 = 0;
  OCR2A = 0;
  OCR1A = 0;
  OCR1B = 0;
  ICR1 = 0;
  TCNT1 = 0;
  Serial.clearOutput();
  Serial.setInput("");
}

void setDigitalPin(uint8_t pin, bool high) {
  uint8_t port = digitalPinToPort(pin);
  uint8_t mask = digitalPinToBitMask(pin);
  if (port == NOT_A_PIN) return;
  if (high) mockPortIn[port] |= mask;
  else mockPortIn[port] &= static_cast<uint8_t>(~mask);
}

void setUp() {
  resetMockState();
}

void tearDown() {}
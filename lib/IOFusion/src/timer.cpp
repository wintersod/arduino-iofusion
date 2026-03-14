#include "timer.h"

// Static member definitions
volatile Timer2Callback Timer2Driver::_cbs[Timer2Driver::MAX_CALLBACKS] = { nullptr, nullptr, nullptr, nullptr };

Timer2Driver::Timer2Driver() {}

void Timer2Driver::clearCallbacks() {
  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    _cbs[i] = nullptr;
  }
}

uint16_t Timer2Driver::beginHz(float freqHz) {
  if (freqHz <= 0) return 0;
  // Stop timer2
  TCCR2A = 0;
  TCCR2B = 0;
  TIMSK2 = 0; // disable interrupts

  noInterrupts();
  clearCallbacks();
  interrupts();

  const uint32_t F_CPU32 = F_CPU;
  // Timer2 is 8-bit; use CTC mode with OCR2A top (WGM21=1)
  // Try prescalers to find OCR value within 0..255
  const uint16_t presVals[] = {1, 8, 32, 64, 128, 256, 1024};
  uint16_t chosenOCR = 0;
  uint16_t chosenPres = 1;
  bool found = false;
  for (uint8_t i = 0; i < sizeof(presVals)/sizeof(presVals[0]); ++i) {
    float pres = static_cast<float>(presVals[i]);
    float ocr = (F_CPU32 / (pres * freqHz)) - 1.0f;
    if (ocr >= 0.0f && ocr <= 255.0f) {
      chosenOCR = (uint16_t)(ocr + 0.5f);
      chosenPres = presVals[i];
      found = true;
      break;
    }
  }
  if (!found) return 0;

  // Configure CTC mode
  TCCR2A = _BV(WGM21);
  // Set OCR
  OCR2A = (uint8_t)chosenOCR;
  // Set prescaler bits
  uint8_t csbits = 0;
  switch (chosenPres) {
    case 1: csbits = _BV(CS20); break;
    case 8: csbits = _BV(CS21); break;
    case 32: csbits = _BV(CS20) | _BV(CS21); break;
    case 64: csbits = _BV(CS22); break;
    case 128: csbits = _BV(CS22) | _BV(CS20); break;
    case 256: csbits = _BV(CS22) | _BV(CS21); break;
    case 1024: csbits = _BV(CS22) | _BV(CS21) | _BV(CS20); break;
    default: csbits = _BV(CS20); break;
  }
  TCCR2B = csbits;

  // enable compare match A interrupt
  TIMSK2 |= _BV(OCIE2A);
  return (uint16_t)chosenOCR;
}

void Timer2Driver::stop() {
  noInterrupts();
  TIMSK2 &= ~_BV(OCIE2A);
  TCCR2A = 0;
  TCCR2B = 0;
  clearCallbacks();
  interrupts();
}

bool Timer2Driver::attachCallback(Timer2Callback cb) {
  if (cb == nullptr) return false;
  noInterrupts();
  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    if (_cbs[i] == cb) {
      interrupts();
      return true;
    }
  }
  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    if (_cbs[i] == nullptr) {
      _cbs[i] = cb;
      interrupts();
      return true;
    }
  }
  interrupts();
  return false;
}

void Timer2Driver::detachCallback(Timer2Callback cb) {
  noInterrupts();
  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    if (_cbs[i] == cb) {
      _cbs[i] = nullptr;
      break;
    }
  }
  interrupts();
}

// No isSampleDue() flag — use attachCallback() for ISR work.

// ISR for Timer2 Compare Match A
ISR(TIMER2_COMPA_vect) {
  // call callback if present
  Timer2Driver::handleInterrupt();
  // encoder tick should be attached via Timer2Driver::attachCallback()
}

void Timer2Driver::handleInterrupt() {
  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    Timer2Callback cb = _cbs[i];
    if (cb) cb();
  }
}

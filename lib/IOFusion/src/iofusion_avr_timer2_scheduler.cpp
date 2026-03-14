#include "iofusion_avr_timer2_scheduler.h"

namespace IOFusion {

volatile Timer2Callback AvrTimer2Scheduler::_cbs[AvrTimer2Scheduler::MAX_CALLBACKS] = { nullptr, nullptr, nullptr, nullptr };

AvrTimer2Scheduler::AvrTimer2Scheduler() {}

void AvrTimer2Scheduler::clearCallbacks() {
  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    _cbs[i] = nullptr;
  }
}

uint16_t AvrTimer2Scheduler::beginHz(uint32_t freqHz) {
  if (freqHz == 0U) return 0;
  TCCR2A = 0;
  TCCR2B = 0;
  TIMSK2 = 0;

  noInterrupts();
  clearCallbacks();
  interrupts();

  const uint32_t cpuFreq = F_CPU;
  const uint16_t presVals[] = {1, 8, 32, 64, 128, 256, 1024};
  uint16_t chosenOcr = 0;
  uint16_t chosenPres = 1;
  bool found = false;
  for (uint8_t i = 0; i < sizeof(presVals) / sizeof(presVals[0]); ++i) {
    uint32_t divisor = static_cast<uint32_t>(presVals[i]) * freqHz;
    if (divisor == 0U) continue;
    uint32_t counts = cpuFreq / divisor;
    if (counts > 0U && counts <= 256U) {
      chosenOcr = static_cast<uint16_t>(counts - 1U);
      chosenPres = presVals[i];
      found = true;
      break;
    }
  }
  if (!found) return 0;

  TCCR2A = _BV(WGM21);
  OCR2A = static_cast<uint8_t>(chosenOcr);

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
  TIMSK2 |= _BV(OCIE2A);
  return chosenOcr;
}

void AvrTimer2Scheduler::stop() {
  noInterrupts();
  TIMSK2 &= static_cast<uint8_t>(~_BV(OCIE2A));
  TCCR2A = 0;
  TCCR2B = 0;
  clearCallbacks();
  interrupts();
}

bool AvrTimer2Scheduler::attachCallback(Timer2Callback cb) {
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

void AvrTimer2Scheduler::detachCallback(Timer2Callback cb) {
  noInterrupts();
  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    if (_cbs[i] == cb) {
      _cbs[i] = nullptr;
      break;
    }
  }
  interrupts();
}

void AvrTimer2Scheduler::handleInterrupt() {
  for (uint8_t i = 0; i < MAX_CALLBACKS; ++i) {
    Timer2Callback cb = _cbs[i];
    if (cb) cb();
  }
}

} // namespace IOFusion

ISR(TIMER2_COMPA_vect) {
  IOFusion::AvrTimer2Scheduler::handleInterrupt();
}
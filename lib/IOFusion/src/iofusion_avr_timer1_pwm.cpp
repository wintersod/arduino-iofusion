#include "iofusion_avr_timer1_pwm.h"

namespace IOFusion {

AvrTimer1Pwm::AvrTimer1Pwm() {}

bool AvrTimer1Pwm::begin(uint32_t freqHz) {
  if (freqHz == 0U) return false;

  const uint16_t pres[] = {1, 8, 64, 256, 1024};
  const uint32_t cpuFreq = F_CPU;
  uint32_t top = 0;
  uint16_t chosenPres = 1;
  for (uint8_t i = 0; i < sizeof(pres) / sizeof(pres[0]); ++i) {
    uint32_t divisor = static_cast<uint32_t>(pres[i]) * freqHz;
    if (divisor == 0U) continue;
    uint32_t counts = cpuFreq / divisor;
    if (counts > 0U && counts <= 65536U) {
      top = counts - 1U;
      chosenPres = pres[i];
      break;
    }
  }
  if (top == 0 || top > 65535) return false;

  uint16_t newTop = static_cast<uint16_t>(top);
  uint16_t newPresBits = 0;
  switch (chosenPres) {
    case 1: newPresBits = _BV(CS10); break;
    case 8: newPresBits = _BV(CS11); break;
    case 64: newPresBits = _BV(CS11) | _BV(CS10); break;
    case 256: newPresBits = _BV(CS12); break;
    case 1024: newPresBits = _BV(CS12) | _BV(CS10); break;
    default: newPresBits = _BV(CS10); break;
  }

  uint16_t dutyCounts[2];
  for (uint8_t i = 0; i < 2; ++i) {
    dutyCounts[i] = percentToCounts(_dutyPercent[i], newTop);
  }

  noInterrupts();
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);

  TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
  uint8_t tccr1b = _BV(WGM13) | _BV(WGM12);
  TCCR1B = tccr1b;

  ICR1 = newTop;
  _applyDuty(0, dutyCounts[0]);
  _applyDuty(1, dutyCounts[1]);
  TCNT1 = 0;

  tccr1b |= newPresBits;
  TCCR1B = tccr1b;

  _top = newTop;
  _presBits = newPresBits;
  _frequencyHz = freqHz;
  _configured = true;
  interrupts();

  return true;
}

void AvrTimer1Pwm::stop() {
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 0;
  OCR1B = 0;
  interrupts();
  pinMode(9, INPUT);
  pinMode(10, INPUT);
  _top = 0;
  _presBits = 0;
  _frequencyHz = 0;
  _dutyPercent[0] = 0;
  _dutyPercent[1] = 0;
  _configured = false;
}

void AvrTimer1Pwm::setDuty(uint8_t channel, uint8_t percent) {
  if (channel > 1) return;
  if (percent > 100U) percent = 100U;
  _dutyPercent[channel] = percent;
  if (!_configured || _top == 0) return;
  uint16_t counts = percentToCounts(percent, _top);
  noInterrupts();
  _applyDuty(channel, counts);
  interrupts();
}

void AvrTimer1Pwm::_applyDuty(uint8_t channel, uint16_t value) {
  if (channel == 0) OCR1A = value;
  else if (channel == 1) OCR1B = value;
}

uint16_t AvrTimer1Pwm::percentToCounts(uint8_t percent, uint16_t top) const {
  if (top == 0) return 0;
  if (percent == 0U) return 0;
  if (percent >= 100U) return top;
  uint32_t value = (static_cast<uint32_t>(percent) * top + 50U) / 100U;
  if (value > top) value = top;
  return static_cast<uint16_t>(value);
}

} // namespace IOFusion
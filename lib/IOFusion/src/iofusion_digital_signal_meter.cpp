#include "iofusion_digital_signal_meter.h"

namespace IOFusion {

DigitalSignalMeter::DigitalSignalMeter() {}

bool DigitalSignalMeter::begin(const uint8_t* pins, uint8_t count, uint16_t windowTicks, uint32_t tickHz, bool usePullup) {
  if (count == 0 || count > MAX_PINS) return false;
  if (windowTicks == 0 || tickHz == 0U) return false;
  _pinCount = count;
  _windowTicks = windowTicks;
  _tickHz = tickHz;
  for (uint8_t i = 0; i < _pinCount; ++i) {
    _pins[i] = pins[i];
    if (usePullup) pinMode(_pins[i], INPUT_PULLUP);
    else pinMode(_pins[i], INPUT);
    uint8_t port = digitalPinToPort(_pins[i]);
    _pinPortIn[i] = portInputRegister(port);
    _pinMask[i] = digitalPinToBitMask(_pins[i]);
    if (port == NOT_A_PIN || _pinPortIn[i] == nullptr || _pinMask[i] == 0) return false;
    _edgeCnt[i] = 0;
    _highCnt[i] = 0;
    _lastState[i] = (_pinPortIn[i] && ((*_pinPortIn[i] & _pinMask[i]) != 0)) ? 1 : 0;
    _freqDeciHz[i] = 0;
    _dutyDeciPercent[i] = 0;
  }
  _samplesInWindow = 0;
  _windowReady = false;
  return true;
}

void DigitalSignalMeter::onTick() {
  if (_windowReady) return;
  for (uint8_t i = 0; i < _pinCount; ++i) {
    uint8_t state = (_pinPortIn[i] && ((*_pinPortIn[i] & _pinMask[i]) != 0)) ? 1 : 0;
    if (state) _highCnt[i]++;
    if (state && !_lastState[i]) _edgeCnt[i]++;
    _lastState[i] = state;
  }
  _samplesInWindow++;
  if (_samplesInWindow >= _windowTicks) {
    _windowReady = true;
  }
}

void DigitalSignalMeter::updateIfReady() {
  if (!_windowReady) return;

  uint16_t samples = 0;
  uint16_t edgeCnt[MAX_PINS];
  uint16_t highCnt[MAX_PINS];

  noInterrupts();
  samples = _samplesInWindow;
  for (uint8_t i = 0; i < _pinCount; ++i) {
    edgeCnt[i] = _edgeCnt[i];
    highCnt[i] = _highCnt[i];
    _edgeCnt[i] = 0;
    _highCnt[i] = 0;
  }
  _samplesInWindow = 0;
  _windowReady = false;
  interrupts();

  if (samples == 0 || _tickHz == 0U) {
    for (uint8_t i = 0; i < _pinCount; ++i) {
      _freqDeciHz[i] = 0;
      _dutyDeciPercent[i] = 0;
    }
    return;
  }

  for (uint8_t i = 0; i < _pinCount; ++i) {
    uint64_t scaledFreq = static_cast<uint64_t>(edgeCnt[i]) * static_cast<uint64_t>(_tickHz) * 10ULL;
    _freqDeciHz[i] = static_cast<uint32_t>((scaledFreq + (samples / 2U)) / samples);
    uint32_t scaledDuty = static_cast<uint32_t>(highCnt[i]) * 1000U;
    _dutyDeciPercent[i] = static_cast<uint16_t>((scaledDuty + (samples / 2U)) / samples);
  }
}

uint8_t DigitalSignalMeter::getPinCount() const { return _pinCount; }

uint32_t DigitalSignalMeter::getFrequencyDeciHz(uint8_t idx) const {
  if (idx >= _pinCount) return 0;
  noInterrupts();
  uint32_t value = _freqDeciHz[idx];
  interrupts();
  return value;
}

uint16_t DigitalSignalMeter::getDutyDeciPercent(uint8_t idx) const {
  if (idx >= _pinCount) return 0;
  noInterrupts();
  uint16_t value = _dutyDeciPercent[idx];
  interrupts();
  return value;
}

} // namespace IOFusion
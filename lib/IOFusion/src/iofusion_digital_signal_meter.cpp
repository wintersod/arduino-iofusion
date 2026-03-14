/**
 * @file iofusion_digital_signal_meter.cpp
 * @brief Implementation of the fixed-point digital signal meter.
 */
#include "iofusion_digital_signal_meter.h"

namespace IOFusion {

DigitalSignalMeter::DigitalSignalMeter() {}

bool DigitalSignalMeter::begin(const uint8_t* pins, uint8_t count, uint16_t windowTicks, uint32_t tickHz, bool usePullup) {
  if (count == 0 || count > MAX_PINS) return false;
  if (windowTicks == 0 || tickHz == 0U) return false;
  _pinCount = count;
  _windowTicks = windowTicks;
  _tickHz = tickHz;
  _sampleTick = 0;
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
    _lastRiseTick[i] = 0U;
    _lastPeriodTicks[i] = 0U;
    _firstRiseTickInWindow[i] = 0U;
    _lastRiseTickInWindow[i] = 0U;
    _highTicksSinceRise[i] = 0U;
    _lastCycleHighTicks[i] = 0U;
    _hasRiseTick[i] = 0U;
    _hasCycle[i] = 0U;
    _freqDeciHz[i] = 0;
    _dutyDeciPercent[i] = 0;
  }
  _samplesInWindow = 0;
  _windowReady = false;
  return true;
}

void DigitalSignalMeter::onTick() {
  ++_sampleTick;
  for (uint8_t i = 0; i < _pinCount; ++i) {
    uint8_t state = (_pinPortIn[i] && ((*_pinPortIn[i] & _pinMask[i]) != 0)) ? 1 : 0;
    if (_windowReady) {
      _lastState[i] = state;
      continue;
    }
    if (state) _highCnt[i]++;
    if (state && !_lastState[i]) {
      if (_edgeCnt[i] == 0U) {
        _firstRiseTickInWindow[i] = _sampleTick;
      }
      _edgeCnt[i]++;
      _lastRiseTickInWindow[i] = _sampleTick;
      if (_hasRiseTick[i]) {
        uint32_t periodTicks = _sampleTick - _lastRiseTick[i];
        if (periodTicks != 0U) {
          _lastPeriodTicks[i] = periodTicks;
          uint32_t highTicks = _highTicksSinceRise[i];
          if (highTicks > periodTicks) highTicks = periodTicks;
          _lastCycleHighTicks[i] = highTicks;
          _hasCycle[i] = 1U;
        }
      }
      _lastRiseTick[i] = _sampleTick;
      _highTicksSinceRise[i] = 0U;
      _hasRiseTick[i] = 1U;
    }
    if (state && _hasRiseTick[i]) {
      _highTicksSinceRise[i]++;
    }
    _lastState[i] = state;
  }
  if (_windowReady) return;
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
  uint32_t sampleTick = 0;
  uint32_t lastRiseTick[MAX_PINS];
  uint32_t lastPeriodTicks[MAX_PINS];
  uint32_t firstRiseTickInWindow[MAX_PINS];
  uint32_t lastRiseTickInWindow[MAX_PINS];
  uint32_t lastCycleHighTicks[MAX_PINS];
  uint8_t hasCycle[MAX_PINS];

  noInterrupts();
  samples = _samplesInWindow;
  sampleTick = _sampleTick;
  for (uint8_t i = 0; i < _pinCount; ++i) {
    edgeCnt[i] = _edgeCnt[i];
    highCnt[i] = _highCnt[i];
    lastRiseTick[i] = _lastRiseTick[i];
    lastPeriodTicks[i] = _lastPeriodTicks[i];
    firstRiseTickInWindow[i] = _firstRiseTickInWindow[i];
    lastRiseTickInWindow[i] = _lastRiseTickInWindow[i];
    lastCycleHighTicks[i] = _lastCycleHighTicks[i];
    hasCycle[i] = _hasCycle[i];
    _edgeCnt[i] = 0;
    _highCnt[i] = 0;
    _firstRiseTickInWindow[i] = 0U;
    _lastRiseTickInWindow[i] = 0U;
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
    if (edgeCnt[i] >= 3U) {
      uint32_t spanTicks = lastRiseTickInWindow[i] - firstRiseTickInWindow[i];
      uint16_t completedPeriods = static_cast<uint16_t>(edgeCnt[i] - 1U);
      if (spanTicks != 0U && completedPeriods != 0U) {
        uint64_t scaledFreq = static_cast<uint64_t>(completedPeriods) * static_cast<uint64_t>(_tickHz) * 10ULL;
        _freqDeciHz[i] = static_cast<uint32_t>((scaledFreq + (spanTicks / 2U)) / spanTicks);
      } else {
        _freqDeciHz[i] = 0U;
      }
      uint32_t scaledDuty = static_cast<uint32_t>(highCnt[i]) * 1000U;
      _dutyDeciPercent[i] = static_cast<uint16_t>((scaledDuty + (samples / 2U)) / samples);
    } else if (hasCycle[i] != 0U && lastPeriodTicks[i] != 0U) {
      uint32_t ageTicks = sampleTick - lastRiseTick[i];
      uint64_t staleTicks = static_cast<uint64_t>(lastPeriodTicks[i]) * 2ULL;
      if (staleTicks < static_cast<uint64_t>(_windowTicks)) {
        staleTicks = static_cast<uint64_t>(_windowTicks);
      }
      if (static_cast<uint64_t>(ageTicks) <= staleTicks) {
        uint64_t scaledFreq = static_cast<uint64_t>(_tickHz) * 10ULL;
        _freqDeciHz[i] = static_cast<uint32_t>((scaledFreq + (lastPeriodTicks[i] / 2U)) / lastPeriodTicks[i]);
        uint64_t scaledDuty = static_cast<uint64_t>(lastCycleHighTicks[i]) * 1000ULL;
        _dutyDeciPercent[i] = static_cast<uint16_t>((scaledDuty + (lastPeriodTicks[i] / 2U)) / lastPeriodTicks[i]);
      } else {
        _freqDeciHz[i] = 0U;
        _dutyDeciPercent[i] = 0U;
      }
    } else {
      _freqDeciHz[i] = 0U;
      _dutyDeciPercent[i] = 0U;
    }
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
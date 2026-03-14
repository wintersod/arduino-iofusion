#include "iofusion_analog_sampler.h"

namespace IOFusion {

AnalogSampler::AnalogSampler() {
  for (uint8_t i = 0; i < MAX_CHANNELS; ++i) _lastValues[i] = 0;
}

bool AnalogSampler::begin(const uint8_t* channels, uint8_t channelCount) {
  if (channelCount == 0 || channelCount > MAX_CHANNELS) return false;
  for (uint8_t i = 0; i < channelCount; ++i) {
    uint8_t c = channels[i];
    if (c > 5) return false;
    _channels[i] = c;
  }
  _channelCount = channelCount;
  for (uint8_t ch = 0; ch < _channelCount; ++ch) _lastValues[ch] = 0;
  noInterrupts();
  _sampleRequested = false;
  interrupts();
  return true;
}

void AnalogSampler::onTick() {
  _sampleRequested = true;
}

void AnalogSampler::sampleIfDue() {
  noInterrupts();
  if (!_sampleRequested) {
    interrupts();
    return;
  }
  _sampleRequested = false;
  interrupts();

  for (uint8_t i = 0; i < _channelCount; ++i) {
    uint8_t ch = _channels[i];
    (void)analogRead(ch);
    delayMicroseconds(5);
    int value = analogRead(ch);
    _lastValues[i] = value;
  }
}

uint8_t AnalogSampler::getChannelCount() const { return _channelCount; }

uint16_t AnalogSampler::getMilliVolts(uint8_t idx) const {
  if (idx >= _channelCount) return 0;
  uint32_t scaled = static_cast<uint32_t>(_lastValues[idx]) * _vrefMillivolts;
  return static_cast<uint16_t>((scaled + 511U) / 1023U);
}

float AnalogSampler::getValue(uint8_t idx) const {
  if (idx >= _channelCount) return 0.0f;
  return static_cast<float>(getMilliVolts(idx)) / 1000.0f;
}

void AnalogSampler::setVref(float vref) {
  if (vref <= 0.0f) return;
  uint32_t millivolts = static_cast<uint32_t>(vref * 1000.0f + 0.5f);
  if (millivolts == 0 || millivolts > 65535U) return;
  _vrefMillivolts = static_cast<uint16_t>(millivolts);
}

void AnalogSampler::setVrefMillivolts(uint16_t vrefMillivolts) {
  if (vrefMillivolts == 0) return;
  _vrefMillivolts = vrefMillivolts;
}

} // namespace IOFusion
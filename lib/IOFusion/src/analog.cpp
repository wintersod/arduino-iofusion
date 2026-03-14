#include "analog.h"

AnalogSampler::AnalogSampler() {
  for (uint8_t i = 0; i < MAX_CHANNELS; ++i) _lastValues[i] = 0;
}

bool AnalogSampler::begin(const uint8_t* channels, uint8_t channelCount) {
  if (channelCount == 0 || channelCount > MAX_CHANNELS) return false;
  for (uint8_t i = 0; i < channelCount; ++i) {
    uint8_t c = channels[i];
    if (c > 5) return false; // only A0..A5 on typical AVR
    _channels[i] = c;
  }
  _channelCount = channelCount;
  for (uint8_t ch = 0; ch < _channelCount; ++ch) _lastValues[ch] = 0;
  // Initialize analog input pins (no pinMode for analog pins required on AVR)
  noInterrupts();
  _sampleRequested = false;
  interrupts();
  return true;
}

void AnalogSampler::onTick() {
  // called from ISR context — just set the request flag
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
    // Discard first reading after switching channel to allow S/H capacitor to settle.
    (void)analogRead(ch);
    delayMicroseconds(5);
    int v = analogRead(ch);
    _lastValues[i] = v;
  }
}

uint8_t AnalogSampler::getChannelCount() const { return _channelCount; }

float AnalogSampler::getValue(uint8_t idx) const {
  if (idx >= _channelCount) return 0.0f;
  // ADC range is 0..1023, scale by configured Vref
  return ((float)_lastValues[idx] * _vref) / 1023.0f;
}

void AnalogSampler::setVref(float vref) {
  if (vref <= 0.0f) return;
  _vref = vref;
}

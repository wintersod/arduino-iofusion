#ifndef MOCK_IOFUSION_ANALOG_SAMPLER_H
#define MOCK_IOFUSION_ANALOG_SAMPLER_H

#include <cstdint>

namespace IOFusion {

class AnalogSampler {
public:
  AnalogSampler() = default;

  void setValues(const float* values, uint8_t count) {
    _count = count;
    for (uint8_t i = 0; i < count && i < kMax; ++i) _values[i] = values[i];
  }

  uint8_t getChannelCount() const { return _count; }

  float getValue(uint8_t idx) const {
    if (idx >= _count) return 0.0f;
    return _values[idx];
  }

private:
  static constexpr uint8_t kMax = 8;
  float _values[kMax] = {0.0f};
  uint8_t _count = 0;
};

} // namespace IOFusion

#endif // MOCK_IOFUSION_ANALOG_SAMPLER_H
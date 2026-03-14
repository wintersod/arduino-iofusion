#ifndef MOCK_IOFUSION_DIGITAL_SIGNAL_METER_H
#define MOCK_IOFUSION_DIGITAL_SIGNAL_METER_H

#include <cstdint>

namespace IOFusion {

class DigitalSignalMeter {
public:
  DigitalSignalMeter() = default;

  void setValues(const uint32_t* freqDeciHz, const uint16_t* dutyDeciPercent, uint8_t count) {
    _count = count;
    for (uint8_t i = 0; i < count && i < kMax; ++i) {
      _freqDeciHz[i] = freqDeciHz[i];
      _dutyDeciPercent[i] = dutyDeciPercent[i];
    }
  }

  uint8_t getPinCount() const { return _count; }

  uint32_t getFrequencyDeciHz(uint8_t idx) const {
    if (idx >= _count) return 0;
    return _freqDeciHz[idx];
  }

  uint16_t getDutyDeciPercent(uint8_t idx) const {
    if (idx >= _count) return 0;
    return _dutyDeciPercent[idx];
  }

private:
  static constexpr uint8_t kMax = 8;
  uint32_t _freqDeciHz[kMax] = {0};
  uint16_t _dutyDeciPercent[kMax] = {0};
  uint8_t _count = 0;
};

} // namespace IOFusion

#endif // MOCK_IOFUSION_DIGITAL_SIGNAL_METER_H
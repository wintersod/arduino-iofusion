#ifndef MOCK_IOFUSION_AVR_TIMER1_PWM_H
#define MOCK_IOFUSION_AVR_TIMER1_PWM_H

#include <cstdint>

namespace IOFusion {

class AvrTimer1Pwm {
public:
  AvrTimer1Pwm() = default;

  bool begin(uint32_t freqHz) {
    _lastBeginFreq = freqHz;
    return _beginOk;
  }

  void setBeginOk(bool ok) { _beginOk = ok; }

  void setDuty(uint8_t channel, uint8_t percent) {
    if (channel > 1) return;
    _lastDutyChannel = channel;
    _lastDutyPercent = percent;
  }

  uint32_t lastBeginFreq() const { return _lastBeginFreq; }
  uint8_t lastDutyChannel() const { return _lastDutyChannel; }
  uint8_t lastDutyPercent() const { return _lastDutyPercent; }

private:
  bool _beginOk = true;
  uint32_t _lastBeginFreq = 0;
  uint8_t _lastDutyChannel = 0;
  uint8_t _lastDutyPercent = 0;
};

} // namespace IOFusion

#endif // MOCK_IOFUSION_AVR_TIMER1_PWM_H
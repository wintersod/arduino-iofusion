#include <Arduino.h>


#include "version_info.h"
#include "iofusion_avr_timer2_scheduler.h"
#include "iofusion_analog_sampler.h"
#include "iofusion_digital_signal_meter.h"
#include "iofusion_quadrature_signal_generator.h"
#include "iofusion_avr_timer1_pwm.h"
#include "serial_command_protocol.h"

namespace {
  struct PinMapConfig {
    const uint8_t* analogPins;
    uint8_t analogPinCount;
    const uint8_t* digitalPins;
    uint8_t digitalPinCount;
  };

  struct EncoderConfig {
    uint8_t outputPinA;
    uint8_t outputPinB;
    uint8_t inputPinUp;
    uint8_t inputPinDown;
  };

  struct TimingConfig {
    uint32_t timerTickHz;
    uint16_t digitalWindowTicks;
  };

  struct PwmConfig {
    uint32_t frequencyHz;
    uint8_t channel0DutyPercent;
    uint8_t channel1DutyPercent;
  };

  struct RuntimeConfig {
    PinMapConfig pins;
    EncoderConfig encoder;
    TimingConfig timing;
    PwmConfig pwm;
  };

  struct ModuleHealth {
    bool analog = false;
    bool digital = false;
    bool encoder = false;
    bool pwm = false;
    bool timer = false;
  };

  constexpr uint8_t kAnalogPins[] = {0, 1, 2, 3, 4, 5};
  constexpr uint8_t kDigitalPins[] = {2, 3, 8, 11, 12, 13};

  const RuntimeConfig kRuntimeConfig = {
    {kAnalogPins, static_cast<uint8_t>(sizeof(kAnalogPins) / sizeof(kAnalogPins[0])),
     kDigitalPins, static_cast<uint8_t>(sizeof(kDigitalPins) / sizeof(kDigitalPins[0]))},
    {4, 5, 6, 7},
    {10000U, 500},
    {100U, 50U, 25U},
  };

  class FirmwareRuntime {
  public:
    explicit FirmwareRuntime(const RuntimeConfig& config)
      : _config(config),
        _cmdProtocol(
          _analogSampler,
          _digitalSignalMeter,
          _quadratureGenerator,
          _timer1Pwm,
          _config.pins.analogPins,
          _config.pins.analogPinCount,
          _config.pins.digitalPins,
          _config.pins.digitalPinCount) {}

    void setup() {
      Serial.begin(115200);
      delay(100);
      Serial.print("Firmware version: ");
      Serial.println(FW_VERSION);

      pinMode(LED_BUILTIN, OUTPUT);

      _analogSampler.begin(_config.pins.analogPins, _config.pins.analogPinCount);
      _health.analog = _analogSampler.getChannelCount() > 0;
      if (!_health.analog) Serial.println(F("{\"error\":\"analog init failed\"}"));

      _health.digital = _digitalSignalMeter.begin(
        _config.pins.digitalPins,
        _config.pins.digitalPinCount,
        _config.timing.digitalWindowTicks,
        _config.timing.timerTickHz,
        true);
      if (!_health.digital) Serial.println(F("{\"error\":\"digital init failed\"}"));

      _health.encoder = _quadratureGenerator.begin(
        _config.encoder.outputPinA,
        _config.encoder.outputPinB,
        _config.encoder.inputPinUp,
        _config.encoder.inputPinDown);
      if (!_health.encoder) Serial.println(F("{\"error\":\"encoder init failed\"}"));

      _health.pwm = _timer1Pwm.begin(_config.pwm.frequencyHz);
      if (!_health.pwm) {
        Serial.println(F("{\"error\":\"pwm init failed\"}"));
      } else {
        _timer1Pwm.setDuty(0, _config.pwm.channel0DutyPercent);
        _timer1Pwm.setDuty(1, _config.pwm.channel1DutyPercent);
      }

      _health.timer = _timer2Scheduler.beginHz(_config.timing.timerTickHz) > 0;
      if (_health.timer) {
        _health.timer = _timer2Scheduler.attachCallback(timerTickHandler);
      }
      if (!_health.timer) {
        Serial.println(F("{\"error\":\"timer2 init failed\"}"));
      }

      refreshProtocolStatus();
    }

    void loop() {
      if (_health.analog) _analogSampler.sampleIfDue();
      if (_health.digital) _digitalSignalMeter.updateIfReady();
      _cmdProtocol.processSerial();
    }

    void onTimerTickIsr() {
      if (_health.analog) _analogSampler.onTick();
      if (_health.digital) _digitalSignalMeter.onTick();
      if (_health.encoder) _quadratureGenerator.onTick();
    }

  private:
    void refreshProtocolStatus() {
      _cmdProtocol.setModuleStatus(
        _health.analog,
        _health.digital,
        _health.encoder,
        _health.pwm,
        _health.timer);
    }

    const RuntimeConfig& _config;
    IOFusion::AvrTimer2Scheduler _timer2Scheduler;
    IOFusion::AnalogSampler _analogSampler;
    IOFusion::DigitalSignalMeter _digitalSignalMeter;
    IOFusion::QuadratureSignalGenerator _quadratureGenerator;
    IOFusion::AvrTimer1Pwm _timer1Pwm;
    IOFusion::SerialCommandProtocol _cmdProtocol;
    ModuleHealth _health;
  };

  FirmwareRuntime runtime(kRuntimeConfig);

  void timerTickHandler() {
    runtime.onTimerTickIsr();
  }
}

void setup() {
  runtime.setup();
}

void loop() {
  runtime.loop();
}

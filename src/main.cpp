/**
 * @file main.cpp
 * @brief Arduino firmware composition root for the IOFusion demo application.
 */
#include <Arduino.h>


#include "version_info.h"
#include "iofusion_avr_timer2_scheduler.h"
#include "iofusion_analog_sampler.h"
#include "iofusion_digital_signal_meter.h"
#include "iofusion_quadrature_signal_generator.h"
#include "iofusion_avr_timer1_pwm.h"
#include "serial_command_protocol.h"

namespace {
  /** @brief Static analog and digital pin mapping used by the firmware runtime. */
  struct PinMapConfig {
    const uint8_t* analogPins;
    uint8_t analogPinCount;
    const uint8_t* digitalPins;
    uint8_t digitalPinCount;
  };

  /** @brief Static pin configuration for the quadrature signal generator. */
  struct EncoderConfig {
    uint8_t outputPinA;
    uint8_t outputPinB;
    uint8_t inputPinUp;
    uint8_t inputPinDown;
  };

  /** @brief Timer-related runtime configuration. */
  struct TimingConfig {
    uint32_t timerTickHz;
    uint16_t analogRefreshPeriodMs;
  };

  /** @brief Digital measurement defaults applied during startup. */
  struct DigitalMeasurementConfig {
    uint16_t windowTicks;
    bool usePullup;
  };

  /** @brief Default PWM configuration applied during startup. */
  struct PwmConfig {
    uint32_t frequencyHz;
    uint8_t channel0DutyPercent;
    uint8_t channel1DutyPercent;
  };

  /** @brief Aggregate firmware configuration stored entirely in static data. */
  struct RuntimeConfig {
    PinMapConfig pins;
    EncoderConfig encoder;
    TimingConfig timing;
    DigitalMeasurementConfig digital;
    PwmConfig pwm;
  };

  /** @brief Runtime health bits reported by the serial protocol. */
  struct ModuleHealth {
    bool analog = false;
    bool digital = false;
    bool encoder = false;
    bool pwm = false;
    bool timer = false;
  };

  constexpr uint8_t kAnalogPins[] = {0, 1, 2, 3, 4, 5};
  constexpr uint8_t kDigitalPins[] = {2, 3, 8, 11, 12, 13};
  // Encoder control pins 6/7 assume a push-pull source that drives LOW when idle.

  const RuntimeConfig kRuntimeConfig = {
    {kAnalogPins, static_cast<uint8_t>(sizeof(kAnalogPins) / sizeof(kAnalogPins[0])),
     kDigitalPins, static_cast<uint8_t>(sizeof(kDigitalPins) / sizeof(kDigitalPins[0]))},
    {4, 5, 6, 7},
    {10000U, 100},
    {500, false},
    {100U, 50U, 25U},
  };

  /** @brief Single static composition root for all firmware modules. */
  class FirmwareRuntime {
  public:
    /** @brief Builds the runtime around one immutable static configuration block. */
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

    /** @brief Initializes peripherals, protocol state, and timer-driven modules. */
    void setup() {
      _analogRefreshTicks = computeAnalogRefreshTicks(
        _config.timing.timerTickHz,
        _config.timing.analogRefreshPeriodMs);

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
        _config.digital.windowTicks,
        _config.timing.timerTickHz,
        _config.digital.usePullup);
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

      _health.timer = _timer2Scheduler.beginHz(_config.timing.timerTickHz);
      if (_health.timer) {
        _health.timer = _timer2Scheduler.attachCallback(timerTickHandler);
      }
      if (!_health.timer) {
        Serial.println(F("{\"error\":\"timer2 init failed\"}"));
      }

      refreshProtocolStatus();
    }

    /** @brief Services loop-context work that must stay out of the ISR path. */
    void loop() {
      if (_health.analog) _analogSampler.sampleIfDue();
      if (_health.digital) _digitalSignalMeter.updateIfReady();
      _cmdProtocol.processSerial();
    }

    /** @brief Timer2 ISR trampoline target used to fan out periodic work. */
    void onTimerTickIsr() {
      if (_health.analog) {
        ++_analogRefreshTickCounter;
        if (_analogRefreshTickCounter >= _analogRefreshTicks) {
          _analogRefreshTickCounter = 0;
          _analogSampler.onTick();
        }
      }
      if (_health.digital) _digitalSignalMeter.onTick();
      if (_health.encoder) _quadratureGenerator.onTick();
    }

  private:
    static uint16_t computeAnalogRefreshTicks(uint32_t tickHz, uint16_t refreshPeriodMs) {
      if (tickHz == 0U || refreshPeriodMs == 0U) return 1U;
      uint32_t ticks = (tickHz * static_cast<uint32_t>(refreshPeriodMs) + 999U) / 1000U;
      if (ticks == 0U) return 1U;
      if (ticks > 65535U) return 65535U;
      return static_cast<uint16_t>(ticks);
    }

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
    uint16_t _analogRefreshTicks = 1U;
    uint16_t _analogRefreshTickCounter = 0U;
  };

  FirmwareRuntime runtime(kRuntimeConfig);

  /** @brief Free-function trampoline attached to Timer2 ISR callback dispatch. */
  void timerTickHandler() {
    runtime.onTimerTickIsr();
  }
}

/** @brief Arduino setup entry point. */
void setup() {
  runtime.setup();
}

/** @brief Arduino main loop entry point. */
void loop() {
  runtime.loop();
}

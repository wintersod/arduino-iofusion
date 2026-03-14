#ifndef SERIAL_COMMAND_PROTOCOL_H
#define SERIAL_COMMAND_PROTOCOL_H

#include <Arduino.h>

#include "iofusion_analog_sampler.h"
#include "iofusion_digital_signal_meter.h"
#include "iofusion_quadrature_signal_generator.h"
#include "iofusion_avr_timer1_pwm.h"

#ifndef IOFUSION_PROTOCOL_DEBUG
#define IOFUSION_PROTOCOL_DEBUG 0
#endif

namespace IOFusion {

// Compact serial API for sensor queries and PWM control.
// High-rate responses use integer engineering units to reduce AVR serial and formatting overhead.
class SerialCommandProtocol {
public:
  SerialCommandProtocol(AnalogSampler& analog,
                        DigitalSignalMeter& digi,
                        QuadratureSignalGenerator& encoder,
                        AvrTimer1Pwm& pwm,
                        const uint8_t* analogPins,
                        uint8_t analogCount,
                        const uint8_t* digitalPins,
                        uint8_t digitalCount);

  // Publishes current module health for the status command.
  void setModuleStatus(bool analogOk,
                       bool digiOk,
                       bool encoderOk,
                       bool pwmOk,
                       bool timerOk);
  // Consumes serial input, parses one-line ASCII commands, and emits compact JSON responses.
  void processSerial();

private:
  static bool parseUnsignedLong(const char* token, unsigned long& value);
  void printJsonBool(bool value);
  void beginOkResponse();
  void endResponse();
  void respondError(const __FlashStringHelper* code, const __FlashStringHelper* message);
  void respondOkAck(const __FlashStringHelper* operation);
  void respondAnalog();
  void respondDigital();
  void respondEncoder();
  void respondStatus();
  void respondCapabilities();
  void respondHelp();
  void handleCommand(char* cmd);
  void dispatchCommand();

  AnalogSampler& _analog;
  DigitalSignalMeter& _digi;
  QuadratureSignalGenerator& _encoder;
  AvrTimer1Pwm& _pwm;
  const uint8_t* _analogPins;
  uint8_t _analogCount;
  const uint8_t* _digitalPins;
  uint8_t _digitalCount;
  bool _analogOk = false;
  bool _digiOk = false;
  bool _encoderOk = false;
  bool _pwmOk = false;
  bool _timerOk = false;

  static constexpr size_t kCmdBufferSize = 64;
  static constexpr unsigned long kCmdIdleTimeoutMs = 75;
  char _cmdBuffer[kCmdBufferSize] = {0};
  size_t _cmdLength = 0;
  unsigned long _lastByteTimeMs = 0;
};

} // namespace IOFusion

#endif // SERIAL_COMMAND_PROTOCOL_H
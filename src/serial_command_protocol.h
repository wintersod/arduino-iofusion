/**
 * @file serial_command_protocol.h
 * @brief Compact serial command protocol for IOFusion firmware control and status.
 */
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

/**
 * @brief Compact serial API for sensor queries and PWM control.
 *
 * High-rate responses use integer engineering units to reduce AVR serial and formatting
 * overhead. Encoder responses report current direction and position as lightweight status
 * fields, not as a guaranteed atomic snapshot pair.
 */
class SerialCommandProtocol {
public:
  /**
   * @brief Constructs the protocol dispatcher.
   * @param analog Analog sampler backing the `analog?` query.
   * @param digi Digital meter backing the `digital?` query.
   * @param encoder Quadrature generator backing the `encoder?` query.
   * @param pwm PWM driver backing write commands.
   * @param analogPins Ordered analog pin list used by protocol metadata.
   * @param analogCount Number of entries in @p analogPins.
   * @param digitalPins Ordered digital pin list used by protocol metadata.
   * @param digitalCount Number of entries in @p digitalPins.
   */
  SerialCommandProtocol(AnalogSampler& analog,
                        DigitalSignalMeter& digi,
                        QuadratureSignalGenerator& encoder,
                        AvrTimer1Pwm& pwm,
                        const uint8_t* analogPins,
                        uint8_t analogCount,
                        const uint8_t* digitalPins,
                        uint8_t digitalCount);

  /**
   * @brief Publishes current module health for the `status` command.
   */
  void setModuleStatus(bool analogOk,
                       bool digiOk,
                       bool encoderOk,
                       bool pwmOk,
                       bool timerOk);
  /**
   * @brief Consumes serial input and emits responses for any complete commands.
   *
   * Oversized frames are discarded until newline rather than truncated into a different command.
   */
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
  bool _discardingFrame = false;
};

} // namespace IOFusion

#endif // SERIAL_COMMAND_PROTOCOL_H
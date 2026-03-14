#ifndef CMDLINE_H
#define CMDLINE_H

#include <Arduino.h>

#include "analog.h"
#include "digiin.h"
#include "encoder.h"
#include "pwm.h"

class CmdLine {
public:
  CmdLine(AnalogSampler& analog,
          DigiIn& digi,
          EncoderGenerator& encoder,
          Timer1PWM& pwm,
          const uint8_t* analogPins,
          uint8_t analogCount,
          const uint8_t* digitalPins,
          uint8_t digitalCount);

  void setModuleStatus(bool analogOk,
                       bool digiOk,
                       bool encoderOk,
                       bool pwmOk,
                       bool timerOk);
  void processSerial();

private:
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
  DigiIn& _digi;
  EncoderGenerator& _encoder;
  Timer1PWM& _pwm;
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

#endif // CMDLINE_H

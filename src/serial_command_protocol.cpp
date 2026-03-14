#include "serial_command_protocol.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

namespace IOFusion {

SerialCommandProtocol::SerialCommandProtocol(AnalogSampler& analog,
                                             DigitalSignalMeter& digi,
                                             QuadratureSignalGenerator& encoder,
                                             AvrTimer1Pwm& pwm,
                                             const uint8_t* analogPins,
                                             uint8_t analogCount,
                                             const uint8_t* digitalPins,
                                             uint8_t digitalCount)
  : _analog(analog),
    _digi(digi),
    _encoder(encoder),
    _pwm(pwm),
    _analogPins(analogPins),
    _analogCount(analogCount),
    _digitalPins(digitalPins),
    _digitalCount(digitalCount) {}

void SerialCommandProtocol::setModuleStatus(bool analogOk,
                                            bool digiOk,
                                            bool encoderOk,
                                            bool pwmOk,
                                            bool timerOk) {
  _analogOk = analogOk;
  _digiOk = digiOk;
  _encoderOk = encoderOk;
  _pwmOk = pwmOk;
  _timerOk = timerOk;
}

bool SerialCommandProtocol::parseUnsignedLong(const char* token, unsigned long& value) {
  if (token == nullptr || *token == '\0') return false;
  char* endp = nullptr;
  value = strtoul(token, &endp, 10);
  return endp != token && *endp == '\0';
}

void SerialCommandProtocol::printJsonBool(bool value) {
  Serial.print(value ? F("true") : F("false"));
}

void SerialCommandProtocol::beginOkResponse() {
#if IOFUSION_PROTOCOL_DEBUG
  Serial.print(F("{\"api\":\"1\",\"status\":\"ok\",\"data\":"));
#endif
}

void SerialCommandProtocol::endResponse() {
#if IOFUSION_PROTOCOL_DEBUG
  Serial.println(F("}"));
#else
  Serial.println();
#endif
}

void SerialCommandProtocol::respondError(const __FlashStringHelper* code, const __FlashStringHelper* message) {
#if IOFUSION_PROTOCOL_DEBUG
  Serial.print(F("{\"api\":\"1\",\"status\":\"error\",\"error\":{\"code\":\""));
  Serial.print(code);
  Serial.print(F("\",\"message\":\""));
  Serial.print(message);
  Serial.println(F("\"}}"));
#else
  (void)message;
  Serial.print(F("{\"err\":\""));
  Serial.print(code);
  Serial.println(F("\"}"));
#endif
}

void SerialCommandProtocol::respondOkAck(const __FlashStringHelper* operation) {
#if IOFUSION_PROTOCOL_DEBUG
  beginOkResponse();
  Serial.print(F("{\"operation\":\""));
  Serial.print(operation);
  Serial.print(F("\",\"result\":\"ok\"}"));
  endResponse();
#else
  Serial.print(F("{\"ok\":\""));
  Serial.print(operation);
  Serial.println(F("\"}"));
#endif
}

void SerialCommandProtocol::respondAnalog() {
  beginOkResponse();
#if IOFUSION_PROTOCOL_DEBUG
  Serial.print(F("{\"mv\":["));
#else
  Serial.print(F("{\"mv\":["));
#endif
  for (uint8_t i = 0; i < _analogCount; ++i) {
    Serial.print(_analog.getMilliVolts(i));
    if (i + 1 < _analogCount) Serial.print(F(","));
  }
  Serial.print(F("]}"));
  endResponse();
}

void SerialCommandProtocol::respondDigital() {
  beginOkResponse();
#if IOFUSION_PROTOCOL_DEBUG
  Serial.print(F("{\"f_dhz\":"));
  Serial.print(F("["));
  for (uint8_t i = 0; i < _digitalCount; ++i) {
    Serial.print(_digi.getFrequencyDeciHz(i));
    if (i + 1 < _digitalCount) Serial.print(F(","));
  }
  Serial.print(F("]"));
  Serial.print(F(",\"d_dpct\":"));
  Serial.print(F("["));
  for (uint8_t i = 0; i < _digitalCount; ++i) {
    Serial.print(_digi.getDutyDeciPercent(i));
    if (i + 1 < _digitalCount) Serial.print(F(","));
  }
  Serial.print(F("]"));
#else
  Serial.print(F("{\"f\":["));
  for (uint8_t i = 0; i < _digitalCount; ++i) {
    Serial.print(_digi.getFrequencyDeciHz(i));
    if (i + 1 < _digitalCount) Serial.print(F(","));
  }
  Serial.print(F("],\"d\":["));
  for (uint8_t i = 0; i < _digitalCount; ++i) {
    Serial.print(_digi.getDutyDeciPercent(i));
    if (i + 1 < _digitalCount) Serial.print(F(","));
  }
  Serial.print(F("]"));
#endif
  Serial.print(F("}"));
  endResponse();
}

void SerialCommandProtocol::respondEncoder() {
  beginOkResponse();
#if IOFUSION_PROTOCOL_DEBUG
  Serial.print(F("{\"dir\":"));
  Serial.print(_encoder.getDirection() ? 1 : 0);
  Serial.print(F(",\"pos\":"));
  Serial.print(_encoder.getPosition());
  Serial.print(F("}"));
#else
  Serial.print(F("{\"e\":["));
  Serial.print(_encoder.getDirection() ? 1 : 0);
  Serial.print(F(","));
  Serial.print(_encoder.getPosition());
  Serial.print(F("]}"));
#endif
  endResponse();
}

void SerialCommandProtocol::respondStatus() {
  beginOkResponse();
#if IOFUSION_PROTOCOL_DEBUG
  Serial.print(F("{\"modules\":{"));
  Serial.print(F("\"analog\":"));
  printJsonBool(_analogOk);
  Serial.print(F(",\"digital\":"));
  printJsonBool(_digiOk);
  Serial.print(F(",\"encoder\":"));
  printJsonBool(_encoderOk);
  Serial.print(F(",\"pwm\":"));
  printJsonBool(_pwmOk);
  Serial.print(F(",\"timer2\":"));
  printJsonBool(_timerOk);
  Serial.print(F("},\"counts\":{\"analog\":"));
  Serial.print(_analogCount);
  Serial.print(F(",\"digital\":"));
  Serial.print(_digitalCount);
  Serial.print(F("}}"));
#else
  Serial.print(F("{\"m\":["));
  Serial.print(_analogOk ? 1 : 0);
  Serial.print(F(","));
  Serial.print(_digiOk ? 1 : 0);
  Serial.print(F(","));
  Serial.print(_encoderOk ? 1 : 0);
  Serial.print(F(","));
  Serial.print(_pwmOk ? 1 : 0);
  Serial.print(F(","));
  Serial.print(_timerOk ? 1 : 0);
  Serial.print(F("],\"c\":["));
  Serial.print(_analogCount);
  Serial.print(F(","));
  Serial.print(_digitalCount);
  Serial.print(F("]}"));
#endif
  endResponse();
}

void SerialCommandProtocol::respondCapabilities() {
  beginOkResponse();
#if IOFUSION_PROTOCOL_DEBUG
  Serial.print(F("{\"commands\":[\"analog?\",\"digital?\",\"encoder?\",\"pwm-freq\",\"pwm-duty\",\"status\",\"capabilities\",\"help\"],\"units\":{\"analog\":\"mV\",\"freq\":\"0.1Hz\",\"duty\":\"0.1pct\",\"encoder_dir\":\"1=up,0=down\"},\"pwm\":{\"channels\":2,\"duty_range_pct\":[0,100]},\"pins\":{\"analog\":["));
#else
  Serial.print(F("{\"cmd\":[\"analog?\",\"digital?\",\"encoder?\",\"pwm-freq\",\"pwm-duty\",\"status\",\"capabilities\",\"help\"],\"u\":[\"mV\",\"0.1Hz\",\"0.1pct\",\"1=up\"],\"p\":[2,0,100],\"a\":["));
#endif
  for (uint8_t i = 0; i < _analogCount; ++i) {
    Serial.print(_analogPins[i]);
    if (i + 1 < _analogCount) Serial.print(F(","));
  }
#if IOFUSION_PROTOCOL_DEBUG
  Serial.print(F("],\"digital\":["));
#else
  Serial.print(F("],\"d\":["));
#endif
  for (uint8_t i = 0; i < _digitalCount; ++i) {
    Serial.print(_digitalPins[i]);
    if (i + 1 < _digitalCount) Serial.print(F(","));
  }
#if IOFUSION_PROTOCOL_DEBUG
  Serial.print(F("]}}"));
#else
  Serial.print(F("]}"));
#endif
  endResponse();
}

void SerialCommandProtocol::respondHelp() {
#if IOFUSION_PROTOCOL_DEBUG
  beginOkResponse();
  Serial.print(F("{\"usage\":\"analog? | digital? | encoder? | pwm-freq <hz> | pwm-duty <ch> <pct> | status | capabilities | help\"}"));
  endResponse();
#else
  Serial.print(F("{\"cmd\":[\"analog?\",\"digital?\",\"encoder?\",\"pwm-freq\",\"pwm-duty\",\"status\",\"capabilities\",\"help\"]}"));
#endif
}

void SerialCommandProtocol::handleCommand(char* cmd) {
  while (*cmd && isspace(static_cast<unsigned char>(*cmd))) ++cmd;
  if (*cmd == '\0') return;

  char* tokens[4];
  uint8_t tokenCount = 0;
  char* tok = strtok(cmd, " ");
  while (tok && tokenCount < 4) {
    tokens[tokenCount++] = tok;
    tok = strtok(nullptr, " ");
  }
  if (tokenCount == 0) return;

  for (char* p = tokens[0]; *p; ++p) {
    *p = tolower(static_cast<unsigned char>(*p));
  }

  if (strcmp(tokens[0], "analog?") == 0) {
    respondAnalog();
    return;
  }

  if (strcmp(tokens[0], "digital?") == 0) {
    respondDigital();
    return;
  }

  if (strcmp(tokens[0], "encoder?") == 0) {
    respondEncoder();
    return;
  }

  if (strcmp(tokens[0], "pwm-freq") == 0) {
    if (tokenCount < 2) {
      respondError(F("missing_frequency"), F("missing frequency"));
      return;
    }
    unsigned long freqVal = 0;
    if (!parseUnsignedLong(tokens[1], freqVal) || freqVal == 0UL) {
      respondError(F("invalid_frequency"), F("invalid frequency"));
      return;
    }
    if (_pwm.begin(static_cast<uint32_t>(freqVal))) {
      _pwmOk = true;
      respondOkAck(F("pwm-freq"));
    } else {
      _pwmOk = false;
      respondError(F("pwm_frequency_set_failed"), F("unable to set frequency"));
    }
    return;
  }

  if (strcmp(tokens[0], "pwm-duty") == 0) {
    if (tokenCount < 3) {
      respondError(F("missing_duty_parameters"), F("missing duty parameters"));
      return;
    }
    unsigned long channelVal = 0;
    if (!parseUnsignedLong(tokens[1], channelVal)) {
      respondError(F("invalid_channel"), F("invalid channel"));
      return;
    }
    if (channelVal > 1UL) {
      respondError(F("invalid_channel"), F("invalid channel"));
      return;
    }
    unsigned long dutyVal = 0;
    if (!parseUnsignedLong(tokens[2], dutyVal)) {
      respondError(F("invalid_duty"), F("invalid duty"));
      return;
    }
    if (dutyVal > 100UL) {
      respondError(F("duty_out_of_range"), F("duty must be in range 0..100"));
      return;
    }
    _pwm.setDuty(static_cast<uint8_t>(channelVal), static_cast<uint8_t>(dutyVal));
    respondOkAck(F("pwm-duty"));
    return;
  }

  if (strcmp(tokens[0], "status") == 0) {
    respondStatus();
    return;
  }

  if (strcmp(tokens[0], "capabilities") == 0) {
    respondCapabilities();
    return;
  }

  if (strcmp(tokens[0], "help") == 0) {
    respondHelp();
    return;
  }

  respondError(F("unknown_command"), F("unknown command"));
}

void SerialCommandProtocol::dispatchCommand() {
  if (_cmdLength == 0) return;
  _cmdBuffer[_cmdLength] = '\0';
  handleCommand(_cmdBuffer);
  _cmdLength = 0;
  _cmdBuffer[0] = '\0';
}

void SerialCommandProtocol::processSerial() {
  while (Serial.available() > 0) {
    char c = static_cast<char>(Serial.read());
    if (c == '\r' || c == '\n') {
      dispatchCommand();
      _lastByteTimeMs = 0;
    } else {
      if (_cmdLength < kCmdBufferSize - 1) {
        _cmdBuffer[_cmdLength++] = c;
      }
      _lastByteTimeMs = millis();
    }
  }
  if (_cmdLength > 0 && _lastByteTimeMs != 0) {
    unsigned long now = millis();
    if (now - _lastByteTimeMs >= kCmdIdleTimeoutMs) {
      dispatchCommand();
      _lastByteTimeMs = 0;
    }
  }
}

} // namespace IOFusion
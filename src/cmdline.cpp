#include "cmdline.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

CmdLine::CmdLine(AnalogSampler& analog,
                 DigiIn& digi,
                 EncoderGenerator& encoder,
                 Timer1PWM& pwm,
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

void CmdLine::setModuleStatus(bool analogOk,
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

void CmdLine::printJsonBool(bool value) {
  Serial.print(value ? F("true") : F("false"));
}

void CmdLine::beginOkResponse() {
  Serial.print(F("{\"api\":\"1\",\"status\":\"ok\",\"data\":"));
}

void CmdLine::endResponse() {
  Serial.println(F("}"));
}

void CmdLine::respondError(const __FlashStringHelper* code, const __FlashStringHelper* message) {
  Serial.print(F("{\"api\":\"1\",\"status\":\"error\",\"error\":{\"code\":\""));
  Serial.print(code);
  Serial.print(F("\",\"message\":\""));
  Serial.print(message);
  Serial.println(F("\"}}"));
}

void CmdLine::respondOkAck(const __FlashStringHelper* operation) {
  beginOkResponse();
  Serial.print(F("{\"operation\":\""));
  Serial.print(operation);
  Serial.print(F("\",\"result\":\"ok\"}"));
  endResponse();
}

void CmdLine::respondAnalog() {
  beginOkResponse();
  Serial.print(F("{\"analog\":{"));
  for (uint8_t i = 0; i < _analogCount; ++i) {
    Serial.print(F("\"a"));
    Serial.print(_analogPins[i]);
    Serial.print(F("\":"));
    Serial.print(_analog.getValue(i), 3);
    if (i + 1 < _analogCount) Serial.print(F(","));
  }
  Serial.print(F("}}"));
  endResponse();
}

void CmdLine::respondDigital() {
  beginOkResponse();
  Serial.print(F("{\"digital\":{"));
  for (uint8_t i = 0; i < _digitalCount; ++i) {
    Serial.print(F("\"d"));
    Serial.print(_digitalPins[i]);
    Serial.print(F("\":{\"freq\":"));
    Serial.print(_digi.getFrequency(i), 1);
    Serial.print(F(",\"duty\":"));
    Serial.print(_digi.getDutyCycle(i), 1);
    Serial.print(F("}"));
    if (i + 1 < _digitalCount) Serial.print(F(","));
  }
  Serial.print(F("}}"));
  endResponse();
}

void CmdLine::respondEncoder() {
  beginOkResponse();
  Serial.print(F("{\"encoder\":{\"direction\":\""));
  Serial.print(_encoder.getDirection() ? F("UP") : F("DOWN"));
  Serial.print(F("\",\"position\":"));
  Serial.print(_encoder.getPosition());
  Serial.print(F("}}"));
  endResponse();
}

void CmdLine::respondStatus() {
  beginOkResponse();
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
  endResponse();
}

void CmdLine::respondCapabilities() {
  beginOkResponse();
  Serial.print(F("{\"commands\":[\"analog?\",\"digital?\",\"encoder?\",\"pwm-freq\",\"pwm-duty\",\"status\",\"capabilities\",\"help\"],\"pwm\":{\"channels\":2,\"duty_range_pct\":[0,100]},\"pins\":{\"analog\":["));
  for (uint8_t i = 0; i < _analogCount; ++i) {
    Serial.print(_analogPins[i]);
    if (i + 1 < _analogCount) Serial.print(F(","));
  }
  Serial.print(F("],\"digital\":["));
  for (uint8_t i = 0; i < _digitalCount; ++i) {
    Serial.print(_digitalPins[i]);
    if (i + 1 < _digitalCount) Serial.print(F(","));
  }
  Serial.print(F("]}}"));
  endResponse();
}

void CmdLine::respondHelp() {
  beginOkResponse();
  Serial.print(F("{\"usage\":\"analog? | digital? | encoder? | pwm-freq <hz> | pwm-duty <ch> <pct> | status | capabilities | help\"}"));
  endResponse();
}

void CmdLine::handleCommand(char* cmd) {
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
    char* endp = nullptr;
    double freqVal = strtod(tokens[1], &endp);
    if (endp == tokens[1] || *endp != '\0' || freqVal <= 0.0) {
      respondError(F("invalid_frequency"), F("invalid frequency"));
      return;
    }
    float freq = static_cast<float>(freqVal);
    if (_pwm.begin(freq)) {
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
    char* endp = nullptr;
    long channelVal = strtol(tokens[1], &endp, 10);
    if (endp == tokens[1] || *endp != '\0') {
      respondError(F("invalid_channel"), F("invalid channel"));
      return;
    }
    int channel = static_cast<int>(channelVal);
    if (channel < 0 || channel > 1) {
      respondError(F("invalid_channel"), F("invalid channel"));
      return;
    }
    endp = nullptr;
    double dutyVal = strtod(tokens[2], &endp);
    if (endp == tokens[2] || *endp != '\0') {
      respondError(F("invalid_duty"), F("invalid duty"));
      return;
    }
    if (dutyVal < 0.0 || dutyVal > 100.0) {
      respondError(F("duty_out_of_range"), F("duty must be in range 0..100"));
      return;
    }
    float duty = static_cast<float>(dutyVal);
    _pwm.setDuty(static_cast<uint8_t>(channel), duty);
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

void CmdLine::dispatchCommand() {
  if (_cmdLength == 0) return;
  _cmdBuffer[_cmdLength] = '\0';
  handleCommand(_cmdBuffer);
  _cmdLength = 0;
  _cmdBuffer[0] = '\0';
}

void CmdLine::processSerial() {
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

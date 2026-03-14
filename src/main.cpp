#include <Arduino.h>


#include "version_info.h"
#include "timer.h"
#include "analog.h"
#include "digiin.h"
#include "encoder.h"
#include "pwm.h"
#include "cmdline.h"

namespace {
  constexpr float kTimerTickHz = 10000.0f; // Timer2 tick frequency

  Timer2Driver timer2;
  AnalogSampler analogSampler;
  DigiIn digiIn;
  EncoderGenerator encoder;
  Timer1PWM pwm;

  bool analogOk = false;
  bool digiOk = false;
  bool encoderOk = false;
  bool pwmOk = false;
  bool timerOk = false;

  const uint8_t kAnalogPins[] = {0, 1, 2, 3, 4, 5};
  const uint8_t kDigitalPins[] = {2, 3, 8, 11, 12, 13};
  // pwm 9,10

  CmdLine cmdLine(
    analogSampler,
    digiIn,
    encoder,
    pwm,
    kAnalogPins,
    static_cast<uint8_t>(sizeof(kAnalogPins) / sizeof(kAnalogPins[0])),
    kDigitalPins,
    static_cast<uint8_t>(sizeof(kDigitalPins) / sizeof(kDigitalPins[0]))
  );

  void timerTickHandler() {
    if (analogOk) analogSampler.onTick();
    if (digiOk) digiIn.onTick();
    if (encoderOk) encoder.onTick();
  }

  void processSerial() {
    cmdLine.processSerial();
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.print("Firmware version: ");
  Serial.println(FW_VERSION);

  pinMode(LED_BUILTIN, OUTPUT);

  analogSampler.begin(kAnalogPins, static_cast<uint8_t>(sizeof(kAnalogPins) / sizeof(kAnalogPins[0])));
  analogOk = analogSampler.getChannelCount() > 0;
  if (!analogOk) Serial.println(F("{\"error\":\"analog init failed\"}"));

  digiOk = digiIn.begin(kDigitalPins, static_cast<uint8_t>(sizeof(kDigitalPins) / sizeof(kDigitalPins[0])), 500, kTimerTickHz, true);
  if (!digiOk) Serial.println(F("{\"error\":\"digital init failed\"}"));

  encoderOk = encoder.begin(4, 5, 6, 7);
  if (!encoderOk) Serial.println(F("{\"error\":\"encoder init failed\"}"));

  pwmOk = pwm.begin(100.0f);
  if (!pwmOk) {
    Serial.println(F("{\"error\":\"pwm init failed\"}"));
  } else {
    pwm.setDuty(0, 50.0f);
    pwm.setDuty(1, 25.0f);
  }

  timerOk = timer2.beginHz(kTimerTickHz) > 0;
  if (!timerOk) {
    Serial.println(F("{\"error\":\"timer2 init failed\"}"));
  } else {
    timer2.attachCallback(timerTickHandler);
  }

  cmdLine.setModuleStatus(analogOk, digiOk, encoderOk, pwmOk, timerOk);
}

void loop() {
  if (analogOk) analogSampler.sampleIfDue();
  if (digiOk) digiIn.updateIfReady();
  processSerial();
}

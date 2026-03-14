#include <unity.h>

#include "Arduino.h"
#include "analog.h"
#include "cmdline.h"
#include "digiin.h"
#include "encoder.h"
#include "pwm.h"

Timer1PWM::Timer1PWM() {}

bool Timer1PWM::begin(float freqHz) {
  return freqHz > 0.0f;
}

void Timer1PWM::setDuty(uint8_t, float) {}

void Timer1PWM::stop() {}

#include "../../src/cmdline.cpp"

namespace {
  void setDigitalPin(uint8_t pin, bool high) {
    uint8_t port = digitalPinToPort(pin);
    uint8_t mask = digitalPinToBitMask(pin);
    if (port == NOT_A_PIN) return;
    if (high) mockPortIn[port] |= mask;
    else mockPortIn[port] &= static_cast<uint8_t>(~mask);
  }

  void clearPorts() {
    for (uint8_t i = 0; i < 8; ++i) {
      mockPortIn[i] = 0;
      mockPortOut[i] = 0;
    }
  }
}

void setUp() {
  mockMillis = 0;
  for (int i = 0; i < 16; ++i) mockAnalogValues[i] = 0;
  clearPorts();
  Serial.clearOutput();
  Serial.setInput("");
}

void tearDown() {}

void test_analog_sampler_basic() {
  AnalogSampler sampler;
  const uint8_t channels[] = {0, 1};
  TEST_ASSERT_TRUE(sampler.begin(channels, 2));

  mockAnalogValues[0] = 512;
  mockAnalogValues[1] = 1023;

  sampler.onTick();
  sampler.sampleIfDue();

  sampler.setVref(5.0f);
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 2.502f, sampler.getValue(0));
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 5.000f, sampler.getValue(1));
}

void test_analog_sampler_invalid_channel() {
  AnalogSampler sampler;
  const uint8_t channels[] = {6};
  TEST_ASSERT_FALSE(sampler.begin(channels, 1));
}

void test_digiin_frequency_and_duty() {
  DigiIn digi;
  const uint8_t pins[] = {2};
  TEST_ASSERT_TRUE(digi.begin(pins, 1, 4, 1000.0f, false));

  setDigitalPin(2, false); // sample 1
  digi.onTick();
  setDigitalPin(2, true);  // sample 2
  digi.onTick();
  setDigitalPin(2, true);  // sample 3
  digi.onTick();
  setDigitalPin(2, false); // sample 4
  digi.onTick();

  digi.updateIfReady();

  TEST_ASSERT_FLOAT_WITHIN(0.1f, 250.0f, digi.getFrequency(0));
  TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, digi.getDutyCycle(0));
}

void test_encoder_generator_steps() {
  EncoderGenerator enc;
  TEST_ASSERT_TRUE(enc.begin(9, 10, 2, 3));

  setDigitalPin(2, true);
  setDigitalPin(3, false);
  enc.onTick();
  enc.onTick();
  enc.onTick();

  TEST_ASSERT_EQUAL_INT32(3, enc.getPosition());
  TEST_ASSERT_TRUE(enc.getDirection());
}

void test_cmdline_status_response_envelope() {
  AnalogSampler analog;
  DigiIn digi;
  EncoderGenerator encoder;
  Timer1PWM pwm;
  const uint8_t analogPins[] = {0, 1};
  const uint8_t digitalPins[] = {2, 3};

  TEST_ASSERT_TRUE(analog.begin(analogPins, 2));
  TEST_ASSERT_TRUE(digi.begin(digitalPins, 2, 4, 1000.0f, false));
  TEST_ASSERT_TRUE(encoder.begin(9, 10, 4, 5));

  CmdLine cmd(analog, digi, encoder, pwm, analogPins, 2, digitalPins, 2);
  cmd.setModuleStatus(true, false, true, true, false);

  Serial.setInput("status\n");
  cmd.processSerial();

  const std::string& output = Serial.getOutput();
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"api\":\"1\""));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"status\":\"ok\""));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"analog\":true"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"digital\":false"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"timer2\":false"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"counts\":{\"analog\":2,\"digital\":2}"));
}

void test_cmdline_capabilities_response_lists_commands_and_pins() {
  AnalogSampler analog;
  DigiIn digi;
  EncoderGenerator encoder;
  Timer1PWM pwm;
  const uint8_t analogPins[] = {0, 1, 2};
  const uint8_t digitalPins[] = {8, 11};

  TEST_ASSERT_TRUE(analog.begin(analogPins, 3));
  TEST_ASSERT_TRUE(digi.begin(digitalPins, 2, 4, 1000.0f, false));
  TEST_ASSERT_TRUE(encoder.begin(9, 10, 4, 5));

  CmdLine cmd(analog, digi, encoder, pwm, analogPins, 3, digitalPins, 2);

  Serial.setInput("capabilities\n");
  cmd.processSerial();

  const std::string& output = Serial.getOutput();
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"commands\":[\"analog?\",\"digital?\",\"encoder?\",\"pwm-freq\",\"pwm-duty\",\"status\",\"capabilities\",\"help\"]"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"analog\":[0,1,2]"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"digital\":[8,11]"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"duty_range_pct\":[0,100]"));
}

void test_cmdline_rejects_out_of_range_pwm_duty() {
  AnalogSampler analog;
  DigiIn digi;
  EncoderGenerator encoder;
  Timer1PWM pwm;
  const uint8_t analogPins[] = {0};
  const uint8_t digitalPins[] = {2};

  TEST_ASSERT_TRUE(analog.begin(analogPins, 1));
  TEST_ASSERT_TRUE(digi.begin(digitalPins, 1, 4, 1000.0f, false));
  TEST_ASSERT_TRUE(encoder.begin(9, 10, 4, 5));

  CmdLine cmd(analog, digi, encoder, pwm, analogPins, 1, digitalPins, 1);

  Serial.setInput("pwm-duty 0 101\n");
  cmd.processSerial();

  const std::string& output = Serial.getOutput();
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"status\":\"error\""));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"code\":\"duty_out_of_range\""));
}

void test_cmdline_analog_query_uses_response_envelope() {
  AnalogSampler analog;
  DigiIn digi;
  EncoderGenerator encoder;
  Timer1PWM pwm;
  const uint8_t analogPins[] = {0};
  const uint8_t digitalPins[] = {2};

  TEST_ASSERT_TRUE(analog.begin(analogPins, 1));
  TEST_ASSERT_TRUE(digi.begin(digitalPins, 1, 4, 1000.0f, false));
  TEST_ASSERT_TRUE(encoder.begin(9, 10, 4, 5));

  mockAnalogValues[0] = 512;
  analog.onTick();
  analog.sampleIfDue();

  CmdLine cmd(analog, digi, encoder, pwm, analogPins, 1, digitalPins, 1);

  Serial.setInput("analog?\n");
  cmd.processSerial();

  const std::string& output = Serial.getOutput();
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"status\":\"ok\""));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"data\":{\"analog\":{\"a0\":"));
}

int main(int argc, char** argv) {
  UNITY_BEGIN();

  RUN_TEST(test_analog_sampler_basic);
  RUN_TEST(test_analog_sampler_invalid_channel);
  RUN_TEST(test_digiin_frequency_and_duty);
  RUN_TEST(test_encoder_generator_steps);
  RUN_TEST(test_cmdline_status_response_envelope);
  RUN_TEST(test_cmdline_capabilities_response_lists_commands_and_pins);
  RUN_TEST(test_cmdline_rejects_out_of_range_pwm_duty);
  RUN_TEST(test_cmdline_analog_query_uses_response_envelope);

  return UNITY_END();
}

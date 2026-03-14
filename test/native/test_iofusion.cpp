#include <unity.h>

#include "Arduino.h"
#include "iofusion_analog_sampler.h"
#include "iofusion_digital_signal_meter.h"
#include "iofusion_quadrature_signal_generator.h"

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
}

void tearDown() {}

void test_analog_sampler_basic() {
  IOFusion::AnalogSampler sampler;
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
  IOFusion::AnalogSampler sampler;
  const uint8_t channels[] = {6};
  TEST_ASSERT_FALSE(sampler.begin(channels, 1));
}

void test_digiin_frequency_and_duty() {
  IOFusion::DigitalSignalMeter digi;
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

  TEST_ASSERT_EQUAL_UINT32(2500, digi.getFrequencyDeciHz(0));
  TEST_ASSERT_EQUAL_UINT16(500, digi.getDutyDeciPercent(0));
}

void test_encoder_generator_steps() {
  IOFusion::QuadratureSignalGenerator enc;
  TEST_ASSERT_TRUE(enc.begin(9, 10, 2, 3));

  setDigitalPin(2, true);
  setDigitalPin(3, false);
  enc.onTick();
  enc.onTick();
  enc.onTick();

  TEST_ASSERT_EQUAL_INT32(3, enc.getPosition());
  TEST_ASSERT_TRUE(enc.getDirection());
}

int main(int argc, char** argv) {
  UNITY_BEGIN();

  RUN_TEST(test_analog_sampler_basic);
  RUN_TEST(test_analog_sampler_invalid_channel);
  RUN_TEST(test_digiin_frequency_and_duty);
  RUN_TEST(test_encoder_generator_steps);

  return UNITY_END();
}

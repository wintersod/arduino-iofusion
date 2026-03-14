#include <unity.h>

#include "Arduino.h"
#include "iofusion_analog_sampler.h"

void test_analog_sampler_basic() {
  IOFusion::AnalogSampler sampler;
  const uint8_t channels[] = {0, 1};
  TEST_ASSERT_TRUE(sampler.begin(channels, 2));

  mockAnalogValues[0] = 512;
  mockAnalogValues[1] = 1023;

  sampler.onTick();
  sampler.sampleIfDue();

  sampler.setVrefMillivolts(5000);
  TEST_ASSERT_EQUAL_UINT16(2502, sampler.getMilliVolts(0));
  TEST_ASSERT_EQUAL_UINT16(5000, sampler.getMilliVolts(1));
}

void test_analog_sampler_invalid_channel() {
  IOFusion::AnalogSampler sampler;
  const uint8_t channels[] = {6};
  TEST_ASSERT_FALSE(sampler.begin(channels, 1));
}
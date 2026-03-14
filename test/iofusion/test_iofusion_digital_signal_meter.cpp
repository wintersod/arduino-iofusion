#include <unity.h>

#include "iofusion_digital_signal_meter.h"

#include "test_support.h"

void test_digital_signal_meter_frequency_and_duty() {
  IOFusion::DigitalSignalMeter digi;
  const uint8_t pins[] = {2};
  TEST_ASSERT_TRUE(digi.begin(pins, 1, 4, 1000.0f, false));

  setDigitalPin(2, false);
  digi.onTick();
  setDigitalPin(2, true);
  digi.onTick();
  setDigitalPin(2, true);
  digi.onTick();
  setDigitalPin(2, false);
  digi.onTick();

  digi.updateIfReady();

  TEST_ASSERT_EQUAL_UINT32(2500, digi.getFrequencyDeciHz(0));
  TEST_ASSERT_EQUAL_UINT16(500, digi.getDutyDeciPercent(0));
}
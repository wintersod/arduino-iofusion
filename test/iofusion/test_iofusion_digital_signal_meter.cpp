#include <unity.h>

#include "iofusion_digital_signal_meter.h"

#include "test_support.h"

void test_digital_signal_meter_frequency_and_duty() {
  IOFusion::DigitalSignalMeter digi;
  const uint8_t pins[] = {2};
  TEST_ASSERT_TRUE(digi.begin(pins, 1, 4, 1000U, false));

  setDigitalPin(2, false);
  digi.onTick();
  setDigitalPin(2, true);
  digi.onTick();
  setDigitalPin(2, false);
  digi.onTick();
  setDigitalPin(2, true);
  digi.onTick();

  digi.updateIfReady();

  TEST_ASSERT_EQUAL_UINT32(5000, digi.getFrequencyDeciHz(0));
  TEST_ASSERT_EQUAL_UINT16(500, digi.getDutyDeciPercent(0));
}

void test_digital_signal_meter_keeps_two_edge_windows_in_cycle_mode() {
  IOFusion::DigitalSignalMeter digi;
  const uint8_t pins[] = {2};
  TEST_ASSERT_TRUE(digi.begin(pins, 1, 10, 100U, false));

  setDigitalPin(2, false);
  digi.onTick();
  setDigitalPin(2, true);
  digi.onTick();
  setDigitalPin(2, true);
  digi.onTick();
  setDigitalPin(2, false);
  digi.onTick();
  for (uint8_t tick = 4; tick < 8; ++tick) {
    setDigitalPin(2, false);
    digi.onTick();
  }
  setDigitalPin(2, true);
  digi.onTick();
  setDigitalPin(2, true);
  digi.onTick();

  digi.updateIfReady();

  TEST_ASSERT_EQUAL_UINT32(125, digi.getFrequencyDeciHz(0));
  TEST_ASSERT_EQUAL_UINT16(250, digi.getDutyDeciPercent(0));
}

void test_digital_signal_meter_uses_completed_periods_for_three_edge_windows() {
  IOFusion::DigitalSignalMeter digi;
  const uint8_t pins[] = {2};
  TEST_ASSERT_TRUE(digi.begin(pins, 1, 10, 100U, false));

  setDigitalPin(2, false);
  digi.onTick();
  setDigitalPin(2, true);
  digi.onTick();
  setDigitalPin(2, true);
  digi.onTick();
  setDigitalPin(2, false);
  digi.onTick();
  setDigitalPin(2, false);
  digi.onTick();
  setDigitalPin(2, true);
  digi.onTick();
  setDigitalPin(2, true);
  digi.onTick();
  setDigitalPin(2, false);
  digi.onTick();
  setDigitalPin(2, false);
  digi.onTick();
  setDigitalPin(2, true);
  digi.onTick();

  digi.updateIfReady();

  TEST_ASSERT_EQUAL_UINT32(250, digi.getFrequencyDeciHz(0));
  TEST_ASSERT_EQUAL_UINT16(500, digi.getDutyDeciPercent(0));
}

void test_digital_signal_meter_reports_low_frequency_from_period() {
  IOFusion::DigitalSignalMeter digi;
  const uint8_t pins[] = {2};
  TEST_ASSERT_TRUE(digi.begin(pins, 1, 5, 100U, false));

  setDigitalPin(2, true);
  for (uint8_t tick = 0; tick < 5; ++tick) {
    if (tick == 2) setDigitalPin(2, false);
    digi.onTick();
  }
  digi.updateIfReady();
  TEST_ASSERT_EQUAL_UINT32(0, digi.getFrequencyDeciHz(0));
  TEST_ASSERT_EQUAL_UINT16(0, digi.getDutyDeciPercent(0));

  for (uint8_t tick = 0; tick < 5; ++tick) {
    setDigitalPin(2, false);
    digi.onTick();
  }
  digi.updateIfReady();
  TEST_ASSERT_EQUAL_UINT32(0, digi.getFrequencyDeciHz(0));
  TEST_ASSERT_EQUAL_UINT16(0, digi.getDutyDeciPercent(0));

  for (uint8_t tick = 0; tick < 5; ++tick) {
    if (tick == 0) setDigitalPin(2, true);
    if (tick == 2) setDigitalPin(2, false);
    digi.onTick();
  }
  digi.updateIfReady();

  TEST_ASSERT_EQUAL_UINT32(100, digi.getFrequencyDeciHz(0));
  TEST_ASSERT_EQUAL_UINT16(200, digi.getDutyDeciPercent(0));
}

void test_digital_signal_meter_clears_stale_frequency_after_missing_edges() {
  IOFusion::DigitalSignalMeter digi;
  const uint8_t pins[] = {2};
  TEST_ASSERT_TRUE(digi.begin(pins, 1, 5, 100U, false));

  setDigitalPin(2, true);
  for (uint8_t tick = 0; tick < 5; ++tick) {
    if (tick == 2) setDigitalPin(2, false);
    digi.onTick();
  }
  digi.updateIfReady();

  for (uint8_t tick = 0; tick < 5; ++tick) {
    setDigitalPin(2, false);
    digi.onTick();
  }
  digi.updateIfReady();

  for (uint8_t tick = 0; tick < 5; ++tick) {
    if (tick == 0) setDigitalPin(2, true);
    if (tick == 2) setDigitalPin(2, false);
    digi.onTick();
  }
  digi.updateIfReady();
  TEST_ASSERT_EQUAL_UINT32(100, digi.getFrequencyDeciHz(0));
  TEST_ASSERT_EQUAL_UINT16(200, digi.getDutyDeciPercent(0));

  for (uint8_t window = 0; window < 5; ++window) {
    for (uint8_t tick = 0; tick < 5; ++tick) {
      setDigitalPin(2, false);
      digi.onTick();
    }
    digi.updateIfReady();
  }

  TEST_ASSERT_EQUAL_UINT32(0, digi.getFrequencyDeciHz(0));
  TEST_ASSERT_EQUAL_UINT16(0, digi.getDutyDeciPercent(0));
}

void test_digital_signal_meter_stale_aging_advances_while_window_waits_for_loop() {
  IOFusion::DigitalSignalMeter digi;
  const uint8_t pins[] = {2};
  TEST_ASSERT_TRUE(digi.begin(pins, 1, 5, 100U, false));

  setDigitalPin(2, true);
  for (uint8_t tick = 0; tick < 5; ++tick) {
    if (tick == 2) setDigitalPin(2, false);
    digi.onTick();
  }
  digi.updateIfReady();

  for (uint8_t tick = 0; tick < 5; ++tick) {
    setDigitalPin(2, false);
    digi.onTick();
  }
  digi.updateIfReady();

  for (uint8_t tick = 0; tick < 5; ++tick) {
    if (tick == 0) setDigitalPin(2, true);
    if (tick == 2) setDigitalPin(2, false);
    digi.onTick();
  }
  digi.updateIfReady();
  TEST_ASSERT_EQUAL_UINT32(100, digi.getFrequencyDeciHz(0));
  TEST_ASSERT_EQUAL_UINT16(200, digi.getDutyDeciPercent(0));

  for (uint8_t tick = 0; tick < 5; ++tick) {
    setDigitalPin(2, false);
    digi.onTick();
  }

  for (uint8_t blockedTick = 0; blockedTick < 20; ++blockedTick) {
    setDigitalPin(2, false);
    digi.onTick();
  }

  digi.updateIfReady();

  TEST_ASSERT_EQUAL_UINT32(0, digi.getFrequencyDeciHz(0));
  TEST_ASSERT_EQUAL_UINT16(0, digi.getDutyDeciPercent(0));
}

void test_digital_signal_meter_ignores_edges_that_happened_while_window_was_pending() {
  IOFusion::DigitalSignalMeter digi;
  const uint8_t pins[] = {2};
  TEST_ASSERT_TRUE(digi.begin(pins, 1, 5, 100U, false));

  setDigitalPin(2, false);
  for (uint8_t tick = 0; tick < 5; ++tick) {
    digi.onTick();
  }

  setDigitalPin(2, true);
  for (uint8_t blockedTick = 0; blockedTick < 5; ++blockedTick) {
    digi.onTick();
  }

  digi.updateIfReady();
  TEST_ASSERT_EQUAL_UINT32(0, digi.getFrequencyDeciHz(0));
  TEST_ASSERT_EQUAL_UINT16(0, digi.getDutyDeciPercent(0));

  for (uint8_t tick = 0; tick < 5; ++tick) {
    setDigitalPin(2, true);
    digi.onTick();
  }
  digi.updateIfReady();

  TEST_ASSERT_EQUAL_UINT32(0, digi.getFrequencyDeciHz(0));
  TEST_ASSERT_EQUAL_UINT16(1000, digi.getDutyDeciPercent(0));
}
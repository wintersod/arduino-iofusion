#include <unity.h>

#include "Arduino.h"
#include "iofusion_avr_timer2_scheduler.h"

namespace {
  uint8_t callbackCountA = 0;
  uint8_t callbackCountB = 0;

  void callbackA() {
    ++callbackCountA;
  }

  void callbackB() {
    ++callbackCountB;
  }
}

void test_avr_timer2_scheduler_begin_sets_ctc_registers() {
  IOFusion::AvrTimer2Scheduler scheduler;

  TEST_ASSERT_TRUE(scheduler.beginHz(10000U));
  TEST_ASSERT_EQUAL_UINT8(_BV(WGM21), TCCR2A);
  TEST_ASSERT_EQUAL_UINT8(_BV(CS21), TCCR2B);
  TEST_ASSERT_EQUAL_UINT8(199, OCR2A);
  TEST_ASSERT_TRUE((TIMSK2 & _BV(OCIE2A)) != 0);
}

void test_avr_timer2_scheduler_begin_accepts_valid_zero_ocr() {
  IOFusion::AvrTimer2Scheduler scheduler;

  TEST_ASSERT_TRUE(scheduler.beginHz(16000000U));
  TEST_ASSERT_EQUAL_UINT8(_BV(WGM21), TCCR2A);
  TEST_ASSERT_EQUAL_UINT8(_BV(CS20), TCCR2B);
  TEST_ASSERT_EQUAL_UINT8(0, OCR2A);
  TEST_ASSERT_TRUE((TIMSK2 & _BV(OCIE2A)) != 0);
}

void test_avr_timer2_scheduler_callback_lifecycle() {
  IOFusion::AvrTimer2Scheduler scheduler;

  callbackCountA = 0;
  callbackCountB = 0;
  TEST_ASSERT_TRUE(scheduler.beginHz(10000U));
  TEST_ASSERT_TRUE(scheduler.attachCallback(callbackA));
  TEST_ASSERT_TRUE(scheduler.attachCallback(callbackA));
  TEST_ASSERT_TRUE(scheduler.attachCallback(callbackB));

  IOFusion::AvrTimer2Scheduler::handleInterrupt();
  TEST_ASSERT_EQUAL_UINT8(1, callbackCountA);
  TEST_ASSERT_EQUAL_UINT8(1, callbackCountB);

  scheduler.detachCallback(callbackA);
  IOFusion::AvrTimer2Scheduler::handleInterrupt();
  TEST_ASSERT_EQUAL_UINT8(1, callbackCountA);
  TEST_ASSERT_EQUAL_UINT8(2, callbackCountB);

  scheduler.stop();
  IOFusion::AvrTimer2Scheduler::handleInterrupt();
  TEST_ASSERT_EQUAL_UINT8(1, callbackCountA);
  TEST_ASSERT_EQUAL_UINT8(2, callbackCountB);
}
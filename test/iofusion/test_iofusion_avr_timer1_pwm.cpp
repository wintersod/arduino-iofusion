#include <unity.h>

#include "Arduino.h"
#include "iofusion_avr_timer1_pwm.h"

void test_avr_timer1_pwm_begin_sets_timer_registers() {
  IOFusion::AvrTimer1Pwm pwm;

  TEST_ASSERT_TRUE(pwm.begin(100));
  TEST_ASSERT_EQUAL_UINT8(_BV(COM1A1) | _BV(COM1B1) | _BV(WGM11), TCCR1A);
  TEST_ASSERT_EQUAL_UINT8(_BV(WGM13) | _BV(WGM12) | _BV(CS11), TCCR1B);
  TEST_ASSERT_EQUAL_UINT16(19999, ICR1);
  TEST_ASSERT_EQUAL_UINT8(OUTPUT, mockPinModes[9]);
  TEST_ASSERT_EQUAL_UINT8(OUTPUT, mockPinModes[10]);
}

void test_avr_timer1_pwm_set_duty_updates_compare_register() {
  IOFusion::AvrTimer1Pwm pwm;

  TEST_ASSERT_TRUE(pwm.begin(100));
  pwm.setDuty(0, 50);
  pwm.setDuty(1, 25);

  TEST_ASSERT_EQUAL_UINT16(10000, OCR1A);
  TEST_ASSERT_EQUAL_UINT16(5000, OCR1B);
}
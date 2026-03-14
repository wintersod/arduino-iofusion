#include <unity.h>

#include "iofusion_quadrature_signal_generator.h"

#include "test_support.h"

void test_quadrature_signal_generator_steps() {
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
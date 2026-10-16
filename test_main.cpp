#include <unity.h>
#include "signal_processing.h"

void setUp() {}
void tearDown() {}

void test_moving_average_partial_window() {
  sp::MovingAverage<4> ma;
  TEST_ASSERT_EQUAL_FLOAT(10.0f, ma.add(10.0f));
  TEST_ASSERT_EQUAL_FLOAT(15.0f, ma.add(20.0f));
  TEST_ASSERT_FALSE(ma.full());
}

void test_moving_average_rolls_over() {
  sp::MovingAverage<3> ma;
  ma.add(1.0f);
  ma.add(2.0f);
  ma.add(3.0f);
  TEST_ASSERT_TRUE(ma.full());
  TEST_ASSERT_EQUAL_FLOAT(2.0f, ma.value());
  ma.add(6.0f);  // window now {2,3,6}
  TEST_ASSERT_EQUAL_FLOAT(11.0f / 3.0f, ma.value());
}

void test_moving_average_reset() {
  sp::MovingAverage<2> ma;
  ma.add(5.0f);
  ma.reset();
  TEST_ASSERT_EQUAL_UINT32(0, ma.count());
  TEST_ASSERT_EQUAL_FLOAT(0.0f, ma.value());
}

void test_hysteresis_turns_on_at_setpoint() {
  sp::HysteresisThreshold t(30.0f, 1.0f);
  TEST_ASSERT_FALSE(t.update(29.9f));
  TEST_ASSERT_TRUE(t.update(30.0f));
}

void test_hysteresis_prevents_chatter() {
  sp::HysteresisThreshold t(30.0f, 1.0f);
  t.update(31.0f);
  TEST_ASSERT_TRUE(t.update(29.5f));   // still inside hysteresis band
  TEST_ASSERT_TRUE(t.update(29.1f));
  TEST_ASSERT_FALSE(t.update(29.0f));  // dropped to setPoint - hysteresis
  TEST_ASSERT_FALSE(t.update(29.9f));  // must reach setPoint again
}

void test_adc_conversion_and_clamping() {
  TEST_ASSERT_EQUAL_FLOAT(0.0f, sp::adcToUnits(0, 4095, 20.0f));
  TEST_ASSERT_EQUAL_FLOAT(20.0f, sp::adcToUnits(4095, 4095, 20.0f));
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, sp::adcToUnits(2048, 4095, 20.0f));
  TEST_ASSERT_EQUAL_FLOAT(20.0f, sp::adcToUnits(5000, 4095, 20.0f));
  TEST_ASSERT_EQUAL_FLOAT(0.0f, sp::adcToUnits(-5, 4095, 20.0f));
  TEST_ASSERT_EQUAL_FLOAT(0.0f, sp::adcToUnits(100, 0, 20.0f));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_moving_average_partial_window);
  RUN_TEST(test_moving_average_rolls_over);
  RUN_TEST(test_moving_average_reset);
  RUN_TEST(test_hysteresis_turns_on_at_setpoint);
  RUN_TEST(test_hysteresis_prevents_chatter);
  RUN_TEST(test_adc_conversion_and_clamping);
  return UNITY_END();
}

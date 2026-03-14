#include <unity.h>

#include "test_support.h"

int main(int argc, char** argv) {
  UNITY_BEGIN();

  RUN_TEST(test_analog_sampler_basic);
  RUN_TEST(test_analog_sampler_invalid_channel);
  RUN_TEST(test_analog_sampler_coalesces_pending_refresh_requests);
  RUN_TEST(test_digital_signal_meter_frequency_and_duty);
  RUN_TEST(test_quadrature_signal_generator_steps);
  RUN_TEST(test_quadrature_signal_generator_steps_backward_on_active_high_down);
  RUN_TEST(test_avr_timer1_pwm_begin_sets_timer_registers);
  RUN_TEST(test_avr_timer1_pwm_set_duty_updates_compare_register);
  RUN_TEST(test_avr_timer2_scheduler_begin_sets_ctc_registers);
  RUN_TEST(test_avr_timer2_scheduler_callback_lifecycle);
  RUN_TEST(test_serial_command_protocol_status_response_envelope);
  RUN_TEST(test_serial_command_protocol_capabilities_response_lists_commands_and_pins);
  RUN_TEST(test_serial_command_protocol_rejects_out_of_range_pwm_duty);
  RUN_TEST(test_serial_command_protocol_analog_query_uses_response_envelope);
  RUN_TEST(test_serial_command_protocol_digital_query_uses_compact_scaled_units);
  RUN_TEST(test_serial_command_protocol_encoder_query_uses_compact_fields);
  RUN_TEST(test_serial_command_protocol_discards_oversized_frame_until_newline);

  return UNITY_END();
}
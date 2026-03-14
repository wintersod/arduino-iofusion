#include <unity.h>

#include "Arduino.h"
#include "iofusion_analog_sampler.h"
#include "serial_command_protocol.h"
#include "iofusion_digital_signal_meter.h"
#include "iofusion_quadrature_signal_generator.h"
#include "iofusion_avr_timer1_pwm.h"

#include "test_support.h"

#include "../../src/serial_command_protocol.cpp"

void test_serial_command_protocol_status_response_envelope() {
  IOFusion::AnalogSampler analog;
  IOFusion::DigitalSignalMeter digi;
  IOFusion::QuadratureSignalGenerator encoder;
  IOFusion::AvrTimer1Pwm pwm;
  const uint8_t analogPins[] = {0, 1};
  const uint8_t digitalPins[] = {2, 3};

  TEST_ASSERT_TRUE(analog.begin(analogPins, 2));
  TEST_ASSERT_TRUE(digi.begin(digitalPins, 2, 4, 1000.0f, false));
  TEST_ASSERT_TRUE(encoder.begin(9, 10, 4, 5));

  IOFusion::SerialCommandProtocol cmd(analog, digi, encoder, pwm, analogPins, 2, digitalPins, 2);
  cmd.setModuleStatus(true, false, true, true, false);

  Serial.setInput("status\n");
  cmd.processSerial();

  const std::string& output = Serial.getOutput();
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"m\":[1,0,1,1,0]"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"c\":[2,2]"));
}

void test_serial_command_protocol_capabilities_response_lists_commands_and_pins() {
  IOFusion::AnalogSampler analog;
  IOFusion::DigitalSignalMeter digi;
  IOFusion::QuadratureSignalGenerator encoder;
  IOFusion::AvrTimer1Pwm pwm;
  const uint8_t analogPins[] = {0, 1, 2};
  const uint8_t digitalPins[] = {8, 11};

  TEST_ASSERT_TRUE(analog.begin(analogPins, 3));
  TEST_ASSERT_TRUE(digi.begin(digitalPins, 2, 4, 1000.0f, false));
  TEST_ASSERT_TRUE(encoder.begin(9, 10, 4, 5));

  IOFusion::SerialCommandProtocol cmd(analog, digi, encoder, pwm, analogPins, 3, digitalPins, 2);

  Serial.setInput("capabilities\n");
  cmd.processSerial();

  const std::string& output = Serial.getOutput();
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"cmd\":[\"analog?\",\"digital?\",\"encoder?\",\"pwm-freq\",\"pwm-duty\",\"status\",\"capabilities\",\"help\"]"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"u\":[\"mV\",\"0.1Hz\",\"0.1pct\",\"1=up\"]"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"a\":[0,1,2]"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"d\":[8,11]"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"p\":[2,0,100]"));
}

void test_serial_command_protocol_rejects_out_of_range_pwm_duty() {
  IOFusion::AnalogSampler analog;
  IOFusion::DigitalSignalMeter digi;
  IOFusion::QuadratureSignalGenerator encoder;
  IOFusion::AvrTimer1Pwm pwm;
  const uint8_t analogPins[] = {0};
  const uint8_t digitalPins[] = {2};

  TEST_ASSERT_TRUE(analog.begin(analogPins, 1));
  TEST_ASSERT_TRUE(digi.begin(digitalPins, 1, 4, 1000.0f, false));
  TEST_ASSERT_TRUE(encoder.begin(9, 10, 4, 5));

  IOFusion::SerialCommandProtocol cmd(analog, digi, encoder, pwm, analogPins, 1, digitalPins, 1);

  Serial.setInput("pwm-duty 0 101\n");
  cmd.processSerial();

  const std::string& output = Serial.getOutput();
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"err\":\"duty_out_of_range\""));
}

void test_serial_command_protocol_analog_query_uses_response_envelope() {
  IOFusion::AnalogSampler analog;
  IOFusion::DigitalSignalMeter digi;
  IOFusion::QuadratureSignalGenerator encoder;
  IOFusion::AvrTimer1Pwm pwm;
  const uint8_t analogPins[] = {0};
  const uint8_t digitalPins[] = {2};

  TEST_ASSERT_TRUE(analog.begin(analogPins, 1));
  TEST_ASSERT_TRUE(digi.begin(digitalPins, 1, 4, 1000.0f, false));
  TEST_ASSERT_TRUE(encoder.begin(9, 10, 4, 5));

  mockAnalogValues[0] = 512;
  analog.onTick();
  analog.sampleIfDue();

  IOFusion::SerialCommandProtocol cmd(analog, digi, encoder, pwm, analogPins, 1, digitalPins, 1);

  Serial.setInput("analog?\n");
  cmd.processSerial();

  const std::string& output = Serial.getOutput();
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("{\"mv\":[2502]}"));
}

void test_serial_command_protocol_digital_query_uses_compact_scaled_units() {
  IOFusion::AnalogSampler analog;
  IOFusion::DigitalSignalMeter digi;
  IOFusion::QuadratureSignalGenerator encoder;
  IOFusion::AvrTimer1Pwm pwm;
  const uint8_t analogPins[] = {0};
  const uint8_t digitalPins[] = {2};

  TEST_ASSERT_TRUE(analog.begin(analogPins, 1));
  TEST_ASSERT_TRUE(digi.begin(digitalPins, 1, 4, 1000.0f, false));
  TEST_ASSERT_TRUE(encoder.begin(9, 10, 4, 5));

  setDigitalPin(2, false);
  digi.onTick();
  setDigitalPin(2, true);
  digi.onTick();
  setDigitalPin(2, true);
  digi.onTick();
  setDigitalPin(2, false);
  digi.onTick();
  digi.updateIfReady();

  IOFusion::SerialCommandProtocol cmd(analog, digi, encoder, pwm, analogPins, 1, digitalPins, 1);

  Serial.setInput("digital?\n");
  cmd.processSerial();

  const std::string& output = Serial.getOutput();
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"f\":[2500]"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"d\":[500]"));
}

void test_serial_command_protocol_encoder_query_uses_compact_fields() {
  IOFusion::AnalogSampler analog;
  IOFusion::DigitalSignalMeter digi;
  IOFusion::QuadratureSignalGenerator encoder;
  IOFusion::AvrTimer1Pwm pwm;
  const uint8_t analogPins[] = {0};
  const uint8_t digitalPins[] = {2};

  TEST_ASSERT_TRUE(analog.begin(analogPins, 1));
  TEST_ASSERT_TRUE(digi.begin(digitalPins, 1, 4, 1000.0f, false));
  TEST_ASSERT_TRUE(encoder.begin(9, 10, 4, 5));

  setDigitalPin(4, true);
  setDigitalPin(5, false);
  encoder.onTick();
  encoder.onTick();

  IOFusion::SerialCommandProtocol cmd(analog, digi, encoder, pwm, analogPins, 1, digitalPins, 1);

  Serial.setInput("encoder?\n");
  cmd.processSerial();

  const std::string& output = Serial.getOutput();
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"e\":[1,2]"));
}

void test_serial_command_protocol_discards_oversized_frame_until_newline() {
  IOFusion::AnalogSampler analog;
  IOFusion::DigitalSignalMeter digi;
  IOFusion::QuadratureSignalGenerator encoder;
  IOFusion::AvrTimer1Pwm pwm;
  const uint8_t analogPins[] = {0, 1};
  const uint8_t digitalPins[] = {2, 3};

  TEST_ASSERT_TRUE(analog.begin(analogPins, 2));
  TEST_ASSERT_TRUE(digi.begin(digitalPins, 2, 4, 1000.0f, false));
  TEST_ASSERT_TRUE(encoder.begin(9, 10, 4, 5));

  IOFusion::SerialCommandProtocol cmd(analog, digi, encoder, pwm, analogPins, 2, digitalPins, 2);
  cmd.setModuleStatus(true, true, true, true, true);

  std::string oversized(80, 'x');
  oversized += "\nstatus\n";
  Serial.setInput(oversized);
  cmd.processSerial();

  const std::string& output = Serial.getOutput();
  TEST_ASSERT_EQUAL(std::string::npos, output.find("unknown_command"));
  TEST_ASSERT_EQUAL(std::string::npos, output.find("\"err\":"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos, output.find("\"m\":[1,1,1,1,1]"));
}
/**
 * @file iofusion_avr_timer1_pwm.h
 * @brief Minimal Timer1 PWM driver for Arduino Uno hardware PWM outputs.
 */
#ifndef IOFUSION_AVR_TIMER1_PWM_H
#define IOFUSION_AVR_TIMER1_PWM_H

#include <Arduino.h>

namespace IOFusion {

/** @brief Minimal AVR Timer1 PWM driver for OC1A/OC1B on Arduino Uno pins 9 and 10. */
class AvrTimer1Pwm {
public:
	/** @brief Constructs an unconfigured Timer1 PWM driver. */
	AvrTimer1Pwm();
	/**
	 * @brief Configures Timer1 for the requested PWM frequency.
	 * @param freqHz Target PWM frequency in hertz.
	 * @retval true Timer1 was configured successfully.
	 * @retval false The requested frequency cannot be represented by the Timer1 prescaler/top combination.
	 */
	bool begin(uint32_t freqHz);
	/**
	 * @brief Sets duty cycle in percent for one hardware PWM channel.
	 * @param channel `0` for OC1A, `1` for OC1B.
	 * @param percent Duty cycle in integer percent. Values above `100` are clamped.
	 */
	void setDuty(uint8_t channel, uint8_t percent);
	/** @brief Stops PWM output and releases the Timer1 output pins. */
	void stop();

private:
	uint16_t _top = 0;
	uint16_t _presBits = 0;
	uint32_t _frequencyHz = 0;
	uint8_t _dutyPercent[2] = {0, 0};
	bool _configured = false;
	uint16_t percentToCounts(uint8_t percent, uint16_t top) const;
	void _applyDuty(uint8_t channel, uint16_t value);
};

} // namespace IOFusion

#endif // IOFUSION_AVR_TIMER1_PWM_H
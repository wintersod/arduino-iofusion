// Timer1-based PWM driver for two hardware channels (OC1A / OC1B)
#ifndef IOFUSION_AVR_TIMER1_PWM_H
#define IOFUSION_AVR_TIMER1_PWM_H

#include <Arduino.h>

namespace IOFusion {

// Minimal AVR Timer1 PWM driver for OC1A/OC1B (Arduino Uno pins 9 and 10).
class AvrTimer1Pwm {
public:
	AvrTimer1Pwm();
	// Configures Timer1 for the requested PWM frequency.
	bool begin(uint32_t freqHz);
	// Sets duty cycle in percent for channel 0 (OC1A) or 1 (OC1B).
	void setDuty(uint8_t channel, uint8_t percent);
	// Stops Timer1 PWM and releases the output pins.
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
// Digital input frequency and duty measurement
#ifndef IOFUSION_DIGITAL_SIGNAL_METER_H
#define IOFUSION_DIGITAL_SIGNAL_METER_H

#include <Arduino.h>

namespace IOFusion {

// Measures digital input frequency and duty cycle using ISR-side sampling and loop()-side reduction.
class DigitalSignalMeter {
public:
	DigitalSignalMeter();
	// Begin monitoring up to 8 pins.
	// windowTicks is the number of ISR samples per measurement window.
	// tickHz is the ISR sampling frequency.
	bool begin(const uint8_t* pins, uint8_t count, uint16_t windowTicks = 1000, uint32_t tickHz = 1000U, bool usePullup = false);

	// ISR-side sampler. Reads current levels and updates edge/high counters.
	void onTick();
	// loop()-side reducer. Publishes frequency and duty after a window completes.
	void updateIfReady();

	uint8_t getPinCount() const;
	// Returns the most recent frequency in deci-Hz.
	uint32_t getFrequencyDeciHz(uint8_t idx) const;
	// Returns the most recent duty cycle in tenths of a percent.
	uint16_t getDutyDeciPercent(uint8_t idx) const;

private:
	static const uint8_t MAX_PINS = 8;
	uint8_t _pins[MAX_PINS];
	uint8_t _pinCount = 0;
	uint16_t _windowTicks = 1000;
	uint32_t _tickHz = 1000U;
	volatile uint8_t* _pinPortIn[MAX_PINS];
	uint8_t _pinMask[MAX_PINS];

	volatile uint16_t _samplesInWindow = 0;
	volatile uint16_t _edgeCnt[MAX_PINS];
	volatile uint16_t _highCnt[MAX_PINS];
	volatile uint8_t _lastState[MAX_PINS];
	volatile bool _windowReady = false;

	uint32_t _freqDeciHz[MAX_PINS];
	uint16_t _dutyDeciPercent[MAX_PINS];
};

} // namespace IOFusion

#endif // IOFUSION_DIGITAL_SIGNAL_METER_H
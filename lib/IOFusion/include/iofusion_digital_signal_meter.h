/**
 * @file iofusion_digital_signal_meter.h
 * @brief Fixed-point digital frequency and duty-cycle measurement for AVR targets.
 */
#ifndef IOFUSION_DIGITAL_SIGNAL_METER_H
#define IOFUSION_DIGITAL_SIGNAL_METER_H

#include <Arduino.h>

namespace IOFusion {

/**
 * @brief Measures digital input frequency and duty cycle using ISR-side sampling.
 *
 * Edge and high-time counters are updated from a periodic ISR and reduced to fixed-point
 * engineering units from loop context.
 */
class DigitalSignalMeter {
public:
	/** @brief Constructs an empty measurement block. */
	DigitalSignalMeter();
	/**
	 * @brief Begins monitoring up to eight digital input pins.
	 * @param pins Array of Arduino digital pin numbers.
	 * @param count Number of pins to monitor.
	 * @param windowTicks Number of ISR samples per measurement window.
	 * @param tickHz ISR sampling frequency in hertz.
	 * @param usePullup When true, enables Arduino internal pull-ups on each monitored pin.
	 * @retval true Configuration is valid and monitoring started.
	 * @retval false Arguments are invalid or any pin cannot be resolved to a port register.
	 */
	bool begin(const uint8_t* pins, uint8_t count, uint16_t windowTicks = 1000, uint32_t tickHz = 1000U, bool usePullup = false);

	/** @brief Samples current pin levels from ISR context and updates counters. */
	void onTick();
	/** @brief Publishes the next completed measurement window from loop context. */
	void updateIfReady();

	/** @brief Returns the configured pin count. */
	uint8_t getPinCount() const;
	/**
	 * @brief Returns the most recent frequency measurement.
	 * @param idx Channel index inside the configured pin list.
	 * @return Frequency in deci-hertz (`0.1 Hz` units), or `0` if @p idx is invalid.
	 */
	uint32_t getFrequencyDeciHz(uint8_t idx) const;
	/**
	 * @brief Returns the most recent duty-cycle measurement.
	 * @param idx Channel index inside the configured pin list.
	 * @return Duty cycle in tenths of a percent (`0.1%` units), or `0` if @p idx is invalid.
	 */
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
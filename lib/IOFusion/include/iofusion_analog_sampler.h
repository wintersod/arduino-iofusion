/**
 * @file iofusion_analog_sampler.h
 * @brief Best-effort analog snapshot sampler for AVR-based Arduino boards.
 */
#ifndef IOFUSION_ANALOG_SAMPLER_H
#define IOFUSION_ANALOG_SAMPLER_H

#include <Arduino.h>

namespace IOFusion {

/**
 * @brief Defers ADC reads to loop() while ISR code only requests a refresh.
 *
 * Multiple ISR requests are intentionally coalesced into one pending refresh so the
 * analog subsystem always publishes the latest available snapshot within the loop budget.
 * During boot, readings remain at their initialized state until the first scheduled
 * refresh request is serviced from loop context.
 */
class AnalogSampler {
public:
	/** @brief Constructs an empty sampler with default 5000 mV reference scaling. */
	AnalogSampler();
	/**
	 * @brief Configures the analog channel list.
	 * @param channels Array of analog channel indices using Arduino numbering `A0..A5 -> 0..5`.
	 * @param count Number of channels in @p channels.
	 * @retval true Configuration is valid and the channel list was copied.
	 * @retval false A channel index is invalid or @p count is out of range.
	 */
	bool begin(const uint8_t* channels, uint8_t count);
	/** @brief Requests a future refresh from ISR context without performing ADC reads. */
	void onTick();
	/**
	 * @brief Performs a pending analog refresh from loop context.
	 *
	 * If loop() falls behind, intermediate requests are coalesced and only the newest
	 * completed channel snapshot is retained.
	 */
	void sampleIfDue();

	/** @brief Returns the configured analog channel count. */
	uint8_t getChannelCount() const;
	/**
	 * @brief Returns the most recent sampled value in millivolts.
	 * @param idx Channel index inside the configured channel list.
	 * @return Latest scaled channel value in millivolts, or `0` if @p idx is invalid.
	 */
	uint16_t getMilliVolts(uint8_t idx) const;
	/**
	 * @brief Returns the most recent sampled value in volts.
	 * @param idx Channel index inside the configured channel list.
	 * @return Latest scaled channel value in volts, or `0.0f` if @p idx is invalid.
	 * @note This helper exists for convenience. The millivolt API is preferred on AVR.
	 */
	float getValue(uint8_t idx) const;
	/**
	 * @brief Sets the analog reference voltage used for scaling in floating-point volts.
	 * @param vref Reference voltage in volts.
	 * @note The integer millivolt API is preferred on constrained targets.
	 */
	void setVref(float vref);
	/**
	 * @brief Sets the analog reference voltage used for scaling in millivolts.
	 * @param vrefMillivolts Reference voltage in millivolts.
	 */
	void setVrefMillivolts(uint16_t vrefMillivolts);

private:
	static const uint8_t MAX_CHANNELS = 6;
	uint8_t _channels[MAX_CHANNELS];
	uint8_t _channelCount = 0;
	volatile bool _sampleRequested = false;
	int _lastValues[MAX_CHANNELS];
	uint16_t _vrefMillivolts = 5000;
};

} // namespace IOFusion

#endif // IOFUSION_ANALOG_SAMPLER_H
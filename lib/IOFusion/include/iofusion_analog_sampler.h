// Simple analog sampler for AVR/Arduino
#ifndef IOFUSION_ANALOG_SAMPLER_H
#define IOFUSION_ANALOG_SAMPLER_H

#include <Arduino.h>

namespace IOFusion {

// Defers ADC reads to loop() and keeps the ISR path to a single pending flag.
class AnalogSampler {
public:
	AnalogSampler();
	// Configure the analog channel list (A0..A5 as 0..5). The channel list is copied.
	bool begin(const uint8_t* channels, uint8_t count);
	// ISR-side request hook. Does not perform ADC reads.
	void onTick();
	// loop()-side sampler. Performs ADC reads only when a request is pending.
	void sampleIfDue();

	uint8_t getChannelCount() const;
	// Returns the most recent sampled value in millivolts.
	uint16_t getMilliVolts(uint8_t idx) const;
	// Returns the most recent sampled voltage in volts.
	float getValue(uint8_t idx) const;
	// Configure reference voltage used for scaling (default 5.0V).
	void setVref(float vref);
	// Configure reference voltage directly in millivolts.
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
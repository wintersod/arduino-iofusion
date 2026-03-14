// Simple Timer2 driver for AVR/Arduino
#ifndef IOFUSION_AVR_TIMER2_SCHEDULER_H
#define IOFUSION_AVR_TIMER2_SCHEDULER_H

#include <Arduino.h>

namespace IOFusion {

typedef void (*Timer2Callback)();

// Minimal Timer2 compare-match scheduler for short ISR callbacks.
class AvrTimer2Scheduler {
public:
	AvrTimer2Scheduler();
	// Starts Timer2 in CTC mode and returns the OCR value used, or 0 on failure.
	uint16_t beginHz(uint32_t freqHz);
	// Stops Timer2 and clears registered callbacks.
	void stop();
	// Registers a callback unless the list is full. Duplicate registration is ignored.
	bool attachCallback(Timer2Callback cb);
	void detachCallback(Timer2Callback cb);
	// ISR dispatch entry point used by the Timer2 compare-match ISR.
	static void handleInterrupt();

private:
	static const uint8_t MAX_CALLBACKS = 4;
	static volatile Timer2Callback _cbs[MAX_CALLBACKS];
	static void clearCallbacks();
};

} // namespace IOFusion

#endif // IOFUSION_AVR_TIMER2_SCHEDULER_H
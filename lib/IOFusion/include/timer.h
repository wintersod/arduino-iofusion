// Simple Timer2 driver for AVR/Arduino
// Provides periodic interrupts on Timer2 and a tiny encoder signal generator.
#ifndef IOFUSION_TIMER_H
#define IOFUSION_TIMER_H

#include <Arduino.h>

typedef void (*Timer2Callback)();

class Timer2Driver {
public:
	Timer2Driver();
	// Start timer at approximate frequency in Hz. Returns actual OCR value used.
	uint16_t beginHz(float freqHz);
	// Stop timer and disable interrupt
	void stop();

	// Attach a callback that will be called from the ISR context.
	// Keep the callback short; heavy work should be deferred to loop().
	bool attachCallback(Timer2Callback cb);
	// Detach a specific callback previously attached. Pass the same function pointer.
	void detachCallback(Timer2Callback cb);
	// ISR entry point
	static void handleInterrupt();

private:
	// internal state accessed from ISR
	static const uint8_t MAX_CALLBACKS = 4;
	static volatile Timer2Callback _cbs[MAX_CALLBACKS];
	static void clearCallbacks();
	// helper functions removed; implementation chooses prescaler directly
};

#endif // IOFUSION_TIMER_H

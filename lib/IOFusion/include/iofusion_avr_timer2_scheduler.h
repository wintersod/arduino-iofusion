/**
 * @file iofusion_avr_timer2_scheduler.h
 * @brief Small Timer2 compare-match scheduler for short ISR callbacks.
 */
#ifndef IOFUSION_AVR_TIMER2_SCHEDULER_H
#define IOFUSION_AVR_TIMER2_SCHEDULER_H

#include <Arduino.h>

namespace IOFusion {

/** @brief Function pointer type for Timer2 ISR callbacks. */
typedef void (*Timer2Callback)();

/** @brief Minimal Timer2 compare-match scheduler for short ISR callbacks. */
class AvrTimer2Scheduler {
public:
	/** @brief Constructs an unconfigured Timer2 scheduler. */
	AvrTimer2Scheduler();
	/**
	 * @brief Starts Timer2 in CTC mode.
	 * @param freqHz Target callback frequency in hertz.
	 * @retval true Timer2 was configured successfully.
	 * @retval false The requested frequency cannot be represented by the Timer2 prescaler/OCR2A combination.
	 */
	bool beginHz(uint32_t freqHz);
	/** @brief Stops Timer2 and clears all registered callbacks. */
	void stop();
	/**
	 * @brief Registers a callback unless the list is full.
	 * @param cb Callback invoked from Timer2 compare-match ISR context.
	 * @retval true Callback was registered or already present.
	 * @retval false @p cb is null or the callback table is full.
	 */
	bool attachCallback(Timer2Callback cb);
	/** @brief Removes a previously registered callback if present. */
	void detachCallback(Timer2Callback cb);
	/** @brief Dispatch entry used by the Timer2 compare-match ISR. */
	static void handleInterrupt();

private:
	static const uint8_t MAX_CALLBACKS = 4;
	static volatile Timer2Callback _cbs[MAX_CALLBACKS];
	static void clearCallbacks();
};

} // namespace IOFusion

#endif // IOFUSION_AVR_TIMER2_SCHEDULER_H
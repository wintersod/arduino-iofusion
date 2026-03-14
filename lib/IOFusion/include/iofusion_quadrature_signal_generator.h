/**
 * @file iofusion_quadrature_signal_generator.h
 * @brief Timer-driven quadrature signal generator controlled by two logic inputs.
 */
#ifndef IOFUSION_QUADRATURE_SIGNAL_GENERATOR_H
#define IOFUSION_QUADRATURE_SIGNAL_GENERATOR_H

#include <Arduino.h>

namespace IOFusion {

/**
 * @brief Generates a quadrature-style output waveform from two active-high control inputs.
 * @note This class is a signal generator, not a physical quadrature decoder.
 */
class QuadratureSignalGenerator {
public:
	/**
	 * @brief Configures output and control pins.
	 * @param pinA Quadrature output A.
	 * @param pinB Quadrature output B.
	 * @param up Active-high control input for forward stepping.
	 * @param down Active-high control input for reverse stepping.
	 * @retval true Pins were configured successfully.
	 * @retval false Any pin is invalid or cannot be mapped to a port register.
	 */
	bool begin(uint8_t pinA, uint8_t pinB, uint8_t up, uint8_t down);
	/** @brief Advances the generator one step from ISR context if a control input is asserted. */
	void onTick();

	/**
	 * @brief Returns the current accumulated position.
	 * @note Position and direction are exposed through separate reads and are treated as
	 * near-real-time status rather than a single atomic snapshot.
	 */
	int32_t getPosition();
	/**
	 * @brief Returns the most recent direction.
	 * @retval true Forward direction.
	 * @retval false Reverse direction.
	 */
	bool getDirection();
	/** @brief Resets the position counter and drives both outputs low. */
	void reset();

private:
	uint8_t _pinA = 255;
	uint8_t _pinB = 255;
	uint8_t _state = 0;
	volatile uint8_t* _portAOut = nullptr;
	volatile uint8_t* _portBOut = nullptr;
	uint8_t _maskA = 0;
	uint8_t _maskB = 0;
	uint8_t _pinUp = 255;
	uint8_t _pinDown = 255;
	volatile uint8_t* _upPortIn = nullptr;
	volatile uint8_t* _downPortIn = nullptr;
	uint8_t _upMask = 0;
	uint8_t _downMask = 0;
	volatile int32_t _position = 0;
	volatile bool _directionUp = true;
};

} // namespace IOFusion

#endif // IOFUSION_QUADRATURE_SIGNAL_GENERATOR_H
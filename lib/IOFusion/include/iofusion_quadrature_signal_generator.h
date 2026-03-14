// Simple quadrature encoder signal generator
#ifndef IOFUSION_QUADRATURE_SIGNAL_GENERATOR_H
#define IOFUSION_QUADRATURE_SIGNAL_GENERATOR_H

#include <Arduino.h>

namespace IOFusion {

// Generates a quadrature-style output from two level inputs. This is not a decoder.
class QuadratureSignalGenerator {
public:
	// pinA/pinB are outputs, up/down are sampled control inputs.
	bool begin(uint8_t pinA, uint8_t pinB, uint8_t up, uint8_t down);
	// ISR-side state advance and output update.
	void onTick();

	int32_t getPosition();
	bool getDirection();
	// Resets position and drives both outputs LOW.
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
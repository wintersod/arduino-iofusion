/**
 * @file iofusion_quadrature_signal_generator.cpp
 * @brief Implementation of the timer-driven quadrature signal generator.
 */
#include "iofusion_quadrature_signal_generator.h"

namespace IOFusion {

bool QuadratureSignalGenerator::begin(uint8_t pinA, uint8_t pinB, uint8_t up, uint8_t down) {
  if (pinA == pinB) return false;
  _pinA = pinA;
  _pinB = pinB;
  pinMode(_pinA, OUTPUT);
  pinMode(_pinB, OUTPUT);
  uint8_t portA = digitalPinToPort(_pinA);
  uint8_t portB = digitalPinToPort(_pinB);
  _portAOut = portOutputRegister(portA);
  _portBOut = portOutputRegister(portB);
  _maskA = digitalPinToBitMask(_pinA);
  _maskB = digitalPinToBitMask(_pinB);
  if (portA == NOT_A_PIN || portB == NOT_A_PIN || _portAOut == nullptr || _portBOut == nullptr || _maskA == 0 || _maskB == 0) return false;
  if (_portAOut) *_portAOut &= static_cast<uint8_t>(~_maskA);
  if (_portBOut) *_portBOut &= static_cast<uint8_t>(~_maskB);
  _state = 0;

  _pinUp = up;
  _pinDown = down;
  pinMode(_pinUp, INPUT_PULLUP);
  pinMode(_pinDown, INPUT_PULLUP);
  uint8_t portUp = digitalPinToPort(_pinUp);
  uint8_t portDown = digitalPinToPort(_pinDown);
  _upPortIn = portInputRegister(portUp);
  _downPortIn = portInputRegister(portDown);
  _upMask = digitalPinToBitMask(_pinUp);
  _downMask = digitalPinToBitMask(_pinDown);
  if (portUp == NOT_A_PIN || portDown == NOT_A_PIN || _upPortIn == nullptr || _downPortIn == nullptr || _upMask == 0 || _downMask == 0) return false;
  noInterrupts();
  _position = 0;
  _directionUp = true;
  interrupts();

  return true;
}

void QuadratureSignalGenerator::onTick() {
  bool upActive = (_upPortIn && ((*_upPortIn & _upMask) != 0));
  bool downActive = (_downPortIn && ((*_downPortIn & _downMask) != 0));
  bool stepped = false;
  if (upActive && !downActive) {
    _directionUp = true;
    _state = (_state + 1) & 3;
    _position++;
    stepped = true;
  } else if (!upActive && downActive) {
    _directionUp = false;
    _state = (_state - 1) & 3;
    _position--;
    stepped = true;
  }
  if (stepped) {
    uint8_t state = _state;
    if (_portAOut) {
      if (state == 2 || state == 3) *_portAOut |= _maskA;
      else *_portAOut &= static_cast<uint8_t>(~_maskA);
    }
    if (_portBOut) {
      if (state == 1 || state == 2) *_portBOut |= _maskB;
      else *_portBOut &= static_cast<uint8_t>(~_maskB);
    }
  }
}

int32_t QuadratureSignalGenerator::getPosition() {
  noInterrupts();
  int32_t value = _position;
  interrupts();
  return value;
}

void QuadratureSignalGenerator::reset() {
  noInterrupts();
  _position = 0;
  _state = 0;
  _directionUp = true;
  if (_portAOut) *_portAOut &= static_cast<uint8_t>(~_maskA);
  if (_portBOut) *_portBOut &= static_cast<uint8_t>(~_maskB);
  interrupts();
}

bool QuadratureSignalGenerator::getDirection() {
  noInterrupts();
  bool direction = _directionUp;
  interrupts();
  return direction;
}

} // namespace IOFusion
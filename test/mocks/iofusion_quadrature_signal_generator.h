#ifndef MOCK_IOFUSION_QUADRATURE_SIGNAL_GENERATOR_H
#define MOCK_IOFUSION_QUADRATURE_SIGNAL_GENERATOR_H

#include <cstdint>

namespace IOFusion {

class QuadratureSignalGenerator {
public:
  QuadratureSignalGenerator() = default;

  void setState(int32_t position, bool directionUp) {
    _position = position;
    _directionUp = directionUp;
  }

  int32_t getPosition() { return _position; }
  bool getDirection() { return _directionUp; }

private:
  int32_t _position = 0;
  bool _directionUp = true;
};

} // namespace IOFusion

#endif // MOCK_IOFUSION_QUADRATURE_SIGNAL_GENERATOR_H
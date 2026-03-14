#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define F(x) x
#define PROGMEM

#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
#define NOT_A_PIN 0xFF

#ifndef _BV
#define _BV(bit) (1U << (bit))
#endif

#define ISR(vector) void vector(void)

#define CS10 0
#define CS11 1
#define CS12 2
#define WGM11 3
#define WGM12 4
#define WGM13 5
#define COM1A1 6
#define COM1B1 7
#define WGM21 1
#define CS20 0
#define CS21 1
#define CS22 2
#define OCIE2A 3

extern uint8_t mockPinModes[64];

inline void pinMode(uint8_t pin, uint8_t mode) {
  if (pin < 64) mockPinModes[pin] = mode;
}

inline void noInterrupts() {}
inline void interrupts() {}
inline void delayMicroseconds(unsigned int) {}

extern unsigned long mockMillis;
inline unsigned long millis() { return mockMillis; }
inline void advanceMillis(unsigned long deltaMs) { mockMillis += deltaMs; }

extern uint8_t mockPortIn[8];
extern uint8_t mockPortOut[8];
extern int mockAnalogValues[16];
extern uint8_t TCCR1A;
extern uint8_t TCCR1B;
extern uint8_t TCCR2A;
extern uint8_t TCCR2B;
extern uint8_t TIMSK2;
extern uint8_t OCR2A;
extern uint16_t OCR1A;
extern uint16_t OCR1B;
extern uint16_t ICR1;
extern uint16_t TCNT1;

inline uint8_t digitalPinToPort(uint8_t pin) {
  if (pin >= 64) return NOT_A_PIN;
  return static_cast<uint8_t>(pin / 8);
}

inline uint8_t digitalPinToBitMask(uint8_t pin) {
  return static_cast<uint8_t>(1U << (pin % 8));
}

inline volatile uint8_t* portInputRegister(uint8_t port) {
  if (port >= 8) return nullptr;
  return &mockPortIn[port];
}

inline volatile uint8_t* portOutputRegister(uint8_t port) {
  if (port >= 8) return nullptr;
  return &mockPortOut[port];
}

inline int analogRead(uint8_t pin) {
  if (pin >= 16) return 0;
  return mockAnalogValues[pin];
}

class MockSerial {
public:
  void begin(unsigned long) {}

  size_t available() const {
    if (_readPos >= _input.size()) return 0;
    return _input.size() - _readPos;
  }

  int read() {
    if (_readPos >= _input.size()) return -1;
    return static_cast<unsigned char>(_input[_readPos++]);
  }

  void setInput(const std::string& s) {
    _input = s;
    _readPos = 0;
  }

  const std::string& getOutput() const { return _output; }
  void clearOutput() { _output.clear(); }

  size_t print(const char* s) {
    if (!s) return 0;
    _output += s;
    return std::strlen(s);
  }

  size_t print(char c) {
    _output.push_back(c);
    return 1;
  }

  size_t print(int v) {
    _output += std::to_string(v);
    return 1;
  }

  size_t print(unsigned int v) {
    _output += std::to_string(v);
    return 1;
  }

  size_t print(long v) {
    _output += std::to_string(v);
    return 1;
  }

  size_t print(unsigned long v) {
    _output += std::to_string(v);
    return 1;
  }

  size_t print(float v, int digits = 2) {
    std::ostringstream ss;
    ss.setf(std::ios::fixed);
    ss << std::setprecision(digits) << v;
    _output += ss.str();
    return 1;
  }

  size_t println() {
    _output.push_back('\n');
    return 1;
  }

  size_t println(const char* s) {
    size_t n = print(s);
    _output.push_back('\n');
    return n + 1;
  }

  template <typename T>
  size_t println(T v) {
    size_t n = print(v);
    _output.push_back('\n');
    return n + 1;
  }

private:
  std::string _input;
  size_t _readPos = 0;
  std::string _output;
};

extern MockSerial Serial;

#endif // MOCK_ARDUINO_H

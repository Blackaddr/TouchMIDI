#ifndef __ROTARYENCODER_H
#define __ROTARYENCODER_H

#include "Encoder.h"

constexpr bool SWAP = true;
constexpr bool NOSWAP = false;

class RotaryEncoder : public Encoder {
public:
  RotaryEncoder(uint8_t pin1, uint8_t pin2, bool swapDirection = false, int divider = 1) : Encoder(pin1,pin2), m_swapDirection(swapDirection), m_divider(divider) {}

  int getChange() {
    int32_t newPosition = read();
    int delta = newPosition - m_lastPosition;
    m_lastPosition = newPosition;
    if (m_swapDirection) { delta = -delta; }
    return delta/m_divider;
  }

  void setDivider(int divider) {
    m_divider = divider;
  }
private:
  bool m_swapDirection;
  int32_t m_lastPosition = 0;
  int32_t m_divider;
};

#endif

#ifndef __ROTARYENCODER_H
#define __ROTARYENCODER_H

#define ENCODER_USE_INTERRUPTS
#include "Encoder.h"

constexpr bool SWAP = true;
constexpr bool NOSWAP = false;

class RotaryEncoder : public Encoder {
public:
  RotaryEncoder(uint8_t pin1, uint8_t pin2, bool swapDirection = false, int divider = 1) : Encoder(pin1,pin2), m_swapDirection(swapDirection), m_divider(divider) {}

  int getChange() {

    // Check if the minimum sampling time has elapsed
    if (m_timerMs < m_samplingIntervalMs) {
      return 0;
    }

    m_timerMs = 0; // reset the sampling timer
    int32_t newPosition = read();
    int delta = newPosition - m_lastPosition;
    m_lastPosition = newPosition;
    if (m_swapDirection) { delta = -delta; }
    return delta/m_divider;
  }

  void setDivider(int divider) {
    m_divider = divider;
  }

  void setSamplingIntervalMs(unsigned intervalMs) { m_samplingIntervalMs = intervalMs; }
private:
  bool m_swapDirection;
  int32_t m_lastPosition = 0;
  int32_t m_divider;
  unsigned m_samplingIntervalMs = 20; ///< the sampling interval in milliseconds
  elapsedMillis m_timerMs;            ///< special Teensy variable that tracks time
};

#endif

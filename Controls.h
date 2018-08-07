#ifndef __CONTROLS_H
#define __CONTROLS_H

#include <vector>
#include "RotaryEncoder.h"

/// Specifies the type of MIDI control
enum class ControlType : unsigned {
  SWITCH_MOMENTARY = 0, ///< a momentary switch, which is only on when pressed.
  SWITCH_LATCHING  = 1,  ///< a latching switch, which toggles between on and off with each press and release
  ROTARY_KNOB      = 2,      ///< a rotary encoder knob
  UNDEFINED        = 255     ///< undefined or uninitialized
};

class Controls {
public:
  Controls() = delete;
  Controls(unsigned numEncoders, unsigned numSwitches) {
    m_encoders.reserve(numEncoders);
    m_switches.reserve(numSwitches);
  }

  unsigned addRotary(uint8_t pin1, uint8_t pin2, bool swapDirection = false, int divider = 1) {
    m_encoders.emplace_back(pin1, pin2, swapDirection, divider);
    return  m_encoders.size()-1;
  }

  unsigned addSwitch(uint8_t pin, unsigned long intervalMilliseconds) {
    m_switches.emplace_back(pin, intervalMilliseconds);
    return m_switches.size()-1;
  }

  int getRotaryAdjustUnit(unsigned index) {
    if (index >= m_encoders.size()) { return 0; } // index is greater than number of encoders
    
    int encoderAdjust = m_encoders[index].getChange();
    if (encoderAdjust != 0) {
      // clip the adjust to maximum abs value of 1.
      int encoderAdjust = (encoderAdjust > 0) ? 1 : -1;
    }

    return encoderAdjust;
  }

  bool isSwitchToggled(unsigned index) {
    if (index >= m_switches.size()) { return 0; } // index is greater than number of switches    
    Bounce &sw = m_switches[index];
    if (sw.update() && sw.fallingEdge()) {
      return true;
    } else {
      return false;
    }
  }
  
  std::vector<RotaryEncoder> m_encoders;
  std::vector<Bounce>  m_switches;
};



#endif

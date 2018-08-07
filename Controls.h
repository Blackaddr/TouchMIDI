#ifndef __CONTROLS_H
#define __CONTROLS_H

#include <vector>
#include <XPT2046_Touchscreen.h>
#include "RotaryEncoder.h"

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

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
  ~Controls() {
    delete touch;
  }

  unsigned addRotary(uint8_t pin1, uint8_t pin2, bool swapDirection = false, int divider = 1) {
    m_encoders.emplace_back(pin1, pin2, swapDirection, divider);
    return  m_encoders.size()-1;
  }

  unsigned addSwitch(uint8_t pin, unsigned long intervalMilliseconds) {
    m_switches.emplace_back(pin, intervalMilliseconds);
    return m_switches.size()-1;
  }

  bool addTouch(unsigned chipSelect, unsigned interrupt, unsigned height, unsigned width)
  {
    touch = new XPT2046_Touchscreen(chipSelect, interrupt);
    if (touch) { 
      touch->begin();
      m_touchHeight = height;
      m_touchWidth = width;
      Serial.println("Controls::addTouch: touch controller started");
      return true;
    } else {
      Serial.println("Controls::addTouch: touch controller failed to start");
      return false;
    }
  }

  TS_Point getTouchPoint() {
      // Retrieve a point  
      TS_Point p = touch->getPoint();
      TS_Point tmp = p;
      p.x = tmp.y;
      p.y = TS_MAXY - tmp.x;
      
      if (touch->touched()) {
        Serial.print("X = "); Serial.print(p.x);
        Serial.print("\tY = "); Serial.print(p.y);
        Serial.print("\tPressure = "); Serial.println(p.z);
      }
 
      // Scale from ~0->4000 to tft.width using the calibration #'s
      p.x = map(p.x, TS_MINX, TS_MAXX, 0, m_touchWidth);
      p.y = map(p.y, TS_MINY, TS_MAXY, 0, m_touchHeight);
      return p;
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

  XPT2046_Touchscreen *touch = nullptr;
  std::vector<RotaryEncoder> m_encoders;
  std::vector<Bounce>  m_switches;
private:
  unsigned m_touchHeight = 0;
  unsigned m_touchWidth  = 0;
};



#endif

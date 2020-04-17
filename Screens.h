#ifndef __SCREENS_H
#define __SCREENS_H

#include <vector>
#include <cctype>
#include <MIDI.h>

#include "ILI9341_t3.h"
#include "ArduinoJson.h"

#include "StringEdit.h"
#include "Bitmap.h"
#include "Graphics.h"
#include "Controls.h"
#include "ScreensUtil.h"
#include "Preset.h"

/// Enumerations for each screen
enum class Screens : unsigned {
  PRESET_NAVIGATION, ///< screen for navigating between and selecting a preset
  PRESET_CONTROL,       ///< screen for editing a preset
  PRESET_CONFIG,     ///< screen for preset configuration
  MIDI_CONTROL_CONFIG, ///< screen for editing a MIDI control
  TOUCH_CALIBRATE,   ///< calibrate the touch screen
  MIDI_MONITOR,      ///< Midi monitoring tool
  UTILITIES          ///< General utilities
};

constexpr unsigned TOUCH_CONTROL_HALFSIZE = 20;
constexpr unsigned ICON_SIZE = 48;
constexpr unsigned ICON_SPACING = 5;
constexpr unsigned BACK_BUTTON_X_POS = 255;
constexpr unsigned BACK_BUTTON_Y_POS = 0;
constexpr unsigned SETTINGS_BUTTON_X_POS = BACK_BUTTON_X_POS-ICON_SIZE-ICON_SPACING;

constexpr unsigned CONTROL_ENCODER = 0;
constexpr unsigned CONTROL_SWITCH  = 0;

extern Screens g_currentScreen;

//using Coordinate = TS_Point;


struct Coordinate {
    Coordinate() : x(0), y(0), control(nullptr) {}
    Coordinate(int16_t x, int16_t y, MidiControl *control) : x(x), y(y), control(control) {}
    int16_t x;
    int16_t y;
    MidiControl *control;

    // Check if the "check" point is within the specified threshold of the center
    bool checkCoordinateRange(TouchPoint &coordinateCheck, unsigned threshold)
    {
        if (abs(x - coordinateCheck.x) > threshold) return false;
        if (abs(y - coordinateCheck.y) > threshold) return false;
        return true;
    }
};


struct TouchArea {
public:
    TouchArea() :
        xStart(-1), xEnd(-1), yStart(-1), yEnd(-1) {}
    TouchArea(int16_t xStart, int16_t xEnd, int16_t yStart, int16_t yEnd) :
        xStart(xStart), xEnd(xEnd), yStart(yStart), yEnd(yEnd) {}
    int16_t xStart;
    int16_t xEnd;
    int16_t yStart;
    int16_t yEnd;

    void setArea(int16_t xStartIn, int16_t xEndIn, int16_t yStartIn, int16_t yEndIn) {
        xStart = xStartIn;
        xEnd   = xEndIn;
        yStart = yStartIn;
        yEnd   = yEndIn;
    }

    bool checkArea(TouchPoint &coord) const {
        if (coord.x < xStart) return false;
        if (coord.x > xEnd)   return false;
        if (coord.y < yStart) return false;
        if (coord.y > yEnd)   return false;
        return true;
    }

};


void    DrawMidiControlConfig(ILI9341_t3 &tft, Controls &controls, MidiControl &midiControl);
void    DrawPresetConfig     (ILI9341_t3 &tft, Controls &controls, Preset &preset);
Screens DrawPresetNavigation (ILI9341_t3 &tft, Controls &controls, PresetArray &presetArray, midi::MidiInterface<HardwareSerial> &midiPort,
        unsigned &activePreset, unsigned &selectedPreset);
Screens DrawPresetControl    (ILI9341_t3 &tft, Controls &controls, Preset &preset, midi::MidiInterface<HardwareSerial> &midiPort);
Screens DrawMidiMonitor      (ILI9341_t3 &tft, Controls &controls, Preset &preset, midi::MidiInterface<HardwareSerial> &midiPort);
Screens DrawUtilities        (ILI9341_t3 &tft, Controls &controls, PresetArray& presetArray);


void infoScreen         (ILI9341_t3 &tft, String message);
bool confirmationScreen(ILI9341_t3 &tft, Controls &controls, String message);
bool saveConfirmation  (ILI9341_t3 &tft, Controls &controls);

TS_Point calibPoint     (ILI9341_t3 &tft, Controls &controls, int16_t x, int16_t y);
TS_Point calcCalibLimits(unsigned p1, unsigned p2, unsigned p3);
Screens  TouchCalib     (ILI9341_t3 &tft, Controls &controls);
void     PrintPreset    (ILI9341_t3 &tft, const Preset &preset);

#endif

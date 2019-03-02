#ifndef __SCREENS_H
#define __SCREENS_H

#include <vector>
#include <cctype>

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
  TOUCH_CALIBRATE,   ///< calibrate the touch screen
};

constexpr unsigned TOUCH_CONTROL_HALFSIZE = 20;
constexpr unsigned ICON_SIZE = 48;
constexpr unsigned ICON_SPACING = 5;
constexpr unsigned BACK_BUTTON_X_POS = 255;
constexpr unsigned SETTINGS_BUTTON_X_POS = BACK_BUTTON_X_POS-ICON_SIZE-ICON_SPACING;


struct Coordinate {
    Coordinate() : x(0), y(0), control(nullptr) {}
    Coordinate(int16_t x, int16_t y, MidiControl *control) : x(x), y(y), control(control) {}
    int16_t x;
    int16_t y;
    MidiControl *control;

    // Check if the "check" point is within the specified threshold of the center
    bool checkCoordinateRange(Coordinate &coordinateCheck, unsigned threshold)
    {
        if (abs(x - coordinateCheck.x) > threshold) return false;
        if (abs(y - coordinateCheck.y) > threshold) return false;
        return true;
    }
};

void DrawPresetConfig(ILI9341_t3 &tft, Controls &controls, Preset &preset);
Screens DrawPresetNavigation(ILI9341_t3 &tft, Controls &controls, const PresetArray *presetArray, unsigned &activePreset, unsigned &selectedPreset);
Screens DrawPresetControl(ILI9341_t3 &tft, Controls &controls, Preset &preset);

TS_Point calibPoint(ILI9341_t3 &tft, Controls &controls, int16_t x, int16_t y);
TS_Point calcCalibLimits(unsigned p1, unsigned p2, unsigned p3);
Screens TouchCalib(ILI9341_t3 &tft, Controls &controls);
void PrintPreset(ILI9341_t3 &tft, const Preset &preset);

#endif

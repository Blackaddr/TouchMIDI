#ifndef __SCREENS_H
#define __SCREENS_H

#include <vector>

#include "ILI9341_t3.h"
#include "ArduinoJson.h"

#include "Preset.h"

enum class Screens : unsigned {
  PRESET_NAVIGATION,
  PRESET_EDIT
};

void DrawPresetNavigation(ILI9341_t3 &tft, const PresetArray *presetArray)
{
  int activePreset = 0;
  int16_t x,y;
  int textSize = 2;
  // Draw the Screen title
  tft.setTextSize(textSize);
  const char *title = "Preset Navigation";
  int16_t titleLength = tft.strPixelLen(const_cast<char *>(title));
  //Serial.println(String("Title length is ") + titleLength);
  
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(tft.width()/2 - titleLength/2, 10);
  tft.println(title);
  tft.println("");

  for (auto it = presetArray->begin(); it != presetArray->end(); ++it) {
    if (activePreset == (*it).index) {
      tft.getCursor(&x,&y);
      tft.fillRect(x,y,160,6*textSize, ILI9341_DARKCYAN);
    }
    tft.println((*it).index + String(" ") + (*it).name);
  }
}

void DrawPresetEdit(ILI9341_t3 &tft)
{
  // Draw the Screen title
  constexpr char title[] = "Preset Name";
  //int16_t titleLength = tft.strPixelLen(const_cast<char *>(title));

  //tft.setCursor(tft.width() - titleLength/2, 0);
  tft.print(title);
}

void PrintPreset(ILI9341_t3 &tft, const Preset &preset)
{
  tft.println(String("Name: ") + preset.name);
  tft.println(String("Index: ") + preset.index);
  tft.println("");

  for (unsigned i=0; i<preset.numControls; i++) {
    tft.println(preset.controls[i].name + String(":"));
    tft.print(String("CC:") + preset.controls[i].cc + String("   "));
    tft.print(String("Type: "));
    
    switch(preset.controls[i].type) {
      case ControlType::SWITCH_MOMENTARY :
          tft.print("MOM");
          break;
      case ControlType::SWITCH_LATCHING :
          tft.print("LAT");
          break;
      case ControlType::ROTARY_KNOB :
          tft.print("KNOB");
          break;
      default :
          break;
    }

    tft.println(String("   Value: ") + preset.controls[i].value);
    
  }
  
}

void DrawPresetInfo(ILI9341_t3 &tft, JsonObject &root)
{
  
}
#endif

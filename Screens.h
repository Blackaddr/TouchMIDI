#ifndef __SCREENS_H
#define __SCREENS_H

#include "ILI9341_t3.h"
#include "ArduinoJson.h"

#include "Preset.h"

enum class Screens : unsigned {
  PRESET_NAVIGATION,
  PRESET_EDIT
};

void DrawPresetNavigation(ILI9341_t3 &tft)
{
  // Draw the Screen title
  char title[] = "Preset Navigation";
  int16_t titleLength = tft.strPixelLen(&title[0]);
  Serial.println(String("Title length is ") + titleLength);

  tft.fillScreen(ILI9341_BLACK);
  //tft.setCursor(tft.width() - titleLength/2, 0);
  tft.setCursor(50,10);
  tft.println(title);
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
  tft.println(preset.name);
  tft.println(preset.name);
  
}

void DrawPresetInfo(ILI9341_t3 &tft, JsonObject &root)
{
  
}
#endif

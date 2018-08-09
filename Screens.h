#ifndef __SCREENS_H
#define __SCREENS_H

#include <vector>
#include <cctype>

#include "ILI9341_t3.h"
#include "ArduinoJson.h"

#include "ScreensUtil.h"
#include "Preset.h"

/// Enumerations for each screen
enum class Screens : unsigned {
  PRESET_NAVIGATION, ///< screen for navigating between and selecting a preset
  PRESET_EDIT,       ///< screen for editing a preset
  TOUCH_CALIBRATE,   ///< calibrate the touch screen
};

Screens DrawPresetNavigation(ILI9341_t3 &tft, Controls &controls, const PresetArray *presetArray, unsigned &activePreset, unsigned &selectedPreset)
{
    int16_t x,y;
    bool updateRequired = true;
  
    while(true) {
        if (updateRequired) {
            // Draw the Screen title
            clearScreen(tft);  
            const char *title = "Preset Navigation";
            tft.setCursor(0,MARGIN);
            printCentered(tft, const_cast<char*>(title));
            tft.println("");
          
            for (auto it = presetArray->begin(); it != presetArray->end(); ++it) {
              
                if (selectedPreset == (*it).index) {
                    tft.getCursor(&x,&y);
                    // TODO Fix rect width here
                    tft.fillRect(x,y,160,DEFAULT_TEXT_HEIGHT, ILI9341_DARKCYAN);      
                }
            
                if (activePreset == (*it).index) {
                    tft.setTextColor(ILI9341_RED);
                    tft.println(String("*") + (*it).index + String("* ") + (*it).name);
                    tft.setTextColor(ILI9341_WHITE);
                } else {      
                   tft.println((*it).index + String(" ") + (*it).name);
                }
            }
            updateRequired = false;
        }

        controls.getTouchPoint();
        
        int knobAdjust = controls.getRotaryAdjustUnit(0);
        if (knobAdjust != 0) {
          selectedPreset = adjustWithWrap(selectedPreset, knobAdjust, presetArray->size()-1);
          Serial.println(String("Knob adjusted by ") + knobAdjust + String(", selectedPreset is now ") + selectedPreset);
          updateRequired = true;
        }
        
        if (controls.isSwitchToggled(0)) {
          Serial.println(String("Setting activePreset to ") + selectedPreset);
          if (activePreset == selectedPreset) {
              // goto to edit screen
              return Screens::TOUCH_CALIBRATE;
          } else {
              activePreset = selectedPreset;
              updateRequired = true;
          }
        }

        delay(100);
    } // end while loop

}

Screens DrawPresetEdit(ILI9341_t3 &tft, Controls &controls, Preset &preset)
{
  int16_t x,y;
  
  // Draw the Preset Edit Screen
  clearScreen(tft);
  tft.setCursor(0,MARGIN);
  char *presetName = const_cast<char*>(preset.name.c_str());
  printCentered(tft, const_cast<char*>(presetName));
  tft.println("");

  // TODO: DRAW THE CONTROLS
  while(true) {

    
  }
}

TS_Point calibPoint(ILI9341_t3 &tft, Controls &controls, int16_t x, int16_t y)
{
  tft.drawFastHLine(x-10, y, 20, ILI9341_WHITE);
  tft.drawFastVLine(x, y-10, 20, ILI9341_WHITE);
  while(!controls.touch->touched()) {}
  TS_Point point = controls.getTouchRawPoint();
  Serial.println(String("Touchpoint raw: ") + point.x + String("   ") + point.y);

  while(controls.touch->touched()) {} // wait for touch1 released
  tft.fillRect(x-10, y-10, 20, 20, ILI9341_BLACK);
  return point;
}

TS_Point calcCalibLimits(unsigned p1, unsigned p2, unsigned p3)
{
  // Perform the calibration calculations.
  int L2 = (p3 - p1)/2;
  int LA = (p2 - p1);
  int LB = (p3 - p2);
  float Lavg =  (L2 + LA + LB)/3;
  float minf = ((p1 - Lavg) + (p2 - 2*Lavg) + (p3 - 3*Lavg))/3;
  float maxf = ((p3 + Lavg) + (p3 + 2*Lavg) + (p1 + 3*Lavg))/3;
  Serial.println(String("Min/Max is") + minf + "/" + maxf);
  TS_Point point;
  point.x = static_cast<unsigned>(minf);
  point.y = static_cast<unsigned>(maxf);
  return point;
}

Screens TouchCalib(ILI9341_t3 &tft, Controls &controls)
{
  TS_Point point1, point2, point3, point4, point5;
  clearScreen(tft);
  tft.setCursor(0, MARGIN);
  printCentered(tft, "Calibration");
  printCentered(tft, "Touch the cross");

  point1 = calibPoint(tft, controls, tft.width()/2, tft.height()/2); // center
  point2 = calibPoint(tft, controls, tft.width()/4, tft.height()/2); // left of center
  point3 = calibPoint(tft, controls, tft.width()*3/4, tft.height()/2); // right of center
  point4 = calibPoint(tft, controls, tft.width()/2, tft.height()/4); // above center
  point5 = calibPoint(tft, controls, tft.width()/2, tft.height()*3/4); // below center

  TS_Point xCalib = calcCalibLimits(point2.x, point1.x, point3.x);
  TS_Point yCalib = calcCalibLimits(point4.y, point1.y, point5.y);

  controls.setCalib(xCalib.x, xCalib.y, yCalib.x, yCalib.y);
  
  return Screens::PRESET_NAVIGATION;
}

enum class StringEditSymbols : uint8_t {
  DONE = 1,
  BACKSPACE = 2,
  SHIFT = 3,
  NUM_SYMBOLS
};

void StringEdit(ILI9341_t3 &tft, String &inputString, RotaryEncoder &encoder, Bounce &selButton)
{
  int16_t x,y;
  constexpr unsigned LEFT_MARGIN = 15;
  int textSize = 3;
  const unsigned CHAR_WIDTH = 6*textSize; // font width, * text scale
  const unsigned CHAR_WIDTH_SPACE = CHAR_WIDTH*2;
  const unsigned CHAR_HEIGHT = 8*textSize;
  char newString[32];
  inputString.toCharArray(newString, inputString.length()+1);
  uint8_t selectedChar = 'a';
  unsigned newStringIndex = inputString.length();
  bool characterShift = false;
  bool updateRequired = true;

  while(true)
  {
    if (updateRequired) {
      
      // Draw the Screen title
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(textSize);
      int16_t titleLength = tft.strPixelLen(newString);
      //Serial.println(String("Input length is ") + titleLength);
      
      tft.fillScreen(ILI9341_BLACK);

      // Print the string
      tft.setCursor(tft.width()/2 - titleLength/2, 10);
      tft.printf("%s_\n\n",newString);
      //tft.println("");
    
      // Print the alphabet
      unsigned charsPerLine = (tft.width()-LEFT_MARGIN) / CHAR_WIDTH_SPACE;
      setCursorX(tft, LEFT_MARGIN);
      unsigned charIndex = 0;

      // Print the letters
      uint8_t ch = 'a';
      //for (uint8_t ch='a'; ch<='z'; ch++)
      while(true)
      {
        // Check if this is the selected char
        if (ch == selectedChar) {
          
          tft.getCursor(&x,&y);
          x -= CHAR_WIDTH/2;  
          if (ch > static_cast<uint8_t>(StringEditSymbols::SHIFT)) {
            // This is a letter or number
            tft.fillRect(x,y,CHAR_WIDTH*2,CHAR_HEIGHT, ILI9341_DARKCYAN);  
            
          } else {
            // This is a symbol
            unsigned width;
            switch(ch) {
              case static_cast<uint8_t>(StringEditSymbols::DONE) :
                tft.fillRect(x,y,CHAR_WIDTH*5,CHAR_HEIGHT, ILI9341_DARKCYAN);  
                break;
              case static_cast<uint8_t>(StringEditSymbols::BACKSPACE) :
                tft.fillRect(x,y,CHAR_WIDTH*4,CHAR_HEIGHT, ILI9341_DARKCYAN);  
                break;
              case static_cast<uint8_t>(StringEditSymbols::SHIFT) :
                tft.fillRect(x,y,CHAR_WIDTH*6,CHAR_HEIGHT, ILI9341_DARKCYAN);  
                break;
              default :
                break;
            }
          }
        }

        // Check for symbols first
        if (ch < static_cast<uint8_t>(StringEditSymbols::NUM_SYMBOLS)) {
          // it's a symbol
          switch(ch) {
            case static_cast<uint8_t>(StringEditSymbols::DONE) :
              tft.printf("DONE ");
              break;
            case static_cast<uint8_t>(StringEditSymbols::BACKSPACE) :
              tft.printf("DEL ");
              break;
            case static_cast<uint8_t>(StringEditSymbols::SHIFT) :
              tft.printf("SHIFT ");
              break;
            default :
              break;
          }
          charIndex = 0;
        } else {
          // it's a letter or number
          char printChar = characterShift ? toupper(ch) : ch;
          if (charIndex >= charsPerLine-1) {          
            tft.printf("%c\n", printChar);
            charIndex = 0;          
            setCursorX(tft, LEFT_MARGIN);
          } else {
            tft.printf("%c ", printChar);
            charIndex++;
          }
        }


        // Switch to letters, numbers or symbols
        if (ch == 'z') { ch = '0';}
        else if (ch == '9') { ch = static_cast<uint8_t>(StringEditSymbols::DONE); tft.printf("\n\n"); setCursorX(tft, LEFT_MARGIN);}
        else if (ch == static_cast<uint8_t>(StringEditSymbols::SHIFT)) { break; }
        else { ch++; }
      }
      
      updateRequired = false;
    }

    // Check for rotary movement
    int knobAdjust = encoder.getChange();
    if (knobAdjust != 0) {
      int adjust = (knobAdjust > 0) ? 1 : -1;

      if (adjust > 0) {
        // going up
        switch (selectedChar) {
          case 'z' :
            selectedChar = '0';
            break;
          case '9' :
            selectedChar = static_cast<uint8_t>(StringEditSymbols::DONE);
            break;
          case static_cast<uint8_t>(StringEditSymbols::SHIFT) :
            selectedChar = 'a';
            break;
          default :
            selectedChar++;
        }
        
      } else {
        // going down
        switch (selectedChar) {
          case 'a' :
            selectedChar = static_cast<uint8_t>(StringEditSymbols::SHIFT);
            break;            
          case '0' :
            selectedChar = 'z';
            break;
          case static_cast<uint8_t>(StringEditSymbols::DONE) :
            selectedChar = '9';
            break;
          default :
            selectedChar--;
        }
      }
      updateRequired =  true;
      //Serial.println(String("encoder adjusted by ") + knobAdjust + String(", selectedChar is now ") + selectedChar);
    }

    // Check for button
    if (selButton.update() && selButton.fallingEdge()) {

      // Check for symbol
      if (selectedChar < static_cast<uint8_t>(StringEditSymbols::NUM_SYMBOLS)) {
        switch (selectedChar) {
            case static_cast<uint8_t>(StringEditSymbols::DONE) :
              inputString.remove(0);
              inputString.concat(newString);
              return;
              // TODO copy newString to oldstring
              //return;
              break;
            case static_cast<uint8_t>(StringEditSymbols::BACKSPACE) :
              if (newStringIndex > 0) {
                // only backspace if not on first char
                newString[newStringIndex-1] = 0;
                newStringIndex--;
              }
              break;
            case static_cast<uint8_t>(StringEditSymbols::SHIFT) :
              characterShift = !characterShift;
              break;
            default :
              break;
        }
        updateRequired = true;
      } else {
        char storedChar = characterShift ? toupper(selectedChar) : selectedChar;
        newString[newStringIndex] = storedChar;
        newStringIndex++;
        newString[newStringIndex] = 0;
        updateRequired = true;
        
      }


    }

    delay(100);

  }
  
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

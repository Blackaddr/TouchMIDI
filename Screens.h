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
  PRESET_EDIT        ///< screen for editing a preset
};

void DrawPresetNavigation(ILI9341_t3 &tft, const PresetArray *presetArray, unsigned activePreset, unsigned selectedPreset)
{
  int16_t x,y;
  
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

}

void DrawPresetEdit(ILI9341_t3 &tft, Preset &preset)
{
  int16_t x,y;
  
  // Draw the Preset Edit Screen
  clearScreen(tft);
  tft.setCursor(0,MARGIN);
  char *presetName = const_cast<char*>(preset.name.c_str());
  printCentered(tft, const_cast<char*>(presetName));
  tft.println("");

  // TODO: DRAW THE CONTROLS
  

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

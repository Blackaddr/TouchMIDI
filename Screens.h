#ifndef __SCREENS_H
#define __SCREENS_H

#include <vector>
#include <cctype>

#include "ILI9341_t3.h"
#include "ArduinoJson.h"

#include "Bitmap.h"
#include "Graphics.h"
#include "Controls.h"
#include "ScreensUtil.h"
#include "Preset.h"

// Prototypes
void StringEdit(ILI9341_t3 &tft, String &inputString, RotaryEncoder &encoder, Bounce &selButton);

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

void DrawPresetConfig(ILI9341_t3 &tft, Controls &controls, Preset &preset)
{

    bool redrawScreen = true;
    int16_t nameEditButtonPosition;

    while (true) {

        // Draw the Preset Edit Screen
        if (redrawScreen) {
            clearScreen(tft);
            tft.setCursor(0,MARGIN);

            // print the preset number in the top left
            tft.print(preset.index);
            char *presetName = const_cast<char*>(preset.name.c_str());
            tft.setCursor(tft.width()/10,MARGIN);
            tft.print(const_cast<char*>(presetName));

            // Draw the icons
            bmpDraw(tft, "back48.bmp", BACK_BUTTON_X_POS,0); // shifting more than 255 pixels seems to wrap the screen
            nameEditButtonPosition = tft.getCursorX() + MARGIN;
            bmpDraw(tft, "edit48.bmp", nameEditButtonPosition, 0);
            redrawScreen = false;
        }

        // Check for touch activity
        if (controls.isTouched()) {
            TS_Point touchPoint = controls.getTouchPoint();
            Coordinate touchCoordinate(touchPoint.x, touchPoint.y, nullptr);

            // Check the back button
            if (touchPoint.x > static_cast<int16_t>(BACK_BUTTON_X_POS) && touchPoint.y < static_cast<int16_t>(ICON_SIZE) ) {
                // wait until the screen is no longer touched to take action
                while (controls.isTouched()) {}
                return;
            }

            // Check the name edit button button
            if ( touchPoint.x > static_cast<int16_t>(nameEditButtonPosition) && touchPoint.y < static_cast<int16_t>(ICON_SIZE) &&
                 touchPoint.x < static_cast<int16_t>(nameEditButtonPosition + ICON_SIZE) ) {
                // wait until the screen is no longer touched to take action
                while (controls.isTouched()) {}

                // call the string edit screen
                StringEdit(tft, preset.name, controls.m_encoders[0], controls.m_switches[0]);
                redrawScreen = true;
            }

            // wait for touch release
            while (controls.isTouched()) {}
        }
    }
}

Screens DrawPresetNavigation(ILI9341_t3 &tft, Controls &controls, const PresetArray *presetArray, unsigned &activePreset, unsigned &selectedPreset)
{
    int16_t x,y;
    bool redrawScreen = true;
  
    while(true) {
        if (redrawScreen) {
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
            redrawScreen = false;
        }

        controls.getTouchPoint();
        
        int knobAdjust = controls.getRotaryAdjustUnit(0);
        if (knobAdjust != 0) {
          selectedPreset = adjustWithWrap(selectedPreset, knobAdjust, presetArray->size()-1);
          Serial.println(String("Knob adjusted by ") + knobAdjust + String(", selectedPreset is now ") + selectedPreset);
          redrawScreen = true;
        }
        
        if (controls.isSwitchToggled(0)) {
          Serial.println(String("Setting activePreset to ") + selectedPreset);
          if (activePreset == selectedPreset) {
              // goto to edit screen
              return Screens::PRESET_CONTROL;
          } else {
              activePreset = selectedPreset;
              redrawScreen = true;
          }
        }

        delay(100);
    } // end while loop

}

Screens DrawPresetControl(ILI9341_t3 &tft, Controls &controls, Preset &preset)
{
    unsigned activeControl = 0;

    // Create an array to store the knob and switch center locations. This will
    // be used to map touch points to knobs, etc.
    Coordinate controlLocations[MAX_NUM_CONTROLS];

    bool redrawScreen = true;
    bool redoLayout = true;
    bool redrawControls = true;

    unsigned numKnobs = 0;
    unsigned numSwitches = 0;

    while(true) {

        // Check if we need to calculate the icon layout
        if (redoLayout) {
            // Get the number of knobs vs switches
            numKnobs = 0;
            numSwitches = 0;
            for (auto it = preset.controls.begin(); it != preset.controls.end(); it++) {
                if ((*it).type == ControlType::ROTARY_KNOB) {
                    numKnobs++;
                } else {
                    numSwitches++;
                }
            }

            // First configure the knob locations
            unsigned knobOffset = tft.width() / (numKnobs+1);
            int16_t xPos;
            int16_t yPos;
            xPos = knobOffset; // overwrite
            yPos = 100;

            unsigned idx = 0;
            for (auto it = preset.controls.begin(); it != preset.controls.end(); it++) {
              if ((*it).type == ControlType::ROTARY_KNOB) {
                  controlLocations[idx] = Coordinate(xPos, yPos, &(*it));
                  xPos += knobOffset;
                  idx++;
              }
            }

            // Next configure the switch locations
            unsigned switchOffset = tft.width() / (numSwitches+1);
            xPos = switchOffset;
            yPos = tft.height()/2 + 75;
            for (auto it = preset.controls.begin(); it != preset.controls.end(); it++) {
              if ((*it).type == ControlType::SWITCH_MOMENTARY || (*it).type == ControlType::SWITCH_LATCHING) {

                  controlLocations[idx] = Coordinate(xPos, yPos, &(*it));
                  xPos += switchOffset;
                  idx++;
              }
            }
            redoLayout = false;
        } // redoLayout

        if (redrawScreen) {
            // Draw the Preset Edit Screen
            clearScreen(tft);
            tft.setCursor(0,MARGIN);

            // print the preset number in the top left
            tft.print(preset.index);
            char *presetName = const_cast<char*>(preset.name.c_str());
            tft.setCursor(tft.width()/10,MARGIN);
            tft.print(const_cast<char*>(presetName));

            // Draw the icons
            bmpDraw(tft, "back48.bmp", BACK_BUTTON_X_POS,0); // shifting more than 255 pixels seems to wrap the screen
            bmpDraw(tft, "seting48.bmp", SETTINGS_BUTTON_X_POS, 0);
            redrawScreen = false;
        }


        if (redrawControls) {
            // Draw a light filled box behind the active control and black behind the others

            // Draw the controls
            for (unsigned i=0; i<MAX_NUM_CONTROLS; i++) {
                if (controlLocations[i].control) {
                    MidiControl &controlPtr = *controlLocations[i].control;
                    // valid pointer to control
                    uint16_t color = (activeControl == i) ? ILI9341_YELLOW : ILI9341_BLACK;
                    drawActiveControl(tft, controlLocations[i].x, controlLocations[i].y, color);

                    if (controlPtr.type == ControlType::ROTARY_KNOB) {
                        drawKnob(tft, controlPtr, controlLocations[i].x, controlLocations[i].y);
                    } else {
                        drawSwitch(tft, controlPtr, controlLocations[i].x, controlLocations[i].y);
                    }
                }
            }
            redrawControls = false;
        }

        // run a loop waiting for a control input from touch, rotary or switch
        while(true) {
            MidiControl &control = *controlLocations[activeControl].control;

            // Check for touch activity
            if (controls.isTouched()) {
                TS_Point touchPoint = controls.getTouchPoint();
                Coordinate touchCoordinate(touchPoint.x, touchPoint.y, nullptr);

                // wait until the screen is no longer touched before taking action
                while (controls.isTouched()) {}

                // Check the back button
                if (touchPoint.x > static_cast<int16_t>(BACK_BUTTON_X_POS) && touchPoint.y < static_cast<int16_t>(ICON_SIZE) ) {
                    return Screens::PRESET_NAVIGATION;
                }

                // Check for the settings button
                if (touchPoint.x > static_cast<int16_t>(SETTINGS_BUTTON_X_POS) && touchPoint.y < static_cast<int16_t>(ICON_SIZE) ) {
                    DrawPresetConfig(tft, controls, preset);
                    // The preset config screen cleared the settings icon so redraw it
                    //bmpDraw(tft, "seting48.bmp", SETTINGS_BUTTON_X_POS, 0);
                    redoLayout = true;
                    redrawScreen = true;
                    redrawControls = true;
                    break; // break out of control-detect loop
                }

                // Check for controls
                for (auto i=0; i<MAX_NUM_CONTROLS; i++) {
                    if ( controlLocations[i].checkCoordinateRange(touchCoordinate, TOUCH_CONTROL_HALFSIZE) ) {
                        activeControl = i;
                        redrawControls = true;
                        break; // break out of the for loop
                    }
                }

                // break out of the control-detect loop if we need to redraw controls
                if (redrawControls) { break; }
            }

            // Check for rotary activity
            int adjust = controls.getRotaryAdjustUnit(0);
            if (adjust != 0) {
                // primary encoder
                Serial.println(String("Adjust by ") + adjust);

                if (control.type == ControlType::ROTARY_KNOB) {
                    control.value = adjustWithSaturation(control.value, adjust, 0, 127);
                    control.updated = true;
                    redrawControls = true;
                    break;
                }
            }

            // Check for pushbutton control
            if (controls.isSwitchToggled(0)) {
                Serial.println("Toggled!");
                if (control.type == ControlType::SWITCH_MOMENTARY) {
                    control.value = static_cast<unsigned>(toggleValue(control.value, 127));
                    control.updated = true;
                    redrawControls = true;
                    break;
                }
            }

            delay(100); // this is needed for control sampling to work
        } // end while loop to process control inputs

    } // end outer while loop
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
            //unsigned width;
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

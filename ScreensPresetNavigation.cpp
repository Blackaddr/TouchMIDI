/*
 * ScreensPresetNavigation.cpp
 *
 *  Created on: Mar. 2, 2019
 *      Author: blackaddr
 */
#include "Screens.h"

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




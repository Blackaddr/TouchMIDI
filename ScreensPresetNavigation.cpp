/*
 * ScreensPresetNavigation.cpp
 *
 *  Created on: Mar. 2, 2019
 *      Author: blackaddr
 */
#include "Screens.h"

constexpr int SELECTED_TEXT_WIDTH = 160;

void updatePresetArrayIndices(PresetArray &presetArray)
{
    int i= 0;
    for (auto it = presetArray.begin(); it != presetArray.end(); ++it) {
        presetArray[i].index = i;
        i++;
    }
}

Screens DrawPresetNavigation(ILI9341_t3 &tft, Controls &controls, PresetArray &presetArray, unsigned &activePreset, unsigned &selectedPreset)
{
    int16_t x,y;
    bool redrawScreen = true;

    // Calculate button locations
    const unsigned BOTTOM_ICON_ROW_Y_POS = tft.height() - ICON_SIZE;
    const unsigned ADD_BUTTON_X_POS = BACK_BUTTON_X_POS;
    const unsigned ADD_BUTTON_Y_POS = BOTTOM_ICON_ROW_Y_POS;
    const unsigned REMOVE_BUTTON_X_POS = ADD_BUTTON_X_POS - ICON_SIZE - ICON_SPACING;
    const unsigned REMOVE_BUTTON_Y_POS = BOTTOM_ICON_ROW_Y_POS;
    const unsigned SAVE_BUTTON_X_POS = REMOVE_BUTTON_X_POS-ICON_SIZE-ICON_SPACING;
    const unsigned SAVE_BUTTON_Y_POS = BOTTOM_ICON_ROW_Y_POS;

    const TouchArea SAVE_BUTTON_AREA(SAVE_BUTTON_X_POS, SAVE_BUTTON_X_POS+ICON_SIZE, SAVE_BUTTON_Y_POS, SAVE_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea ADD_BUTTON_AREA(ADD_BUTTON_X_POS, ADD_BUTTON_X_POS+ICON_SIZE, ADD_BUTTON_Y_POS, ADD_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea REMOVE_BUTTON_AREA(REMOVE_BUTTON_X_POS, REMOVE_BUTTON_X_POS+ICON_SIZE, REMOVE_BUTTON_Y_POS, REMOVE_BUTTON_Y_POS+ICON_SIZE);

    while(true) {
        if (redrawScreen) {
            // Draw the Screen title
            clearScreen(tft);
            const char *title = "Preset Navigation";
            tft.setCursor(0,MARGIN);
            printCentered(tft, const_cast<char*>(title));
            tft.println("");

            // Draw the icons
            bmpDraw(tft, "save48.bmp", SAVE_BUTTON_X_POS, SAVE_BUTTON_Y_POS);
            bmpDraw(tft, "add48.bmp", ADD_BUTTON_X_POS, ADD_BUTTON_Y_POS);
            bmpDraw(tft, "remove48.bmp", REMOVE_BUTTON_X_POS, REMOVE_BUTTON_Y_POS);

            for (auto it = presetArray.begin(); it != presetArray.end(); ++it) {

                if (selectedPreset == (*it).index) {
                    tft.getCursor(&x,&y);
                    // TODO Fix rect width here
                    tft.fillRect(x,y,SELECTED_TEXT_WIDTH,DEFAULT_TEXT_HEIGHT, ILI9341_DARKCYAN);
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

        // Check for touch activity
        if (controls.isTouched()) {
            TouchPoint touchPoint = controls.getTouchPoint();

            // Check the save button
            if (SAVE_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                if (confirmationScreen(tft, controls, "Confirm SAVE ALL?")) {

                    for (auto it=presetArray.begin(); it != presetArray.end(); ++it) {
                        StaticJsonBuffer<1024> jsonBuffer; // stack buffer
                        JsonObject& root = jsonBuffer.createObject();
                        presetToJson(*it, root);

                        char presetFilename[] = "PRESETX.JSN";
                        constexpr unsigned PRESET_ID_INDEX = 6;
                        presetFilename[PRESET_ID_INDEX] = (*it).index + 0x30;
                        writePresetToFile(presetFilename, root); // Write to the SD card
                    }
                }
                redrawScreen = true;
            }

            // Check the add button
            if (ADD_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release

                if (confirmationScreen(tft, controls, "Confirm ADD?")) {
                    // insert a new preset before index selectedPreset
                    auto presetToInsertBefore = presetArray.begin() + selectedPreset;
                    presetArray.insert(presetToInsertBefore, Preset());
                    updatePresetArrayIndices(presetArray);
                }
                redrawScreen = true;
            }

            // Check the remove button
            if (REMOVE_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                if (confirmationScreen(tft, controls, "Confirm REMOVE?")) {
                    auto presetToErase = presetArray.begin() + selectedPreset;
                    presetArray.erase(presetToErase);
                    updatePresetArrayIndices(presetArray);
                }
                redrawScreen = true;
            }

            // wait for touch release
            while (controls.isTouched()) {}
        }

        //controls.getTouchPoint();

        int knobAdjust = controls.getRotaryAdjustUnit(0);
        if (knobAdjust != 0) {
          selectedPreset = adjustWithWrap(selectedPreset, knobAdjust, presetArray.size()-1);
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




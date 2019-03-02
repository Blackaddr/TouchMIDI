/*
 * ScreensPresetConfig.cpp
 *
 *  Created on: Mar. 2, 2019
 *      Author: blackaddr
 */
#include "Screens.h"

// This screen provides a way for editing a preset.
const TouchArea BACK_BUTTON_AREA(BACK_BUTTON_X_POS, BACK_BUTTON_X_POS+ICON_SIZE, 0, ICON_SIZE);

constexpr int SELECTED_TEXT_WIDTH = 160;
constexpr unsigned CONTROL_ENCODER = 0;
constexpr unsigned CONTROL_SWITCH  = 0;

void DrawPresetConfig(ILI9341_t3 &tft, Controls &controls, Preset &preset)
{

    bool redrawScreen = true;
    int16_t nameEditButtonPosition;
    TouchArea editNameArea;
    int16_t x,y;
    auto selectedControl = preset.controls.begin();

    while (true) {

        // Draw the Preset Edit Screen
        if (redrawScreen) {
            clearScreen(tft);
            tft.setCursor(0,MARGIN);

            // 1) print the preset number in the top left
            tft.print(preset.index);
            char *presetName = const_cast<char*>(preset.name.c_str());
            tft.setCursor(tft.width()/10,MARGIN);
            tft.print(const_cast<char*>(presetName));

            // 2) Draw the icons

            // BACK button
            bmpDraw(tft, "back48.bmp", BACK_BUTTON_X_POS,0); // shifting more than 255 pixels seems to wrap the screen

            // NAME EDIT button
            nameEditButtonPosition = tft.getCursorX() + MARGIN;
            bmpDraw(tft, "edit48.bmp", nameEditButtonPosition, 0);
            redrawScreen = false;
            editNameArea.setArea(nameEditButtonPosition, nameEditButtonPosition+ICON_SIZE, 0, ICON_SIZE);

            // Draw the Controls with their type and names.
            tft.setCursor(0,ICON_SIZE); // start row under icons
            for (auto it = preset.controls.begin(); it != preset.controls.end(); ++it) {
                //tft.printf("%s\n",(*it).name.c_str());
                tft.getCursor(&x,&y);
                // TODO Fix rect width here
                if (it == selectedControl) {
                    tft.fillRect(x,y,SELECTED_TEXT_WIDTH,DEFAULT_TEXT_HEIGHT, ILI9341_DARKCYAN);
                }

                // color code the control types
                char controlTypeChar = ' ';
                uint16_t color = ILI9341_WHITE;
                if ((*it).type == ControlType::ROTARY_KNOB)      {controlTypeChar = 'K'; color = ILI9341_BLUE;}
                if ((*it).type == ControlType::SWITCH_LATCHING)  {controlTypeChar = 'L'; color = ILI9341_RED;}
                if ((*it).type == ControlType::SWITCH_MOMENTARY) {controlTypeChar = 'M'; color = ILI9341_GREEN;}
                tft.printf("(");
                tft.setTextColor(color);
                tft.print(controlTypeChar);
                tft.setTextColor(ILI9341_WHITE);
                tft.printf(") ");
                tft.printf("%s\n",(*it).name.c_str());
            }
        }

        // Check for touch activity
        if (controls.isTouched()) {
            TouchPoint touchPoint = controls.getTouchPoint();
            //Coordinate touchCoordinate(touchPoint.x, touchPoint.y, nullptr);

            // Check the back button
            if (BACK_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                return;
            }

            // Check the name edit button button
            if (editNameArea.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                StringEdit(tft, preset.name, *controls.touch, controls.m_encoders[0], controls.m_switches[0]);
                redrawScreen = true;
            }

            // wait for touch release
            while (controls.isTouched()) {}
        }

        // Check for encoder activity
        int knobAdjust = controls.getRotaryAdjustUnit(CONTROL_ENCODER);
        if (knobAdjust > 0) {
            if (selectedControl != preset.controls.end()-1) {
                ++selectedControl;
                redrawScreen = true;
            }
        } else if (knobAdjust < 0) {
            if (selectedControl != preset.controls.begin()) {
                --selectedControl;
                redrawScreen = true;
            }
        }

        if (controls.isSwitchToggled(CONTROL_SWITCH)) {
          // TODO: Edit the preset control
        }

        delay(100); // needed in order for encoder activity sampling/filteirng
    }
}




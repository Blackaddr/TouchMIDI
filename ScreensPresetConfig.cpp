/*
 * ScreensPresetConfig.cpp
 *
 *  Created on: Mar. 2, 2019
 *      Author: blackaddr
 */
#include "Screens.h"

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
                //StringEdit(tft, preset.name, controls.m_encoders[0], controls.m_switches[0]);
                StringEdit(tft, preset.name, *controls.touch, controls.m_encoders[0], controls.m_switches[0]);
                redrawScreen = true;
            }

            // wait for touch release
            while (controls.isTouched()) {}
        }
    }
}




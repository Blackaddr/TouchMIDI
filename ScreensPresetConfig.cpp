/*
 * ScreensPresetConfig.cpp
 *
 *  Created on: Mar. 2, 2019
 *      Author: blackaddr
 */
#include "FileAccess.h"
#include "Screens.h"

// This screen provides a way for editing a preset.
const TouchArea BACK_BUTTON_AREA(BACK_BUTTON_X_POS, BACK_BUTTON_X_POS+ICON_SIZE, 0, ICON_SIZE);


constexpr int SELECTED_TEXT_WIDTH = 200;

void DrawPresetConfig(ILI9341_t3 &tft, Controls &controls, Preset &preset)
{

    bool redrawScreen = true;
    int16_t nameEditButtonPosition;
    TouchArea editNameArea;
    int16_t x,y;
    auto selectedControl = preset.controls.begin();

    // Calculate button locations
    const unsigned BOTTOM_ICON_ROW_Y_POS = tft.height() - ICON_SIZE;
    const unsigned ADD_BUTTON_X_POS = BACK_BUTTON_X_POS;
    const unsigned ADD_BUTTON_Y_POS = BOTTOM_ICON_ROW_Y_POS;
    const unsigned REMOVE_BUTTON_X_POS = ADD_BUTTON_X_POS - ICON_SIZE - ICON_SPACING;
    const unsigned REMOVE_BUTTON_Y_POS = BOTTOM_ICON_ROW_Y_POS;
    const unsigned MOVEUP_BUTTON_X_POS = REMOVE_BUTTON_X_POS - ICON_SIZE - ICON_SPACING;
    const unsigned MOVEUP_BUTTON_Y_POS = BOTTOM_ICON_ROW_Y_POS;
    const unsigned MOVEDN_BUTTON_X_POS = MOVEUP_BUTTON_X_POS - ICON_SIZE - ICON_SPACING;
    const unsigned MOVEDN_BUTTON_Y_POS = BOTTOM_ICON_ROW_Y_POS;
    const unsigned SAVE_BUTTON_X_POS = MOVEDN_BUTTON_X_POS-ICON_SIZE-ICON_SPACING;
    const unsigned SAVE_BUTTON_Y_POS = BOTTOM_ICON_ROW_Y_POS;

    const TouchArea SAVE_BUTTON_AREA(SAVE_BUTTON_X_POS, SAVE_BUTTON_X_POS+ICON_SIZE, SAVE_BUTTON_Y_POS, SAVE_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea ADD_BUTTON_AREA(ADD_BUTTON_X_POS, ADD_BUTTON_X_POS+ICON_SIZE, ADD_BUTTON_Y_POS, ADD_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea REMOVE_BUTTON_AREA(REMOVE_BUTTON_X_POS, REMOVE_BUTTON_X_POS+ICON_SIZE, REMOVE_BUTTON_Y_POS, REMOVE_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea MOVEUP_BUTTON_AREA(MOVEUP_BUTTON_X_POS, MOVEUP_BUTTON_X_POS+ICON_SIZE, MOVEUP_BUTTON_Y_POS, MOVEUP_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea MOVEDN_BUTTON_AREA(MOVEDN_BUTTON_X_POS, MOVEDN_BUTTON_X_POS+ICON_SIZE, MOVEDN_BUTTON_Y_POS, MOVEDN_BUTTON_Y_POS+ICON_SIZE);

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
            bmpDraw(tft, "back48.bmp", BACK_BUTTON_X_POS, BACK_BUTTON_Y_POS); // shifting more than 255 pixels seems to wrap the screen
            bmpDraw(tft, "add48.bmp", ADD_BUTTON_X_POS, ADD_BUTTON_Y_POS);
            bmpDraw(tft, "remove48.bmp", REMOVE_BUTTON_X_POS, REMOVE_BUTTON_Y_POS);
            bmpDraw(tft, "moveup48.bmp", MOVEUP_BUTTON_X_POS, MOVEUP_BUTTON_Y_POS);
            bmpDraw(tft, "movedn48.bmp", MOVEDN_BUTTON_X_POS, MOVEDN_BUTTON_Y_POS);
            bmpDraw(tft, "save48.bmp", SAVE_BUTTON_X_POS, SAVE_BUTTON_Y_POS);

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

            // Check the back button
            if (BACK_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                return;
            }

            // Check the name edit button button
            if (editNameArea.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                StringEdit(tft, preset.name, controls, controls.m_encoders[0], controls.m_switches[0]);
                redrawScreen = true;
            }

            // Check the save button
            if (SAVE_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                if (confirmationScreen(tft, controls, "Confirm SAVE?")) {
                    StaticJsonBuffer<1024> jsonBuffer; // stack buffer
                    JsonObject& root = jsonBuffer.createObject();
                    presetToJson(preset, root);

                    char presetFilename[] = "PRESETX.JSN";
                    constexpr unsigned PRESET_ID_INDEX = 6;
                    presetFilename[PRESET_ID_INDEX] = preset.index + 0x30;
                    writePresetToFile(presetFilename, root); // Write to the SD card
                }
                redrawScreen = true;
            }

            // Check the add button
            if (ADD_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release

                if (confirmationScreen(tft, controls, "Confirm ADD?")) {
                    MidiControl newControl = MidiControl(
                        String("*New*"),
                        String("shortName"),
                        0,
                        InputControl::NOT_CONFIGURED,
                        ControlType::SWITCH_MOMENTARY,
                        0
                      );
                    selectedControl = preset.controls.insert(selectedControl, newControl);
                    preset.numControls++;
                }
                redrawScreen = true;
            }

            // Check the remove button
            if (REMOVE_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                if (confirmationScreen(tft, controls, "Confirm REMOVE?")) {
                    auto previousControl = preset.controls.begin();
                    if (selectedControl != preset.controls.begin()) { previousControl = selectedControl-1; }
                    preset.controls.erase(selectedControl);
                    selectedControl = previousControl;
                    if (preset.numControls > 1) { preset.numControls--; }
                }
                redrawScreen = true;
            }

            // Check the MOVEUP button
            if (MOVEUP_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                if (selectedControl !=  preset.controls.begin()) { // can't go above the top one
                    // swap the preset with the previous by inserting a copy of the selected
                    // preset before the previous, then delete the old one.
                    unsigned selectedIndex = std::distance(preset.controls.begin(), selectedControl);
                    auto controlToInsertBefore = selectedControl-1;
                    preset.controls.insert(controlToInsertBefore, *selectedControl);

                    auto controlToErase = preset.controls.begin() + selectedIndex + 1;
                    preset.controls.erase(controlToErase);
                    selectedControl = preset.controls.begin() + selectedIndex - 1;
                    redrawScreen = true;
                }
            }

            // Check the MOVEUDN button
            if (MOVEDN_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                if (selectedControl < preset.controls.end()-1 ) { // can't go below the last
                    // swap the preset with the next by inserting a copy of the selected
                    // preset after the next, then delete the old one.
                    unsigned selectedIndex = std::distance(preset.controls.begin(), selectedControl);
                    auto controlToInsertBefore = selectedControl+2;
                    preset.controls.insert(controlToInsertBefore, *selectedControl);

                    auto controlToErase = preset.controls.begin() + selectedIndex;
                    preset.controls.erase(controlToErase);
                    selectedControl = preset.controls.begin() + selectedIndex +1;
                    redrawScreen = true;
                }
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
            DrawMidiControlConfig(tft, controls, (*selectedControl));
            redrawScreen = true;
        }

        delay(100); // needed in order for encoder activity sampling/filteirng
    }
}




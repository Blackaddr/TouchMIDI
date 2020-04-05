/*
 * ScreensPresetNavigation.cpp
 *
 *  Created on: Mar. 2, 2019
 *      Author: blackaddr
 */
#include <MIDI.h>
#include "Screens.h"
#include "MidiProc.h"
#include "MidiDefs.h"

using namespace midi;

constexpr int SELECTED_TEXT_WIDTH = 160;

Screens g_currentScreen = Screens::PRESET_NAVIGATION;

void moveUp  (PresetArray &presetArray, unsigned &activePreset, unsigned &selectedPreset, bool &redrawScreen);
void moveDown(PresetArray &presetArray, unsigned &activePreset, unsigned &selectedPreset, bool &redrawScreen);

void updatePresetArrayIndices(PresetArray &presetArray)
{
    int i= 0;
    for (auto it = presetArray.begin(); it != presetArray.end(); ++it) {
        presetArray[i].index = i;
        i++;
    }
}

Screens DrawPresetNavigation(ILI9341_t3 &tft, Controls &controls, PresetArray &presetArray, midi::MidiInterface<HardwareSerial> &midiPort,
        unsigned &activePreset, unsigned &selectedPreset)
{
    int16_t x,y;
    bool redrawScreen = true;
    bool selectTriggeredMidi = false;
    int  midiAdjust = 0;

    // Calculate button locations
    const unsigned BOTTOM_ICON_ROW_Y_POS = tft.height() - ICON_SIZE;
    const unsigned ADD_BUTTON_X_POS      = BACK_BUTTON_X_POS;
    const unsigned ADD_BUTTON_Y_POS      = BOTTOM_ICON_ROW_Y_POS;
    const unsigned REMOVE_BUTTON_X_POS   = ADD_BUTTON_X_POS - ICON_SIZE - ICON_SPACING;
    const unsigned REMOVE_BUTTON_Y_POS   = BOTTOM_ICON_ROW_Y_POS;
    const unsigned MOVEUP_BUTTON_X_POS   = REMOVE_BUTTON_X_POS - ICON_SIZE - ICON_SPACING;
    const unsigned MOVEUP_BUTTON_Y_POS   = BOTTOM_ICON_ROW_Y_POS;
    const unsigned MOVEDN_BUTTON_X_POS   = MOVEUP_BUTTON_X_POS - ICON_SIZE - ICON_SPACING;
    const unsigned MOVEDN_BUTTON_Y_POS   = BOTTOM_ICON_ROW_Y_POS;
    const unsigned SAVE_BUTTON_X_POS     = MOVEDN_BUTTON_X_POS-ICON_SIZE-ICON_SPACING;
    const unsigned SAVE_BUTTON_Y_POS     = BOTTOM_ICON_ROW_Y_POS;
    const unsigned EXTRA_BUTTON_X_POS    = BACK_BUTTON_X_POS;
    const unsigned EXTRA_BUTTON_Y_POS    = BOTTOM_ICON_ROW_Y_POS - ICON_SIZE - ICON_SPACING;


    const TouchArea SAVE_BUTTON_AREA(SAVE_BUTTON_X_POS, SAVE_BUTTON_X_POS+ICON_SIZE, SAVE_BUTTON_Y_POS, SAVE_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea ADD_BUTTON_AREA(ADD_BUTTON_X_POS, ADD_BUTTON_X_POS+ICON_SIZE, ADD_BUTTON_Y_POS, ADD_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea REMOVE_BUTTON_AREA(REMOVE_BUTTON_X_POS, REMOVE_BUTTON_X_POS+ICON_SIZE, REMOVE_BUTTON_Y_POS, REMOVE_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea MOVEUP_BUTTON_AREA(MOVEUP_BUTTON_X_POS, MOVEUP_BUTTON_X_POS+ICON_SIZE, MOVEUP_BUTTON_Y_POS, MOVEUP_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea MOVEDN_BUTTON_AREA(MOVEDN_BUTTON_X_POS, MOVEDN_BUTTON_X_POS+ICON_SIZE, MOVEDN_BUTTON_Y_POS, MOVEDN_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea EXTRA_BUTTON_AREA(EXTRA_BUTTON_X_POS, EXTRA_BUTTON_X_POS+ICON_SIZE, EXTRA_BUTTON_Y_POS, EXTRA_BUTTON_Y_POS+ICON_SIZE);

    while(true) {
        if (redrawScreen) {
            // Draw the Screen title
            clearScreen(tft);
            const char *title = "Preset Navigation\n";
            tft.setCursor(0,MARGIN);
            printCentered(tft, const_cast<char*>(title));
            tft.println("");

            // Draw the icons
            bmpDraw(tft, "save48.bmp", SAVE_BUTTON_X_POS, SAVE_BUTTON_Y_POS);
            bmpDraw(tft, "add48.bmp", ADD_BUTTON_X_POS, ADD_BUTTON_Y_POS);
            bmpDraw(tft, "remove48.bmp", REMOVE_BUTTON_X_POS, REMOVE_BUTTON_Y_POS);
            bmpDraw(tft, "moveup48.bmp", MOVEUP_BUTTON_X_POS, MOVEUP_BUTTON_Y_POS);
            bmpDraw(tft, "movedn48.bmp", MOVEDN_BUTTON_X_POS, MOVEDN_BUTTON_Y_POS);
            bmpDraw(tft, "extra48.bmp", EXTRA_BUTTON_X_POS, EXTRA_BUTTON_Y_POS);

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
                if (confirmationScreen(tft, controls, "Confirm SAVE ALL?\n")) {

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

                if (confirmationScreen(tft, controls, "Confirm ADD?\n")) {
                    // insert a new preset before index selectedPreset
                    auto presetToInsertBefore = presetArray.begin() + selectedPreset;
                    presetArray.insert(presetToInsertBefore, Preset());
                    updatePresetArrayIndices(presetArray);

                    if (activePreset >= selectedPreset) { activePreset++; }
                }
                redrawScreen = true;
            }

            // Check the remove button
            if (REMOVE_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                if (confirmationScreen(tft, controls, "Confirm REMOVE?\n")) {
                    auto presetToErase = presetArray.begin() + selectedPreset;
                    presetArray.erase(presetToErase);
                    updatePresetArrayIndices(presetArray);

                    if (activePreset >= selectedPreset) { activePreset--; }
                }
                redrawScreen = true;
            }

            // Check the MOVEUP button
            if (MOVEUP_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                moveUp(presetArray, activePreset, selectedPreset, redrawScreen);
            }

            // Check the MOVEUDN button
            if (MOVEDN_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                moveDown(presetArray, activePreset, selectedPreset, redrawScreen);
            }

            // Check the EXTRA button
            if (EXTRA_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                return Screens::MIDI_MONITOR;
            }

            // wait for touch release
            while (controls.isTouched()) {}
        }

        // Check for MIDI
        while (midiInQueue->size() > 0) {
            MidiWord midiWord;
            {
                std::lock_guard<std::mutex> lock(midiInQueueMutex);
                midiWord = midiInQueue->front();
                midiInQueue->pop();
            } // mutex unlocks

            if ((midiWord.type == midi::ControlChange) && (midiWord.data2 == MIDI_OFF_VALUE)) {
                int adjust = 0;
                switch (midiWord.data1) {
                case MIDI_CC_SPECIAL_UP     : midiAdjust = -1; break;
                case MIDI_CC_SPECIAL_DOWN   : midiAdjust =  1; break;
                case MIDI_CC_SPECIAL_SELECT : selectTriggeredMidi = true; break;
                default: break;
                }
            }
        }

        int knobAdjust = controls.getRotaryAdjustUnit(0) + midiAdjust;
        if (knobAdjust != 0) {
          selectedPreset = adjustWithWrap(selectedPreset, knobAdjust, presetArray.size()-1);
          Serial.println(String("Knob adjusted by ") + knobAdjust + String(", selectedPreset is now ") + selectedPreset);
          redrawScreen = true;
          midiAdjust = 0;
        }

        if (controls.isSwitchToggled(0) || selectTriggeredMidi) {
          Serial.println(String("Setting activePreset to ") + selectedPreset);
          if (activePreset == selectedPreset) {
              // goto to edit screen
              return Screens::PRESET_CONTROL;
          } else {
              activePreset = selectedPreset;
              midiProgramSend(activePreset, MIDI_PROGRAM_CHANNEL);
              redrawScreen = true;
              selectTriggeredMidi = false;
          }
        }

        delay(100);
        yield();
    } // end while loop

}

void moveUp(PresetArray &presetArray, unsigned &activePreset, unsigned &selectedPreset, bool &redrawScreen)
{
    if (selectedPreset > 0) { // can't go above the top one
        // swap the preset with the previous by inserting a copy of the selected
        // preset before the previous, then delete the old one.
        auto presetToInsertBefore = presetArray.begin() + selectedPreset-1;
        presetArray.insert(presetToInsertBefore, presetArray[selectedPreset]);

        auto presetToErase = presetArray.begin() + selectedPreset+1;
        presetArray.erase(presetToErase);

        if (activePreset == selectedPreset-1) { activePreset++; }
        else if (activePreset == selectedPreset) { activePreset--;}
        selectedPreset--;
        updatePresetArrayIndices(presetArray);
        midiProgramSend(activePreset, MIDI_PROGRAM_CHANNEL);

        redrawScreen = true;
    }

}

void moveDown(PresetArray &presetArray, unsigned &activePreset, unsigned &selectedPreset, bool &redrawScreen)
{
    if (selectedPreset < (*presetArray.end()).index ) { // can't go below the last
        // swap the preset with the next by inserting a copy of the selected
        // preset after the next, then delete the old one.
        auto presetToInsertBefore = presetArray.begin() + selectedPreset+2;
        presetArray.insert(presetToInsertBefore, presetArray[selectedPreset]);

        auto presetToErase = presetArray.begin() + selectedPreset;
        presetArray.erase(presetToErase);

        if (activePreset == selectedPreset+1) { activePreset--; }
        else if (activePreset == selectedPreset) { activePreset++;}
        selectedPreset++;
        updatePresetArrayIndices(presetArray);
        midiProgramSend(activePreset, MIDI_PROGRAM_CHANNEL);

        redrawScreen = true;
    }
}






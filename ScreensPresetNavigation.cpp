/*
 * ScreensPresetNavigation.cpp
 *
 *  Created on: Mar. 2, 2019
 *      Author: blackaddr
 */
#include <MIDI.h>
#include "FileAccess.h"
#include "Screens.h"
#include "Preset.h"
#include "MidiProc.h"
#include "MidiDefs.h"

using namespace midi;

constexpr unsigned NUM_PRESET_LINES_DRAW = 8;
static    unsigned firstPresetLine = 0;
static    bool     updatePresetLine[NUM_PRESET_LINES_DRAW];

constexpr int PRESET_TEXT_START_XPOS = MARGIN;
constexpr int PRESET_TEXT_START_YPOS = MARGIN + 2*DEFAULT_TEXT_HEIGHT;

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

void updateAllPresetDrawLines() {
    for (unsigned i=0; i < NUM_PRESET_LINES_DRAW; i++) {
        updatePresetLine[i] = true;
    }
}

void updatePresetDrawByIndex(unsigned presetIndex) {
    int presetLineIndex = presetIndex - firstPresetLine;
    if ((presetLineIndex >= 0) && (presetLineIndex < (int)NUM_PRESET_LINES_DRAW)) {
        updatePresetLine[presetLineIndex] = true;
    }
}

void drawPresetLines(ILI9341_t3 &tft, PresetArray &presetArray, unsigned activePreset, unsigned selectedPreset)
{
    int16_t x,y;
    tft.setCursor(PRESET_TEXT_START_XPOS, PRESET_TEXT_START_YPOS);

    for (unsigned i=0; i < min(NUM_PRESET_LINES_DRAW,presetArray.size()) ; i++) {

        if (updatePresetLine[i]) {
            unsigned presetToUpdate = (firstPresetLine + i) % presetArray.size(); // wraps around
            Preset& preset = presetArray[presetToUpdate];
            tft.setCursor(PRESET_TEXT_START_XPOS, PRESET_TEXT_START_YPOS + i*DEFAULT_TEXT_HEIGHT);

            uint16_t color = (selectedPreset == preset.index) ? ILI9341_DARKCYAN : ILI9341_BLACK;
            tft.getCursor(&x,&y);
            tft.fillRect(x,y,SELECTED_TEXT_WIDTH,DEFAULT_TEXT_HEIGHT, color);

            if (activePreset == preset.index) {
                tft.setTextColor(ILI9341_RED);
                tft.println(String("*") + preset.index + String("* ") + preset.name);
                tft.setTextColor(ILI9341_WHITE);
            } else {
               tft.println(String(" ") + preset.index + String(" ") + preset.name);
            }

            updatePresetLine[i] = false;
        } // end updatePresetLine
    }
}

Screens DrawPresetNavigation(ILI9341_t3 &tft, Controls &controls, PresetArray &presetArray, midi::MidiInterface<HardwareSerial> &midiPort,
        unsigned &activePreset, unsigned &selectedPreset)
{
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
    const unsigned UTILS_BUTTON_X_POS    = BACK_BUTTON_X_POS;
    const unsigned UTILS_BUTTON_Y_POS    = EXTRA_BUTTON_Y_POS -ICON_SIZE - ICON_SPACING;
    const unsigned SETLIST_BUTTON_X_POS = BACK_BUTTON_X_POS;
    const unsigned SETLIST_BUTTON_Y_POS = UTILS_BUTTON_Y_POS -ICON_SIZE - ICON_SPACING;


    const TouchArea SAVE_BUTTON_AREA(SAVE_BUTTON_X_POS, SAVE_BUTTON_X_POS+ICON_SIZE, SAVE_BUTTON_Y_POS, SAVE_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea ADD_BUTTON_AREA(ADD_BUTTON_X_POS, ADD_BUTTON_X_POS+ICON_SIZE, ADD_BUTTON_Y_POS, ADD_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea REMOVE_BUTTON_AREA(REMOVE_BUTTON_X_POS, REMOVE_BUTTON_X_POS+ICON_SIZE, REMOVE_BUTTON_Y_POS, REMOVE_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea MOVEUP_BUTTON_AREA(MOVEUP_BUTTON_X_POS, MOVEUP_BUTTON_X_POS+ICON_SIZE, MOVEUP_BUTTON_Y_POS, MOVEUP_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea MOVEDN_BUTTON_AREA(MOVEDN_BUTTON_X_POS, MOVEDN_BUTTON_X_POS+ICON_SIZE, MOVEDN_BUTTON_Y_POS, MOVEDN_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea EXTRA_BUTTON_AREA(EXTRA_BUTTON_X_POS, EXTRA_BUTTON_X_POS+ICON_SIZE, EXTRA_BUTTON_Y_POS, EXTRA_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea UTILS_BUTTON_AREA(UTILS_BUTTON_X_POS, UTILS_BUTTON_X_POS+ICON_SIZE, UTILS_BUTTON_Y_POS, UTILS_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea SETLIST_BUTTON_AREA(SETLIST_BUTTON_X_POS, SETLIST_BUTTON_X_POS+ICON_SIZE, SETLIST_BUTTON_Y_POS, SETLIST_BUTTON_Y_POS+ICON_SIZE);

    while(true) {
        if (redrawScreen) {
            // Draw the Screen title
            clearScreen(tft);
            const char *title = "Preset Navigation\n";
            tft.setCursor(0,MARGIN);
            printCentered(tft, const_cast<char*>(title));
            tft.println("");

            // Draw the icons
            bmpDraw(tft, SAVE_ICON_PATH,   SAVE_BUTTON_X_POS,     SAVE_BUTTON_Y_POS);
            bmpDraw(tft, ADD_ICON_PATH,    ADD_BUTTON_X_POS,      ADD_BUTTON_Y_POS);
            bmpDraw(tft, REMOVE_ICON_PATH, REMOVE_BUTTON_X_POS,   REMOVE_BUTTON_Y_POS);
            bmpDraw(tft, MOVEUP_ICON_PATH, MOVEUP_BUTTON_X_POS,   MOVEUP_BUTTON_Y_POS);
            bmpDraw(tft, MOVEDN_ICON_PATH, MOVEDN_BUTTON_X_POS,   MOVEDN_BUTTON_Y_POS);
            bmpDraw(tft, EXTRA_ICON_PATH,  EXTRA_BUTTON_X_POS,    EXTRA_BUTTON_Y_POS);
            bmpDraw(tft, UTILS_ICON_PATH,  UTILS_BUTTON_X_POS,    UTILS_BUTTON_Y_POS);
            bmpDraw(tft, SETLIST_ICON_PATH, SETLIST_BUTTON_X_POS, SETLIST_BUTTON_Y_POS);


            updateAllPresetDrawLines();
            drawPresetLines(tft, presetArray, activePreset, selectedPreset);
            redrawScreen = false;
        } // end redraw screen

        drawPresetLines(tft, presetArray, activePreset, selectedPreset);

        // Check for touch activity
        if (controls.isTouched()) {
            TouchPoint touchPoint = controls.getTouchPoint();

            // Check the save button
            if (SAVE_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                if (confirmationScreen(tft, controls, "Confirm SAVE ALL?\n")) {

                    constexpr size_t MAX_INFO_SIZE = 32;
                    char infoText[MAX_INFO_SIZE];
                    for (auto it=presetArray.begin(); it != presetArray.end(); ++it) {
                        StaticJsonBuffer<1024> jsonBuffer; // stack buffer
                        JsonObject& root = jsonBuffer.createObject();
                        presetToJson(*it, root);

                        char presetFilename[] = "PRESETX.JSN";
                        constexpr unsigned PRESET_ID_INDEX = 6;
                        presetFilename[PRESET_ID_INDEX] = (*it).index + 0x30;

                        snprintf(infoText, MAX_INFO_SIZE, "Saving %s", presetFilename);
                        infoScreen(tft, infoText);
                        writePresetToFile((*it).index, root); // Write to storage
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

                    if (activePreset >= selectedPreset) { activePreset++; setActivePreset(&presetArray[activePreset]); }
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

                    if (activePreset >= selectedPreset) { activePreset--; setActivePreset(&presetArray[activePreset]); }
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

            // Check the UTILS button
            if (UTILS_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                return Screens::UTILITIES;
            }

            // Check the SETLIST button
            if (SETLIST_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                return Screens::SETLIST;
            }

            // wait for touch release
            while (controls.isTouched()) {}
        } // end controls.isTouched()

        // Check for MIDI
        MidiWord midiWord;
        while (getNextMidiWord(midiWord)) {
            if ((midiWord.type == midi::ControlChange) && (midiWord.data2 == MIDI_OFF_VALUE)) {
                switch (midiWord.data1) {
                case MIDI_CC_SPECIAL_UP     : midiAdjust = -1; break;
                case MIDI_CC_SPECIAL_DOWN   : midiAdjust =  1; break;
                case MIDI_CC_SPECIAL_SELECT : selectTriggeredMidi = true; break;
                default: break;
                }
            }
        }

        int  knobAdjust = midiAdjust;
        bool switchToggled = false;
        ControlEvent controlEvent;

        while (getNextControlEvent(controlEvent)) {
            switch(controlEvent.eventType) {
            case ControlEventType::ENCODER : knobAdjust += controlEvent.value; break;
            case ControlEventType::SWITCH  : switchToggled = true; break;
            default :
                break;
            }
        }


        if (knobAdjust != 0) {
            knobAdjust =  adjustAsUnit(knobAdjust); // clamp from -1 to +1
            // set the previous selected preset to update
            unsigned prevSelectedPreset = selectedPreset;
            // update the new selected preset;
            selectedPreset = adjustWithWrap(selectedPreset, knobAdjust, presetArray.size()-1);
            if (prevSelectedPreset != selectedPreset) {
                updatePresetDrawByIndex(prevSelectedPreset);
                updatePresetDrawByIndex(selectedPreset);
            }

            if ((selectedPreset == 0) && (firstPresetLine != 0)) {
                // whenever the first preset is zero, reset the first drawline to zero and update all lines
                firstPresetLine = 0;
                updateAllPresetDrawLines();
            } else if ( (firstPresetLine == 0) && selectedPreset == presetArray.size()-1 ) {
                // we rolled up past the first preset we have to jump to the bottom preset
                firstPresetLine = presetArray.size()-NUM_PRESET_LINES_DRAW;
                updateAllPresetDrawLines();
            } else if (selectedPreset == (firstPresetLine + NUM_PRESET_LINES_DRAW)) {
                // advanced past last to next or back to zero
                // the drawing window is going to shift down one
                firstPresetLine = (firstPresetLine + 1) % presetArray.size();
                updateAllPresetDrawLines();
            } else if ( (selectedPreset == firstPresetLine-1) ) {
                // shift the drawing window up by one
                firstPresetLine = (firstPresetLine - 1) % presetArray.size();
            }
            midiAdjust = 0;
        }

        if (switchToggled || selectTriggeredMidi) {
            Serial.println(String("Setting activePreset to ") + selectedPreset);
            if (activePreset == selectedPreset) {
              // goto to edit screen
              return Screens::PRESET_CONTROL;
            } else {
              updatePresetDrawByIndex(activePreset); // update the previously active
              activePreset = selectedPreset;
              updatePresetDrawByIndex(activePreset); // update tne new active
              setActivePreset(&presetArray[activePreset]);
              midiProgramSend(activePreset, MIDI_PROGRAM_CHANNEL);
              selectTriggeredMidi = false;
            }
        }

        yield();
    } // end while loop

}

void moveUp(PresetArray &presetArray, unsigned &activePreset, unsigned &selectedPreset, bool &redrawScreen)
{
    if (selectedPreset > 0) { // can't go above the top one

        // make sure the previous active and selected will be reprinted
        updatePresetDrawByIndex(selectedPreset);
        updatePresetDrawByIndex(activePreset);

        // swap the preset with the previous by inserting a copy of the selected
        // preset before the previous, then delete the old one.
        auto presetToInsertBefore = presetArray.begin() + selectedPreset-1;
        presetArray.insert(presetToInsertBefore, presetArray[selectedPreset]);

        auto presetToErase = presetArray.begin() + selectedPreset+1;
        presetArray.erase(presetToErase);

        if      (activePreset == selectedPreset-1) { activePreset++; setActivePreset(&presetArray[activePreset]);}
        else if (activePreset == selectedPreset)   { activePreset--; setActivePreset(&presetArray[activePreset]);}
        selectedPreset--;
        updatePresetArrayIndices(presetArray);
        midiProgramSend(activePreset, MIDI_PROGRAM_CHANNEL);

        updatePresetDrawByIndex(selectedPreset);
        updatePresetDrawByIndex(activePreset);
    }

}

void moveDown(PresetArray &presetArray, unsigned &activePreset, unsigned &selectedPreset, bool &redrawScreen)
{
    if (selectedPreset < (*presetArray.end()).index ) { // can't go below the last

        // make sure the previous active and selected will be reprinted
        updatePresetDrawByIndex(selectedPreset);
        updatePresetDrawByIndex(activePreset);

        // swap the preset with the next by inserting a copy of the selected
        // preset after the next, then delete the old one.
        auto presetToInsertBefore = presetArray.begin() + selectedPreset+2;
        presetArray.insert(presetToInsertBefore, presetArray[selectedPreset]);

        auto presetToErase = presetArray.begin() + selectedPreset;
        presetArray.erase(presetToErase);

        if      (activePreset == selectedPreset+1) { activePreset--; setActivePreset(&presetArray[activePreset]);}
        else if (activePreset == selectedPreset) { activePreset++; setActivePreset(&presetArray[activePreset]);}
        selectedPreset++;
        updatePresetArrayIndices(presetArray);
        midiProgramSend(activePreset, MIDI_PROGRAM_CHANNEL);

        updatePresetDrawByIndex(selectedPreset);
        updatePresetDrawByIndex(activePreset);
    }
}






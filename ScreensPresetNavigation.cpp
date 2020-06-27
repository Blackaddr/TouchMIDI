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
#include "ListDisplay.h"

using namespace midi;

constexpr unsigned NUM_LINES_DRAW = 8;

constexpr int PRESET_TEXT_START_XPOS = MARGIN;
constexpr int PRESET_TEXT_START_YPOS = MARGIN + 2*DEFAULT_TEXT_HEIGHT;

Screens g_currentScreen = Screens::PRESET_NAVIGATION;

static ListDisplay listDisplay(NUM_LINES_DRAW);

void moveUp  (PresetArray &presetArray, bool &redrawScreen, ListDisplay& listDisplay);
void moveDown(PresetArray &presetArray, bool &redrawScreen, ListDisplay& listDisplay);

void updatePresetArrayIndices(PresetArray &presetArray)
{
    int i= 0;
    for (auto it = presetArray.begin(); it != presetArray.end(); ++it) {
        presetArray[i].index = i;
        i++;
    }
}


void drawPresetLines(ILI9341_t3 &tft, PresetArray &presetArray)
{
    int16_t x,y;
    tft.setCursor(PRESET_TEXT_START_XPOS, PRESET_TEXT_START_YPOS);

    for (unsigned i=0; i < min(NUM_LINES_DRAW,presetArray.size()) ; i++) {

        if (listDisplay.getUpdate(i)) {
            unsigned listIndex = listDisplay.getIndex(i);
            Preset& preset = presetArray[listIndex];
            //const char* setlist = presetArray[listIndex].c_str();
            tft.setCursor(PRESET_TEXT_START_XPOS, PRESET_TEXT_START_YPOS + i*DEFAULT_TEXT_HEIGHT);

            uint16_t color = (listDisplay.getSelected() == listIndex) ? ILI9341_DARKCYAN : ILI9341_BLACK;
            tft.getCursor(&x,&y);
            tft.fillRect(x,y,SELECTED_TEXT_WIDTH,DEFAULT_TEXT_HEIGHT, color);

            if (getActivePresetIndex() == listIndex) {
                tft.setTextColor(ILI9341_RED);
                tft.println(String("*") + preset.index + String("* ") + preset.name);
                tft.setTextColor(ILI9341_WHITE);
            } else {
                tft.println(String(" ") + preset.index + String(" ") + preset.name);
            }
        }

//        if (updatePresetLine[i]) {
//            unsigned presetToUpdate = (firstPresetLine + i) % presetArray.size(); // wraps around
//            Preset& preset = presetArray[presetToUpdate];
//            tft.setCursor(PRESET_TEXT_START_XPOS, PRESET_TEXT_START_YPOS + i*DEFAULT_TEXT_HEIGHT);
//
//            uint16_t color = (selectedPreset == preset.index) ? ILI9341_DARKCYAN : ILI9341_BLACK;
//            tft.getCursor(&x,&y);
//            tft.fillRect(x,y,SELECTED_TEXT_WIDTH,DEFAULT_TEXT_HEIGHT, color);
//
//            if (activePreset == preset.index) {
//                tft.setTextColor(ILI9341_RED);
//                tft.println(String("*") + preset.index + String("* ") + preset.name);
//                tft.setTextColor(ILI9341_WHITE);
//            } else {
//               tft.println(String(" ") + preset.index + String(" ") + preset.name);
//            }
//
//            updatePresetLine[i] = false;
//        } // end updatePresetLine
    }
}

Screens DrawPresetNavigation(ILI9341_t3 &tft, Controls &controls, PresetArray &presetArray, midi::MidiInterface<HardwareSerial> &midiPort)
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

    listDisplay.reset();
    listDisplay.setSize(presetArray.size());

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


            //updateAllPresetDrawLines();
            listDisplay.setUpdateAll();
            //drawPresetLines(tft, presetArray, activePreset, selectedPreset);
            drawPresetLines(tft, presetArray);
            redrawScreen = false;
        } // end redraw screen

        drawPresetLines(tft, presetArray);

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
                        writePresetToFile((*it).index, getActiveSetlist(), root); // Write to storage
                    }
                }
                redrawScreen = true;
            }

            // Check the add button
            if (ADD_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release

                if (confirmationScreen(tft, controls, "Confirm ADD?\n")) {
                    // insert a new preset before index selectedPreset
                    //auto presetToInsertBefore = presetArray.begin() + selectedPreset;
                    auto presetToInsertBefore = presetArray.begin() + listDisplay.getSelected();
                    presetArray.insert(presetToInsertBefore, Preset());
                    updatePresetArrayIndices(presetArray);

                    listDisplay.setSize(presetArray.size());

                    //if (activePreset >= selectedPreset) { activePreset++; setActivePreset(&presetArray[activePreset]); }
                    if (getActivePresetIndex() >= listDisplay.getSelected()) {
                        //activePreset++;
                        setActivePreset(&presetArray[getActivePresetIndex()+1]);
                    }
                }
                redrawScreen = true;
            }

            // Check the remove button
            if (REMOVE_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                if (confirmationScreen(tft, controls, "Confirm REMOVE?\n")) {
                    //auto presetToErase = presetArray.begin() + selectedPreset;
                    auto presetToErase = presetArray.begin() + listDisplay.getSelected();
                    presetArray.erase(presetToErase);
                    updatePresetArrayIndices(presetArray);
                    listDisplay.setSize(presetArray.size());

                    //if (activePreset >= selectedPreset) { activePreset--; setActivePreset(&presetArray[activePreset]); }
                    if (getActivePresetIndex() >= listDisplay.getSelected()) {
                        //activePreset--;
                        setActivePreset(&presetArray[getActivePresetIndex()+1]);
                    }
                }
                redrawScreen = true;
            }

            // Check the MOVEUP button
            if (MOVEUP_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                moveUp(presetArray, redrawScreen, listDisplay);
            }

            // Check the MOVEUDN button
            if (MOVEDN_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                moveDown(presetArray, redrawScreen, listDisplay);
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

        // Check for encoder activity
        if (knobAdjust != 0) {
            if (knobAdjust > 0) {
                listDisplay.next();
            } else if (knobAdjust < 0) {
                listDisplay.previous();
            }
            midiAdjust = 0;
        }

        if (switchToggled || selectTriggeredMidi) {

            Serial.println(String("Setting activePreset to ") + listDisplay.getSelected());
            listDisplay.setUpdate(getActivePresetIndex());


            if (getActivePresetIndex() == listDisplay.getSelected()) {
                // goto to edit screen
                return Screens::PRESET_CONTROL;
            } else {
                unsigned activePresetIndex = listDisplay.getSelected();
                listDisplay.setUpdate(activePresetIndex);

                setActivePreset(&presetArray[activePresetIndex]);
                midiProgramSend(activePresetIndex, MIDI_PROGRAM_CHANNEL);
                selectTriggeredMidi = false;
            }
        }

        yield();
    } // end while loop

}

void moveUp(PresetArray &presetArray, bool &redrawScreen, ListDisplay& listDisplay)
{
    unsigned activeIndex = getActivePresetIndex();

    if (listDisplay.getSelected() > 0) { // can't go above the top one

        // make sure the previous active and selected will be reprinted
        listDisplay.setUpdate(activeIndex);


        // swap the preset with the previous by inserting a copy of the selected
        // preset before the previous, then delete the old one.
        auto presetToInsertBefore = presetArray.begin() + listDisplay.getSelected()-1;
        presetArray.insert(presetToInsertBefore, presetArray[listDisplay.getSelected()]);

        auto presetToErase = presetArray.begin() + listDisplay.getSelected()+1;
        presetArray.erase(presetToErase);

        if      (activeIndex == listDisplay.getSelected()-1) { activeIndex++; setActivePreset(&presetArray[activeIndex]);}
        else if (activeIndex == listDisplay.getSelected())   { activeIndex--; setActivePreset(&presetArray[activeIndex]);}
        listDisplay.previous();

        updatePresetArrayIndices(presetArray);
        midiProgramSend(activeIndex, MIDI_PROGRAM_CHANNEL);

        listDisplay.setUpdate(listDisplay.getSelected());
        listDisplay.setUpdate(activeIndex);
    }

}

void moveDown(PresetArray &presetArray, bool &redrawScreen, ListDisplay& listDisplay)
{
    unsigned selectedIndex = listDisplay.getSelected();
    unsigned activeIndex = getActivePresetIndex();

    if (selectedIndex < (*presetArray.end()).index ) { // can't go below the last

        listDisplay.setUpdate(activeIndex);

        auto presetToInsertBefore = presetArray.begin() + listDisplay.getSelected()+2;
        presetArray.insert(presetToInsertBefore, presetArray[listDisplay.getSelected()]);

        auto presetToErase = presetArray.begin() + listDisplay.getSelected();
        presetArray.erase(presetToErase);

        if      (activeIndex == listDisplay.getSelected()+1) { activeIndex--; setActivePreset(&presetArray[activeIndex]);}
        else if (activeIndex == listDisplay.getSelected())   { activeIndex++; setActivePreset(&presetArray[activeIndex]);}
        listDisplay.next();

        updatePresetArrayIndices(presetArray);
        midiProgramSend(activeIndex, MIDI_PROGRAM_CHANNEL);

        listDisplay.setUpdate(listDisplay.getSelected());
        listDisplay.setUpdate(activeIndex);
    }
}






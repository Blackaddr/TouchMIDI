/*
 * ScreensPlaylist.c
 *
 *  Created on: May 9, 2020
 *      Author: blackaddr
 */

#include "VectorSupport.h"
#include "filePaths.h"
#include "FileAccess.h"
#include "ListDisplay.h"
#include "Screens.h"

// This screen provides a way for editing a preset.
const TouchArea BACK_BUTTON_AREA(BACK_BUTTON_X_POS, BACK_BUTTON_X_POS+ICON_SIZE, 0, ICON_SIZE);

constexpr unsigned NUM_LINES_DRAW = 8;
unsigned activeIndex = 0;
bool     updateLine[NUM_LINES_DRAW];

constexpr int TEXT_START_XPOS = MARGIN;
constexpr int TEXT_START_YPOS = MARGIN + 2*DEFAULT_TEXT_HEIGHT;

ListDisplay listDisplay(NUM_LINES_DRAW);

void drawLines(ILI9341_t3 &tft, SetlistArray &setlistArray)
{
    int16_t x,y;
    tft.setCursor(TEXT_START_XPOS, TEXT_START_YPOS);

    for (unsigned i=0; i < min(NUM_LINES_DRAW, setlistArray.size()) ; i++) {

        if (listDisplay.getUpdate(i)) {
            unsigned listIndex = listDisplay.getIndex(i);
            Serial.printf("drawing line %d, index %d\n", i, listIndex);
            const char* setlist = setlistArray[listIndex].c_str();
            tft.setCursor(TEXT_START_XPOS, TEXT_START_YPOS + i*DEFAULT_TEXT_HEIGHT);

            uint16_t color = (listDisplay.getSelected() == listIndex) ? ILI9341_DARKCYAN : ILI9341_BLACK;
            tft.getCursor(&x,&y);
            tft.fillRect(x,y,SELECTED_TEXT_WIDTH,DEFAULT_TEXT_HEIGHT, color);

            if (activeIndex == listIndex) {
                tft.setTextColor(ILI9341_RED);
                tft.println(String("*") + setlist + String("*"));
                tft.setTextColor(ILI9341_WHITE);
            } else {
               tft.println(setlist);
            }
        }
    }
}

Screens DrawSetlist(ILI9341_t3 &tft, Controls &controls, PresetArray& presetArray)
{
    bool redrawScreen = true;
    bool updateScreen = false;

    SetlistArray& setlistArray = updateSetlistList(); // gets a list of presets
    listDisplay.setSize(setlistArray.size());
    //Serial.printf("DrawSetlist(): size is %d\n", setlistArray.size());

    const unsigned BOTTOM_ICON_ROW_Y_POS = tft.height() - ICON_SIZE;
    const unsigned ADD_BUTTON_X_POS      = BACK_BUTTON_X_POS;
    const unsigned ADD_BUTTON_Y_POS      = BOTTOM_ICON_ROW_Y_POS;
    const unsigned REMOVE_BUTTON_X_POS   = ADD_BUTTON_X_POS - ICON_SIZE - ICON_SPACING;
    const unsigned REMOVE_BUTTON_Y_POS   = BOTTOM_ICON_ROW_Y_POS;

    const TouchArea ADD_BUTTON_AREA   (ADD_BUTTON_X_POS, ADD_BUTTON_X_POS+ICON_SIZE, ADD_BUTTON_Y_POS, ADD_BUTTON_Y_POS+ICON_SIZE);
    const TouchArea REMOVE_BUTTON_AREA(REMOVE_BUTTON_X_POS, REMOVE_BUTTON_X_POS+ICON_SIZE, REMOVE_BUTTON_Y_POS, REMOVE_BUTTON_Y_POS+ICON_SIZE);

    // Calculate button locations
    while (true) {
        // Draw the Preset Edit Screen
        if (redrawScreen) {
            redrawScreen = false;

            clearScreen(tft);

            bmpDraw(tft, ADD_ICON_PATH,    ADD_BUTTON_X_POS,      ADD_BUTTON_Y_POS);
            bmpDraw(tft, REMOVE_ICON_PATH, REMOVE_BUTTON_X_POS,   REMOVE_BUTTON_Y_POS);

            // print the title centered
            printCenteredJustified(tft, "PLAYLIST CONTROL", (tft.width()-ICON_SIZE)/2, MARGIN);

            // Draw the icons
            bmpDraw(tft, BACK_ICON_PATH, BACK_BUTTON_X_POS, BACK_BUTTON_Y_POS); // shifting more than 255 pixels seems to wrap the screen

            // Draw the menu entries
            tft.setCursor(MARGIN, MARGIN + 2*DEFAULT_TEXT_HEIGHT);

            listDisplay.setUpdateAll();
            drawLines(tft, setlistArray);
        }

        if (updateScreen) {
            updateScreen = false;
            // Draw the menu entries
            tft.setCursor(MARGIN, MARGIN + 2*DEFAULT_TEXT_HEIGHT);
            drawLines(tft, setlistArray);
        }

        // Check for touch activity
        if (controls.isTouched()) {
            TouchPoint touchPoint = controls.getTouchPoint();

            // Check the back button
            if (BACK_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                return Screens::PRESET_NAVIGATION;
            }

            // Check the add button
            if (ADD_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release

                if (confirmationScreen(tft, controls, "Confirm ADD?\n")) {
                    String setlistName;
                    StringEdit(tft, setlistName, controls);
                    createNewSetlist(setlistName.c_str());
                    updateSetlistList();
                    listDisplay.setSize(setlistArray.size());
                    redrawScreen = true;
                }
                redrawScreen = true;
            }

            // Check the remove button
            if (REMOVE_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                if (confirmationScreen(tft, controls, "Confirm REMOVE?\n")) {

                }
                redrawScreen = true;
            }

            // wait for touch release
            while (controls.isTouched()) {}
        }

        int  knobAdjust = 0;
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
        if (knobAdjust > 0) {
            listDisplay.next();
            updateScreen = true;
        } else if (knobAdjust < 0) {
            listDisplay.previous();
            updateScreen = true;
        }

        // check for switch activity
        if (switchToggled) {
            listDisplay.setUpdate(activeIndex);
            activeIndex = listDisplay.getSelected();
            listDisplay.setUpdate(activeIndex);
            const char* setlist = setlistArray[activeIndex].c_str();
            setActiveSetlist(setlist);
            readPresetFromFile(&presetArray, setlist);
            updateScreen = true;
        }

    } // end while(true)
} // end drawPlaylist()

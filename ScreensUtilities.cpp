/*
 * ScreensUtilities.cpp
 *
 *  Created on: Apr. 17, 2020
 *      Author: blackaddr
 */
#include "VectorSupport.h"
#include "FileAccess.h"
#include "Screens.h"

// This screen provides a way for editing a preset.
const TouchArea BACK_BUTTON_AREA(BACK_BUTTON_X_POS, BACK_BUTTON_X_POS+ICON_SIZE, 0, ICON_SIZE);

enum class MenuItem {
    FLASH_ERASE,
    COPY_FLASH_TO_SD,
    COPY_SD_TO_FLASH,
    REBOOT
};

Screens DrawUtilities(ILI9341_t3 &tft, Controls &controls, PresetArray& presetArray)
{
    bool redrawScreen = true;
    bool updateScreen = false;

    // Built the menu entries
    struct MenuEntry {
        MenuEntry(MenuItem menuItem, String str, int16_t yPos) : menuItem(menuItem), str(str), yPos(yPos) {}
        MenuItem menuItem;
        String str;
        int16_t yPos;
    };
    std::vector<MenuEntry> menuEntries;
    menuEntries.emplace_back(MenuEntry(MenuItem::FLASH_ERASE,      {"FLASH Erase"},      {MARGIN + 2*DEFAULT_TEXT_HEIGHT}));
    menuEntries.emplace_back(MenuEntry(MenuItem::COPY_FLASH_TO_SD, {"Copy FLASH -> SD"}, {MARGIN + 3*DEFAULT_TEXT_HEIGHT}));
    menuEntries.emplace_back(MenuEntry(MenuItem::COPY_SD_TO_FLASH, {"Copy SD -> FLASH"}, {MARGIN + 4*DEFAULT_TEXT_HEIGHT}));
    menuEntries.emplace_back(MenuEntry(MenuItem::REBOOT,           {"Reboot"},           {MARGIN + 5*DEFAULT_TEXT_HEIGHT}));
    auto selectedControl = menuEntries.begin();
    auto previousSelectedControl = selectedControl;

    // Calculate button locations
    //const unsigned BOTTOM_ICON_ROW_Y_POS = tft.height() - ICON_SIZE;

    while (true) {

        // Draw the Preset Edit Screen
        if (redrawScreen) {
            redrawScreen = false;

            clearScreen(tft);

            // print the title centered
            printCenteredJustified(tft, "UTILITIES", tft.width()/2, MARGIN);

            // Draw the icons
            bmpDraw(tft, "back48.bmp", BACK_BUTTON_X_POS, BACK_BUTTON_Y_POS); // shifting more than 255 pixels seems to wrap the screen

            // Draw the menu entries
            tft.setCursor(MARGIN, MARGIN + 2*DEFAULT_TEXT_HEIGHT);
            selectedControl == menuEntries.begin();

            for (auto it = menuEntries.begin(); it != menuEntries.end(); ++it) {
                tft.setCursor(MARGIN, (*it).yPos);
                uint16_t color = (it == selectedControl) ? ILI9341_DARKCYAN : ILI9341_BLACK;
                tft.fillRect(MARGIN,(*it).yPos,SELECTED_TEXT_WIDTH,DEFAULT_TEXT_HEIGHT, color);
                tft.printf("%s\n", (*it).str.c_str());
            }
        }

        if (updateScreen) {
            updateScreen = false;
            // Draw the menu entries
            tft.setCursor(MARGIN, MARGIN + 2*DEFAULT_TEXT_HEIGHT);
            for (auto it = menuEntries.begin(); it != menuEntries.end(); ++it) {

                // Only redraw when necessary
                if ((it == selectedControl) || (it ==  previousSelectedControl)) {
                    tft.setCursor(MARGIN, (*it).yPos);
                    uint16_t color = (it == selectedControl) ? ILI9341_DARKCYAN : ILI9341_BLACK;
                    tft.fillRect(MARGIN,(*it).yPos,SELECTED_TEXT_WIDTH,DEFAULT_TEXT_HEIGHT, color);
                    tft.printf("%s\n", (*it).str.c_str());
                }
            }
        }

        // Check for touch activity
        if (controls.isTouched()) {
            TouchPoint touchPoint = controls.getTouchPoint();

            // Check the back button
            if (BACK_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                return Screens::PRESET_NAVIGATION;
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
            previousSelectedControl = selectedControl;
            if (selectedControl != menuEntries.end()-1) {
                ++selectedControl;
                updateScreen = true;
            }
        } else if (knobAdjust < 0) {
            previousSelectedControl = selectedControl;
            if (selectedControl != menuEntries.begin()) {
                --selectedControl;
                updateScreen = true;
            }
        }

        // check for switch activity
        if (switchToggled) {
            switch((*selectedControl).menuItem) {
                case MenuItem::FLASH_ERASE :
                    if (confirmationScreen(tft, controls, "ERASE Flash?\n")) {
                        infoScreen(tft, "Please wait...");
                        SerialFlash.eraseAll();
                    }
                    redrawScreen = true;
                    break;
                case MenuItem::COPY_FLASH_TO_SD :
                    if (confirmationScreen(tft, controls, "Copy FLASH->SD?\n")) {
                        infoScreen(tft, "Please wait...");
                        copyFlashToSd();
                    }
                    redrawScreen = true;
                    break;
                case MenuItem::COPY_SD_TO_FLASH :
                    if (confirmationScreen(tft, controls, "Copy SD->Flash?\n")) {
                        infoScreen(tft, "Please wait...");
                        copySdToFlash();
                    }
                    redrawScreen = true;
                    break;
                case MenuItem::REBOOT :
                    if (confirmationScreen(tft, controls, "Reboot?\n")) {
                        infoScreen(tft, "Please wait...");
                        rebootTeensy();
                    }
                    break;
                default :
                    break;
            }
        }

    } // end while(true)
} // end drawUtilities()







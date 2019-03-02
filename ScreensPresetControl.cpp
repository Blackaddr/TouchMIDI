/*
 * ScreensPresetControl.cpp
 *
 *  Created on: Mar. 2, 2019
 *      Author: blackaddr
 */
#include "Screens.h"

Screens DrawPresetControl(ILI9341_t3 &tft, Controls &controls, Preset &preset)
{
    unsigned activeControl = 0;

    // Create an array to store the knob and switch center locations. This will
    // be used to map touch points to knobs, etc.
    Coordinate controlLocations[MAX_NUM_CONTROLS];

    bool redrawScreen = true;
    bool redoLayout = true;
    bool redrawControls = true;

    unsigned numKnobs = 0;
    unsigned numSwitches = 0;

    while(true) {

        // Check if we need to calculate the icon layout
        if (redoLayout) {
            // Get the number of knobs vs switches
            numKnobs = 0;
            numSwitches = 0;
            for (auto it = preset.controls.begin(); it != preset.controls.end(); it++) {
                if ((*it).type == ControlType::ROTARY_KNOB) {
                    numKnobs++;
                } else {
                    numSwitches++;
                }
            }

            // First configure the knob locations
            unsigned knobOffset = tft.width() / (numKnobs+1);
            int16_t xPos;
            int16_t yPos;
            xPos = knobOffset; // overwrite
            yPos = 100;

            unsigned idx = 0;
            for (auto it = preset.controls.begin(); it != preset.controls.end(); it++) {
              if ((*it).type == ControlType::ROTARY_KNOB) {
                  controlLocations[idx] = Coordinate(xPos, yPos, &(*it));
                  xPos += knobOffset;
                  idx++;
              }
            }

            // Next configure the switch locations
            unsigned switchOffset = tft.width() / (numSwitches+1);
            xPos = switchOffset;
            yPos = tft.height()/2 + 75;
            for (auto it = preset.controls.begin(); it != preset.controls.end(); it++) {
              if ((*it).type == ControlType::SWITCH_MOMENTARY || (*it).type == ControlType::SWITCH_LATCHING) {

                  controlLocations[idx] = Coordinate(xPos, yPos, &(*it));
                  xPos += switchOffset;
                  idx++;
              }
            }
            redoLayout = false;
        } // redoLayout

        if (redrawScreen) {
            // Draw the Preset Edit Screen
            clearScreen(tft);
            tft.setCursor(0,MARGIN);

            // print the preset number in the top left
            tft.print(preset.index);
            char *presetName = const_cast<char*>(preset.name.c_str());
            tft.setCursor(tft.width()/10,MARGIN);
            tft.print(const_cast<char*>(presetName));

            // Draw the icons
            bmpDraw(tft, "back48.bmp", BACK_BUTTON_X_POS,0); // shifting more than 255 pixels seems to wrap the screen
            bmpDraw(tft, "seting48.bmp", SETTINGS_BUTTON_X_POS, 0);
            redrawScreen = false;
        }


        if (redrawControls) {
            // Draw a light filled box behind the active control and black behind the others

            // Draw the controls
            for (unsigned i=0; i<MAX_NUM_CONTROLS; i++) {
                if (controlLocations[i].control) {
                    MidiControl &controlPtr = *controlLocations[i].control;
                    // valid pointer to control
                    uint16_t color = (activeControl == i) ? ILI9341_YELLOW : ILI9341_BLACK;
                    drawActiveControl(tft, controlLocations[i].x, controlLocations[i].y, color);

                    if (controlPtr.type == ControlType::ROTARY_KNOB) {
                        drawKnob(tft, controlPtr, controlLocations[i].x, controlLocations[i].y);
                    } else {
                        drawSwitch(tft, controlPtr, controlLocations[i].x, controlLocations[i].y);
                    }
                }
            }
            redrawControls = false;
        }

        // run a loop waiting for a control input from touch, rotary or switch
        while(true) {
            MidiControl &control = *controlLocations[activeControl].control;

            // Check for touch activity
            if (controls.isTouched()) {
                TS_Point touchPoint = controls.getTouchPoint();
                Coordinate touchCoordinate(touchPoint.x, touchPoint.y, nullptr);

                // wait until the screen is no longer touched before taking action
                while (controls.isTouched()) {}

                // Check the back button
                if (touchPoint.x > static_cast<int16_t>(BACK_BUTTON_X_POS) && touchPoint.y < static_cast<int16_t>(ICON_SIZE) ) {
                    return Screens::PRESET_NAVIGATION;
                }

                // Check for the settings button
                if (touchPoint.x > static_cast<int16_t>(SETTINGS_BUTTON_X_POS) && touchPoint.y < static_cast<int16_t>(ICON_SIZE) ) {
                    DrawPresetConfig(tft, controls, preset);
                    // The preset config screen cleared the settings icon so redraw it
                    //bmpDraw(tft, "seting48.bmp", SETTINGS_BUTTON_X_POS, 0);
                    redoLayout = true;
                    redrawScreen = true;
                    redrawControls = true;
                    break; // break out of control-detect loop
                }

                // Check for controls
                for (auto i=0; i<MAX_NUM_CONTROLS; i++) {
                    if ( controlLocations[i].checkCoordinateRange(touchCoordinate, TOUCH_CONTROL_HALFSIZE) ) {
                        activeControl = i;
                        redrawControls = true;
                        break; // break out of the for loop
                    }
                }

                // break out of the control-detect loop if we need to redraw controls
                if (redrawControls) { break; }
            }

            // Check for rotary activity
            int adjust = controls.getRotaryAdjustUnit(0);
            if (adjust != 0) {
                // primary encoder
                Serial.println(String("Adjust by ") + adjust);

                if (control.type == ControlType::ROTARY_KNOB) {
                    control.value = adjustWithSaturation(control.value, adjust, 0, 127);
                    control.updated = true;
                    redrawControls = true;
                    break;
                }
            }

            // Check for pushbutton control
            if (controls.isSwitchToggled(0)) {
                Serial.println("Toggled!");
                if (control.type == ControlType::SWITCH_MOMENTARY) {
                    control.value = static_cast<unsigned>(toggleValue(control.value, 127));
                    control.updated = true;
                    redrawControls = true;
                    break;
                }
            }

            delay(100); // this is needed for control sampling to work
        } // end while loop to process control inputs

    } // end outer while loop
}




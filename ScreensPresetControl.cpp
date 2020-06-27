/*
 * ScreensPresetControl.cpp
 *
 *  Created on: Mar. 2, 2019
 *      Author: blackaddr
 *  This screen is for normal operation. This screen shows all the controls for a preset
 *  and allows a control to be adjusted using the buttons/knobs on the hand controller.
 *  If the input control matches an incoming MIDI CC, then that input control will update
 *  the parameter.
 */
#include <memory>
#include <queue>
#include <MIDI.h>
#include "filePaths.h"
#include "Screens.h"
#include "MidiProc.h"

using namespace midi;

unsigned maxScanline = 0;

// This screen presents a given presets controls for real-time use.
Screens DrawPresetControl(ILI9341_t3 &tft, Controls &controls, Preset &preset, MidiInterface<HardwareSerial> &midiPort)
{
    unsigned activeControl = 0;

    // Create an array to store the knob and switch center locations. This will
    // be used to map touch points to knobs, etc.
    Coordinate controlLocations[MAX_NUM_CONTROLS];

    bool redrawScreen = true;
    bool redoLayout = true;
    bool redrawControls = true;
    bool redrawActiveControl = true;

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

        //unsigned scanline = tft.readcommand8(0x45, 1);
        //if (scanline > maxScanline) { maxScanline = scanline; Serial.println(String("Scanline: ") + maxScanline); }

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
            bmpDraw(tft, BACK_ICON_PATH,     BACK_BUTTON_X_POS,0); // shifting more than 255 pixels seems to wrap the screen
            bmpDraw(tft, SETTINGS_ICON_PATH, SETTINGS_BUTTON_X_POS, 0);
            redrawScreen = false;

            // flag all controls to update display
            for (unsigned i=0; i<MAX_NUM_CONTROLS; i++) {
                if (controlLocations[i].control) { // valid control pointer
                    MidiControl &controlPtr = *controlLocations[i].control;
                    controlPtr.updated = true;
                }
            }
        }

        // Draw the controls
        int valueXPos = tft.width()  - MARGIN;
        int valueYPos = tft.height() - 2*MARGIN;

        for (unsigned i=0; i<MAX_NUM_CONTROLS; i++) {

            if (controlLocations[i].control) { // valid control pointer
                MidiControl &controlPtr = *controlLocations[i].control;
                if (controlPtr.updated || (redrawActiveControl && (activeControl == i))) { // check if control is updated

                    // set the background based on whether active or now
                    uint16_t color = (activeControl == i) ? ILI9341_YELLOW : ILI9341_BLACK;
                    drawActiveControl(tft, controlLocations[i].x, controlLocations[i].y, color); // draw the active select background

                    if (controlPtr.type == ControlType::ROTARY_KNOB) {
                        drawKnob(tft, controlPtr, controlLocations[i].x, controlLocations[i].y);

                        if (i == activeControl) {
                            // Draw the knob value in the lower right corner
                            char valueText[4];
                            uint2dec3(midiToPercent(controlPtr.value), valueText, 2); // 2 is justify right
                            valueText[3] = '\0';
                            tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK); // force the background to be redrawn as black
                            printRightJustified(tft, valueText, valueXPos, valueYPos);
                        }

                    } else {
                        drawSwitch(tft, controlPtr, controlLocations[i].x, controlLocations[i].y);
                        // Clear the value text box when a switch is the active control
                        if (i == activeControl) {
                            tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); // force the background to be redrawn as black
                            printRightJustified(tft, "   \n", valueXPos, valueYPos);
                        }
                    }
                    controlPtr.updated = false;
                } // end if control updated
            } // end if valid control
        } // end for loop over controls
        redrawControls = false;
        redrawActiveControl = false;

        // run a loop waiting for a control input from touch, rotary or switch, or external MIDI input.
        while(true) {
            MidiControl &control = *controlLocations[activeControl].control;

            // Check for MIDI activity
            MidiWord midiWord;
            if (getNextMidiWord(midiWord)) {

                MidiType type    = midiWord.type;
                DataByte ccId    = midiWord.data1;

                if (type == midi::ControlChange) {
                    for (auto it = preset.controls.begin(); it != preset.controls.end(); ++it) {

                        /// check if the CC matches this control and it's been updated
                        if ( ccId == (*it).cc && (*it).updated) {
                            redrawControls = true;
                        }
                    }// end preset control map FOR loop
                } // end if CC
            } // end if midi available

            // Check for touch activity
            if (controls.isTouched()) {
                TouchPoint touchPoint = controls.getTouchPoint();
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
                    redoLayout = true;
                    redrawScreen = true;
                    redrawControls = true;
                    break; // break out of control-detect loop
                }

                // Check for control touch points
                for (auto i=0; i<MAX_NUM_CONTROLS; i++) {
                    if ( controlLocations[i].checkCoordinateRange(touchPoint, TOUCH_CONTROL_HALFSIZE) ) {
                        // Change the active control, both the old and the new must be updated.
                        control.updated = true; // this will cause the currently active to be updated on the next draw
                        activeControl = i;
                        redrawActiveControl = true;
                        redrawControls = true;
                        break; // break out of the for loop
                    }
                }

                // break out of the control-detect loop if we need to redraw controls
                if (redrawControls || redrawActiveControl) { break; }
            }

            // Check for physical input controls
            ControlEvent controlEvent;
            while (getNextControlEvent(controlEvent)) {
                switch(controlEvent.eventType) {
                case ControlEventType::ENCODER :
                {
                    int knobAdjust = controlEvent.value;
                    // primary encoder
                    if (control.type == ControlType::ROTARY_KNOB) {
                        control.value = adjustWithSaturation(control.value, knobAdjust, 0, MIDI_VALUE_MAX);
                        control.updated = true;
                        redrawActiveControl = true;

                        // Send the MIDI message
                        MidiWord midiWord;
                        midiWord.type = midi::MidiType::ControlChange;
                        midiWord.data1 = control.cc;
                        midiWord.data2 = control.value;
                        midiWord.channel = MIDI_CC_CHANNEL;
                        midiSendWord(midiWord);
                        break;
                    }
                    break;
                }

                case ControlEventType::SWITCH :
                {
                    // If it's a toggled switch update the stored value using toggling
                    if (control.type == ControlType::SWITCH_LATCHING) {
                        control.value = static_cast<unsigned>(toggleValue(control.value, MIDI_ON_VALUE, MIDI_OFF_VALUE));
                        control.updated = true;
                    }

                    // Redraw the active control if it is a switch type
                    if ( (control.type == ControlType::SWITCH_LATCHING) ||
                         (control.type == ControlType::SWITCH_MOMENTARY) ) {
                        redrawActiveControl = true;

                        MidiWord midiWord;
                        midiWord.type = midi::MidiType::ControlChange;
                        midiWord.data1 = control.cc;
                        midiWord.data2 = control.value;
                        midiWord.channel = MIDI_CC_CHANNEL;
                        midiSendWord(midiWord);
                    }
                    break;
                }

                default :
                    break;
                }

            }

            if (redrawControls || redrawActiveControl) { break; }

            yield();
        } // end while loop to process control inputs

    } // end outer while loop
}




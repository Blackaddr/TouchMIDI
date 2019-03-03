/*
 * ScreensMidiControlConfig.cpp
 *
 *  Created on: Mar. 2, 2019
 *      Author: blackaddr
 */
#include "Screens.h"

// This screen provides a way for editing a preset.
const TouchArea BACK_BUTTON_AREA(BACK_BUTTON_X_POS, BACK_BUTTON_X_POS+ICON_SIZE, 0, ICON_SIZE);

constexpr int SELECTED_TEXT_WIDTH = 200;

constexpr int NUM_CONTROL_FIELDS = 4;
const char *CONTROL_FIELD_NAMES[NUM_CONTROL_FIELDS] = {"Short Name: ", "CC: ", "Type: ", "Value: "};

void DrawMidiControlConfig(ILI9341_t3 &tft, Controls &controls, MidiControl &midiControl)
{

    bool redrawScreen = true;
    int16_t nameEditButtonPosition;
    TouchArea editNameArea;
    unsigned selectedField = 0;
    int activeField = -1; // -1 means no field is active

    while (true) {

        // Draw the Preset Edit Screen
        if (redrawScreen) {

            clearScreen(tft);
            tft.setCursor(0,MARGIN);

            // 1) print the midi control name
            tft.setCursor(tft.width()/10,MARGIN);
            tft.print(midiControl.name.c_str());

            // 2) Draw the icons

            // BACK
            bmpDraw(tft, "back48.bmp", BACK_BUTTON_X_POS,0); // shifting more than 255 pixels seems to wrap the screen

            // NAME EDIT button
            nameEditButtonPosition = tft.getCursorX() + MARGIN;
            bmpDraw(tft, "edit48.bmp", nameEditButtonPosition, 0);
            redrawScreen = false;
            editNameArea.setArea(nameEditButtonPosition, nameEditButtonPosition+ICON_SIZE, 0, ICON_SIZE);

            // Print the control info
            tft.setCursor(0,ICON_SIZE); // start row under icons

            // Print the selected box background
            uint16_t boxColor = (activeField >= 0) ? ILI9341_RED : ILI9341_DARKCYAN;
            tft.fillRect(0, ICON_SIZE + (selectedField+1)*DEFAULT_TEXT_HEIGHT,SELECTED_TEXT_WIDTH,DEFAULT_TEXT_HEIGHT, boxColor);
            tft.printf("\nShort Name: %s\n", midiControl.shortName.c_str());
            tft.printf("CC: %d\n", midiControl.cc);
            tft.printf("Type: %s\n", Controls::ControlTypeToString(midiControl.type));
            tft.printf("Value: %d\n", midiControl.value);
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
                StringEdit(tft, midiControl.name, *controls.touch, controls.m_encoders[CONTROL_ENCODER], controls.m_switches[CONTROL_SWITCH]);
                redrawScreen = true;
            }

            // wait for touch release
            while (controls.isTouched()) {}
        }

        // Check for encoder activity
        if (activeField > 0) {
            // A field is currently being edited

            switch(activeField) {
            case 1 :
            // CC adjust
            {
                int knobAdjust = controls.getRotaryAdjust(CONTROL_ENCODER);
                unsigned newValue = adjustWithSaturation(midiControl.cc, knobAdjust, MIDI_CC_MIN, MIDI_CC_MAX);
                if (newValue != midiControl.cc) { midiControl.cc = newValue; redrawScreen = true; }
                break;
            }
            case 2:
            {
                int knobAdjust = controls.getRotaryAdjustUnit(CONTROL_ENCODER);
                ControlType newValue = static_cast<ControlType>(adjustWithWrap(
                        static_cast<int>(midiControl.type), knobAdjust, static_cast<int>(ControlType::NUM_TYPES))
                        );
                if (newValue != midiControl.type) { midiControl.type = newValue; redrawScreen = true; }
            }
                break;
            case 3:
                // Value adjust
                {
                    unsigned newValue;
                    if ((midiControl.type == ControlType::SWITCH_LATCHING) || (midiControl.type == ControlType::SWITCH_MOMENTARY)) {
                        // switch behavior, only MIDI ON/OFF is permitted
                        int knobAdjust = controls.getRotaryAdjustUnit(CONTROL_ENCODER);
                        if (knobAdjust != 0) {
                            if (midiControl.value == MIDI_ON_VALUE) { midiControl.value = MIDI_OFF_VALUE; }
                            else { midiControl.value = MIDI_ON_VALUE; }
                            redrawScreen = true;
                        }
                    } else {
                        // knob behavior
                        int knobAdjust = controls.getRotaryAdjust(CONTROL_ENCODER);
                        newValue = adjustWithSaturation(midiControl.value, knobAdjust, MIDI_VALUE_MIN, MIDI_VALUE_MAX);
                        if (newValue != midiControl.value) { midiControl.value = newValue; redrawScreen = true; }
                    }
                    break;
                }
            default :
                break;
            }

        } else {
            // fields are being selected
            int knobAdjust = controls.getRotaryAdjustUnit(CONTROL_ENCODER);
            if (knobAdjust != 0) {
                selectedField = adjustWithSaturation(selectedField, knobAdjust, 0, NUM_CONTROL_FIELDS-1);
                redrawScreen = true;
            }
        }


        if (controls.isSwitchToggled(CONTROL_SWITCH)) {
            // encoder button was pushed
            if (activeField > 0) {
                // switch to select mode
                selectedField = activeField;
                activeField = -1;
            } else {
                // potentially swich to active mode depending on the selected field
                switch(selectedField) {
                case 0:
                  StringEdit(tft, midiControl.shortName, *(controls.touch), controls.m_encoders[CONTROL_ENCODER], controls.m_switches[CONTROL_SWITCH]);
                  break;
                case 1:
                  activeField = (activeField == 1 ? -1 : 1);
                  break;
                case 2:
                  activeField = (activeField == 2 ? -1 : 2);
                  break;
                case 3:
                  activeField = (activeField == 3 ? -1 : 3);
                  break;
                default:
                  break;
                }
            }
            redrawScreen = true;

        }

        delay(100); // needed in order for encoder activity sampling/filteirng
    } // while
}




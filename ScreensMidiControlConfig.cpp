/*
 * ScreensMidiControlConfig.cpp
 *
 *  Created on: Mar. 2, 2019
 *      Author: blackaddr
 */
#include "filePaths.h"
#include "Screens.h"
#include "ListDisplay.h"

// This screen provides a way for editing a preset.
const TouchArea BACK_BUTTON_AREA(BACK_BUTTON_X_POS, BACK_BUTTON_X_POS+ICON_SIZE, 0, ICON_SIZE);

constexpr int CONTROL_TEXT_START_XPOS = MARGIN;
constexpr int CONTROL_TEXT_START_YPOS = ICON_SIZE + DEFAULT_TEXT_HEIGHT;

constexpr int NUM_CONTROL_FIELDS = 5;
const char *CONTROL_FIELD_NAMES[NUM_CONTROL_FIELDS] = {"Short Name: ", "CC: ", "Input Control: ", "Type: ", "Value: "};

static ListDisplay listDisplay(NUM_CONTROL_FIELDS);

void redrawLines(ILI9341_t3 &tft, MidiControl &midiControl, int activeField)
{
    for (unsigned i=0; i<NUM_CONTROL_FIELDS; i++) {
        if (listDisplay.getUpdate(i)) {
            // This line needs to be updated, first draw the background box
            uint16_t boxColor = ILI9341_BLACK;
            if (i == listDisplay.getSelected()) {
                boxColor = (activeField == static_cast<int>(i)) ? ILI9341_RED : ILI9341_DARKCYAN; // selected line is either cyan or red
            }
            tft.fillRect(0, ICON_SIZE + (i+1)*DEFAULT_TEXT_HEIGHT, tft.width(), DEFAULT_TEXT_HEIGHT, boxColor);

            // Now draw the text
            tft.setCursor(CONTROL_TEXT_START_XPOS, CONTROL_TEXT_START_YPOS + i*DEFAULT_TEXT_HEIGHT);
            switch(i) {
            case 0 : tft.printf("%s%s\n", CONTROL_FIELD_NAMES[i], midiControl.shortName.c_str()); break;
            case 1 : tft.printf("%s%d\n", CONTROL_FIELD_NAMES[i], midiControl.cc); break;
            case 2 : tft.printf("%s%s\n", CONTROL_FIELD_NAMES[i], MidiControl::InputControlToString(midiControl.inputControl)); break;
            case 3 : tft.printf("%s%s\n", CONTROL_FIELD_NAMES[i], Controls::ControlTypeToString(midiControl.type)); break;
            case 4 : tft.printf("%s%d\n", CONTROL_FIELD_NAMES[i], midiControl.value);
            }
        }
    }
}

void DrawMidiControlConfig(ILI9341_t3 &tft, Controls &controls, MidiControl &midiControl)
{

    bool redrawScreen = true;
    bool updateScreen = false;
    int16_t nameEditButtonPosition;
    TouchArea editNameArea;
    int activeField = -1; // -1 means no field is active
    listDisplay.setSize(NUM_CONTROL_FIELDS);

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
            bmpDraw(tft, BACK_ICON_PATH, BACK_BUTTON_X_POS,0); // shifting more than 255 pixels seems to wrap the screen

            // NAME EDIT button
            nameEditButtonPosition = tft.getCursorX() + MARGIN;
            bmpDraw(tft, EDIT_ICON_PATH, nameEditButtonPosition, 0);
            editNameArea.setArea(nameEditButtonPosition, nameEditButtonPosition+ICON_SIZE, 0, ICON_SIZE);

            // Print the control info
            tft.setCursor(0,ICON_SIZE); // start row under icons

            activeField = -1;
            listDisplay.reset();
            listDisplay.setSize(NUM_CONTROL_FIELDS);
            listDisplay.setUpdateAll();
            redrawLines(tft, midiControl, activeField);
            redrawScreen = false;
        }

        if (updateScreen) {
            redrawLines(tft, midiControl, activeField);
            updateScreen = false;
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
                StringEdit(tft, midiControl.name, controls);
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
        if ((activeField > 0) && (knobAdjust != 0)) {
            // A field is currently being edited
            listDisplay.setUpdate(listDisplay.getSelected());

            switch(activeField) {
            case 1 :
            // CC adjust
            {
                unsigned newValue = adjustWithSaturation(midiControl.cc, knobAdjust, MIDI_CC_MIN, MIDI_CC_MAX);
                if (newValue != midiControl.cc) { midiControl.cc = newValue; updateScreen = true; }
                break;
            }
            case 2 :
            // InputControl
            {
                InputControl newValue = static_cast<InputControl>(adjustWithWrap(
                        static_cast<int>(midiControl.inputControl), knobAdjust, static_cast<int>(InputControl::NUM_CONTROLS))
                        );
                if (newValue != midiControl.inputControl) { midiControl.inputControl = newValue; updateScreen = true; }
            }
                break;
            case 3:
            // ControlType
            {
                ControlType newValue = static_cast<ControlType>(adjustWithWrap(
                        static_cast<int>(midiControl.type), knobAdjust, static_cast<int>(ControlType::NUM_TYPES))
                        );
                if (newValue != midiControl.type) { midiControl.type = newValue; updateScreen = true; }
            }
                break;
            case 4:
                // Value adjust
                {
                    unsigned newValue;
                    if ((midiControl.type == ControlType::SWITCH_LATCHING) || (midiControl.type == ControlType::SWITCH_MOMENTARY)) {
                        // switch behavior, only MIDI ON/OFF is permitted
                        if (knobAdjust != 0) {
                            if (midiControl.value == MIDI_ON_VALUE) { midiControl.value = MIDI_OFF_VALUE; }
                            else { midiControl.value = MIDI_ON_VALUE; }
                            updateScreen = true;
                        }
                    } else {
                        // knob behavior
                        newValue = adjustWithSaturation(midiControl.value, knobAdjust, MIDI_VALUE_MIN, MIDI_VALUE_MAX);
                        if (newValue != midiControl.value) { midiControl.value = newValue; updateScreen = true; }
                    }
                    break;
                }
            default :
                break;
            }

        } else {
            // fields are being selected
            if (knobAdjust != 0) {
                if (knobAdjust > 0) {
                    listDisplay.next();
                } else if (knobAdjust < 0) {
                    listDisplay.previous();
                }
                updateScreen = true;
            }
        }


        if (switchToggled) {
            // encoder button was pushed
            Serial.printf("selected(): %d active: %d\n", listDisplay.getSelected(), activeField);
            if (activeField > 0) {
                // switch to select mode
                activeField = -1;
            } else {
                // potentially switch to active mode depending on the selected field
                switch(listDisplay.getSelected()) {
                case 0:
                    StringEdit(tft, midiControl.shortName, controls);
                    redrawScreen = true;
                    break;
                case 1:
                case 2:
                case 3:
                case 4:
                    activeField = (activeField == (int)listDisplay.getSelected() ? -1 : listDisplay.getSelected());
                    break;
                default:
                  break;
                }
            }
            if (!redrawScreen) {
                listDisplay.setUpdate(listDisplay.getSelected());
                updateScreen = true;
            }
        }
    } // while
}




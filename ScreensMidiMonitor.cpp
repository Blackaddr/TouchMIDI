/*
 * ScreensMidiMonitor.cpp
 *
 *  Created on: Sept. 6, 2019
 *      Author: blackaddr
 *
 *  This screen is accessed from the "EXTRA" button off the PRESET NAVIGATION
 */
#include <array>
#include <MIDI.h>
#include "Screens.h"

constexpr int MIDI_CHANNEL = 1;

using namespace midi;


// TODO Add a MIDI scrolling log
//struct LogEntry {
//    char type[3];
//    DataByte data1;
//    DataByte data2;
//};
//
//constexpr size_t NUM_ENTRIES = 32;
//std::array<LogEntry, NUM_ENTRIES> entryLog;


// This screen presents a given presets controls for real-time use.
Screens DrawMidiMonitor(ILI9341_t3 &tft, Controls &controls, Preset &preset, MidiInterface<HardwareSerial> &midiPort)
{
    Serial.println("MIDI MONITOR");
    bool redrawScreen = true;

    while(true) {

        if (redrawScreen) {
            // Draw the Preset Edit Screen
            clearScreen(tft);
            tft.setCursor(0,MARGIN);

            // print the preset number in the top left
            tft.setCursor(tft.width()/10,MARGIN);
            tft.println("MIDI MONITOR");

            // Draw the icons
            bmpDraw(tft, "back48.bmp", BACK_BUTTON_X_POS,0); // shifting more than 255 pixels seems to wrap the screen
            redrawScreen = false;
        }

        // run a loop waiting for a control input from touch, rotary or switch
        while(true) {
            //MidiControl &control = *controlLocations[activeControl].control;

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

            }

            // Check for pushbutton control
            if (controls.isSwitchToggled(0)) {
                // Clear the screen
                redrawScreen = true;
                break;
            }

            while (midiPort.read()) {
                Serial.println("MIDI received!");
                MidiType type = midiPort.getType();
                DataByte data1 = midiPort.getData1();
                DataByte data2 = midiPort.getData2();

                String typeString;
                switch (type) {
                case midi::ControlChange :
                    typeString = "CC ";
                    break;
                case midi::ProgramChange :
                    typeString = "PC ";
                    break;
                case midi::SystemExclusive :
                    typeString = "EX ";
                    break;
                default :
                    typeString = "?? ";
                }

                tft.println(String(typeString + data1 + String(" ") + data2));
            }

            delay(100); // this is needed for control sampling to work
        } // end while loop to process control inputs

    } // end outer while loop
}




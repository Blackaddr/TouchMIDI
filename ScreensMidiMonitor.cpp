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
#include "filePaths.h"
#include "Screens.h"
#include "MidiProc.h"

//constexpr int MIDI_CHANNEL = 1;

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

constexpr unsigned MAX_LINES = 12;

// This screen presents a given presets controls for real-time use.
Screens DrawMidiMonitor(ILI9341_t3 &tft, Controls &controls, Preset &preset, MidiInterface<HardwareSerial> &midiPort)
{
    bool redrawScreen = true;
    unsigned wordCount = 0;
    unsigned lineCount = 0;

    while(true) {

        if (redrawScreen) {
            // Draw the Preset Edit Screen
            clearScreen(tft);
            tft.setCursor(0,MARGIN);

            // print the preset number in the top left
            tft.setCursor(tft.width()/10,MARGIN);
            tft.println("MIDI MONITOR\n");

            // Draw the icons
            bmpDraw(tft, BACK_ICON_PATH, BACK_BUTTON_X_POS,0); // shifting more than 255 pixels seems to wrap the screen
            redrawScreen = false;
        }

        // run a loop waiting for a control input from touch, rotary or switch
        while(true) {

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
            ControlEvent controlEvent;
            while (getNextControlEvent(controlEvent)) {
                switch(controlEvent.eventType) {
                case ControlEventType::SWITCH : redrawScreen = true; break; // clear the screen
                default : break;
                }
            }

            // Check for MIDI messages
            MidiWord midiWord;
            while (isMidiWordReady()) {

                if (lineCount == MAX_LINES) {
                    lineCount = 0;
                    redrawScreen = true;
                    break;
                }

                getNextMidiWord(midiWord);
                String typeString;

                switch(midiWord.type) {
                case midi::ControlChange :
                    typeString = "CC";
                    break;
                case midi::ProgramChange :
                    typeString = "PC";
                    break;
                case midi::SystemExclusive :
                    typeString = "EX";
                    break;
                default :
                    typeString = "?? ";
                }
                tft.printf("%s %02X %2X ", typeString.c_str(), midiWord.data1, midiWord.data2);
                wordCount++;
                if ((wordCount % 2) == 0) {
                    lineCount++;
                    tft.printf("\n");
                }
            }

            if (redrawScreen) { break; }
        } // end while loop to process control inputs

    } // end outer while loop
}




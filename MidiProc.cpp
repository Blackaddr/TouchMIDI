/*
 * MidiProc.cpp
 *
 *  Created on: Dec. 15, 2019
 *      Author: blackaddr
 */
#include "MidiProc.h"
#include "MidiDefs.h"
#include "Preset.h"
#include "Screens.h"

using namespace midi;

std::mutex midiInQueueMutex;
std::queue<MidiWord> *midiInQueue = new std::queue<MidiWord>();

std::mutex midiOutQueueMutex;
std::queue<MidiWord> *midiOutQueue = new std::queue<MidiWord>();

void remapMidiSend(MidiWord &midiWord, volatile Preset &activePreset);

void processMidi(void *rawMidiPortPtr)
{
    // it's okay to crash here if the pointers invalid to avoid failing silently
    MidiInterface<HardwareSerial> &midiPort = *(reinterpret_cast<MidiInterface<HardwareSerial>*>(rawMidiPortPtr));
    while(true) {
        // Check for MIDI IN activity
        if (midiPort.read()) {
            MidiWord midiWord;
            midiWord.type    = midiPort.getType();
            midiWord.data1   = midiPort.getData1();
            midiWord.data2   = midiPort.getData2();
            midiWord.channel = midiPort.getChannel();

            // We have a special hook here if we're on the NAV screen. In this case we
            // trap on certain CCs to convert them custom CC that is not sent out over MIDI
            // No other MIDI will be passed while on the nav screen as that screen takes control over
            // sending MIDI out messages.
            if (g_currentScreen == Screens::PRESET_NAVIGATION) {
                if ((midiWord.type == midi::ControlChange) && (midiWord.data2 == MIDI_OFF_VALUE)) {
                    switch (midiWord.data1) {
                    case MIDI_PRESET_UP     : midiWord.data1 = MIDI_CC_SPECIAL_UP;     break;
                    case MIDI_PRESET_SELECT : midiWord.data1 = MIDI_CC_SPECIAL_SELECT; break;
                    case MIDI_PRESET_DOWN   : midiWord.data1 = MIDI_CC_SPECIAL_DOWN;   break;
                    default: break;
                    }

                    // add the new CC to the input queue
                    {
                        std::lock_guard<std::mutex> lock(midiInQueueMutex);
                        if (midiInQueue->size() >= MIDI_QUEUE_MAX_SIZE) {
                            // queue is full, pop the oldest then add
                            midiInQueue->pop();
                        }
                        midiInQueue->emplace(midiWord);
                    }
                }
            } else {
                // Not on the Nav screen.
                //Serial.println(String("MidiProc(): MIDI Received: ") + midiWord.type + String(" ") + midiWord.data1 + String(" ") + midiWord.data2);
                remapMidiSend(midiWord, getActivePreset() );
                //Serial.println(String("MidiProc(): MIDI Remapped: ") + midiWord.type + String(" ") + midiWord.data1 + String(" ") + midiWord.data2);
                {
                    std::lock_guard<std::mutex> lock(midiInQueueMutex);
                    if (midiInQueue->size() >= MIDI_QUEUE_MAX_SIZE) {
                        // queue is full, pop the oldest then add
                        midiInQueue->pop();
                    }
                    midiInQueue->emplace(midiWord);
                }
            }
        }

        // Check the MIDI OUT queue
        if (midiOutQueue->size() > 0) {
            MidiWord midiWord;
            {
                std::lock_guard<std::mutex> lock(midiOutQueueMutex);
                midiWord = midiOutQueue->front();
                midiOutQueue->pop();
            }

            Serial.printf("MidiProc(): MIDI send: type:%d data1:%d data2:%d ch:%d\n", midiWord.type, midiWord.data1, midiWord.data2, midiWord.channel);
            midiPort.send(midiWord.type, midiWord.data1, midiWord.data2, midiWord.channel);

        }
        yield();
    }
}

void remapMidiSend(MidiWord &midiWord, volatile Preset &activePresetIn)
{
    bool midiDropMessage = false;
    Preset& activePreset = (Preset&)activePresetIn;
    if (midiWord.type == midi::ControlChange) {
        for (auto it = activePreset.controls.begin(); it != activePreset.controls.end(); ++it) {

            unsigned inputControl = MidiControl::GetInputControlMappedCC((*it).inputControl);
            Serial.printf("inputControl: %d, midi CC: %d\n", inputControl, midiWord.data1);
            if ( midiWord.data1 == inputControl) {

                Serial.printf("Remap %d to %d with %d\n", midiWord.data1, (*it).cc, midiWord.data2);
                midiWord.data1 = (*it).cc; // remap to the assigned CC

                if ((*it).type == ControlType::SWITCH_LATCHING) {
                    // Toggle the stored value each time MIDI ON is received
                    if (midiWord.data2 == MIDI_ON_VALUE) {
                        (*it).value = static_cast<unsigned>(toggleValue((*it).value, MIDI_ON_VALUE, MIDI_OFF_VALUE));
                        midiWord.data2 = (*it).value;
                        (*it).updated = true;
                    } else {
                        midiDropMessage = true;
                    }
                } else {
                    // For all other types, update with the instantaneous value
                    (*it).value = adjustWithSaturation(0, midiWord.data2 & 0x7f, 0, MIDI_VALUE_MAX);
                    (*it).updated = true;
                }
            } // end if mapping match found
        }// end preset control map FOR loop
    } // end if CC

    // Transmit the midi word
    if (!midiDropMessage) { midiSendWord(midiWord); }
}

bool getNextMidiWord(MidiWord &midiWord)
{
    bool isMidiAvailable = false;
    {
        std::lock_guard<std::mutex> lock(midiInQueueMutex);
        midiWord = midiInQueue->front();
        if (!midiInQueue->empty()) {
            isMidiAvailable = true;
            midiInQueue->pop();
        }
    } // mutex unlocks
    return isMidiAvailable;
}

void midiSendWord(MidiWord midiWord)
{
    Serial.printf("midiSendWord(): type: %d data1: %d data2: %d\n", midiWord.type, midiWord.data1, midiWord.data2);
    {
        std::lock_guard<std::mutex> lock(midiOutQueueMutex);
        if (midiOutQueue->size() >= MIDI_QUEUE_MAX_SIZE) {
            // queue is full, pop the oldest then add
            midiOutQueue->pop();
        }
        midiOutQueue->emplace(midiWord);
    }
}

void midiProgramSend(unsigned programNumber, unsigned channel)
{
    MidiWord midiWord;
    midiWord.type = midi::MidiType::ProgramChange;
    midiWord.data1 = programNumber;
    midiWord.data2 = 0;
    midiWord.channel = channel;

    midiSendWord(midiWord);
}

bool isMidiWordReady(void)
{
    return (!midiInQueue->empty());
}



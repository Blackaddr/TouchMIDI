/*
 * MidiProc.cpp
 *
 *  Created on: Dec. 15, 2019
 *      Author: blackaddr
 */
#include "MidiProc.h"
#include "MidiDefs.h"
#include "Screens.h"

using namespace midi;

std::mutex midiInQueueMutex;
std::queue<MidiWord> *midiInQueue = new std::queue<MidiWord>();

std::mutex midiOutQueueMutex;
std::queue<MidiWord> *midiOutQueue = new std::queue<MidiWord>();

void processMidi(void *rawMidiPortPtr)
{
    // it's okay to crash here if the pointers invalid to avoid failing silently
    MidiInterface<HardwareSerial> &midiPort = *(reinterpret_cast<MidiInterface<HardwareSerial>*>(rawMidiPortPtr));
    while(true) {
        // Check for MIDI IN activity
        if (midiPort.read()) {
            MidiWord midiWord;
            midiWord.type  = midiPort.getType();
            midiWord.data1 = midiPort.getData1();
            midiWord.data2 = midiPort.getData2();

            // We have a special hook here if we're on the NAV screen. In this case we
            // trap on certain CCs to convert them custom CC that is not sent out over MIDI
            // No other MIDI will be passed while on the nav screen as that screen takes control over
            // sending MIDI out messages.
            //Serial.println(String("MidiProc(): MIDI Received: ") + midiWord.type + String(" ") + midiWord.data1 + String(" ") + midiWord.data2);
            //Serial.println(String("MidiProc(): Screen: ") + static_cast<unsigned>(g_currentScreen));
            if (g_currentScreen == Screens::PRESET_NAVIGATION) {
                if ((midiWord.type == midi::ControlChange) && (midiWord.data2 == MIDI_OFF_VALUE)) {
                    switch (midiWord.data1) {
                    case MIDI_PRESET_UP     : midiWord.data1 = MIDI_CC_SPECIAL_UP;     break;
                    case MIDI_PRESET_SELECT : midiWord.data1 = MIDI_CC_SPECIAL_SELECT; break;
                    case MIDI_PRESET_DOWN   : midiWord.data1 = MIDI_CC_SPECIAL_DOWN;   break;
                    default: break;
                    }
                    //Serial.println(String("MidiProc(): MIDI MODIFIED: ") + midiWord.type + String(" ") + midiWord.data1 + String(" ") + midiWord.data2);

                    // add the new CC to the queue, but don't send it.
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
                //Serial.println(String("MidiProc(): MIDI Received: ") + midiWord.type + String(" ") + midiWord.data1 + String(" ") + midiWord.data2);
                {
                    std::lock_guard<std::mutex> lock(midiInQueueMutex);
                    if (midiInQueue->size() >= MIDI_QUEUE_MAX_SIZE) {
                        // queue is full, pop the oldest then add
                        midiInQueue->pop();
                    }
                    midiInQueue->emplace(midiWord);
                }

//                // Software THRU
//                {
//                    std::lock_guard<std::mutex> lock(midiOutQueueMutex);
//                    if (midiOutQueue->size() >= MIDI_QUEUE_MAX_SIZE) {
//                        // queue is full, pop the oldest then add
//                        midiOutQueue->pop();
//                    }
//                    midiOutQueue->emplace(midiWord);
//                }
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
            Serial.println("Send midi\n");
            midiPort.send(midiWord.type, midiWord.data1, midiWord.data2, midiWord.channel);
        }
        yield();
    }
}

void midiSendWord(MidiWord midiWord)
{
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

    //Serial.println("Add PGM change");
    midiSendWord(midiWord);
}



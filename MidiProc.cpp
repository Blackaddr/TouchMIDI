/*
 * MidiProc.cpp
 *
 *  Created on: Dec. 15, 2019
 *      Author: blackaddr
 */
#include "MidiProc.h"

using namespace midi;

std::mutex midiQueueMutex;
std::queue<MidiWord> *midiQueue = new std::queue<MidiWord>();

//void processMidi(MidiInterface<HardwareSerial> &midiPort)
void processMidi(void *rawMidiPortPtr)
{
    // it's okay to crash here if the pointers invalid to avoid failing silently
    MidiInterface<HardwareSerial> &midiPort = *(reinterpret_cast<MidiInterface<HardwareSerial>*>(rawMidiPortPtr));
    while(true) {
        // Check for MIDI activity
        if (midiPort.read()) {
            //Serial.println("MIDI received!");
            MidiWord midiWord;
            midiWord.type  = midiPort.getType();
            midiWord.data1 = midiPort.getData1();
            midiWord.data2 = midiPort.getData2();

            //Serial.println(String("MIDI Received: ") + midiWord.type + String(" ") + midiWord.data1 + String(" ") + midiWord.data2);

            if (midiQueue->size() < MIDI_QUEUE_MAX_SIZE) {
                std::lock_guard<std::mutex> lock(midiQueueMutex);
                midiQueue->push(midiWord);
            } // else drop if queue is full
        }
        yield();
    }
}



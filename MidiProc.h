/*
 * MidiProc.h
 *
 *  Created on: Dec. 15, 2019
 *      Author: blackaddr
 */
#ifndef MIDIPROC_H_
#define MIDIPROC_H_

#include <Arduino.h>
#include <queue>
#include <MIDI.h>
#include <TeensyThreads.h>

#include "MidiDefs.h"

constexpr int MIDI_CHANNEL = 1;
struct MidiWord {
    midi::MidiType type;
    midi::DataByte data1;
    midi::DataByte data2;
    midi::Channel  channel;
};

constexpr size_t MIDI_QUEUE_MAX_SIZE = 128;

extern std::mutex midiInQueueMutex;
extern std::queue<MidiWord> *midiInQueue;

extern std::mutex midiOutQueueMutex;
extern std::queue<MidiWord> *midiOutQueue;

void processMidi(void *rawMidiPortPtr);
void midiSendWord(MidiWord midiWord);
void midiProgramSend(unsigned programNumber, unsigned channel);

#endif /* MIDIPROC_H_ */

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

constexpr int MIDI_CHANNEL = 1;
struct MidiWord {
    midi::MidiType type;
    midi::DataByte data1;
    midi::DataByte data2;
};

constexpr size_t MIDI_QUEUE_MAX_SIZE = 128;

extern std::mutex midiQueueMutex;
//std::shared_ptr<std::queue<MidiWord>> midiQueue = nullptr; // shared pointer doesn't seem to work on Teensyduino anymore
extern std::queue<MidiWord> *midiQueue;

void processMidi(void *rawMidiPortPtr);
//void processMidi(midi::MidiInterface<HardwareSerial> &midiPort);

#endif /* MIDIPROC_H_ */

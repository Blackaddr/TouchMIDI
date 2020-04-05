/*
 * MidiDefs.h
 *
 *  Created on: Apr. 1, 2020
 *      Author: blackaddr
 */

#ifndef MIDIDEFS_H_
#define MIDIDEFS_H_

constexpr int MIDI_PROGRAM_CHANNEL = 1;
constexpr int MIDI_CC_CHANNEL      = 1;

// These CCs will be trapped while on the navigation screen
constexpr int MIDI_PRESET_UP     = 32;
constexpr int MIDI_PRESET_SELECT = 33;
constexpr int MIDI_PRESET_DOWN   = 34;

// and converted to these in the message queue. This will signal
// the nav screen to send a program change.
constexpr int MIDI_CC_SPECIAL_UP     = 253;
constexpr int MIDI_CC_SPECIAL_SELECT = 254;
constexpr int MIDI_CC_SPECIAL_DOWN   = 255;


#endif /* MIDIDEFS_H_ */

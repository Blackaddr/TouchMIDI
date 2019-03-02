#ifndef __PRESET_H
#define __PRESET_H

#include "VectorSupport.h"

#include "ArduinoJson.h"
#include "Misc.h"
#include "Controls.h"
using std::vector;

constexpr int MAX_NUM_CONTROLS = 6;
constexpr unsigned MAX_PRESETS = 32;
constexpr unsigned MAX_NAME_SIZE = 32;
constexpr unsigned MAX_SHORT_NAME_SIZE = 4;

/// Contains the necessary information for a single MIDI control parameter
struct MidiControl {
  String name; ///< the name of the parameter
  String shortName; ///< the short name for the parameter
  unsigned cc; ///< the CC assigned to the parameter
  ControlType type; ///< the type of control, usually a switch or encoder
  unsigned value; ///<  the current assigned value to the MIDI parameter
  bool updated = false; ///< when true the value has been updated

  /// Construct with values initialzed as specified
  MidiControl(String name, String shortName, int cc, ControlType type, unsigned value)
      : name(name), shortName(shortName), cc(cc), type(type), value(value) {
      name.reserve(MAX_NAME_SIZE);
      shortName.reserve(MAX_SHORT_NAME_SIZE);
  }
  /// Construct with default values
  MidiControl()
      : name(""), shortName(""), cc(0), type(ControlType::SWITCH_MOMENTARY), value(0) {
      name.reserve(MAX_NAME_SIZE);
      shortName.reserve(MAX_SHORT_NAME_SIZE);
  }
};

/// Contains the necessary information for a single Preset
struct Preset {
  String name; ///< the name of the preset
  unsigned index; ///< the index of the preset
  unsigned numControls; ///< the number of MIDI controls for this preset
  vector<MidiControl> controls; ///< a vector of MidiControls
  Preset() { name.reserve(MAX_NAME_SIZE); }
};

using PresetArray = std::vector<Preset>;

PresetArray *createPresetArray(void);
void destroyPresetArray(PresetArray *presetArray);
void jsonToPreset(JsonObject &jsonObj, Preset &preset);
Preset createDefaultPreset(unsigned index, unsigned numControls);
void createDefaultPresets(PresetArray *presetArray, unsigned numPresets, unsigned numControls);

#endif

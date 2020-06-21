#ifndef __PRESET_H
#define __PRESET_H

#include <atomic>
#include "VectorSupport.h"

#include "ArduinoJson.h"
#include "Misc.h"
#include "Controls.h"
using std::vector;

constexpr int      MAX_NUM_CONTROLS      = 6;
constexpr unsigned MAX_PRESETS           = 32;
constexpr unsigned MAX_NAME_SIZE         = 32;
constexpr unsigned MAX_SHORT_NAME_SIZE   = 4;
constexpr unsigned MAX_SETLIST_NAME_SIZE = 32;

const char DEFAULT_SETLIST[]   = "DEFAULT";

enum class InputControl : unsigned {
    NOT_CONFIGURED = 0,
    SW1,
    SW2,
    SW3,
    SW4,
    EXP1,
    EXP2,
    NUM_CONTROLS
};

/// Contains the necessary information for a single MIDI control parameter
struct MidiControl {
  String name; ///< the name of the parameter
  String shortName; ///< the short name for the parameter
  unsigned cc; ///< the CC assigned to the parameter
  InputControl inputControl; ///< option input control from foot switch processor
  ControlType type; ///< the type of control, usually a switch or encoder
  unsigned value; ///<  the current assigned value to the MIDI parameter
  //std::atomic_bool updated; ///< when true the value has been updated
  bool updated;

  /// Construct with values initialized as specified
  MidiControl(String name, String shortName, int cc, InputControl inputControl, ControlType type, unsigned value)
      : name(name), shortName(shortName), cc(cc), inputControl(inputControl), type(type), value(value), updated(true) {
      name.reserve(MAX_NAME_SIZE);
      shortName.reserve(MAX_SHORT_NAME_SIZE);
  }
  /// Construct with default values
  MidiControl()
      : name(""), shortName(""), cc(0), inputControl(InputControl::NOT_CONFIGURED), type(ControlType::SWITCH_MOMENTARY), value(0), updated(true) {
      name.reserve(MAX_NAME_SIZE);
      shortName.reserve(MAX_SHORT_NAME_SIZE);
  }

  static const char* InputControlToString(InputControl type) {
        switch (type) {
        case InputControl::NOT_CONFIGURED :
            return "NOT CONFIG";
        case InputControl::SW1 :
            return "SW1";
        case InputControl::SW2 :
            return "SW2";
        case InputControl::SW3 :
            return "SW3";
        case InputControl::SW4 :
            return "SW4";
        case InputControl::EXP1 :
            return "EXP1";
        case InputControl::EXP2 :
            return "EXP2";
        default:
            return "UNDEFINED";
        }
}

// TODO, replace this with some that reads from a mapped file
static unsigned GetInputControlMappedCC(InputControl type) {
      switch (type) {
          case InputControl::SW1 :
              return 32;
          case InputControl::SW2 :
              return 33;
          case InputControl::SW3 :
              return 34;
          case InputControl::SW4 :
              return 35;
          case InputControl::EXP1 :
              return 36;
          case InputControl::EXP2 :
              return 37;
          case InputControl::NOT_CONFIGURED :
          default:
              return 0;
      }
}



};

/// Contains the necessary information for a single Preset
struct Preset {
  String name; ///< the name of the preset
  unsigned index; ///< the index of the preset
  unsigned numControls; ///< the number of MIDI controls for this preset
  vector<MidiControl> controls; ///< a vector of MidiControls
  Preset(unsigned index = 0) : index(index), numControls(0) {
      name.reserve(MAX_NAME_SIZE);
      name = String("*new*");
  }
};

using PresetArray = std::vector<Preset>;

PresetArray *createPresetArray(void);
void destroyPresetArray(PresetArray *presetArray);
void jsonToPreset(JsonObject &jsonObj, Preset &preset);
void presetToJson(Preset &preset, JsonObject &root);
Preset createDefaultPreset(unsigned index, unsigned numControls);
void createDefaultPresets(PresetArray *presetArray, unsigned numPresets, unsigned numControls);

Preset& getActivePreset(void);
unsigned getActivePresetIndex(void);
void    setActivePreset(Preset* activePreset);

void    setActiveSetlist(const char* setlistName);
const char* getActiveSetlist(void);
void    getActiveSetlist(char* setlistName);

#endif

#ifndef __PRESET_H
#define __PRESET_H

#include <vector>
#include <array>

#include "ArduinoJson.h"
#include "Misc.h"
#include "Controls.h"
using std::vector;

constexpr int MAX_NUM_CONTROLS = 6;
constexpr unsigned MAX_PRESETS = 32;
constexpr unsigned MAX_NAME_SIZE = 32;

// These calls must be define in order to get vector to work on arduino
namespace std {
void __throw_bad_alloc() {
  Serial.println("Unable to allocate memory");
}
void __throw_length_error( char const*e ) {
  Serial.print("Length Error :"); Serial.println(e);
}
}

/// Contains the necessary information for a single MIDI control parameter
struct MidiControl {
  String name; ///< the name of the parameter
  unsigned cc; ///< the CC assigned to the parameter
  ControlType type; ///< the type of control, usually a switch or encoder
  unsigned value; ///<  the current assigned value to the MIDI parameter

  /// Construct with values initialzed as specified
  MidiControl(String name, int cc, ControlType type, unsigned value) : name(name), cc(cc), type(type), value(value) { name.reserve(MAX_NAME_SIZE); }
  /// Construct with default values
  MidiControl() : name(""), cc(0), type(ControlType::SWITCH_MOMENTARY), value(0) { name.reserve(MAX_NAME_SIZE); }
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
//using PresetArray = std::array<Preset, MAX_PRESETS>;

// Create and reserve memory for MAX_PRESETS vector of presets
PresetArray *createPresetArray(void)
{
  PresetArray *presetArray = new PresetArray();
  Serial.println("Reserving");
  presetArray->reserve(MAX_PRESETS);
  Serial.println("Done reserving");
  return presetArray;
}

// Calling this function on a processor with no MMU will likely result in memory fragmentation
void destroyPresetArray(PresetArray *presetArray)
{
  delete presetArray;
}

/// convert a JSON object containing a single preset into a preset structure
/// @param jsonObj a reference to the JsonObject containing the preset information
/// @param preset  a reference to the preset in which to store or update the data from the JSON object
void jsonToPreset(JsonObject &jsonObj, Preset &preset)
{
  preset.name = String(static_cast<const char *>(jsonObj["presetName"]));
  preset.index = jsonObj["presetIndex"];
  preset.numControls = jsonObj["numControls"];

  Serial.println(String("Preset name: ") + preset.name);
  Serial.println(String("Preset index: ") + preset.index);
  Serial.println(String("Preset numControls: ") + preset.numControls + String("\n"));
  
  for (unsigned i=0; i<preset.numControls; i++) {    

    MidiControl newControl = MidiControl(

      String(static_cast<const char *>(jsonObj["controls"][i]["name"])),      
      static_cast<unsigned>(jsonObj["controls"][i]["params"][0]),
      static_cast<ControlType>(static_cast<unsigned>(jsonObj["controls"][i]["params"][1])),
      static_cast<unsigned>(jsonObj["controls"][i]["params"][2])
      );
    addToVector(preset.controls, newControl, i);

#ifdef DEBUG
      Serial.println(String("controlName: ") + preset.controls[i].name);
      Serial.println(String("CC: ") + preset.controls[i].cc);
      Serial.println(String("Type: ") + static_cast<unsigned>(preset.controls[i].type));
      Serial.println(String("value: ") + preset.controls[i].value);
#endif
  }
}

/// Create a new preset with defaults for the supplied index and number of controls
/// @param index the preset will be configured for the specified index
/// @param numControls specifies the number of controls to initialize
Preset createDefaultPreset(unsigned index, unsigned numControls)
{
  Preset preset;
  preset.name = String(String("Preset") + index);
  preset.index = index;
  preset.numControls = numControls;

  for (unsigned i=0; i<preset.numControls; i++) {
    //Serial.println("Creating a control");
    MidiControl newControl = MidiControl(
      String(String("Control")+i), 16, ControlType::ROTARY_KNOB, 0);
    addToVector(preset.controls, newControl, i);
  }
  return preset;
}

/// Create and initialize default presets
/// @param presetArray pointer to the vector of presets
/// @param numPresets number of presets to initialize
/// @param numControls the number of controls to initialize in each preset
void createDefaultPresets(PresetArray *presetArray, unsigned numPresets, unsigned numControls)
{
  Serial.println(String("Current array size is ") + presetArray->size() + String(" capacity is ") + presetArray->capacity());
  for (unsigned i=0; i<numPresets; i++) {
    addToVector((*presetArray), createDefaultPreset(i, numControls), i);
  }
}
#endif

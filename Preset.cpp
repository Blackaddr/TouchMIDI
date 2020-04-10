/*
 * Preset.cpp
 *
 *  Created on: Mar. 2, 2019
 *      Author: blackaddr
 */
#include "Preset.h"

//#define DEBUG
Preset* g_activePreset = nullptr;

// Create and reserve memory for MAX_PRESETS vector of presets
PresetArray *createPresetArray(void)
{
  PresetArray *presetArray = new PresetArray();
  //Serial.println("Reserving");
  presetArray->reserve(MAX_PRESETS);
  //Serial.println("Done reserving");
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

//  Serial.println(String("Preset name: ") + preset.name);
//  Serial.println(String("Preset index: ") + preset.index);
//  Serial.println(String("Preset numControls: ") + preset.numControls + String("\n"));

  for (unsigned i=0; i<preset.numControls; i++) {

    MidiControl newControl = MidiControl(

        String(static_cast<const char *>(jsonObj["controls"][i]["name"])),
        String(static_cast<const char *>(jsonObj["controls"][i]["shortName"])),
        static_cast<unsigned>(jsonObj["controls"][i]["params"][0]), // CC
        static_cast<InputControl>(static_cast<unsigned>(jsonObj["controls"][i]["params"][1])), // InputControl
        static_cast<ControlType> (static_cast<unsigned>(jsonObj["controls"][i]["params"][2])), // ControlType
        static_cast<unsigned>(jsonObj["controls"][i]["params"][3]) // value
      );
    addToVector(preset.controls, newControl, i);

#ifdef DEBUG
      Serial.println(String("controlName: ") + preset.controls[i].name);
      Serial.println(String("controlShortName: ") + preset.controls[i].shortName);
      Serial.println(String("CC: ") + preset.controls[i].cc);
      Serial.println(String("Input Control: ") + static_cast<unsigned>(preset.controls[i].inputControl));
      Serial.println(String("Type: ") + static_cast<unsigned>(preset.controls[i].type));
      Serial.println(String("value: ") + preset.controls[i].value);
#endif
  }
}

void presetToJson(Preset &preset, JsonObject &root)
{
    //Serial.printf("Saving preset %s\n", preset.name.c_str());
    root["presetName"] = preset.name.c_str();
    root["presetIndex"] = preset.index;
    root["numControls"] = preset.numControls;

    JsonArray& controls = root.createNestedArray("controls");

    for (unsigned i=0; i < preset.numControls; i++) {
        JsonObject &controlItem = controls.createNestedObject();
        controlItem["name"]      = preset.controls[i].name.c_str();
        controlItem["shortName"] = preset.controls[i].shortName.c_str();
        JsonArray &params = controlItem.createNestedArray("params");
        params.add(preset.controls[i].cc);
        params.add(static_cast<unsigned>(preset.controls[i].inputControl));
        params.add(static_cast<unsigned>(preset.controls[i].type));
        params.add(preset.controls[i].value);
    }

    //root.prettyPrintTo(Serial);
    //root.printTo(Serial);
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
      String(String("Control")+i), String(String("Ctl")+i), 16, InputControl::NOT_CONFIGURED, ControlType::ROTARY_KNOB, 0);
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
  //Serial.println(String("Current array size is ") + presetArray->size() + String(" capacity is ") + presetArray->capacity());
  for (unsigned i=0; i<numPresets; i++) {
    addToVector((*presetArray), createDefaultPreset(i, numControls), i);
  }
}

Preset* getActivePreset(void)
{
    return g_activePreset;
}
void setActivePreset(Preset* activePreset)
{
    if (activePreset) {
        g_activePreset = activePreset;
    }
}

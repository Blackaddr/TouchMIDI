#ifndef __PRESET_H
#define __PRESET_H

#include <vector>

#include "ArduinoJson.h"
using std::vector;

constexpr int MAX_NUM_CONTROLS = 6;

enum class ControlType : unsigned {
  SWITCH_MOMENTARY = 0,
  SWITCH_LATCHING = 1,
  ROTARY_KNOB = 2
};

struct MidiControl {
  String name;
  int cc;
  ControlType type;
  int value;
  MidiControl(String name, int cc, ControlType type, int value) : name(name), cc(cc), type(type), value(value) {}
  MidiControl() : name(""), cc(0), type(ControlType::SWITCH_MOMENTARY), value(0) {}
};

struct Preset {
  String name;
  int index;
  int numControls;
  vector<MidiControl> controls;
};

void jsonToPreset(JsonObject &jsonObj, Preset &preset)
{
  preset.name = String(static_cast<const char *>(jsonObj["presetName"]));
  preset.index = jsonObj["presetIndex"];
  preset.numControls = jsonObj["numControls"];

//  Serial.println(String("Processing ") + preset.numControls + String (" presets"));
//
//  MidiControl ctrl;
//  ctrl.name = String(static_cast<const char *>(jsonObj["controlName"]));
//  Serial.println(String("The preset name is ") + ctrl.name);
//
//  ctrl.cc = static_cast<int>(jsonObj["params"][0]);
//  Serial.println(String("The preset CC is ") + ctrl.cc);
//
//  ctrl.type = static_cast<ControlType>(static_cast<unsigned>(jsonObj["params"][1]));
//  Serial.println(String("The preset type is ") + static_cast<unsigned>(ctrl.type));
//
//  ctrl.value = static_cast<int>(jsonObj["params"][2]);
//  Serial.println(String("The preset value is ") + static_cast<unsigned>(ctrl.value));
  
  for (int i=0; i<preset.numControls; i++) {    
    preset.controls.emplace_back(MidiControl(
      String(static_cast<const char *>(jsonObj["controlName"])),
      static_cast<int>(jsonObj["params"][0]),
      static_cast<ControlType>(static_cast<unsigned>(jsonObj["params"][1])),
      static_cast<int>(jsonObj["params"][2])
      ));
      Serial.println("end of loop");
  }
}

#endif

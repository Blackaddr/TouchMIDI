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

  Serial.println(String("Preset name: ") + preset.name);
  Serial.println(String("Preset index: ") + preset.index);
  Serial.println(String("Preset numControls: ") + preset.numControls + String("\n"));
  
  for (int i=0; i<preset.numControls; i++) {    
    preset.controls.emplace_back(MidiControl(

      String(static_cast<const char *>(jsonObj["controls"][i]["name"])),      
      static_cast<int>(jsonObj["controls"][i]["params"][0]),
      static_cast<ControlType>(static_cast<unsigned>(jsonObj["controls"][i]["params"][1])),
      static_cast<int>(jsonObj["controls"][i]["params"][2])
      ));      
      Serial.println(String("controlName: ") + preset.controls[i].name);
      Serial.println(String("CC: ") + preset.controls[i].cc);
      Serial.println(String("Type: ") + static_cast<int>(preset.controls[i].type));
      Serial.println(String("value: ") + preset.controls[i].value);
  }
}

#endif

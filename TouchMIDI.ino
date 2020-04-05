#include <SPI.h>
#include <Wire.h>      // this is needed even tho we aren't using it
#include <ILI9341_t3.h>
#include <SPI.h>
#include <MIDI.h>
#include "TeensyThreads.h"

using namespace midi;

#include "FileAccess.h"
#include "ArduinoJson.h"
#include "Bounce.h"

#include "Misc.h"
#include "MidiProc.h"
#include "Controls.h"
#include "Screens.h"
#include "Preset.h"

/////////////////////
// SD CARD
/////////////////////
File file;
#define SDCARD_CS 3

/////////////////////
// TOUCH
/////////////////////
// The STMPE610 uses hardware SPI on the shield, and #8
constexpr unsigned STMPE_CS = 6;
constexpr unsigned STMPE_IRQ = 2;

/////////////////////
// TFT DISPLAY
/////////////////////
// The display also uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC  9
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

/////////////////////
// CONTROLS
/////////////////////
Controls controls(1,1);
Screens nextScreen;

constexpr size_t JSON_BUFFER_SIZE = 1024;
char jsonTextBuffer[JSON_BUFFER_SIZE];
StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
JsonObject *jsonObj;
midi::MidiInterface<HardwareSerial> *midiPortPtr = nullptr;


constexpr unsigned PRESET_ID_INDEX = 6;
char presetFilename[] = "PRESET0.JSN";
char calibFilename[] = "TCALIB.BIN";
PresetArray *presetArray = nullptr;
unsigned activePreset = 0;
unsigned selectedPreset = 0;

void setup(void) {

  delay(1000);
  Serial.begin(115200);
  if (!Serial) { yield(); }
  delay(1000);
  pinMode(23,INPUT);

  // Setup MIDI OUT
  //MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, midiPort);
  //midi::MidiInterface<Type> midiPort((HardwareSerial&)Serial1);
  midiPortPtr = new midi::MidiInterface<HardwareSerial>((HardwareSerial&)Serial1);
  midiPortPtr->begin(MIDI_CHANNEL_OMNI);
  midiPortPtr->turnThruOff();

  pinMode(SDCARD_CS, OUTPUT);
  digitalWrite(SDCARD_CS, 1); // disable the SD CARD
  pinMode(STMPE_CS, OUTPUT);
  digitalWrite(STMPE_CS, 1); // disable the Touch interface
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, 1); // disable the TFT interface

  Serial.println("Creating Preset Array");
  presetArray = createPresetArray();
  //createDefaultPresets(presetArray, 2, 2);

  // Check the SD Card
  if (!SD.begin(SDCARD_CS)) {
    Serial.println("SD Card init seems to fail");
  } else {
      for (unsigned i=0; i<MAX_PRESETS; i++) {
        presetFilename[PRESET_ID_INDEX] = i + 0x30;
        file = SD.open(presetFilename);
        if (!file) {
          //Serial.println(String("Can't open ") + presetFilename);
        } else {
          // Read the file contents
          Serial.println(String("Processing ") + presetFilename);
          size_t availableBytes = file.available();
          Serial.println(String("Reading ") + availableBytes + String(" bytes"));
          if (availableBytes > 0) {
            file.read(jsonTextBuffer, availableBytes);
            jsonObj = &jsonBuffer.parseObject(jsonTextBuffer);
            if (!jsonObj->success()) {
              Serial.println("Parsing JSON object failed");
            } else {
              Preset newPreset;
              jsonToPreset(*jsonObj, newPreset);
              addToVector(*presetArray, newPreset, i);
            }
            jsonBuffer.clear();  
          }
          file.close();
        }
      }
  }
  digitalWrite(SDCARD_CS,1);
  
  tft.begin();
  tft.setRotation(3); // left-handed
  //tft.setRotation(1);  // right-handed
  Serial.println(String("Height is ") + tft.height() + String(", width is ") + tft.width());

    // Setup the Controls.
  controls.addRotary(19, 22, SWAP, 3);
  controls.addSwitch(23, 10);
  controls.addTouch(STMPE_CS, STMPE_IRQ, tft.height(), tft.width());
  //controls.touchFlipAxis(true, true);
  
  Serial.println("FINISHED: setup()");

  delay(100);

//  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
//  Serial.print("\nDisplay Power Mode: 0x"); Serial.println(x, HEX);
//  x = tft.readcommand8(ILI9341_RDMADCTL);
//  Serial.print("\nMADCTL Mode: 0x"); Serial.println(x, HEX);
//  x = tft.readcommand8(ILI9341_RDPIXFMT);
//  Serial.print("\nPixel Format: 0x"); Serial.println(x, HEX);
//  x = tft.readcommand8(ILI9341_RDIMGFMT);
//  Serial.print("\nImage Format: 0x"); Serial.println(x, HEX);
//  x = tft.readcommand8(ILI9341_RDSELFDIAG);
//  Serial.print("\nSelf Diagnostic: 0x"); Serial.println(x, HEX);
//  x = tft.readcommand8(0x4);
//  Serial.print("\nID 0: 0x"); Serial.println(x, HEX);
//  x = tft.readcommand8(0x4,0);
//  Serial.print("\nID 0: 0x"); Serial.println(x, HEX);
//  x = tft.readcommand8(0x4,1);
//  Serial.print("\nID 1: 0x"); Serial.println(x, HEX);
//  x = tft.readcommand8(0x4,2);
//  Serial.print("\nID 2: 0x"); Serial.println(x, HEX);
  

  nextScreen = Screens::PRESET_NAVIGATION;

  file = SD.open(calibFilename);
  if (file) {
    TouchCalibration touchCalib;
    file.read(reinterpret_cast<uint8_t*>(&touchCalib), sizeof(touchCalib));
    file.close();
    //controls.setCalib(touchCalib);
    controls.setCalib(410,3900,300,3800);
    Serial.println("Calibration data loaded");
  } else {
    Serial.println("Failed to load calibration data");
    nextScreen = TouchCalib(tft, controls);
  }

  Serial.println("Launching MIDI thread");
  threads.addThread(processMidi,midiPortPtr);

//  while (true) {
//    StringEdit(tft, (*presetArray)[0].name, knob0, sw0);
//  }

}


void loop()
{

  while(true) {
    switch(nextScreen) {
      case Screens::PRESET_NAVIGATION :
        g_currentScreen = nextScreen;
        nextScreen = DrawPresetNavigation(tft, controls, (*presetArray), *midiPortPtr, activePreset, selectedPreset);
        break;
      case Screens::PRESET_CONTROL :
        g_currentScreen = nextScreen;
        nextScreen = DrawPresetControl(tft, controls, (*presetArray)[activePreset], *midiPortPtr);
        break;
      case Screens::TOUCH_CALIBRATE :
        g_currentScreen = nextScreen;
        nextScreen = TouchCalib(tft, controls);
        {
          TouchCalibration touchCalib = controls.getCalib();        
          file = SD.open(calibFilename, FILE_WRITE);
          if (file) {
            file.write(reinterpret_cast<uint8_t*>(&touchCalib), sizeof(touchCalib));
            file.close();
            Serial.println("Calibration data saved");
          } else {
            Serial.println("Failed to save calib data");
          }
        }
        break;
      case Screens::MIDI_MONITOR :
        g_currentScreen = nextScreen;
        nextScreen = DrawMidiMonitor(tft, controls, (*presetArray)[activePreset], *midiPortPtr);
      default:
        g_currentScreen = nextScreen;
        nextScreen = DrawPresetNavigation(tft, controls, (*presetArray), *midiPortPtr, activePreset, selectedPreset);
    }
      
  }     
  delay(100);

}

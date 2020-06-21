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
//#define SDCARD_CS 3

/////////////////////
// Serial Flash
/////////////////////
//#define SERIALFLASH_CS 8

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

midi::MidiInterface<HardwareSerial> *midiPortPtr = nullptr;

PresetArray *presetArray = nullptr;
//unsigned activePreset = 0;
//unsigned selectedPreset = 0;
bool serialFlashPresent = false;
bool sdCardPresent = false;

void setup(void) {

  delay(1000);
  Serial.begin(9600);
  if (!Serial) { delay(100); yield(); }
  delay(1000);
  pinMode(23,INPUT);

  // Setup MIDI OUT
  midiPortPtr = new midi::MidiInterface<HardwareSerial>((HardwareSerial&)Serial1);
  midiPortPtr->begin(MIDI_CHANNEL_OMNI);
  midiPortPtr->turnThruOff();

  pinMode(SERIALFLASH_CS, OUTPUT);
  digitalWrite(SERIALFLASH_CS, 1); // disable the Serial Flash
  pinMode(SDCARD_CS, OUTPUT);
  digitalWrite(SDCARD_CS, 1); // disable the SD CARD
  pinMode(STMPE_CS, OUTPUT);
  digitalWrite(STMPE_CS, 1); // disable the Touch interface
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, 1); // disable the TFT interface

  Serial.println("Creating Preset Array");
  presetArray = createPresetArray();
  setActivePreset(&((*presetArray)[0]));

  // Setup the other chip selects
  setOtherChipSelects(TFT_CS, STMPE_CS);
  disableAllChipSelects();

  // Setup the TFT
  tft.begin();
  tft.setRotation(3); // left-handed
  //tft.setRotation(1);  // right-handed
  Serial.println(String("Height is ") + tft.height() + String(", width is ") + tft.width());

  // Setup the Controls.
  controls.addRotary(19, 22, SWAP, 3);
  controls.addSwitch(23, 10);
  controls.addTouch(STMPE_CS, STMPE_IRQ, tft.height(), tft.width());
  //controls.touchFlipAxis(true, true);

  // Setup the SerialFlash
  serialFlashPresent = initSerialFlash(SERIALFLASH_CS);
  if (!serialFlashPresent) {
    Serial.println("SerialFlash init seems to fail");
  } else {
    Serial.println("SerialFlash detected");
  }
  disableSerialFlashChipSelect(); // disable the Serial Flash

  // Setup the SD card
  sdCardPresent = initSdCard(SDCARD_CS);
  if (!sdCardPresent) {
    Serial.println("No SD card detect");
  } else { Serial.println("SD Card detected"); } 

  // Read the presets
  if (sdCardPresent) { 
    setStorageType(StorageType::SD_CARD);
    Serial. println("Reading presets from SD");
  } else if (serialFlashPresent) {
    setStorageType(StorageType::FLASH);
    Serial.println("Reading presets from flash"); 
  }
  loadConfig();
  readPresetFromFile(presetArray, getActiveSetlist());
  disableSdCardChipSelect(); // Disable the SDCard
  
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
  bool isCalibRead = readCalib(controls);

  if (!isCalibRead) {
    Serial.println("Failed to load calibration data");
    nextScreen = TouchCalib(tft, controls);
  }

  Serial.println("Launching threads");
  threads.addThread(processMidi,midiPortPtr);
  threads.addThread(processControls, &controls);

}


void loop()
{

  while(true) {
    switch(nextScreen) {
      case Screens::PRESET_NAVIGATION :
        g_currentScreen = nextScreen;
        nextScreen = DrawPresetNavigation(tft, controls, (*presetArray), *midiPortPtr);
        break;
      case Screens::PRESET_CONTROL :
        g_currentScreen = nextScreen;
        nextScreen = DrawPresetControl(tft, controls, getActivePreset(), *midiPortPtr);
        break;
      case Screens::TOUCH_CALIBRATE :
        g_currentScreen = nextScreen;
        nextScreen = TouchCalib(tft, controls);
        {
          writeCalib(controls);          
        }
        break;
      case Screens::MIDI_MONITOR :
        g_currentScreen = nextScreen;
        nextScreen = DrawMidiMonitor(tft, controls, *midiPortPtr);
        break;
      case Screens::UTILITIES :
        g_currentScreen = nextScreen;
        nextScreen = DrawUtilities(tft, controls, *presetArray);
        break;
      case Screens::SETLIST :
        g_currentScreen = nextScreen;
        nextScreen = DrawSetlist(tft, controls, *presetArray);
        break;
      default:
        g_currentScreen = nextScreen;
        nextScreen = DrawPresetNavigation(tft, controls, (*presetArray), *midiPortPtr);
    }
      
  }     
  delay(100);

}

/***************************************************
  This is our touchscreen painting example for the Adafruit ILI9341 Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/


#include <SPI.h>
#include <Wire.h>      // this is needed even tho we aren't using it
#include <ILI9341_t3.h>
#include <XPT2046_Touchscreen.h>
#include <SD.h>
#include <SPI.h>

//#include <Adafruit_GFX.h>
#include "ArduinoJson.h"
#include "Bounce.h"
#include "Encoder.h"

#include "Screens.h"
#include "Preset.h"

/////////////////////
// SD CARD
/////////////////////
File file;
#define SDCARD_CS 3

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

// The STMPE610 uses hardware SPI on the shield, and #8
#define STMPE_CS 6
//Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);
XPT2046_Touchscreen ts(STMPE_CS, 2);

// The display also uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC  9
ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

bool needUpdate = true;
Bounce sw0 = Bounce(23, 10);
Encoder knob0 = Encoder(19, 22);

char jsonTextBuffer[1024];
StaticJsonBuffer<1024> jsonBuffer;
JsonObject *jsonObj;

constexpr unsigned MAX_PRESETS = 32;
constexpr unsigned PRESET_ID_INDEX = 6;
char presetFilename[] = "PRESET0.JSN";
Preset presetArray[MAX_PRESETS];

void setup(void) {

  Serial.begin(57600);
  while (!Serial) {}
  pinMode(23,INPUT);

  pinMode(SDCARD_CS, OUTPUT);
  digitalWrite(SDCARD_CS, 1); // disable the SD CARD
  pinMode(STMPE_CS, OUTPUT);
  digitalWrite(STMPE_CS, 1); // disable the Touch interface
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, 1); // disable the TFT interface

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
                jsonToPreset(*jsonObj, presetArray[i]);
              }
          }
          
        }
      }
  }
  digitalWrite(SDCARD_CS,1);
//  Serial.end();
//  Serial.begin(57600);
//  while (!Serial) {}
  
  tft.begin();
  tft.setRotation(3);

  if (!ts.begin()) {
    Serial.println("Couldn't start touchscreen controller");
    while (1);
  }
  Serial.println("Touchscreen started");
  Serial.println(String("Height is ") + tft.height() + String(", width is ") + tft.width());
  
  tft.fillScreen(ILI9341_BLACK);
  
  Serial.println("FINISHED: setup()");

}


void loop()
{
  // See if there's any  touch data for us
  if (ts.bufferEmpty()) {
    return;
  }
  /*
  // You can also wait for a touch
  if (! ts.touched()) {
    return;
  }
  */

  // Retrieve a point  
  TS_Point p = ts.getPoint();
  TS_Point tmp = p;
  p.x = tmp.y;
  p.y = TS_MAXY - tmp.x;
  

  if (ts.touched()) {
  Serial.print("X = "); Serial.print(p.x);
  Serial.print("\tY = "); Serial.print(p.y);
  Serial.print("\tPressure = "); Serial.println(p.z);  
  }
 
 
  // Scale from ~0->4000 to tft.width using the calibration #'s
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

  /*
  Serial.print("("); Serial.print(p.x);
  Serial.print(", "); Serial.print(p.y);
  Serial.println(")");
  */

  if (sw0.update()) {
    if (sw0.fallingEdge()) {
      //DrawPresetNavigation(tft);
      Serial.println("Button pressed");
      //jsonToPreset(*jsonObj, preset);
      Serial.println("Printing preset");
      PrintPreset(tft, presetArray[1]);
    }
  }
  delay(100);

}

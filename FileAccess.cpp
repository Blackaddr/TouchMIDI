/*
 * FileAccess.cpp
 *
 *  Created on: Mar. 3, 2019
 *      Author: blackaddr
 */
#include <cstring>
#include <SD.h>
#include <SerialFlash.h>
#include "ArduinoJson.h"


#include "Misc.h"
#include "Controls.h"
#include "FileAccess.h"

static StorageType g_storageType = StorageType::SD_CARD;

constexpr size_t   MIN_PRESET_SIZE = 2048;
constexpr unsigned PRESET_ID_INDEX = 6;
const char calibFilename[] = "TCALIB.BIN";

static int g_sdCardChipSelect      = -1;
static int g_serialFlashChipSelect = -1;
static int g_tftChipSelect         = -1;
static int g_touchChipSelect       = -1;

void setStorageType(StorageType storageType)
{
    g_storageType = storageType;
}

StorageType getStorageType(void) { return g_storageType; }

void setOtherChipSelects(unsigned tftChipSelect, unsigned touchChipSelect)
{
    g_tftChipSelect   = tftChipSelect;
    g_touchChipSelect = touchChipSelect;
}

void disableAllChipSelects(void)
{
    if (g_sdCardChipSelect >= 0)      { digitalWrite(g_sdCardChipSelect, 1); }
    if (g_serialFlashChipSelect >= 0) { digitalWrite(g_serialFlashChipSelect, 1); }
    if (g_tftChipSelect >= 0)         { digitalWrite(g_tftChipSelect, 1); }
    if (g_touchChipSelect >= 0)       { digitalWrite(g_touchChipSelect, 1); }
}

bool compareFiles(File &sdFile, SerialFlashFile &flashFile);
const char* getFilenameExt(const char *filename);

bool initSdCard(unsigned chipSelect) {
    bool status =  SD.begin(chipSelect);
    g_sdCardChipSelect = chipSelect;
    disableSdCardChipSelect();
    return status;
}

void disableSdCardChipSelect(void) {
    digitalWrite(g_sdCardChipSelect, 1); // disable the chip select
}

bool initSerialFlash(unsigned chipSelect) {
    g_serialFlashChipSelect = chipSelect;
    bool status = SerialFlash.begin(chipSelect);
    disableSerialFlashChipSelect();
    return status;
}

void disableSerialFlashChipSelect(void)
{
    digitalWrite(g_serialFlashChipSelect, 1); // disable the chip select
}


////////////////////////////////////////////////////////////////////////////////
// CALIBRATION DATA
////////////////////////////////////////////////////////////////////////////////
bool readCalibFromSd(Controls& controls)
{
    bool status = false;
    File file = SD.open(calibFilename);
    if (file) {
      TouchCalibration touchCalib;
      file.read(reinterpret_cast<uint8_t*>(&touchCalib), sizeof(touchCalib));
      file.close();
      //controls.setCalib(touchCalib);
      controls.setCalib(410,3900,300,3800);
      status = true;
      Serial.println("Calibration data loaded");
    }
    return status;
}

bool readCalibFromFlash(Controls& controls)
{
    bool status = false;
    SerialFlashFile file = SerialFlash.open(calibFilename);
    if (file) {
      TouchCalibration touchCalib;
      file.read(reinterpret_cast<uint8_t*>(&touchCalib), sizeof(touchCalib));
      file.close();
      //controls.setCalib(touchCalib);
      controls.setCalib(410,3900,300,3800);
      status = true;
      Serial.println("Calibration data loaded");
    }
    return status;
}

bool writeCalibToSd(Controls& controls)
{
    bool status = true;
    File file = SD.open(calibFilename, FILE_WRITE);
    TouchCalibration touchCalib = controls.getCalib();
    if (file) {
        file.write(reinterpret_cast<uint8_t*>(&touchCalib), sizeof(touchCalib));
        file.close();
        Serial.println("Calibration data saved");
    } else {
        status = false;
        Serial.println("Failed to save calib data");
    }
    return status;
}

bool writeCalibToFlash(Controls& controls)
{
    bool status = true;
    SerialFlashFile file = SerialFlash.open(calibFilename);
    TouchCalibration touchCalib = controls.getCalib();
    if (file) {
        file.write(reinterpret_cast<uint8_t*>(&touchCalib), sizeof(touchCalib));
        file.close();
        Serial.println("Calibration data saved");
    } else {
        status = false;
        Serial.println("Failed to save calib data");
    }
    return status;
}

bool readCalib(Controls& controls) {
    if (getStorageType() == StorageType::SD_CARD) {
        return readCalibFromSd(controls);
    } else if (getStorageType() == StorageType::FLASH) {
        return readCalibFromFlash(controls);
    }
    return false;
}

bool writeCalib(Controls& controls) {
    if (getStorageType() == StorageType::SD_CARD) {
        return writeCalibToSd(controls);
    } else if (getStorageType() == StorageType::FLASH) {
        return writeCalibToFlash(controls);
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// PRESET DATA
////////////////////////////////////////////////////////////////////////////////
bool readPresetFromFlash(PresetArray* presetArray)
{
    constexpr size_t JSON_BUFFER_SIZE = 1024;
    char jsonTextBuffer[JSON_BUFFER_SIZE];
    StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
    JsonObject *jsonObj;
    char presetFilename[] = "PRESET0.JSN";
    SerialFlashFile file;

    for (unsigned i=0; i<MAX_PRESETS; i++) {
        presetFilename[PRESET_ID_INDEX] = i + 0x30;
        file = SerialFlash.open(presetFilename);
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
    return true;
}

bool readPresetFromSd(PresetArray* presetArray)
{
    constexpr size_t JSON_BUFFER_SIZE = 1024;
    char jsonTextBuffer[JSON_BUFFER_SIZE];
    StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
    JsonObject *jsonObj;
    char presetFilename[] = "PRESET0.JSN";
    File file;

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
    return true;
}

bool readPresetFromFile(PresetArray* presetArray) {
    if (getStorageType() == StorageType::SD_CARD) {
        return readPresetFromSd(presetArray);
    } else if (getStorageType() == StorageType::FLASH) {
        return readPresetFromFlash(presetArray);
    }
    return false;
}

void writePresetToSd(const char *filename, JsonObject &jsonObject)
{
    // create  buffer to hold the contexts
    constexpr int BUFFER_SIZE = MIN_PRESET_SIZE;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    //serializeJson(jsonObject, buffer);
    jsonObject.prettyPrintTo(buffer);
    unsigned length = strlen(buffer);
    if ((length < 1) || (length > BUFFER_SIZE) ) { return; }

    // Check if file exists
    if (SD.exists(filename)) {
        // file already exists
        SD.remove(filename);
    }
    File file = SD.open(filename, O_WRITE | O_CREAT); // was FILE_WRITE
    if (!file) {
        Serial.println(String("writePresetToFille(): ERROR cannot create ") + filename);
        return;
    }
    file.write(buffer, length);
    file.close();
}

void writePresetToFlash(const char *filename, JsonObject &jsonObject)
{
    // create  buffer to hold the contexts
    constexpr int BUFFER_SIZE = MIN_PRESET_SIZE;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    //serializeJson(jsonObject, buffer);
    jsonObject.prettyPrintTo(buffer);
    unsigned length = strlen(buffer);
    if ((length < 1) || (length > BUFFER_SIZE) ) { return; }

    // Check if file exists
    if (SerialFlash.exists(filename)) {
        // file already exists
        SerialFlash.remove(filename);
    }
    SerialFlash.create(filename, length);
    SerialFlashFile file = SerialFlash.open(filename);
    if (!file) {
        Serial.println(String("writePresetToFille(): ERROR cannot create ") + filename);
        return;
    }
    file.write(buffer, length);
    file.close();
}

void writePresetToFile(const char *filename, JsonObject &jsonObject) {
    if (getStorageType() == StorageType::SD_CARD) {
        return writePresetToSd(filename, jsonObject);
    } else if (getStorageType() == StorageType::FLASH) {
        return writePresetToFlash(filename, jsonObject);
    }
}

//void writePresetToFile(const char *filename, JsonObject &jsonObject)
//{
//    // create  buffer to hold the contexts
//    constexpr int BUFFER_SIZE = MIN_PRESET_SIZE;
//    char buffer[BUFFER_SIZE];
//    memset(buffer, 0, BUFFER_SIZE);
//    //serializeJson(jsonObject, buffer);
//    jsonObject.prettyPrintTo(buffer);
//    unsigned length = strlen(buffer);
//    if ((length < 1) || (length > BUFFER_SIZE) ) { return; }
//
//    if (getStorageType() == StorageType::SD_CARD) {
//
//    } else if (getStorageType() == StorageType::FLASH) {
//
//    }
//}

void copyFileIfDifferentToFlash(File f, const char* filename)
{
    if (f) {
        // Check if this file already exists on the flash
        if (SerialFlash.exists(filename)) { // exists on flash
            SerialFlashFile ff = SerialFlash.open(filename);
            if (ff && ff.size() == f.size()) { // same size
                if (compareFiles(f, ff) == true) {
                    // files are identical
                    Serial.printf("Skipping %s\n", filename);
                    f.close();
                    ff.close();
                    return;
                }
            }
        }
        // File is not same size or not identical
        SerialFlash.remove(filename); // remove the old file
        size_t length = f.size();

        // create the file on the Flash chip and copy data
        if (SerialFlash.create(filename, length)) {
            SerialFlashFile ff = SerialFlash.open(filename);
            if (ff) {
                //Serial.print("  copying");
                // copy data loop
                Serial.printf("Copying %s\n", filename);
                size_t count = 0;
                while (count < length) {
                    char buf[256];
                    unsigned int n;
                    n = f.read(buf, 256);
                    ff.write(buf, n);
                    count = count + n;
                }
                ff.close();
            } else {
                Serial.println("  error opening freshly created file!");
            }
        } else {
            Serial.println("  unable to create file");
        }
        f.close();
    }
}

void copySdToFlash(void) {
    char presetFilename[] = "PRESET0.JSN";
    File sdRootdir = SD.open("/"); // Open the SD card
    File file;

    // Copy all BMP files
    while(true) {
        file = sdRootdir.openNextFile();
        if (!file) { break; }
        const char* filename = file.name();
        const char* ext = getFilenameExt(filename);

        if ((strcmp(ext,"BMP") == 0) || (strcmp(ext,"bmp") == 0)) {
            copyFileIfDifferentToFlash(file, filename);
        }
    }

    // Copy the calibration file if exits
    {
        file = SD.open(calibFilename);
        if (file) {
            copyFileIfDifferentToFlash(file, file.name());
        } else {
            Serial.printf("%s not found on SD card\n", file.name());
        }
    }

    // Copy the preset files
    for (unsigned i=0; i<MAX_PRESETS; i++) {
        presetFilename[PRESET_ID_INDEX] = i + 0x30;
        Serial.printf("Checking for %s\n", presetFilename);
        File f = SD.open(presetFilename);

        if (f) {
            copyFileIfDifferentToFlash(file, presetFilename);
        }
    } // end for loop
    sdRootdir.close();
}

//void copyFlashToSd(void) {
//    char presetFilename[] = "PRESET0.JSN";
//    SerialFlashFile rootdir = .open("/"); // Open the SD card
//    File file;
//
//    // Copy all BMP files
//    while(true) {
//        file = sdRootdir.openNextFile();
//        if (!file) { break; }
//        const char* filename = file.name();
//        const char* ext = getFilenameExt(filename);
//
//        if ((strcmp(ext,"BMP") == 0) || (strcmp(ext,"bmp") == 0)) {
//            copyFileIfDifferentToFlash(file, filename);
//        }
//    }
//
//    // Copy the calibration file if exits
//    {
//        file = SD.open(calibFilename);
//        if (file) {
//            copyFileIfDifferentToFlash(file, file.name());
//        } else {
//            Serial.printf("%s not found on SD card\n", file.name());
//        }
//    }
//
//    // Copy the preset files
//    for (unsigned i=0; i<MAX_PRESETS; i++) {
//        presetFilename[PRESET_ID_INDEX] = i + 0x30;
//        Serial.printf("Checking for %s\n", presetFilename);
//        File f = SD.open(presetFilename);
//
//        if (f) {
//            copyFileIfDifferentToFlash(file, presetFilename);
//        }
//    } // end for loop
//    sdRootdir.close();
//}

const char *getFilenameExt(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

bool compareFiles(File &sdFile, SerialFlashFile &flashFile) {
    sdFile.seek(0);
    flashFile.seek(0);
  unsigned long count = sdFile.size();
  while (count > 0) {
    char buf1[128], buf2[128];
    unsigned long n = count;
    if (n > 128) n = 128;
    sdFile.read(buf1, n);
    flashFile.read(buf2, n);
    if (memcmp(buf1, buf2, n) != 0) return false; // differ
    count = count - n;
  }
  return true;  // all data identical
}

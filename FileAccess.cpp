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
#include "filePaths.h"
#include "FileAccess.h"

static StorageType g_storageType = StorageType::SD_CARD;


constexpr size_t   MIN_PRESET_SIZE  = 2048;
constexpr unsigned PRESET_ID_INDEX  = 6;

static bool g_isSdCardInit          = false;
static int  g_sdCardChipSelect      = -1;
static int  g_serialFlashChipSelect = -1;
static int  g_tftChipSelect         = -1;
static int  g_touchChipSelect       = -1;

SetlistArray g_setlistArray;

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
    if (!g_isSdCardInit) {
        bool status =  SD.begin(chipSelect);
        g_sdCardChipSelect = chipSelect;
        disableSdCardChipSelect();
        g_isSdCardInit = status;
        return status;
    } else { return g_isSdCardInit; }
}

bool isSdCardnit() { return g_isSdCardInit; }

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
      Serial.printf("Calibration data loaded from %s\n", calibFilename);
    } else {
        Serial.printf("Failed to load calibration data from SD at %s\n", calibFilename);
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
      Serial.printf("Calibration data loaded from %s\n", calibFilename);
    } else {
        Serial.printf("Failed to load calibration data from FLASH at %s\n", calibFilename);
    }
    return status;
}

bool writeCalibToSd(Controls& controls)
{
    bool status = true;
    File file = SD.open(calibFilename, O_WRITE | O_CREAT);
    TouchCalibration touchCalib = controls.getCalib();
    if (file) {
        file.write(reinterpret_cast<uint8_t*>(&touchCalib), sizeof(touchCalib));
        file.close();
        Serial.printf("Calibration data saved to %s\n", calibFilename);
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
        Serial.printf("Calibration data saved to %s\n", calibFilename);
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
bool readPresetFromFlash(PresetArray* presetArray, const char* setlistName)
{
    constexpr size_t JSON_BUFFER_SIZE = 1024;
    char jsonTextBuffer[JSON_BUFFER_SIZE];
    StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
    JsonObject *jsonObj;
    char presetFilename[MAX_FILENAME_CHARS] = "";
    SerialFlashFile file;

    for (unsigned i=0; i<MAX_PRESETS; i++) {
        createPresetFilename(i, setlistName, presetFilename);
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

bool readPresetFromSd(PresetArray* presetArray, const char* setlistName)
{
    constexpr size_t JSON_BUFFER_SIZE = 1024;
    char jsonTextBuffer[JSON_BUFFER_SIZE];
    StaticJsonBuffer<JSON_BUFFER_SIZE> jsonBuffer;
    JsonObject *jsonObj;
    char presetFilename[MAX_FILENAME_CHARS] = "";
    File file;

    for (unsigned i=0; i<MAX_PRESETS; i++) {
        createPresetFilename(i, setlistName, presetFilename);
        file = SD.open(presetFilename);
        if (!file) {
          Serial.println(String("Can't open ") + presetFilename);
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

bool readPresetFromFile(PresetArray* presetArray, const char* setlistName) {
    if (getStorageType() == StorageType::SD_CARD) {
        return readPresetFromSd(presetArray, setlistName);
    } else if (getStorageType() == StorageType::FLASH) {
        return readPresetFromFlash(presetArray, setlistName);
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

void writePresetToFile(unsigned presetIndex, const char* setlistName, JsonObject &jsonObject) {

    char presetFilename[MAX_FILENAME_CHARS] = "";
    createPresetFilename(presetIndex, setlistName, presetFilename);

    if (getStorageType() == StorageType::SD_CARD) {
        return writePresetToSd(presetFilename, jsonObject);
    } else if (getStorageType() == StorageType::FLASH) {
        return writePresetToFlash(presetFilename, jsonObject);
    }
}


void copyFileIfDifferentToFlash(File f, const char* filename)
{
    Serial.printf("copyFileIfDifferentToFlash(): processing %s\n", filename);
    if (f) {
        size_t sdFileLength = f.size();
        Serial.printf("File size is %d\n", sdFileLength);

        // Check if this file already exists on the flash
        if (SerialFlash.exists(filename)) { // exists on flash
            Serial.printf("File exists on flash\n", sdFileLength);
            SerialFlashFile ff = SerialFlash.open(filename);
            if (ff) {
                if (ff.size() == sdFileLength) { // same size
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
        }
        Serial.printf("Creating the new file\n");

        // create the file on the Flash chip and copy data
        if (SerialFlash.create(filename, sdFileLength)) {
            SerialFlashFile ff = SerialFlash.open(filename);
            if (ff) {
                // copy data loop
                Serial.printf("Copying %s\n", filename);
                size_t count = 0;
                while (count < sdFileLength) {
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

void copyFileIfDifferentToSd(SerialFlashFile ff, const char* filename)
{
    Serial.printf("copyFileIfDifferentToSd(): processing %s\n", filename);
    if (ff) {
        // Check if this file already exists on the flash
        if (SD.exists(filename)) { // exists on SD
            File f = SD.open(filename);
            if (f && f.size() == ff.size()) { // same size
                if (compareFiles(f, ff) == true) {
                    // files are identical
                    Serial.printf("Skipping %s\n", filename);
                    f.close();
                    ff.close();
                    return;
                }
            }
            f.close();
        }
        // File is not same size or not identical
        ff.seek(0); // rewind the flash file
        SD.remove(filename); // remove the old file
        size_t length = ff.size();

        // create the file on the SD chip and copy data
        File f = SD.open(filename, O_WRITE | O_CREAT);
        if (f) {
            // copy data loop
            Serial.printf("Copying %s, size %d\n", filename, length);
            int remaining = length;
            while (remaining > 0) {
                char buf[256];
                unsigned n = ff.read(buf, 256);
                f.write(buf, n);
                remaining -= n;
            }
            f.close();

        } else {
            Serial.println("  unable to create file");
        }
        ff.close();
    }
}

// Note the the Flash filesystem is really just file paths using / to mimic
// a directory structure, but there isn't one really.
void copySdToFlash(void) {

    if (!initSdCard(SDCARD_CS)) {
        Serial.println("Cannot initialize SD");
        return;
    }

    char presetFilename[MAX_FILENAME_CHARS] = "";
    File sdRootdir = SD.open(DATA_PATH); // Open the SD card
    File file;

    // Copy all BMP files
    while(true) {
        file = sdRootdir.openNextFile();
        if (!file) { break; }
        const char* filename = file.name();
        const char* ext = getFilenameExt(filename);

        if ((strcmp(ext,"BMP") == 0) || (strcmp(ext,"bmp") == 0)) {
            char fullPathName[MAX_FILENAME_CHARS] = "";
            strncat(fullPathName, DATA_PATH, strlen(DATA_PATH)+1);
            strncat(fullPathName, filename, strlen(filename));
            copyFileIfDifferentToFlash(file, fullPathName);
        }
    }

    // Copy the calibration file if exits
    {
        if (SD.exists(calibFilename)) {
            file = SD.open(calibFilename);
            if (file) {
                copyFileIfDifferentToFlash(file, calibFilename);
            }
        } else {
            Serial.printf("%s not found on SD card\n", file.name());
        }

    }

    // Copy the preset files
    for (unsigned i=0; i<MAX_PRESETS; i++) {
        //createPresetFilename(i, getActiveSetlist(), presetFilename); // TODO, this needs to step through ALL setlists
        SetlistArray& setListList = getSetlistList();

        for (auto it = setListList.begin(); it != setListList.end(); ++it) {
            createPresetFilename(i, (*it).c_str(), presetFilename);
            Serial.printf("Checking for %s\n", presetFilename);
            if (SD.exists(presetFilename)) {
                File f = SD.open(presetFilename);
                if (f) {
                    copyFileIfDifferentToFlash(f, presetFilename);
                }
            }
        }
    } // end for loop
    sdRootdir.close();
}

void copyFlashToSd(void) {
    if (!initSdCard(SDCARD_CS)) {
        Serial.println("Cannot initialize SD");
        return;
    }
    Serial.printf("copyFlashtoSd(): running...\n");

    // First ensure the directories exist on the SD card.
    if (!SD.exists(DATA_PATH)) {
        Serial.printf("Creating dir %s\n", DATA_PATH);
        SD.mkdir(DATA_PATH);
    }
    if (!SD.exists(PRESETS_DIR)) {
        Serial.printf("Creating dir %s\n", PRESETS_DIR);
        SD.mkdir(PRESETS_DIR);
    }

    char presetFilename[MAX_FILENAME_CHARS] = "";
    char filename[MAX_FILENAME_CHARS];
    uint32_t fileSize = 0;
    SerialFlash.opendir();
    SerialFlashFile file;

    // Copy all BMP files
    while(true) {
        bool fileFound = SerialFlash.readdir(filename, sizeof(filename), fileSize);
        if (!fileFound) { break; }
        file = SerialFlash.open(filename);
        const char* ext = getFilenameExt(filename);
        Serial.printf("copyFlashtoSd(): found %s\n", filename);

        if ((strcmp(ext,"BMP") == 0) || (strcmp(ext,"bmp") == 0)) {
            copyFileIfDifferentToSd(file, filename);
        }
        file.close();
    }

    // Copy the calibration file if exits
    {
        file = SerialFlash.open(calibFilename);
        if (file) {
            copyFileIfDifferentToSd(file, calibFilename);
            file.close();
        } else {
            Serial.printf("%s not found on Flash\n", calibFilename);
        }
    }

    // Copy the preset files
    for (unsigned i=0; i<MAX_PRESETS; i++) {
        createPresetFilename(i, getActiveSetlist(), presetFilename); // TODO this needs to copy ALL active setlists
        file = SerialFlash.open(presetFilename);

        if (file) {
            copyFileIfDifferentToSd(file, presetFilename);
            file.close();
        }
    } // end for loop
}

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

void createPresetFilename(unsigned presetNumber, const char* setlistName, char* filenameString)
{
    strncpy(filenameString, PRESETS_DIR, strlen(PRESETS_DIR)+1); // copy the preset dir name
    if (setlistName) {
        strncat(filenameString, setlistName, strlen(setlistName)+1); // copy in the setlist name (a subdir)
        strncat(filenameString, "/", 2);
    }
    strncat(filenameString, "PRESET", 7); // copy int the filename
    char presetNum[3] = "";
    itoa(presetNumber, presetNum, 10);
    strncat(filenameString, presetNum, 3);
    strncat(filenameString, ".JSN", 4); // add the extension
}

void getSetlistNameFromFullPath(char* fullPathName, char* setlistName)
{

    setlistName[0] = '\0';
    char* token = strtok(fullPathName, "/");
    //Serial.printf("The first token is %s\n", token);
    // Check if the first directory is "PRESET"
    if (strncmp(token, "PRESETS", 7) == 0) {
        token = strtok(NULL, "/");
        if (strlen(token) > 0 ) {
            //Serial.printf("getSetlistNameFromPath(): The second token is %s\n", token);
            strncpy(setlistName, token, MAX_FILENAME_CHARS);
        }
    }
}

SetlistArray& getSetlistListFromSd()
{
    File presetDir = SD.open(PRESETS_DIR); // Open the SD card
    File file;

    // First empty the array
    while(!g_setlistArray.empty()) {
        g_setlistArray.pop_back();
    }

    // Read all directory names
    while(true) {
        file = presetDir.openNextFile();
        if (!file) { break; }
        const char* filename = file.name();

        if (file.isDirectory()) {
            //Serial.printf("Found setlist %s\n", filename);
            g_setlistArray.emplace_back(String(filename));
        }
    }
    return g_setlistArray;
}

SetlistArray& getSetlistListFromFlash()
{
    char fullPathName[MAX_FILENAME_CHARS] = "";
    char setlistName[MAX_FILENAME_CHARS] = "";
    uint32_t fileSize = 0;
    SerialFlash.opendir(); // resets back to the first entry

    // First empty the array
    while(!g_setlistArray.empty()) {
        g_setlistArray.pop_back();
    }

    while(true) {
        bool fileFound = SerialFlash.readdir(fullPathName, sizeof(fullPathName), fileSize);
        getSetlistNameFromFullPath(fullPathName, setlistName);
        if(strlen(setlistName) > 0) {
            // check if it already exists in the setlist array
            bool foundName = false;
            for (auto it = g_setlistArray.begin(); it != g_setlistArray.end(); ++it) {
                if (strncmp((*it).c_str(), setlistName, MAX_FILENAME_CHARS) == 0) {
                    foundName = true;
                    break;
                }
            }
            if (!foundName) {
                g_setlistArray.emplace_back(String(setlistName));
            }
        }
        if (!fileFound) { break; }
    }

    return g_setlistArray;
}

SetlistArray& getSetlistList()
{
    if (getStorageType() == StorageType::SD_CARD) {
        return getSetlistListFromSd();
    } else if (getStorageType() == StorageType::FLASH) {
        return getSetlistListFromFlash();
    } else { SetlistArray setlistArray; return g_setlistArray; } // return empty array
}

// NOT TESTED
void createNewPresetSd(const char* presetName) {
    if (!initSdCard(SDCARD_CS)) {
        Serial.println("createNewPresetSd(): Cannot initialize SD");
        return;
    }

    // Ensure the directory does not exist.
    char fullPathName[MAX_FILENAME_CHARS] = "";
    strncpy(fullPathName, PRESETS_DIR, MAX_FILENAME_CHARS);
    strncat(fullPathName, presetName, MAX_FILENAME_CHARS);

    if (!SD.exists(fullPathName)) {
        Serial.printf("Creating dir %s\n", fullPathName);
        SD.mkdir(fullPathName);
    }

}

// NOT TESTED
void createNewPresetFlash(const char* presetName) {

}

void createNewPreset(const char* presetName)
{
    if (getStorageType() == StorageType::SD_CARD) {
        return createNewPresetSd(presetName);
    } else if (getStorageType() == StorageType::FLASH) {
        return createNewPresetFlash(presetName);
    }
}


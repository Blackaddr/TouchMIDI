/*
 * FileAccess.h
 *
 *  Created on: Mar. 3, 2019
 *      Author: blackaddr
 */

#ifndef FILEACCESS_H_
#define FILEACCESS_H_

#include "SD.h" // This library is safe and doesn't leave CS asserted
#include <SerialFlash.h>
#include "ArduinoJson.h"

#include "filePaths.h"
#include "Preset.h"

#define SDCARD_CS 3
#define SERIALFLASH_CS 8

enum class StorageType {
    FLASH,
    SD_CARD
};

void setStorageType(StorageType storageType);
StorageType getStorageType(void);

void setOtherChipSelects(unsigned tftChipSelect, unsigned touchChipSelect);
void disableAllChipSelects(void);

bool initSdCard(unsigned chipSelect);
void disableSdCardChipSelect(void);

bool initSerialFlash(unsigned chipSelect);
void disableSerialFlashChipSelect(void);

bool readCalib (Controls& controls);
bool writeCalib(Controls& controls);

bool readPresetFromFile(PresetArray* presetArray);
void writePresetToFile (unsigned presetIndex, JsonObject &jsonObject);

void copySdToFlash(void);
void copyFlashToSd(void);

void createPresetFilename(unsigned presetNumber, char* filenameString);


#endif /* FILEACCESS_H_ */

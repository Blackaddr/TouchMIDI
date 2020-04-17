/*
 * FileAccess.h
 *
 *  Created on: Mar. 3, 2019
 *      Author: blackaddr
 */

#ifndef FILEACCESS_H_
#define FILEACCESS_H_

#include "SD.h"
#include <SerialFlash.h>
//#include "SdFs.h"
#include "ArduinoJson.h"

//extern SdFat SD;

bool initSerialFlash(unsigned chipSelect);
void writePresetToFile(const char *filename, JsonObject &jsonObject);


#endif /* FILEACCESS_H_ */

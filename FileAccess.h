/*
 * FileAccess.h
 *
 *  Created on: Mar. 3, 2019
 *      Author: blackaddr
 */

#ifndef FILEACCESS_H_
#define FILEACCESS_H_

#include "ArduinoJson.h"

void writePresetToFile(const char *filename, JsonObject &jsonObject);


#endif /* FILEACCESS_H_ */

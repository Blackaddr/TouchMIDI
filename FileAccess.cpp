/*
 * FileAccess.cpp
 *
 *  Created on: Mar. 3, 2019
 *      Author: blackaddr
 */
#include <cstring>
#include "FileAccess.h"

//SdFat SD;

void writePresetToFile(const char *filename, JsonObject &jsonObject)
{
    // create  buffer to hold the contexts
    constexpr int BUFFER_SIZE = 1024;
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
    //elapsedMillis timeElapsed = 0;
    file.write(buffer, length);
    //Serial.println(String("writePresetToFille():: successfully written bytes: ") + length + String(" millis: ") + timeElapsed);
    file.close();
}



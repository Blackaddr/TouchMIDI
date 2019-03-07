/*
 * Bitmap.h
 *
 *  Created on: Dec 14, 2018
 *      Author: blackaddr
 */
#include <cstdint>
#include "ILI9341_t3.h"
#include "FileAccess.h"

#ifndef __BITMAP_H
#define __BITMAP_H

// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.

#define BUFFPIXEL 20

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File &f);

uint32_t read32(File &f);

void bmpDraw(ILI9341_t3 &tft, const char *filename, uint8_t x, uint16_t y);



#endif /* SCH_BITMAP_H_ */


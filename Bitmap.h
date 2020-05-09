/*
 * Bitmap.h
 *
 *  Created on: Dec 14, 2018
 *      Author: blackaddr
 */
#include <cstdint>
#include "ILI9341_t3.h"

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

//#define bmpDraw bmpDrawFlash

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

//uint16_t read16(File &f);
//
//uint32_t read32(File &f);

void bmpDraw(ILI9341_t3 &tft, const char *filename, uint8_t x, uint16_t y);
void bmpDrawFlash(ILI9341_t3 &tft, const char *filename, uint8_t x, uint16_t y);
void bmpDrawSd(ILI9341_t3 &tft, const char *filename, uint8_t x, uint16_t y);

const char SAVE_ICON_PATH[]    = "/data/SAVE48.BMP";
const char BACK_ICON_PATH[]    = "/data/BACK48.BMP";
const char ADD_ICON_PATH[]     = "/data/ADD48.BMP";
const char REMOVE_ICON_PATH[]  = "/data/REMOVE48.BMP";
const char MOVEUP_ICON_PATH[]  = "/data/MOVEUP48.BMP";
const char MOVEDN_ICON_PATH[]  = "/data/MOVEDN48.BMP";
const char UTILS_ICON_PATH[]   = "/data/UTILS48.BMP";
const char EXTRA_ICON_PATH[]   = "/data/EXTRA48.BMP";
const char SETLIST_ICON_PATH[] = "/data/SETLST48.BMP";

#endif /* SCH_BITMAP_H_ */


#ifndef __GRAPHICS_H
#define __GRAPHICS_H

#include "Preset.h"
#include "ScreensUtil.h"

void fillArc(ILI9341_t3 &tft, int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour);
void drawKnob(ILI9341_t3 &tft, MidiControl &control, int16_t xPos, int16_t yPos);
void drawSwitch(ILI9341_t3 &tft, MidiControl &control, int16_t xPos, int16_t yPos);
void drawActiveControl(ILI9341_t3 &tft, int16_t xPos, int16_t yPos, uint16_t color);



#endif

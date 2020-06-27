#ifndef __SCREENSUTIL_H
#define __SCREENSUTIL_H

#include <cstdint>
#include "ILI9341_t3.h"

constexpr int16_t MARGIN = 10;
constexpr int DEFAULT_TEXT_SCALE=2;
constexpr int DEFAULT_TEXT_HEIGHT = 8*DEFAULT_TEXT_SCALE;
constexpr int SELECTED_TEXT_WIDTH = 230;
//constexpr int SELECTED_TEXT_NO_RIGHT_ICON_WIDTH = 280;

void printCentered(ILI9341_t3 &tft, const char *strIn);
void printCenteredJustified(ILI9341_t3 &tft, const char *strIn, int16_t xPos, int16_t yPos);
void printRightJustified(ILI9341_t3 &tft, const char *strIn, int16_t xPos, int16_t yPos);
void clearTextRightJustified(ILI9341_t3 &tft, const char *strIn, int16_t xPos, int16_t yPos);
void setCursorX(ILI9341_t3 &tft, int16_t xPos);
void setCursorY(ILI9341_t3 &tft, int16_t yPos);
void clearScreen(ILI9341_t3 &tft);

#endif

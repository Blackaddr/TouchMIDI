/*
 * ScreensUtil.cpp
 *
 *  Created on: Jan. 7, 2019
 *      Author: blackaddr
 */
#include "ScreensUtil.h"

void printCentered(ILI9341_t3 &tft, const char *strIn)
{
  int16_t x,y;
  char *str = const_cast<char *>(strIn); // tft object expects non-const
  tft.getCursor(&x,&y);
  //const char *strPtr = str.c_str();
  //int16_t titleLength = tft.strPixelLen(str);
  int16_t titleLength = tft.measureTextWidth(str, 0);
  int xPos = tft.width()/2 - titleLength/2;
  tft.setCursor(xPos, y);
  //Serial.print("printCentered(): "); Serial.print(String("length: ") + titleLength + String(" xPos: ") + xPos);
  tft.println(str);
}

void printCenteredJustified(ILI9341_t3 &tft, const char *strIn, int16_t xPos, int16_t yPos)
{
    char *str = const_cast<char *>(strIn); // tft object expects non-const
    int16_t stringLength = tft.measureTextWidth(str, 0);
    tft.setCursor(xPos - stringLength/2 + 1, yPos);
    tft.println(str);
}

void printRightJustified(ILI9341_t3 &tft, const char *strIn, int16_t xPos, int16_t yPos)
{
    char *str = const_cast<char *>(strIn); // tft object expects non-const
    //int16_t stringLength = tft.strPixelLen(str);
    int16_t stringLength = tft.measureTextWidth(str, 0);
    tft.setCursor(xPos - stringLength, yPos);
    tft.println(str);
}

void setCursorX(ILI9341_t3 &tft, int16_t xPos)
{
  int16_t x,y;
  tft.getCursor(&x, &y);
  x = xPos;
  tft.setCursor(x,y);
}

void setCursorY(ILI9341_t3 &tft, int16_t yPos)
{
  int16_t x,y;
  tft.getCursor(&x,&y);
  y = yPos;
  tft.setCursor(x,y);
}

void clearScreen(ILI9341_t3 &tft)
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(DEFAULT_TEXT_SCALE);
}

void clearTextRightJustified(ILI9341_t3 &tft, const char *strIn, int16_t xPos, int16_t yPos)
{
    char *str = const_cast<char *>(strIn); // tft object expects non-const
    unsigned width = tft.measureTextWidth(str);
    unsigned height = 8 * tft.getTextSize();
    tft.fillRect(xPos-width, yPos, width, height, ILI9341_BLACK);
}


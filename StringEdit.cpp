/*
 * StringEdit.cpp
 *
 *  Created on: Jan. 7, 2019
 *      Author: blackaddr
 */
#include <cstdint>

#include "Screens.h"
#include "StringEdit.h"

constexpr unsigned TOP_MARGIN  = 10;
constexpr unsigned LEFT_MARGIN = 15;

constexpr int textSize = 3;
constexpr unsigned CHAR_WIDTH = 6*textSize; // font width, * text scale
constexpr unsigned CHAR_WIDTH_SPACE = CHAR_WIDTH*2;
constexpr unsigned CHAR_HEIGHT = 8*textSize;

constexpr int DONE_X_POS  = LEFT_MARGIN;
constexpr int DONE_Y_POS  = TOP_MARGIN + (unsigned)(CHAR_HEIGHT*7.5f);
constexpr int DONE_WIDTH  = 4*CHAR_WIDTH;
constexpr int DONE_HEIGHT = CHAR_HEIGHT;

constexpr int DEL_X_POS = LEFT_MARGIN + 5*CHAR_WIDTH;
constexpr int DEL_Y_POS = DONE_Y_POS;
constexpr int DEL_WIDTH = 3*CHAR_WIDTH;
constexpr int DEL_HEIGHT = DONE_HEIGHT;

constexpr int SHIFT_X_POS = LEFT_MARGIN + 9*CHAR_WIDTH;
constexpr int SHIFT_Y_POS = DONE_Y_POS;
constexpr int SHIFT_WIDTH = 5*CHAR_WIDTH;
constexpr int SHIFT_HEIGHT = DONE_HEIGHT;


void StringEdit(ILI9341_t3 &tft, String &inputString, Controls &controls)
{
  int16_t x,y;

  char newString[32];
  inputString.toCharArray(newString, inputString.length()+1);
  uint8_t ch = 'a';
  uint8_t selectedChar = 'a';
  unsigned newStringIndex = inputString.length();
  bool characterShift = false;
  bool updateRequired = true;

  while(true)
  {
    if (updateRequired) {

      // Draw the Screen title
      tft.setTextColor(ILI9341_CYAN);
      tft.setTextSize(textSize);
      tft.fillScreen(ILI9341_BLACK);

      // Print the string
      tft.setCursor(LEFT_MARGIN, TOP_MARGIN);
      tft.printf("%s_\n",newString);

      // Print the alphabet
      tft.setTextColor(ILI9341_WHITE);
      unsigned charsPerLine = (tft.width()-LEFT_MARGIN) / CHAR_WIDTH_SPACE;
      tft.setCursor(LEFT_MARGIN, TOP_MARGIN + (unsigned)(CHAR_HEIGHT*1.5f));

      unsigned charIndex = 0;

      // Print the letters
      ch = 'a';
      while(true)
      {
        // Check if this is the selected char
        if (ch == selectedChar) {

          tft.getCursor(&x,&y);
          x -= CHAR_WIDTH/2;
          if (ch > static_cast<uint8_t>(StringEditSymbols::SHIFT)) {
            // This is a letter or number
            tft.fillRect(x,y,CHAR_WIDTH*2,CHAR_HEIGHT, ILI9341_DARKCYAN);

          } else {
            // This is a symbol
            //unsigned width;
            switch(ch) {
              case static_cast<uint8_t>(StringEditSymbols::SPACE) :
              tft.fillRect(x,y,CHAR_WIDTH*6,CHAR_HEIGHT, ILI9341_DARKCYAN);
              break;
              case static_cast<uint8_t>(StringEditSymbols::DONE) :
                tft.fillRect(x,y,CHAR_WIDTH*5,CHAR_HEIGHT, ILI9341_DARKCYAN);
                break;
              case static_cast<uint8_t>(StringEditSymbols::BACKSPACE) :
                tft.fillRect(x,y,CHAR_WIDTH*4,CHAR_HEIGHT, ILI9341_DARKCYAN);
                break;
              case static_cast<uint8_t>(StringEditSymbols::SHIFT) :
                tft.fillRect(x,y,CHAR_WIDTH*6,CHAR_HEIGHT, ILI9341_DARKCYAN);
                break;
              default :
                break;
            }
          }
        }

        // Check for symbols first
        if (ch < static_cast<uint8_t>(StringEditSymbols::NUM_SYMBOLS)) {
          // it's a symbol
          switch(ch) {
            case static_cast<uint8_t>(StringEditSymbols::SPACE) :
            tft.printf("SPACE ");
            break;
            case static_cast<uint8_t>(StringEditSymbols::DONE) :
              tft.printf("DONE ");
              break;
            case static_cast<uint8_t>(StringEditSymbols::BACKSPACE) :
              tft.printf("DEL ");
              break;
            case static_cast<uint8_t>(StringEditSymbols::SHIFT) :
              tft.printf("SHIFT ");
              break;
            default :
              break;
          }
          charIndex = 0;
        } else {
          // it's a letter or number
          char printChar = characterShift ? toupper(ch) : ch;
          if (charIndex >= charsPerLine-1) {
            tft.printf("%c\n", printChar);
            charIndex = 0;
            setCursorX(tft, LEFT_MARGIN);
          } else {
            tft.printf("%c ", printChar);
            charIndex++;
          }
        }


        // Switch to letters, numbers or symbols
        if (ch == 'z') { ch = '0';}
        else if (ch == '9') { ch = static_cast<uint8_t>(StringEditSymbols::SPACE);}
        else if (ch == static_cast<uint8_t>(StringEditSymbols::SPACE)) {
            ch = static_cast<uint8_t>(StringEditSymbols::DONE); tft.printf("\n\n"); setCursorX(tft, LEFT_MARGIN);
        }
        else if (ch == static_cast<uint8_t>(StringEditSymbols::SHIFT)) { break; }
        else { ch++; }
      }

      updateRequired = false;
    }

    // Check for rotary movement
    ControlEvent controlEvent;
    int knobAdjust = 0;
    int switchToggled = 0;
    while (getNextControlEvent(controlEvent)) {
        switch(controlEvent.eventType) {
        case ControlEventType::ENCODER : knobAdjust    = controlEvent.value; break;
        case ControlEventType::SWITCH  : switchToggled = controlEvent.value; break;
        default : break;
        }
    }

    if (knobAdjust != 0) {
      int adjust = (knobAdjust > 0) ? 1 : -1;

      if (adjust > 0) {
        // going up
        switch (selectedChar) {
          case 'z' :
            selectedChar = '0';
            break;
          case '9' :
            selectedChar = static_cast<uint8_t>(StringEditSymbols::SPACE);
            break;
          case static_cast<uint8_t>(StringEditSymbols::SHIFT) :
            selectedChar = 'a';
            break;
          default :
            selectedChar++;
        }

      } else {
        // going down
        switch (selectedChar) {
          case 'a' :
            selectedChar = static_cast<uint8_t>(StringEditSymbols::SHIFT);
            break;
          case '0' :
            selectedChar = 'z';
            break;
          case static_cast<uint8_t>(StringEditSymbols::SPACE) :
            selectedChar = '9';
            break;
          default :
            selectedChar--;
        }
      }
      updateRequired =  true;
    }

    // Check for button
    if (switchToggled) {

      // Check for symbol
      if (selectedChar < static_cast<uint8_t>(StringEditSymbols::NUM_SYMBOLS)) {

        switch (selectedChar) {
          case static_cast<uint8_t>(StringEditSymbols::SPACE) :
              newString[newStringIndex] = ' ';
              newStringIndex++;
              newString[newStringIndex] = 0;
              updateRequired = true;
              break;
            case static_cast<uint8_t>(StringEditSymbols::DONE) :
              inputString.remove(0);
              inputString.concat(newString);
              return;
            case static_cast<uint8_t>(StringEditSymbols::BACKSPACE) :
              if (newStringIndex > 0) {
                // only backspace if not on first char
                newString[newStringIndex-1] = 0;
                newStringIndex--;
              }
              break;
            case static_cast<uint8_t>(StringEditSymbols::SHIFT) :
              characterShift = !characterShift;
              break;
            default :
              break;
        }
        updateRequired = true;
      } else {
        char storedChar = characterShift ? toupper(selectedChar) : selectedChar;
        newString[newStringIndex] = storedChar;
        newStringIndex++;
        newString[newStringIndex] = 0;
        updateRequired = true;

      }
    }

    // Check for touch activity
    if (controls.isTouched()) {
        TouchPoint touchPoint = controls.getTouchPoint();
        Coordinate touchCoordinate(touchPoint.x, touchPoint.y, nullptr);

        // wait until the screen is no longer touched before taking action
        while (controls.isTouched()) {}

        // Check the DONE button
        if (touchPoint.x > static_cast<int16_t>(DONE_X_POS) && touchPoint.x < static_cast<int16_t>(DONE_X_POS+DONE_WIDTH) &&
           (touchPoint.y > static_cast<int16_t>(DONE_Y_POS) )) {
            ch = selectedChar = static_cast<uint8_t>(StringEditSymbols::DONE);
            updateRequired = true;
        }

        // Check the DEL button
        if (touchPoint.x > static_cast<int16_t>(DEL_X_POS) && touchPoint.x < static_cast<int16_t>(DEL_X_POS+DONE_WIDTH) &&
           (touchPoint.y > static_cast<int16_t>(DEL_Y_POS) )) {
            ch = selectedChar = static_cast<uint8_t>(StringEditSymbols::BACKSPACE);
            updateRequired = true;
        }

        // Check the SHIFT button
        if (touchPoint.x > static_cast<int16_t>(SHIFT_X_POS) && touchPoint.x < static_cast<int16_t>(SHIFT_X_POS+DONE_WIDTH) &&
           (touchPoint.y > static_cast<int16_t>(SHIFT_Y_POS) )) {
            ch = selectedChar = static_cast<uint8_t>(StringEditSymbols::SHIFT);
            updateRequired = true;
        }
    }

    if (!updateRequired) { yield(); }

  }

}



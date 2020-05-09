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

constexpr unsigned CHARS_PER_LINE = 8;

constexpr int textSize = 3;
constexpr unsigned CHAR_WIDTH = 6*textSize; // font width, * text scale
constexpr unsigned CHAR_WIDTH_SPACE = CHAR_WIDTH*2;
constexpr unsigned CHAR_HEIGHT = 8*textSize;

constexpr int SPACE_X_POS = LEFT_MARGIN + 8*CHAR_WIDTH;
constexpr int SPACE_Y_POS = TOP_MARGIN + (unsigned)(CHAR_HEIGHT*5.5f);
constexpr int SPACE_WIDTH = 5*CHAR_WIDTH;
constexpr int SPACE_HEIGHT = CHAR_HEIGHT;

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

// This function is brute force and not portable to different screens sizes.
// Maybe one day compute it directly instaed of using branches.
void getPosition(char ch, int16_t* xPos, int16_t* yPos)
{
    if ((!xPos) || (!yPos)) { return; }
    if (ch >= 'a' && ch <= 'h') {
        *xPos = LEFT_MARGIN + 2*CHAR_WIDTH*(ch-'a');
        *yPos = TOP_MARGIN + (unsigned)(CHAR_HEIGHT*1.5f);
    }
    if (ch >= 'i' && ch <= 'p') {
        *xPos = LEFT_MARGIN + 2*CHAR_WIDTH*(ch-'i');
        *yPos = TOP_MARGIN + (unsigned)(CHAR_HEIGHT*2.5f);
    }
    if (ch >= 'q' && ch <= 'x') {
        *xPos = LEFT_MARGIN + 2*CHAR_WIDTH*(ch-'q');
        *yPos = TOP_MARGIN + (unsigned)(CHAR_HEIGHT*3.5f);
    }
    if (ch >= 'y' && ch <= 'z') {
        *xPos = LEFT_MARGIN + 2*CHAR_WIDTH*(ch-'y');
        *yPos = TOP_MARGIN + (unsigned)(CHAR_HEIGHT*4.5f);
    }
    if (ch >= '0' && ch <= '5') {
        *xPos = LEFT_MARGIN + 2*CHAR_WIDTH*(2+ch-'0');
        *yPos = TOP_MARGIN + (unsigned)(CHAR_HEIGHT*4.5f);
    }
    if (ch >= '6' && ch <= '9') {
        *xPos = LEFT_MARGIN + 2*CHAR_WIDTH*(ch-'6');
        *yPos = TOP_MARGIN + (unsigned)(CHAR_HEIGHT*5.5f);
    }
    if (ch == (char)StringEditSymbols::SPACE) {
        *xPos = SPACE_X_POS;
        *yPos = SPACE_Y_POS;
    }
    if (ch == (char)StringEditSymbols::DONE) {
        *xPos = DONE_X_POS;
        *yPos = DONE_Y_POS;
    }
    if (ch == (char)StringEditSymbols::BACKSPACE) {
        *xPos = DEL_X_POS;
        *yPos = DEL_Y_POS;
    }
    if (ch == (char)StringEditSymbols::SHIFT) {
        *xPos = SHIFT_X_POS;
        *yPos = SHIFT_Y_POS;
    }
}

// This function will take in a pixel position and return the nearest char
bool getChar(unsigned char* ch, unsigned height, unsigned width, int16_t xPos, int16_t yPos)
{
    if ( (yPos < (int)(TOP_MARGIN + 1.5f*CHAR_HEIGHT)) || (yPos > (int)(height-TOP_MARGIN)) || (xPos < (int)LEFT_MARGIN) || (xPos > (int)(LEFT_MARGIN + 8*2*CHAR_WIDTH)) ) {
        return false; // not valid
    }

    unsigned hoffset = (xPos - LEFT_MARGIN + CHAR_WIDTH) / (2*CHAR_WIDTH);

    if (yPos < (TOP_MARGIN + 2.5f*CHAR_HEIGHT)) { // 'a' row
        *ch = 'a' + hoffset;
        return true;
    }

    if (yPos < (TOP_MARGIN + 3.5f*CHAR_HEIGHT)) {
        *ch = 'i' + hoffset;
        return true;
    }

    if (yPos < (TOP_MARGIN + 4.5f*CHAR_HEIGHT)) {
        *ch = 'q' + hoffset;
        return true;
    }

    if (yPos < (TOP_MARGIN + 5.5f*CHAR_HEIGHT)) {
        if (hoffset < 2) {
            *ch = 'y' + hoffset;
        } else {
            *ch = '0' + hoffset - 2;
        }
        return true;
    }

    if (yPos < (TOP_MARGIN + 6.5f*CHAR_HEIGHT)) {
        if (hoffset < 4) {
            *ch = '6' + hoffset;
            return true;
        } else {
            *ch = static_cast<unsigned char>(StringEditSymbols::SPACE);
        }
        return true;
    }
    return false;

}

void StringEdit(ILI9341_t3 &tft, String &inputString, Controls &controls)
{
  int16_t x,y;

  char newString[32];
  inputString.toCharArray(newString, inputString.length()+1);
  uint8_t ch = 'a';
  uint8_t previousChar = 'a';
  uint8_t selectedChar = 'a';
  unsigned newStringIndex = inputString.length();
  bool characterShift = false;
  bool updateRequired = true;
  bool redrawAllChars = true;
  bool redrawString   = true;

  tft.fillScreen(ILI9341_BLACK);

  while(true)
  {
    if (updateRequired) {

        if (redrawString) {
            // Draw the string being edited
            tft.setTextColor(ILI9341_CYAN);
            tft.setTextSize(textSize);
            // Print the string
            tft.fillRect(LEFT_MARGIN,TOP_MARGIN,CHAR_WIDTH*16,CHAR_HEIGHT, ILI9341_BLACK);
            tft.setCursor(LEFT_MARGIN, TOP_MARGIN);
            tft.printf("%s_\n",newString);
            redrawString = false;
        }

      // Print the alphabet
      tft.setTextColor(ILI9341_WHITE);
      tft.setCursor(LEFT_MARGIN, TOP_MARGIN + (unsigned)(CHAR_HEIGHT*1.5f));

      // Print the letters
      ch = 'a';
      while(true)
      {
        // Check if this is the selected char
        if ((ch == selectedChar) || (ch == previousChar) || redrawAllChars) {

          getPosition(ch, &x, &y);
          tft.setCursor(x,y);
          x -= CHAR_WIDTH/2;

          uint16_t color = (ch == selectedChar) ? ILI9341_DARKCYAN : ILI9341_BLACK;
          if (ch > static_cast<uint8_t>(StringEditSymbols::SHIFT)) {
            // This is a letter or number
            tft.fillRect(x,y,CHAR_WIDTH*2,CHAR_HEIGHT, color);

          } else {
            // This is a symbol
            switch(ch) {
              case static_cast<uint8_t>(StringEditSymbols::SPACE) :
              tft.fillRect(x,y,CHAR_WIDTH*6,CHAR_HEIGHT, color);
              break;
              case static_cast<uint8_t>(StringEditSymbols::DONE) :
                tft.fillRect(x,y,CHAR_WIDTH*5,CHAR_HEIGHT, color);
                break;
              case static_cast<uint8_t>(StringEditSymbols::BACKSPACE) :
                tft.fillRect(x,y,CHAR_WIDTH*4,CHAR_HEIGHT, color);
                break;
              case static_cast<uint8_t>(StringEditSymbols::SHIFT) :
                tft.fillRect(x,y,CHAR_WIDTH*6,CHAR_HEIGHT, color);
                break;
              default :
                break;
            }
          }
        } // end if ch == selectedChar


        // Only draw the selected and the previous to reduce screen draws
        if ((ch == selectedChar) || (ch == previousChar) || redrawAllChars) {

            getPosition(ch, &x, &y);
            tft.setCursor(x,y);

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
              //charIndex = 0;
            } else {
              // it's a letter or number
              char printChar = characterShift ? toupper(ch) : ch;
              tft.printf("%c ", printChar);
            }
        }

        // Switch to letters, numbers or symbols
        if (ch == 'z') { ch = '0';}
        else if (ch == '9') { ch = static_cast<uint8_t>(StringEditSymbols::SPACE);}
        else if (ch == static_cast<uint8_t>(StringEditSymbols::SPACE)) {
            ch = static_cast<uint8_t>(StringEditSymbols::DONE); tft.printf("\n\n"); setCursorX(tft, LEFT_MARGIN);
        }
        else if (ch == static_cast<uint8_t>(StringEditSymbols::SHIFT)) {
            redrawAllChars = false;
            break;
        }
        else { ch++; }
      } // end while(true)

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

      previousChar = selectedChar;
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
        updateRequired = true;
        redrawString = true;

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
                  redrawAllChars = true;
                  break;
                default :
                  break;
            }

        } else {
            char storedChar = characterShift ? toupper(selectedChar) : selectedChar;
            newString[newStringIndex] = storedChar;
            newStringIndex++;
            newString[newStringIndex] = 0;
        }
    }

    // Check for touch activity
    if (controls.isTouched()) {
        TouchPoint touchPoint = controls.getTouchPoint();
        Coordinate touchCoordinate(touchPoint.x, touchPoint.y, nullptr);

        // wait until the screen is no longer touched before taking action
        while (controls.isTouched()) {}
        previousChar = selectedChar;

        // Check for touch on a letter
        uint8_t charTemp;
        if (getChar(&charTemp, tft.height(), tft.width(), touchPoint.x, touchPoint.y)) {
            ch = selectedChar = charTemp;
            updateRequired = true;
        }


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



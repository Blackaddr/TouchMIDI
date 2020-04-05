/*
 * StringEdit.h
 *
 *  Created on: Jan. 7, 2019
 *      Author: blackaddr
 */

#ifndef STRINGEDIT_H_
#define STRINGEDIT_H_

#include "XPT2046_Touchscreen.h"
#include "ILI9341_t3.h"

#include "Controls.h"
#include "ScreensUtil.h"

enum class StringEditSymbols : uint8_t {
  SPACE = 1,
  DONE,
  BACKSPACE,
  SHIFT,
  NUM_SYMBOLS
};

void StringEdit(ILI9341_t3 &tft, String &inputString, Controls &controls, RotaryEncoder &encoder, Bounce &selButton);

#endif /* STRINGEDIT_H_ */

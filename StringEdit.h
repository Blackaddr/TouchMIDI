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
  DONE = 1,
  BACKSPACE = 2,
  SHIFT = 3,
  NUM_SYMBOLS
};

void StringEdit(ILI9341_t3 &tft, String &inputString, XPT2046_Touchscreen &touch, RotaryEncoder &encoder, Bounce &selButton);

#endif /* STRINGEDIT_H_ */

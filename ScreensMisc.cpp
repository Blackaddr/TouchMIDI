/*
 * ScreensMisc.cpp
 *
 *  Created on: Mar. 2, 2019
 *      Author: blackaddr
 */
#include "Screens.h"

void infoScreen(ILI9341_t3 &tft, String message)
{
    unsigned width = tft.width();
    unsigned height = tft.height();

    const unsigned TEXT_HEIGHT = 7;

    const unsigned BOX_WIDTH = (width*7)/10;
    const unsigned BOX_HEIGHT = (height*5)/10;
    const unsigned BOX_X_POS = (width - BOX_WIDTH)/2;
    const unsigned BOX_Y_POS = (height - BOX_HEIGHT)/2;
    const unsigned BORDER_THICKNESS = width/50;
    const unsigned BORDER_RADIUS = BORDER_THICKNESS*2;

    tft.fillRoundRect(BOX_X_POS-BORDER_THICKNESS, BOX_Y_POS - BORDER_THICKNESS,
            BOX_WIDTH + 2*BORDER_THICKNESS, BOX_HEIGHT + 2*BORDER_THICKNESS,
            BORDER_RADIUS, ILI9341_WHITE);

    tft.fillRect(BOX_X_POS, BOX_Y_POS, BOX_WIDTH, BOX_HEIGHT, ILI9341_DARKCYAN);

    // Print the prompt
    tft.setCursor(0, BOX_Y_POS + MARGIN); // start row under icons
    tft.setTextColor(ILI9341_WHITE);
    printCentered(tft, message.c_str());
}

bool confirmationScreen(ILI9341_t3 &tft, Controls &controls, String message)
{
    unsigned width = tft.width();
    unsigned height = tft.height();

    const unsigned TEXT_HEIGHT = 7;

    const unsigned BUTTON_WIDTH = width/5;
    const unsigned BUTTON_HEIGHT = height/5;

    const unsigned BOX_WIDTH = (width*7)/10;
    const unsigned BOX_HEIGHT = (height*5)/10;
    const unsigned BOX_X_POS = (width - BOX_WIDTH)/2;
    const unsigned BOX_Y_POS = (height - BOX_HEIGHT)/2;
    const unsigned BORDER_THICKNESS = width/50;
    const unsigned BORDER_RADIUS = BORDER_THICKNESS*2;

    const unsigned NO_BUTTON_X_POS = width/4 + MARGIN;
    const unsigned NO_BUTTON_Y_POS = BOX_Y_POS + BOX_HEIGHT - MARGIN - BUTTON_HEIGHT;

    const unsigned YES_BUTTON_X_POS = width - width/4 - MARGIN - BUTTON_WIDTH;
    const unsigned YES_BUTTON_Y_POS = BOX_Y_POS + BOX_HEIGHT - MARGIN - BUTTON_HEIGHT;

    const TouchArea NO_BUTTON_AREA (NO_BUTTON_X_POS,  NO_BUTTON_X_POS+BUTTON_WIDTH,  NO_BUTTON_Y_POS,  NO_BUTTON_Y_POS+BUTTON_HEIGHT);
    const TouchArea YES_BUTTON_AREA(YES_BUTTON_X_POS, YES_BUTTON_X_POS+BUTTON_WIDTH, YES_BUTTON_Y_POS, YES_BUTTON_Y_POS+BUTTON_HEIGHT);

    tft.fillRoundRect(BOX_X_POS-BORDER_THICKNESS, BOX_Y_POS - BORDER_THICKNESS,
            BOX_WIDTH + 2*BORDER_THICKNESS, BOX_HEIGHT + 2*BORDER_THICKNESS,
            BORDER_RADIUS, ILI9341_WHITE);

    tft.fillRect(BOX_X_POS, BOX_Y_POS, BOX_WIDTH, BOX_HEIGHT, ILI9341_DARKCYAN);

    // Print the prompt
    tft.setCursor(0, BOX_Y_POS + MARGIN); // start row under icons
    tft.setTextColor(ILI9341_WHITE);
    //printCentered(tft, "Confirm SAVE?");
    printCentered(tft, message.c_str());

    // Print the NO/YES buttons
    tft.fillRect(NO_BUTTON_X_POS, NO_BUTTON_Y_POS, BUTTON_WIDTH, BUTTON_HEIGHT, ILI9341_RED); // NO button
    tft.fillRect(YES_BUTTON_X_POS, YES_BUTTON_Y_POS, BUTTON_WIDTH, BUTTON_HEIGHT, ILI9341_GREEN); // YES button

    tft.setCursor(NO_BUTTON_X_POS, NO_BUTTON_Y_POS);
    printCenteredJustified(tft, "NO\n", NO_BUTTON_X_POS + BUTTON_WIDTH/2, NO_BUTTON_Y_POS + BUTTON_HEIGHT/2 - TEXT_HEIGHT/2);
    tft.setCursor(YES_BUTTON_X_POS, YES_BUTTON_Y_POS);
    printCenteredJustified(tft, "YES\n", YES_BUTTON_X_POS + BUTTON_WIDTH/2, YES_BUTTON_Y_POS + BUTTON_HEIGHT/2 - TEXT_HEIGHT/2);

    while(true) {
        // Check for touch activity
        if (controls.isTouched()) {
            TouchPoint touchPoint = controls.getTouchPoint();

            // Check the YES button
            if (YES_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                return true;
            }

            // Check the NO button
            if (NO_BUTTON_AREA.checkArea(touchPoint)) {
                while (controls.isTouched()) {} // wait for release
                return false;
            }
        }
        delay(100);
    }
}

bool saveConfirmation(ILI9341_t3 &tft, Controls &controls)
{
    return confirmationScreen(tft, controls, "Confirm SAVE?\n");
}


TS_Point calibPoint(ILI9341_t3 &tft, Controls &controls, int16_t x, int16_t y)
{
  tft.drawFastHLine(x-10, y, 20, ILI9341_WHITE);
  tft.drawFastVLine(x, y-10, 20, ILI9341_WHITE);
  while(!controls.touch->touched()) {}
  TS_Point point = controls.getTouchRawPoint();
  Serial.println(String("Touchpoint raw: ") + point.x + String("   ") + point.y);

  while(controls.touch->touched()) {} // wait for touch1 released
  tft.fillRect(x-10, y-10, 20, 20, ILI9341_BLACK);
  return point;
}

TS_Point calcCalibLimits(unsigned p1, unsigned p2, unsigned p3)
{
  // Perform the calibration calculations.
  int L2 = (p3 - p1)/2;
  int LA = (p2 - p1);
  int LB = (p3 - p2);
  float Lavg =  (L2 + LA + LB)/3;
  float minf = ((p1 - Lavg) + (p2 - 2*Lavg) + (p3 - 3*Lavg))/3;
  float maxf = ((p3 + Lavg) + (p3 + 2*Lavg) + (p1 + 3*Lavg))/3;
  Serial.println(String("Min/Max is") + minf + "/" + maxf);
  TS_Point point;
  point.x = static_cast<unsigned>(minf);
  point.y = static_cast<unsigned>(maxf);
  return point;
}

Screens TouchCalib(ILI9341_t3 &tft, Controls &controls)
{
  TS_Point point1, point2, point3, point4, point5;
  clearScreen(tft);
  tft.setCursor(0, MARGIN);
  printCentered(tft, "Calibration\n");
  printCentered(tft, "Touch the cross\n");

  point1 = calibPoint(tft, controls, tft.width()/2, tft.height()/2); // center
  point2 = calibPoint(tft, controls, tft.width()/4, tft.height()/2); // left of center
  point3 = calibPoint(tft, controls, tft.width()*3/4, tft.height()/2); // right of center
  point4 = calibPoint(tft, controls, tft.width()/2, tft.height()/4); // above center
  point5 = calibPoint(tft, controls, tft.width()/2, tft.height()*3/4); // below center

  TS_Point xCalib = calcCalibLimits(point2.x, point1.x, point3.x);
  TS_Point yCalib = calcCalibLimits(point4.y, point1.y, point5.y);

  controls.setCalib(xCalib.x, xCalib.y, yCalib.x, yCalib.y);

  return Screens::PRESET_NAVIGATION;
}



void PrintPreset(ILI9341_t3 &tft, const Preset &preset)
{
  tft.println(String("Name: ") + preset.name);
  tft.println(String("Index: ") + preset.index);
  tft.println("");

  for (unsigned i=0; i<preset.numControls; i++) {
    tft.println(preset.controls[i].name + String(":"));
    tft.print(String("CC:") + preset.controls[i].cc + String("   "));

    tft.print(String("Input Control: "));
    switch(preset.controls[i].inputControl) {
        case InputControl::NOT_CONFIGURED :
          tft.print("NOT_CONFIG");
          break;
        case InputControl::SW1 :
          tft.print("SW1");
          break;
        case InputControl::SW2 :
          tft.print("SW2");
          break;
        case InputControl::SW3 :
          tft.print("SW3");
          break;
        case InputControl::SW4 :
          tft.print("SW4");
          break;
        case InputControl::EXP1 :
          tft.print("EXP1");
          break;
        case InputControl::EXP2 :
          tft.print("EXP2");
          break;
        default :
          break;
        }

    tft.print(String("Type: "));

    switch(preset.controls[i].type) {
      case ControlType::SWITCH_MOMENTARY :
          tft.print("MOM");
          break;
      case ControlType::SWITCH_LATCHING :
          tft.print("LAT");
          break;
      case ControlType::ROTARY_KNOB :
          tft.print("KNOB");
          break;
      default :
          break;
    }
    tft.println(String("   Value: ") + preset.controls[i].value);

  }
}





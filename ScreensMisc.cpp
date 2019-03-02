/*
 * ScreensMisc.cpp
 *
 *  Created on: Mar. 2, 2019
 *      Author: blackaddr
 */
#include "Screens.h"

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
  printCentered(tft, "Calibration");
  printCentered(tft, "Touch the cross");

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




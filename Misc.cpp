/*
 * Misc.cpp
 *
 *  Created on: Jan. 7, 2019
 *      Author: blackaddr
 */
//#include "Math.h"
#include "Misc.h"

int adjustWithWrap(int currentValue, int adjust, int maxVal, int minVal)
{
  // check to make sure the adjust doesn't exceed the difference between min and max
  // if so modulo it.
  int range = maxVal + 1 - minVal;
  int modifiedAdjust = adjust;
  int absAdjust = abs(adjust);
  if (absAdjust > range) {
    // We need to modulo the value
    if (adjust > 0) {
      modifiedAdjust = adjust % range;
    } else {
      modifiedAdjust = -(adjust % range);
    }
  }

  int newValue = currentValue + modifiedAdjust;
  if (newValue > maxVal) {
    // wrap if exceeds max value
    newValue =  minVal + (newValue - maxVal) - 1;
  } else if (newValue < minVal) {
    // wrap if below min value
    Serial.printf("newValue1 is %d\n",newValue);
    Serial.printf("maxValue is %d\n", maxVal);
    newValue = maxVal - (minVal - newValue - 1);
    Serial.printf("newValue2 is %d\n",newValue);
  }
  return newValue;
}

int adjustWithSaturation(int input, int adj, int min, int max) {
    int temp = input + adj;
    if (temp < min) return min;
    if (temp > max) return max;
    return temp;
}

int toggleValue(int input, int onValue, int offValue)
{
    if (input == offValue) {
        return onValue;
    } else {
        return offValue;
    }
}




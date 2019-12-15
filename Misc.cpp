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

/*
 * Converts an unsigned to 3-digit ASCII char, decimal format
 * justify specified howt to align the number.
 * 0-> centre
 * 1-> left
 * 2-> right (default)
 */
void uint2dec3(unsigned in, char *dest, unsigned justify) {

    unsigned huns, tens, ones, tmp;
    huns = in/100; // hunds contains hundreds digit.
    tmp = in - (huns*100);
    tens = tmp/10;
    tmp = tmp - 10*tens;
    ones = tmp;

    // init to spaces
    *(dest) = huns > 0 ? (0x30 + huns) : 0x20;
    *(dest+1) = ( (tens > 0) || (huns > 0) ) ? (0x30 + tens) : 0x20;
    *(dest+2) = 0x30 + ones;
    *(dest+3) = 0x0;

    switch (justify) {
    case 0 :
        // centre justify
        if (*(dest+1) == 0x20) {
            *(dest+1) = *(dest+2);
            *(dest+2) = 0x20;
        }
        break;
    case 1 :
        // left justify
        while (*dest == 0x20) {
            *dest = *(dest+1);
            *(dest+1) = *(dest+2);
            *(dest+2) = 0x20;
        }
        break;
    case 2 :
        break;
    default :
        break;
    }
    return;
}




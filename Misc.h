#ifndef __MISC_H
#define __MISC_H

#include <vector>

/// Add a new element to a vector.
/// @details if the index does not yet exist, add it at the back. Otherwise, index it directly.
/// @param vec reference to the vector that you want to add an element to
/// @param element the element to be added to the specified vector
/// @param the index in the vector to set the element to
template<typename T>
void addToVector(std::vector<T> &vec, T element, unsigned index)
{
  if (index >= vec.size()) {
    vec.emplace_back(element);
  } else {
    vec[index] = element;
  }
}


int adjustWithWrap(int currentValue, int adjust, int maxVal, int minVal = 0)
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

#endif

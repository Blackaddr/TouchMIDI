#ifndef __MISC_H
#define __MISC_H

#include "Arduino.h"
#include "VectorSupport.h"

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


int adjustWithWrap(int currentValue, int adjust, int maxVal, int minVal = 0);
int adjustWithSaturation(int input, int adj, int min, int max);
int toggleValue(int input, int onValue, int offValue=0);

#endif

#ifndef __MISC_H
#define __MISC_H

#include "Arduino.h"
#include "VectorSupport.h"

constexpr int MIDI_CC_MAX    = 127;
constexpr int MIDI_CC_MIN    = 0;
constexpr int MIDI_VALUE_MAX = 127;
constexpr int MIDI_VALUE_MIN = 0;
constexpr int MIDI_ON_VALUE  = 127;
constexpr int MIDI_OFF_VALUE = 0;

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

template<typename T>
void insertToVector(std::vector<T> &vec, T element, unsigned index)
{
  if (index >= vec.size()) {
    vec.emplace_back(element);
  } else {
    vec.insert(index, element);
  }
}


int adjustWithWrap(int currentValue, int adjust, int maxVal, int minVal = 0);
int adjustWithSaturation(int input, int adj, int min, int max);
int toggleValue(int input, int onValue, int offValue=0);
void uint2dec3(unsigned in, char *dest, unsigned justify);


//template <class Container, class Iterator>
//auto nextIterator(Container& cont, Iterator &it) -> decltype (cont.begin()) {
//    if (it != cont.end()) {
//        ++it;
//    }
//}

#endif

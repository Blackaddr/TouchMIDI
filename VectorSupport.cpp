/*
 * VectorSupport.cpp
 *
 *  Created on: Jan. 7, 2019
 *      Author: blackaddr
 */
#include "Arduino.h"
#include "VectorSupport.h"

// These calls must be define in order to get vector to work on arduino
namespace std {
void __throw_bad_alloc() {
  //Serial.println("Unable to allocate memory");
}
void __throw_length_error( char const*e ) {
  //Serial.print("Length Error :"); Serial.println(e);
}
void __throw_out_of_range_fmt( char const*e ) {
  //Serial.print("out of range Error :"); Serial.println(e);
}

}




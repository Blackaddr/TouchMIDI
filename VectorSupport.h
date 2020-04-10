/*
 * VectorSupport.h
 *
 *  Created on: Jan. 7, 2019
 *      Author: blackaddr
 */

#ifndef VECTORSUPPORT_H_
#define VECTORSUPPORT_H_

#include "Arduino.h"
#include <vector>

// These calls must be define in order to get vector to work on arduino
namespace std {
void __throw_bad_alloc();
void __throw_length_error( char const*e );
void __throw_out_of_range_fmt( const char* e);
}

#endif /* VECTORSUPPORT_H_ */

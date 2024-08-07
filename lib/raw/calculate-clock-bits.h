/*
Copyright 2023 Commtech, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"), to deal 
in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in 
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
THE SOFTWARE.
*/

#ifndef CALCULATE_CLOCK_BITS_H
#define CALCULATE_CLOCK_BITS_H

#include <stdint.h>

struct clock_data_fscc {
	unsigned long frequency;
	unsigned char clock_bits[20];
};

struct clock_data_335 {
	unsigned long frequency;
	unsigned int clock_bits;
};

// The FSCC, Sync Com, and Async Com all use the same clock currently.
// Other -335 serial cards use a different clock.
typedef struct clock_data_fscc clock_data_fscc;
typedef struct clock_data_fscc clock_data_synccom;
typedef struct clock_data_fscc clock_data_asynccom;
typedef struct clock_data_335 clock_data_335;

int calculate_clock_bits_fscc(clock_data_fscc *clock_data, unsigned long ppm);
int calculate_clock_bits_asynccom(clock_data_asynccom *clock_data, unsigned long ppm);
int calculate_clock_bits_synccom(clock_data_synccom *clock_data, unsigned long ppm);
int calculate_clock_bits_335(clock_data_335 *clock_data);

#endif
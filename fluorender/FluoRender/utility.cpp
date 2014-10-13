/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
#include "utility.h"
#include <math.h>

double d2r(double d)
{
	return d*0.017453292519943295769236907684886;
}

double r2d(double r)
{
	return r*57.295779513082320876798154814105;
}

void sinCos(double *returnSin, double *returnCos, double theta)
{
	// For simplicity, we'll just use the normal trig functions.
	// Note that on some platforms we may be able to do better

	*returnSin = sin(theta);
	*returnCos = cos(theta);
}

unsigned int bit_reverse(unsigned int v)
{
	unsigned int c; // reverse 32-bit value, 8 bits at time 

	// Option 1: 
	c = (BitReverseTable256[v & 0xff] << 24) |  
		(BitReverseTable256[(v >> 8) & 0xff] << 16) |  
		(BitReverseTable256[(v >> 16) & 0xff] << 8) | 
		(BitReverseTable256[(v >> 24) & 0xff]);

	return c;
}

unsigned int reverse_bit(unsigned int val, unsigned int len)
{
	unsigned int res = val;
	int s = len - 1;

	for (val >>= 1; val; val >>= 1)
	{
		res <<= 1;
		res |= val & 1;
		s--;
	}
	res <<= s;
	res <<= 32-len;
	res >>= 32-len;
	return res;
}

float nCr(int n,int r)
{
	if (n<r||n<0||r<0)
		return 0;
	float result = 1;
	for (int i=0;i<r;i++)
		result *= float(n-i)/float(r-i);

	return result;
}

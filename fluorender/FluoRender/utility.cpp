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

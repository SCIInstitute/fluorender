//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2025 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  

#ifndef _FLTYPEUTILS_H_
#define _FLTYPEUTILS_H_

#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

namespace fluo
{
//---------------------------------------------------------------------------
// Sorting functions
//---------------------------------------------------------------------------

// Wire-sort three numbers
template <class T> inline
void SortIndex(T v[3], int i[3])
{
	T v_tmp; int i_tmp;
	i[0] = 0; i[1] = 1; i[2] = 2;
	if(v[0] > v[1])
	{
		v_tmp = v[0]; v[0] = v[1]; v[1] = v_tmp;
		i_tmp = i[0]; i[0] = i[1]; i[1] = i_tmp;
	}
	if(v[1] > v[2])
	{
		v_tmp = v[1]; v[1] = v[2]; v[2] = v_tmp;
		i_tmp = i[1]; i[1] = i[2]; i[2] = i_tmp;
	}
	if(v[0] > v[1])
	{
		v_tmp = v[0]; v[0] = v[1]; v[1] = v_tmp;
		i_tmp = i[0]; i[0] = i[1]; i[1] = i_tmp;
	}
}

// Bubble-sort in increasing order -- vector form
template <typename T, typename U> inline
void Sort(std::vector<T>& domain, std::vector<U>& range)
{
	for(unsigned int i=0; i<domain.size(); i++)
	{
		for(unsigned int j=i+1; j<domain.size(); j++)
		{
			if(domain[j] < domain[i])
			{
				T domain_tmp = domain[i];
				domain[i] = domain[j];
				domain[j] = domain_tmp;
				U range_tmp = range[i];
				range[i] = range[j];
				range[j] = range_tmp;
			}
		}
	}
}

// Bubble-sort in increasing order -- pointer form
template <typename T, typename U> inline
void Sort(T* domain, U* range, int size)
{
	for(int i=0; i<size; i++)
	{
		for(int j=i+1; j<size; j++)
		{
			if(domain[j] < domain[i])
			{
				T domain_tmp = domain[i];
				domain[i] = domain[j];
				domain[j] = domain_tmp;
				int range_tmp = range[i];
				range[i] = range[j];
				range[j] = range_tmp;
			}
		}
	}
}

// Bubble-sort in increasing order -- no sort index returned
template <class T> inline
void Sort(T* val, int n)
{
	for(int i=0; i<n; i++)
	{
		for(int j=i+1; j<n; j++)
		{
			if(val[j] < val[i])
			{
				T val_tmp = val[i];
				val[i] = val[j];
				val[j] = val_tmp;
			}
		}
	}
}

//---------------------------------------------------------------------------
// Misc Utilities
//---------------------------------------------------------------------------

template <typename T> inline
int MinIndex(T x, T y, T z)
{
	return x < y ? (x < z ? 0 : 2) : (y < z ? 1 : 2);
}

// 2 doubles
inline double Max(double d1, double d2)
{
	return d1>d2?d1:d2;
}

inline double Min(double d1, double d2)
{
	return d1<d2?d1:d2;
}

// 2 Unsigned Integers
inline unsigned int Min(unsigned int d1, unsigned int d2)
{
	return d1<d2?d1:d2;
}

inline unsigned int Max(unsigned int d1, unsigned int d2)
{
	return d1>d2?d1:d2;
}

// 2 Integers
inline int Min(int d1, int d2)
{
	return d1<d2?d1:d2;
}

inline int Max(int d1, int d2)
{
	return d1>d2?d1:d2;
}

// 3 doubles
inline double Min(double d1, double d2, double d3)
{
	double m=d1<d2?d1:d2;
	m=m<d3?m:d3;
	return m;
}

inline double Mid(double a, double b, double c)
{
	return ((a > b) ? ((a < c) ? a : ((b > c) ? b : c)) : \
		((b < c) ? b : ((a > c) ? a : c)));
}

inline double Max(double d1, double d2, double d3)
{
	double m=d1>d2?d1:d2;
	m=m>d3?m:d3;
	return m;
}

// Fast way to check for power of two
inline bool IsPowerOf2(unsigned int n)
{
	return (n & (n-1)) == 0;
}

// Returns a number Greater Than or Equal to dim
// that is an exact power of 2
// Used for determining what size of texture to
// allocate to store an image
inline unsigned int
Pow2(const unsigned int dim)
{
	if (IsPowerOf2(dim)) return dim;
	unsigned int val = 4;
	while (val < dim) val = val << 1;;
	return val;
}

// Clamp a number to a specific range
inline double Clamp(double d, double min, double max)
{
	return d<=min?min:d>=max?max:d;
}

inline float Clamp(float d, float min, float max)
{
	return d<=min?min:d>=max?max:d;
}

inline int Clamp(int i, int min, int max)
{
	return i<min?min:i>max?max:i;
}

inline int Round(double d)
{
	return static_cast<int>(std::round(d));
}

// Absolute value
inline double Abs(double d)
{
	return d<0?-d:d;
}

inline float Abs(float d)
{
	return d<0?-d:d;
}

inline int Abs(int i)
{
	return i<0?-i:i;
}

inline double Pi()
{
	return 3.1415926535897932384626433832795;
}

inline double PiHalf()
{
	return 1.5707963267948966192313216916398;
}

inline double Epsilon(int p=6)
{
    return std::pow(10.0, double(-p));
}

inline bool InEpsilon(double v1, double v2, int p = 6)
{
	return std::abs(v1 - v2) < Epsilon(p);
}

inline double d2r(double d)
{
	return d*0.017453292519943295769236907684886;
}

inline double r2d(double r)
{
	return r*57.295779513082320876798154814105;
}

inline void sinCos(double *returnSin, double *returnCos, double theta)
{
	// For simplicity, we'll just use the normal trig functions.
	// Note that on some platforms we may be able to do better

    *returnSin = std::sin(theta);
    *returnCos = std::cos(theta);
}

inline int RotateClamp(int v, int low, int high)
{
	int r = high - low;
	if (v < low)
		v += r * (-(v-low) / r + 1);
	if (v >= high)
		v -= r * ((v-low) / r);
	return v;
}

inline int RotateClamp2(int v, int low, int high)//inclusive
{
	int r = high - low + 1;
	if (v < low)
		v += r * (-(v - low) / r + 1);
	if (v > high)
		v -= r * ((v - low) / r);
	return v;
}

inline double RotateClamp(double v, int low, int high)
{
	int r = high - low;
	if (v < low)
		v += r * (-(int)(v - low) / r + 1);
	if (v >= high)
		v -= r * ((int)(v - low) / r);
	return v;
}

//use with opengl so they are floats
inline float SmoothStep(float edge0, float edge1, float x)
{
	// Scale, bias and saturate x to 0..1 range
	x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
	// Evaluate polynomial
	return x * x * (3 - 2 * x);
}

inline float SmoothClamp(float x, float edge0, float edge1, float smoothness)
{
	float clamped = std::clamp(x, edge0, edge1);

	if (x < edge0 + smoothness) {
		float t = std::clamp((x - edge0) / smoothness, 0.0f, 1.0f);
		t = t * t * (3 - 2 * t); // smoothstep
		return edge0 + t * (x - edge0);
	}
	else if (x > edge1 - smoothness) {
		float t = std::clamp((edge1 - x) / smoothness, 0.0f, 1.0f);
		t = t * t * (3 - 2 * t); // smoothstep
		return edge1 - t * (edge1 - x);
	}

	return x;
}

} // namespace fluo

#endif//_FLTYPEUTILS_H_


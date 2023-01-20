/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#ifndef _FLKEY_H_
#define _FLKEY_H_

#include <string>

using namespace std;

#define FLKEY_TYPE_DOUBLE	1
#define FLKEY_TYPE_QUATER	2
#define FLKEY_TYPE_BOOLEAN	3
#define FLKEY_TYPE_INT		4
#define FLKEY_TYPE_COLOR	5

class FlKeyCode
{
public:
	int l0;//view: 1
	string l0_name;//view name
	int l1;//volume: 2
	string l1_name;//volume name
	int l2;//volume property: 0
	string l2_name;//volume property name

	FlKeyCode& operator=(const FlKeyCode& keycode)
	{
		l0 = keycode.l0;
		l0_name = keycode.l0_name;
		l1 = keycode.l1;
		l1_name = keycode.l1_name;
		l2 = keycode.l2;
		l2_name = keycode.l2_name;
		return *this;
	}

	int operator==(const FlKeyCode& keycode) const
	{
		return l0==keycode.l0 &&
			l0_name==keycode.l0_name &&
			l1==keycode.l1 &&
			l1_name==keycode.l1_name &&
			l2==keycode.l2 &&
			l2_name==keycode.l2_name;
	}

	int operator!=(const FlKeyCode& keycode) const
	{
		return l0!=keycode.l0 ||
			l0_name!=keycode.l0_name ||
			l1!=keycode.l1 ||
			l1_name!=keycode.l1_name ||
			l2!=keycode.l2 ||
			l2_name!=keycode.l2_name;
	}
};

//virtual
class FlKey
{
public:
	//FlKey();
	virtual ~FlKey() {};

	virtual int GetType() = 0;
	FlKeyCode GetKeyCode()
	{return m_code;}

protected:
	FlKeyCode m_code;
};

#endif//_FLKEY_H_
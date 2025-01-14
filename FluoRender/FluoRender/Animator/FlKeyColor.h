/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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
#ifndef _FLKEY_COLOR_H_
#define _FLKEY_COLOR_H_

#include <FlKey.h>
#include <Color.h>

class FlKeyColor : public FlKey
{
public:
	FlKeyColor()
	{
		m_code.l0 = 0;
		m_code.l0_name = "";
		m_code.l1 = 0;
		m_code.l1_name = "";
		m_code.l2 = 0;
		m_code.l2_name = "";
	}
	FlKeyColor(FlKeyCode keycode, fluo::Color &cval)
	{
		m_code = keycode;
		m_cval = cval;
	}
	~FlKeyColor() {}

	int GetType() { return FLKEY_TYPE_COLOR; }
	void SetValue(fluo::Color &cval) { m_cval = cval; }
	fluo::Color GetValue() { return m_cval; }

private:
	fluo::Color m_cval;
};

#endif//_FLKEY_COLOR_H_

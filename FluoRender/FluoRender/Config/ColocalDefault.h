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
#ifndef _COLOCALDEFAULT_H_
#define _COLOCALDEFAULT_H_

#include <wx/fileconf.h>
#include <limits>

class ColocalDefault
{
public:
	ColocalDefault();
	~ColocalDefault();

	void Read(wxFileConfig& f);
	void Save(wxFileConfig& f);

	bool GetThreshUpdate()
	{
		return m_auto_update && (m_method == 2);
	}
	bool GetColormapUpdate()
	{
		return m_auto_update && m_colormap;
	}
	//reset min max
	void ResetMinMax()
	{
		m_cm_min = std::numeric_limits<double>::max();
		m_cm_max = -m_cm_min;
	}
	void SetMinMax(double v)
	{
		m_cm_min = std::min(v, m_cm_min);
		m_cm_max = std::max(v, m_cm_max);
	}

public:
	//default values
	bool m_use_mask;
	bool m_auto_update;
	//method
	int m_method;//0:dot product; 1:min value; 2:threshold
	//format
	bool m_int_weighted;
	bool m_get_ratio;
	bool m_physical_size;
	bool m_colormap;
	//colormap
	double m_cm_min;
	double m_cm_max;
};
#endif

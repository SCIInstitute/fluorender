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
#ifndef _TREE_LAYER_H
#define _TREE_LAYER_H

#include <string>
#include <Color.h>

class TreeLayer
{
public:
	TreeLayer();
	~TreeLayer();

	int IsA()
	{
		return type;
	}
	std::wstring GetName()
	{
		return m_name;
	}
	void SetName(const std::wstring& name)
	{
		m_name = name;
	}
	unsigned int Id()
	{
		return m_id;
	}
	void Id(unsigned int id)
	{
		m_id = id;
	}

	//layer adjustment
	//gamma
	const fluo::Color GetGammaColor()
	{return m_gamma;}
	void SetGammaColor(const fluo::Color &gamma)
	{m_gamma = gamma;}
	//brightness
	const fluo::Color GetBrightness()
	{return m_brightness;}
	void SetBrightness(const fluo::Color &brightness)
	{m_brightness = brightness;}
	//hdr settings
	const fluo::Color GetHdr()
	{return m_hdr;}
	void SetHdr(const fluo::Color &hdr)
	{m_hdr = hdr;}
	//sync values
	bool GetSync(int i) { if (i >= 0 && i < 3) return m_sync[i]; else return false; }
	void SetSync(int i, bool val) { if (i >= 0 && i < 3) m_sync[i] = val; }

	//randomize color
	virtual void RandomizeColor() {}

protected:
	int type;//-1:invalid, 0:root 1: canvas, 2:volume, 3:mesh, 4:annotations, 5:group, 6:mesh group, 7:ruler, 8:traces
	std::wstring m_name;
	unsigned int m_id;

	//layer adjustment
	fluo::Color m_gamma;
	fluo::Color m_brightness;
	fluo::Color m_hdr;
	bool m_sync[3];//for rgb
};

#endif//_TREE_LAYER_H
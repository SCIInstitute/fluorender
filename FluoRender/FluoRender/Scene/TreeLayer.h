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

#include <Color.h>
#include <ClippingBox.h>
#include <string>

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

	virtual fluo::Color GetColor() { return fluo::Color(1.0); }
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

	virtual fluo::ClippingBox& GetClippingBox() { return m_clipping_box; }
	virtual const fluo::ClippingBox& GetClippingBox() const { return m_clipping_box; }
	virtual void SetClippingBox(const fluo::ClippingBox& box) { m_clipping_box = box; }
	//sync
	virtual void SyncClippingBoxes(const fluo::ClippingBox& cb) {}
	//clip size
	virtual void SetClipValue(fluo::ClipPlane i, int val) { m_clipping_box.SetClipIndex(i, val); }
	virtual void SetClipValues(fluo::ClipPlane i, int val1, int val2) { m_clipping_box.SetClipPairIndex(i, val1, val2); }
	virtual void SetClipValues(const std::array<int, 6>& vals)
	{
		std::array<double, 6> dvals;
		std::transform(vals.begin(), vals.end(), dvals.begin(),
			[](int v) { return static_cast<double>(v); });
		m_clipping_box.SetAllClipsIndex(dvals.data());
	}
	virtual void ResetClipValues() { m_clipping_box.ResetClips(); }
	virtual void ResetClipValues(fluo::ClipPlane i) { m_clipping_box.ResetClips(i); }
	//clip rotation
	virtual void SetClipRotation(int i, double val)
	{
		auto euler = m_clipping_box.GetEuler();
		euler[i] = val;
		m_clipping_box.Rotate(euler);
	}
	virtual void SetClipRotation(const fluo::Vector& euler) { m_clipping_box.Rotate(euler); }
	virtual void SetClipRotation(const fluo::Quaternion& q) { m_clipping_box.Rotate(q); }
	//clip distance
	virtual void SetLink(fluo::ClipPlane i, bool link) { m_clipping_box.SetLink(i, link); }
	virtual bool GetLink(fluo::ClipPlane i) { return m_clipping_box.GetLink(i); }
	virtual void ResetLink() { m_clipping_box.ResetLink(); }
	virtual void SetLinkedDist(fluo::ClipPlane i, int val) { m_clipping_box.SetLinkedDistIndex(i, val); }
	virtual int GetLinkedDist(fluo::ClipPlane i) { return static_cast<int>(std::round(m_clipping_box.GetLinkedDistIndex(i))); }

	virtual void SetOutline(bool outline)
	{
		m_outline = outline;
	}
	virtual bool GetOutline()
	{
		return m_outline;
	}

protected:
	int type;//-1:invalid, 0:root 1: canvas, 2:volume, 3:mesh, 4:annotations, 5:group, 6:mesh group, 7:ruler, 8:traces
	std::wstring m_name;
	unsigned int m_id;

	//layer adjustment
	fluo::Color m_gamma;
	fluo::Color m_brightness;
	fluo::Color m_hdr;
	bool m_sync[3];//for rgb

	//clipping box
	fluo::ClippingBox m_clipping_box;

	bool m_outline;
};

#endif//_TREE_LAYER_H
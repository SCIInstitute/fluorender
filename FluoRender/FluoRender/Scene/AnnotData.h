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
#ifndef _ANNOT_DATA_H_
#define _ANNOT_DATA_H_

#include <TreeLayer.h>
#include <Point.h>

class VolumeData;
class AnnotData;
class AText
{
public:
	AText();
	AText(const std::wstring &str, const fluo::Point &pos);
	~AText();

	std::wstring GetText();
	fluo::Point GetPos();
	void SetText(const std::wstring& str);
	void SetPos(fluo::Point pos);
	void SetInfo(const std::wstring& str);

	friend class AnnotData;

private:
	std::wstring m_txt;
	fluo::Point m_pos;
	std::wstring m_info;
};

class AnnotData : public TreeLayer
{
public:
	AnnotData();
	virtual ~AnnotData();

	//reset counter
	static void ResetID()
	{
		m_num = 0;
	}
	static void SetID(int id)
	{
		m_num = id;
	}
	static int GetID()
	{
		return m_num;
	}

	int GetTextNum();
	std::wstring GetTextText(int index);
	fluo::Point GetTextPos(int index);
	fluo::Point GetTextTransformedPos(int index);
	std::wstring GetTextInfo(int index);
	void AddText(const std::wstring& str, fluo::Point pos, const std::wstring& info);
	void SetTransform(fluo::Transform *tform);
	void SetVolume(const std::shared_ptr<VolumeData>& vd);
	std::shared_ptr<VolumeData> GetVolume();

	void Clear();

	//display functions
	void SetDisp(bool disp)
	{
		m_disp = disp;
	}
	void ToggleDisp()
	{
		m_disp = !m_disp;
	}
	bool GetDisp()
	{
		return m_disp;
	}

	//memo
	void SetMemo(const std::wstring &memo);
	std::wstring GetMemo();
	void SetMemoRO(bool ro);
	bool GetMemoRO();

	//save/load
	std::wstring GetPath();
	int Load(const std::wstring &filename);
	void Save(const std::wstring &filename);

	//info meaning
	std::wstring GetInfoMeaning();
	void SetInfoMeaning(const std::wstring &str);

	bool InsideClippingPlanes(fluo::Point &pos);

private:
	static int m_num;
	std::vector<std::shared_ptr<AText>> m_alist;
	fluo::Transform *m_tform;
	std::weak_ptr<VolumeData> m_vd;

	bool m_disp;

	//memo
	std::wstring m_memo;
	bool m_memo_ro;//read only

	//on disk
	std::wstring m_data_path;

	//atext info meaning
	std::wstring m_info_meaning;

private:
	std::shared_ptr<AText> GetAText(const std::wstring& str);
};


#endif//_ANNOT_DATA_H_
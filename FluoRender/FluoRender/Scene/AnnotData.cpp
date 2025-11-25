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
#include <AnnotData.h>
#include <Global.h>
#include <VolumeData.h>
#include <DataManager.h>
#include <VolumeRenderer.h>
#include <Texture.h>
#include <Transform.h>
#include <Plane.h>
#include <compatibility.h>
#include <fstream>

AText::AText()
{
}

AText::AText(const std::wstring &str, const fluo::Point &pos)
{
	m_txt = str;
	m_pos = pos;
}

AText::~AText()
{
}

std::wstring AText::GetText()
{
	return m_txt;
}

fluo::Point AText::GetPos()
{
	return m_pos;
}

void AText::SetText(const std::wstring& str)
{
	m_txt = str;
}

void AText::SetPos(fluo::Point pos)
{
	m_pos = pos;
}

void AText::SetInfo(const std::wstring& str)
{
	m_info = str;
}

int AnnotData::m_num = 0;

AnnotData::AnnotData()
{
	type = 4;//annotations
	m_num++;
	m_name = L"Antn_" + std::to_wstring(m_num);
	m_tform = 0;
	m_disp = true;
	m_memo_ro = false;
}

AnnotData::~AnnotData()
{
	Clear();
}

int AnnotData::GetTextNum()
{
	return (int)m_alist.size();
}

std::wstring AnnotData::GetTextText(int index)
{
	if (index>=0 && index<(int)m_alist.size())
	{
		auto atext = m_alist[index];
		if (atext)
			return atext->m_txt;
	}
	return L"";
}

fluo::Point AnnotData::GetTextPos(int index)
{
	if (index>=0 && index<(int)m_alist.size())
	{
		auto atext = m_alist[index];
		if (atext)
			return atext->m_pos;
	}
	return fluo::Point(fluo::Vector(0.0));
}

fluo::Point AnnotData::GetTextTransformedPos(int index)
{
	if (index>=0 && index<(int)m_alist.size())
	{
		auto atext = m_alist[index];
		if (atext && m_tform)
			return m_tform->transform(atext->m_pos);
	}
	return fluo::Point(fluo::Vector(0.0));
}

std::wstring AnnotData::GetTextInfo(int index)
{
	if (index>=0 && index<(int)m_alist.size())
	{
		auto atext = m_alist[index];
		if(atext)
			return atext->m_info;
	}
	return L"";
}

void AnnotData::AddText(const std::wstring& str, fluo::Point pos, const std::wstring& info)
{
	auto atext = std::make_shared<AText>(str, pos);
	atext->SetInfo(info);
	m_alist.push_back(atext);
}

void AnnotData::SetTransform(fluo::Transform *tform)
{
	m_tform = tform;
}

void AnnotData::SetVolume(const std::shared_ptr<VolumeData>& vd)
{
	m_vd = vd;
	if (vd)
		m_name += L"_FROM_" + vd->GetName();
}

std::shared_ptr<VolumeData> AnnotData::GetVolume()
{
	return m_vd.lock();
}

void AnnotData::Clear()
{
	m_alist.clear();
}

//memo
void AnnotData::SetMemo(const std::wstring &memo)
{
	m_memo = memo;
}

std::wstring AnnotData::GetMemo()
{
	return m_memo;
}

void AnnotData::SetMemoRO(bool ro)
{
	m_memo_ro = ro;
}

bool AnnotData::GetMemoRO()
{
	return m_memo_ro;
}

//save/load
std::wstring AnnotData::GetPath()
{
	return m_data_path;
}

int AnnotData::Load(const std::wstring &filename)
{
#ifdef _WIN32
	std::wifstream fis(filename);
#else
	std::wifstream fis(ws2s(filename));
#endif
	if (!fis.is_open())
		return 0;

	std::wstring str;
	std::wstring sline;

	while (std::getline(fis, sline))
	{
		if (sline.substr(0, 5) == L"Name: ")
		{
			m_name = sline.substr(6, sline.length()-6);
		}
		else if (sline.substr(0, 8) == L"Display: ")
		{
			str = sline.substr(9, 1);
			if (str == L"0")
				m_disp = false;
			else
				m_disp = true;
		}
		else if (sline.substr(0, 4) == L"Memo:")
		{
			while (std::getline(fis, str))
			{
				if (str.substr(0, 12) == L"Memo Update: ")
				{
					if (str.substr(13, 1) == L"0")
						m_memo_ro = false;
					else
						m_memo_ro = true;
					break;
				}
				else
					m_memo += str + L"\n";
			}
		}
		else if (sline.substr(0, 7) == L"Volume: ")
		{
			str = sline.substr(8, sline.length()-8);
			m_vd = glbin_data_manager.GetVolumeData(str);
			if (auto vd_ptr = m_vd.lock())
			{
				m_tform = vd_ptr->GetTexture()->transform();
			}
		}
		else if (sline.substr(0, 9) == L"Transform:")
		{
			for (int i = 0; i < 4; i++)
			{
				std::getline(fis, str);
				//if (str.substr(0, 4) == "Mat:")
				//{
				//	fluo::Transform tform;
				//	for (int j = 0; j < 4; j++)
				//	{
				//		std::getline(fis, str);
				//		std::istringstream iss(str);
				//		iss >> tform.mat[j][0] >> tform.mat[j][1] >> tform.mat[j][2] >> tform.mat[j][3];
				//	}
				//	m_tform = new fluo::Transform(tform);
				//}
			}
		}
		else if (sline.substr(0, 10) == L"Components:")
		{
			std::getline(fis, str);
			int tab_counter = 0;
			for (size_t i=0; i<str.length(); ++i)
			{
				if (str[i] == L'\t')
					tab_counter++;
				if (tab_counter == 4)
				{
					m_info_meaning = str.substr(i+1, str.length()-i-1);
					break;
				}
			}

			while (std::getline(fis, str))
			{
				if (auto atext = GetAText(str))
					m_alist.push_back(atext);
			}
		}
	}

	m_data_path = filename;
	return 1;
}

void AnnotData::Save(const std::wstring &filename)
{
	std::wofstream os;
	OutputStreamOpenW(os, filename);

	int resx = 1;
	int resy = 1;
	int resz = 1;
	auto vd_ptr = m_vd.lock();
	if (vd_ptr)
		vd_ptr->GetResolution(resx, resy, resz);

	os << L"Name: " << m_name << L"\n";
	os << L"Display: " << m_disp << L"\n";
	os << L"Memo:\n" << m_memo << L"\n";
	os << L"Memo Update: " << m_memo_ro << L"\n";
	if (vd_ptr)
	{
		os << L"Volume: " << vd_ptr->GetName() << L"\n";
		os << L"Voxel size (X Y Z):\n";
		double spcx, spcy, spcz;
		vd_ptr->GetSpacings(spcx, spcy, spcz);
		os << spcx << L"\t" << spcy << L"\t" << spcz << L"\n";
	}


	os << L"\nComponents:\n";
	os << L"ID\tX\tY\tZ\t" << m_info_meaning << L"\n\n";
	for (auto& it : m_alist)
	{
		if (it)
		{
			os << it->m_txt << L"\t";
			os << int(it->m_pos.x()*resx+1.0) << L"\t";
			os << int(it->m_pos.y()*resy+1.0) << L"\t";
			os << int(it->m_pos.z()*resz+1.0) << L"\t";
			os << it->m_info << L"\n";
		}
	}

	os.close();
	m_data_path = filename;
}

std::wstring AnnotData::GetInfoMeaning()
{
	return m_info_meaning;
}

void AnnotData::SetInfoMeaning(const std::wstring &str)
{
	m_info_meaning = str;
}

//test if point is inside the clipping planes
bool AnnotData::InsideClippingPlanes(fluo::Point &pos)
{
	auto vd_ptr = m_vd.lock();
	if (!vd_ptr)
		return true;

	auto cb = vd_ptr->GetVR()->get_clipping_box();
	return cb.ContainsWorld(pos);
}

std::shared_ptr<AText> AnnotData::GetAText(const std::wstring& str)
{
	std::wstring sID;
	std::wstring sX;
	std::wstring sY;
	std::wstring sZ;
	std::wstring sInfo;
	int tab_counter = 0;

	size_t i = 0;
	for (auto c : str)
	{
		if (c == L'\t')
			tab_counter++;
		else
		{
			if (tab_counter == 0)
				sID += c;
			else if (tab_counter == 1)
				sX += c;
			else if (tab_counter == 2)
				sY += c;
			else if (tab_counter == 3)
				sZ += c;
			else if (tab_counter == 4)
			{
				sInfo = str.substr(i, str.length() - i);
				break;
			}
		}
		++i;
	}
	if (tab_counter == 4)
	{
		double x = WSTOD(sX);
		double y = WSTOD(sY);
		double z = WSTOD(sZ);
		int resx = 1;
		int resy = 1;
		int resz = 1;
		auto vd_ptr = m_vd.lock();
		if (vd_ptr)
			vd_ptr->GetResolution(resx, resy, resz);
		x /= resx?resx:1;
		y /= resy?resy:1;
		z /= resz?resz:1;
		fluo::Point pos(x, y, z);
		auto atext = std::make_shared<AText>(sID, pos);
		atext->SetInfo(sInfo);
		return atext;
	}

	return nullptr;
}

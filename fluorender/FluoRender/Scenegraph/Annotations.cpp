/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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

#include "Annotations.hpp"
#include <VolumeData.hpp>
#include <Global.hpp>
#include <Names.hpp>
#include <VolumeFactory.hpp>
#include <FLIVR/Texture.h>
#include <FLIVR/VolumeRenderer.h>
#include <compatibility.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace fluo;

Annotations::Annotations()
{
}

Annotations::Annotations(const Annotations& data, const CopyOp& copyop, bool copy_values) :
	Node(data, copyop, false)
{
	if (copy_values)
		copyValues(data, copyop);
}

Annotations::~Annotations()
{
}

int Annotations::LoadData(const std::wstring &filename)
{
	std::ifstream fis(ws2s(filename), std::ios::in);
	if (fis.bad())
		return 0;

	std::string str, sline, memo;

	while (!fis.eof())
	{
		std::getline(fis, sline);
		if (sline.substr(0, 6) == "Name: ")
		{
			m_name = sline.substr(6, sline.length() - 6);
		}
		else if (sline.substr(0, 9) == "Display: ")
		{
			str = sline.substr(9, 1);
			if (str == "0")
				setValue(gstDisplay, false);
			else
				setValue(gstDisplay, true);
		}
		else if (sline.substr(0, 5) == "Memo:")
		{
			std::getline(fis, str);
			while (str.substr(0, 13) != "Memo Update: " &&
				!fis.eof())
			{
				memo += str + "\n";
				std::getline(fis, str);
			}
			if (str.substr(13, 1) == "0")
				setValue(gstMemoRo, false);
			else
				setValue(gstMemoRo, true);
		}
		else if (sline.substr(0, 8) == "Volume: ")
		{
			str = sline.substr(8, sline.length() - 8);
			fluo::VolumeData* vd = glbin_volf->findFirst(str);
			if (vd)
			{
				setRefValue(gstVolume, vd);
				setValue(gstTransform,
					vd->GetTexture()->transform());
			}
		}
		else if (sline.substr(0, 10) == "Transform:")
		{
			std::getline(fis, str);
			std::getline(fis, str);
			std::getline(fis, str);
			std::getline(fis, str);
		}
		else if (sline.substr(0, 11) == "Components:")
		{
			std::getline(fis, str);
			int tab_counter = 0;
			for (int i = 0; i < (int)str.length(); i++)
			{
				if (str[i] == '\t')
					tab_counter++;
				if (tab_counter == 4)
				{
					setValue(gstInfoHeader, str.substr(i + 1, str.length()-i-1));
					break;
				}
			}

			std::getline(fis, str);
			while (!fis.eof())
			{
				Atext atext = buildAtext(str);
				alist_.push_back(atext);
				std::getline(fis, str);
			}
		}
	}

	setValue(gstDataPath, filename);
	return 1;
}

void Annotations::SaveData(const std::wstring &filename)
{
	std::ofstream os;
	std::string str = ws2s(filename);
	OutputStreamOpen(os, str);

	long resx = 1;
	long resy = 1;
	long resz = 1;
	Referenced* ref;
	getRefValue(gstVolume, &ref);
	VolumeData* vd = dynamic_cast<VolumeData*>(ref);
	if (vd)
	{
		vd->getValue(gstResX, resx);
		vd->getValue(gstResY, resy);
		vd->getValue(gstResZ, resz);
	}

	bool bval;
	os << "Name: " << getName() << "\n";
	getValue(gstDisplay, bval);
	os << "Display: " << bval << "\n";
	getValue(gstMemo, str);
	os << "Memo:\n" << str << "\n";
	getValue(gstMemoRo, bval);
	os << "Memo Update: " << bval << "\n";
	if (vd)
	{
		os << "Volume: " << vd->getName() << "\n";
		os << "Voxel size (X Y Z):\n";
		double spcx, spcy, spcz;
		vd->getValue(gstSpcX, spcx);
		vd->getValue(gstSpcY, spcy);
		vd->getValue(gstSpcZ, spcz);
		os << spcx << "\t" << spcy << "\t" << spcz << "\n";
	}


	os << "\nComponents:\n";
	getValue(gstInfoHeader, str);
	os << "ID\tX\tY\tZ\t" << str << "\n\n";
	for (size_t i = 0; i < alist_.size(); i++)
	{
		os << alist_[i].m_txt << "\t";
		os << int(alist_[i].m_pos.x()*resx + 1.0) << "\t";
		os << int(alist_[i].m_pos.y()*resy + 1.0) << "\t";
		os << int(alist_[i].m_pos.z()*resz + 1.0) << "\t";
		os << alist_[i].m_info << "\n";
	}

	os.close();
	setValue(gstDataPath, filename);
}

void Annotations::addText(const fluo::Point &pos,
	const std::string &str, const std::string &info)
{
	Atext atext;
	atext.m_pos = pos;
	atext.m_txt = str;
	atext.m_info = info;
	alist_.push_back(atext);
}

void Annotations::Draw(int nx, int ny, Transform &mv, Transform &p, bool persp)
{
	Transform tfm;
	getValue(gstTransform, tfm);
	Color color;
	getValue(gstColor, color);

	Point pos;
	std::string str;
	std::wstring wstr;
	float sx, sy;
	sx = 2.0 / nx;
	sy = 2.0 / ny;
	float px, py;
	for (size_t i = 0; i < alist_.size(); ++i)
	{
		pos = alist_[i].m_pos;
		str = alist_[i].m_txt;
		wstr = s2ws(str);
		if (!insideClippingPlanes(pos))
			continue;
		pos = tfm.transform(pos);
		pos = mv.transform(pos);
		pos = p.transform(pos);
		if (pos.x() >= -1.0 && pos.x() <= 1.0 &&
			pos.y() >= -1.0 && pos.y() <= 1.0)
		{
			if (persp && (pos.z() <= 0.0 || pos.z() >= 1.0))
				continue;
			if (!persp && (pos.z() >= 0.0 || pos.z() <= -1.0))
				continue;
			px = pos.x()*nx / 2.0;
			py = pos.y()*ny / 2.0;
			m_text_renderer.RenderText(
				wstr, color,
				px*sx, py*sy, sx, sy);
		}
	}
}

Atext Annotations::buildAtext(const std::string str)
{
	Atext atext;
	std::string sID;
	std::string sX;
	std::string sY;
	std::string sZ;
	std::string sInfo;
	int tab_counter = 0;

	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == '\t')
			tab_counter++;
		else
		{
			if (tab_counter == 0)
				sID += str[i];
			else if (tab_counter == 1)
				sX += str[i];
			else if (tab_counter == 2)
				sY += str[i];
			else if (tab_counter == 3)
				sZ += str[i];
			else if (tab_counter == 4)
			{
				sInfo = str.substr(i, str.length() - i);
				break;
			}
		}
	}
	if (tab_counter == 4)
	{
		double x = std::stod(sX);
		double y = std::stod(sY);
		double z = std::stod(sZ);
		long resx = 1;
		long resy = 1;
		long resz = 1;
		Referenced* ref;
		getRefValue(gstVolume, &ref);
		VolumeData* vd = dynamic_cast<VolumeData*>(ref);
		if (vd)
		{
			vd->getValue(gstResX, resx);
			vd->getValue(gstResY, resy);
			vd->getValue(gstResZ, resz);
		}
		x /= resx ? resx : 1;
		y /= resy ? resy : 1;
		z /= resz ? resz : 1;
		fluo::Point pos(x, y, z);
		atext.m_pos = pos;
		atext.m_txt = sID;
		atext.m_info = sInfo;
	}

	return atext;
}

//test if point is inside the clipping planes
bool Annotations::insideClippingPlanes(Point &pos)
{
	Referenced* ref;
	getRefValue(gstVolume, &ref);
	VolumeData* vd = dynamic_cast<VolumeData*>(ref);
	if (!vd)
		return true;

	std::vector<Plane*> *planes = vd->GetRenderer()->get_planes();
	if (!planes)
		return true;
	if (planes->size() != 6)
		return true;

	fluo::Plane* plane = 0;
	for (int i = 0; i < 6; i++)
	{
		plane = (*planes)[i];
		if (!plane)
			continue;
		if (plane->eval_point(pos) < 0)
			return false;
	}

	return true;
}


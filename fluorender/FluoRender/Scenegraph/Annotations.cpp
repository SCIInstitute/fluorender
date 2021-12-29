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

#include "Annotations.hpp"
#include <VolumeData.hpp>
#include <Global.hpp>
#include <Names.hpp>
#include <VolumeFactory.hpp>
#include <FLIVR/Texture.h>
#include <compatibility.h>
#include <iostream>
#include <fstream>

using namespace fluo;

AText::AText()
{
	addValue(gstText, std::string(""));
	addValue(gstLocation, Point());
	addValue(gstInfo, std::string(""));
}

AText::~AText()
{}

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

int Annotations::LoadData(const std::string &filename)
{
	std::ifstream fis(filename, std::ios::in);
	if (fis.bad())
		return 0;

	std::string str, sline, memo;

	while (std::getline(fis, sline))
	{

		if (sline.substr(0, 6) == "Name: ")
		{
			m_name = sline.substr(6, sline.length() - 7);
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
			str = sline.substr(8, sline.length() - 9);
			fluo::VolumeData* vd = glbin_volf->findFirst(str);
			if (vd)
			{
				setRvalu(gstVolume, vd);
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
					setValue(gstInfoHeader, str.substr(i + 1, str.length()));
					break;
				}
			}

			std::getline(fis, str);
			while (!fis.eof())
			{
				//if (AText* atext = GetAText(str))
				//	m_alist.push_back(atext);
				std::getline(fis, str);
			}
		}
	}

	setValue(gstDataPath, filename);
	return 1;
}

void Annotations::SaveData(const std::string &filename)
{
	std::ofstream os;
	OutputStreamOpen(os, filename);

/*	long resx = 1;
	long resy = 1;
	long resz = 1;
	if (m_vd)
	{
		m_vd->getValue(gstResX, resx);
		m_vd->getValue(gstResY, resy);
		m_vd->getValue(gstResZ, resz);
	}

	os << "Name: " << m_name << "\n";
	os << "Display: " << m_disp << "\n";
	os << "Memo:\n" << m_memo << "\n";
	os << "Memo Update: " << m_memo_ro << "\n";
	if (m_vd)
	{
		os << "Volume: " << m_vd->getName() << "\n";
		os << "Voxel size (X Y Z):\n";
		double spcx, spcy, spcz;
		m_vd->getValue(gstSpcX, spcx);
		m_vd->getValue(gstSpcY, spcy);
		m_vd->getValue(gstSpcZ, spcz);
		os << spcx << "\t" << spcy << "\t" << spcz << "\n";
	}


	os << "\nComponents:\n";
	os << "ID\tX\tY\tZ\t" << m_info_meaning << "\n\n";
	for (int i = 0; i < (int)m_alist.size(); i++)
	{
		AText* atext = m_alist[i];
		if (atext)
		{
			os << atext->m_txt << "\t";
			os << int(atext->m_pos.x()*resx + 1.0) << "\t";
			os << int(atext->m_pos.y()*resy + 1.0) << "\t";
			os << int(atext->m_pos.z()*resz + 1.0) << "\t";
			os << atext->m_info << "\n";
		}
	}
*/
	os.close();
	setValue(gstDataPath, filename);
}


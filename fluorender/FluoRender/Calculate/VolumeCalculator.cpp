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
#include "VolumeCalculator.h"
#include <Scenegraph/VolumeData.h>
#include <FLIVR/Texture.h>
#include <Global/Global.h>

using namespace FL;

VolumeCalculator::VolumeCalculator()
	: m_vd_r(0),
	m_vd_a(0),
	m_vd_b(0),
	m_type(0)
{
}

VolumeCalculator::~VolumeCalculator()
{
}

void VolumeCalculator::SetVolumeA(VolumeData *vd)
{
	m_vd_a = vd;
}

void VolumeCalculator::SetVolumeB(VolumeData *vd)
{
	m_vd_b = vd;
}

VolumeData* VolumeCalculator::GetVolumeA()
{
	return m_vd_a;
}

VolumeData* VolumeCalculator::GetVolumeB()
{
	return m_vd_b;
}

VolumeData* VolumeCalculator::GetResult()
{
	return m_vd_r;
}

void VolumeCalculator::Calculate(int type)
{
	m_type = type;
	m_vd_r = 0;

	switch (m_type)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 8://intersection with mask
		CreateVolumeResult2();
		if (!m_vd_r)
			return;
		m_vd_r->Calculate(m_type, m_vd_a, m_vd_b);
		return;
	case 5:
	case 6:
	case 7:
		if (!m_vd_a || !m_vd_a->GetMask(false))
			return;
		CreateVolumeResult1();
		if (!m_vd_r)
			return;
		m_vd_r->Calculate(m_type, m_vd_a, 0);
		return;
	case 9:
		if (!m_vd_a)
			return;
		CreateVolumeResult1();
		if (!m_vd_r)
			return;
		FillHoles(m_threshold);
		return;
	}
}

void VolumeCalculator::CreateVolumeResult1()
{
	if (!m_vd_a)
		return;

	long res_x, res_y, res_z;
	double spc_x, spc_y, spc_z;
	//m_vd_a->GetResolution(res_x, res_y, res_z);
	m_vd_a->getValue("res x", res_x);
	m_vd_a->getValue("res y", res_y);
	m_vd_a->getValue("res z", res_z);
	//m_vd_a->GetSpacings(spc_x, spc_y, spc_z);
	m_vd_a->getValue("spc x", spc_x);
	m_vd_a->getValue("spc y", spc_y);
	m_vd_a->getValue("spc z", spc_z);
	int brick_size = m_vd_a->GetTexture()->get_build_max_tex_size();

	//int bits = (m_vd_a->GetMaxValue()>255.0)?
	//  16:8;
	int bits = 8;  //it has an unknown problem with 16 bit data

	m_vd_r = Global::instance().getVolumeFactory()->clone(m_vd_a);
	m_vd_r->AddEmptyData(bits,
		res_x, res_y, res_z,
		spc_x, spc_y, spc_z,
		brick_size);
	//m_vd_r->SetSpcFromFile(true);
	m_vd_r->setValue("bits", long(bits));

	std::string name = m_vd_a->getName();
	std::string str_type;
	switch (m_type)
	{
	case 5://substraction
		str_type = "_EXTRACTED";
		break;
	case 6:
		str_type = "_DELETED";
		break;
	case 9:
		str_type = "_FILLED";
		break;
	}
	m_vd_r->setName(name + str_type);
}

void VolumeCalculator::CreateVolumeResult2()
{
	if (!m_vd_a || !m_vd_b)
		return;

	long res_x_a, res_y_a, res_z_a;
	long res_x_b, res_y_b, res_z_b;
	double spc_x_a, spc_y_a, spc_z_a;
	double spc_x_b, spc_y_b, spc_z_b;
	m_vd_a->getValue("res x", res_x_a);
	m_vd_a->getValue("res y", res_y_a);
	m_vd_a->getValue("res z", res_z_a);
	m_vd_b->getValue("res x", res_x_b);
	m_vd_b->getValue("res y", res_y_b);
	m_vd_b->getValue("res z", res_z_b);
	m_vd_a->getValue("spc x", spc_x_a);
	m_vd_a->getValue("spc y", spc_y_a);
	m_vd_a->getValue("spc z", spc_z_a);
	m_vd_b->getValue("spc x", spc_x_b);
	m_vd_b->getValue("spc y", spc_y_b);
	m_vd_b->getValue("spc z", spc_z_b);
	//m_vd_a->GetResolution(res_x_a, res_y_a, res_z_a);
	//m_vd_b->GetResolution(res_x_b, res_y_b, res_z_b);
	//m_vd_a->GetSpacings(spc_x_a, spc_y_a, spc_z_a);
	//m_vd_b->GetSpacings(spc_x_b, spc_y_b, spc_z_b);
	int brick_size = m_vd_a->GetTexture()->get_build_max_tex_size();

	//int bits = (m_vd_a->GetMaxValue()>255.0||m_vd_b->GetMaxValue()>255.0)?
	//  16:8;
	int bits = 8;  //it has an unknown problem with 16 bit data
	long res_x, res_y, res_z;
	double spc_x, spc_y, spc_z;

	res_x = std::max(res_x_a, res_x_b);
	res_y = std::max(res_y_a, res_y_b);
	res_z = std::max(res_z_a, res_z_b);
	spc_x = std::max(spc_x_a, spc_x_b);
	spc_y = std::max(spc_y_a, spc_y_b);
	spc_z = std::max(spc_z_a, spc_z_b);

	m_vd_r = Global::instance().getVolumeFactory()->clone(m_vd_a);
	m_vd_r->AddEmptyData(bits,
		res_x, res_y, res_z,
		spc_x, spc_y, spc_z,
		brick_size);
	//m_vd_r->SetSpcFromFile(true);
	m_vd_r->setValue("bits", long(bits));

	std::string name_a = m_vd_a->getName();
	std::string name_b = m_vd_b->getName();
	size_t len = 15;
	if (name_a.length() > len)
		name_a = name_a.substr(0, len);
	if (name_b.length() > len)
		name_b = name_b.substr(0, len);
	std::string str_type;
	switch (m_type)
	{
	case 1://substraction
		str_type = "_SUB_";
		break;
	case 2://addition
		str_type = "_ADD_";
		break;
	case 3://division
		str_type = "_DIV_";
		break;
	case 4://intersection
		str_type = "_AND_";
		break;
	}
	std::string name = name_a + str_type + name_b;
	m_vd_r->setName(name);
}

//fill holes
void VolumeCalculator::FillHoles(double thresh)
{
	if (!m_vd_a || !m_vd_r)
		return;

	FLIVR::Texture* tex_a = m_vd_a->GetTexture();
	if (!tex_a)
		return;
	Nrrd* nrrd_a = tex_a->get_nrrd(0);
	if (!nrrd_a)
		return;
	void* data_a = nrrd_a->data;
	if (!data_a)
		return;

	FLIVR::Texture* tex_r = m_vd_r->GetTexture();
	if (!tex_r)
		return;
	Nrrd* nrrd_r = tex_r->get_nrrd(0);
	if (!nrrd_r)
		return;
	void* data_r = nrrd_r->data;
	if (!data_r)
		return;

	//resolution
	long nx, ny, nz;
	//m_vd_a->GetResolution(nx, ny, nz);
	m_vd_a->getValue("res x", nx);
	m_vd_a->getValue("res y", ny);
	m_vd_a->getValue("res z", nz);

	//wxProgressDialog *prog_diag = new wxProgressDialog(
	//	"FluoRender: Voxel Consolidation",
	//	"Consolidating... Please wait.",
	//	100, 0,
	//	wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE);
	int progress = 0;
	int total_prg = nx * 2;

	int i, j, k;
	FLTYPE::BBox bbox;
	//first pass: finding BBox
	for (i = 0; i < nx; i++)
	{
		for (j = 0; j < ny; j++)
		for (k = 0; k < nz; k++)
		{
			int index = nx*ny*k + nx*j + i;
			unsigned char value_a = 0;
			if (nrrd_a->type == nrrdTypeUChar)
				value_a = ((unsigned char*)data_a)[index];
			else if (nrrd_a->type == nrrdTypeUShort)
			{
				double int_scale;
				m_vd_a->getValue("int scale", int_scale);
				value_a = (unsigned char)(int_scale * ((unsigned short*)data_a)[index] / 257.0);

			}
			if (value_a > thresh * 255)
			{
				bbox.extend(FLTYPE::Point(i, j, k));
				((unsigned char*)data_r)[index] = 255;
			}
		}
		//if (prog_diag)
		//{
			progress++;
		//	prog_diag->Update(95 * (progress + 1) / total_prg);
		//}
	}

	double dx = (bbox.Max() - bbox.Min()).x() / 2.0;
	double dy = (bbox.Max() - bbox.Min()).y() / 2.0;
	double dz = (bbox.Max() - bbox.Min()).z();

	//second pass: fill holes
	bool found_n, found_p;
	for (i = int(bbox.Min().x()); i <= int(bbox.Max().x()); i++)
	{
		for (j = int(bbox.Min().y()); j <= int(bbox.Max().y()); j++)
		for (k = int(bbox.Min().z()); k <= int(bbox.Max().z()); k++)
		{
			int index = nx*ny*k + nx*j + i;
			unsigned char value_r = ((unsigned char*)data_r)[index];
			if (!value_r)
			{
				//search index
				int si;
				//search -X
				int s_n_x = i;
				found_n = false;
				while (s_n_x >= int(bbox.Min().x()) &&
					s_n_x >= int(i - dx))
				{
					si = nx*ny*k + nx*j + s_n_x;
					if (((unsigned char*)data_r)[si])
					{
						found_n = true;
						break;
					}
					s_n_x--;
				}
				//search +X
				int s_p_x = i;
				found_p = false;
				while (s_p_x <= int(bbox.Max().x()) &&
					s_p_x <= int(i + dx))
				{
					si = nx*ny*k + nx*j + s_p_x;
					if (((unsigned char*)data_r)[si])
					{
						found_p = true;
						break;
					}
					s_p_x++;
				}
				//found X direction?
				if (!found_n || !found_p)
				{
					//((unsigned char*)data_r)[index] = 255;
					continue;
				}
				//search -Y
				int s_n_y = j;
				found_n = false;
				while (s_n_y >= int(bbox.Min().y()) &&
					s_n_y >= int(j - dy))
				{
					si = nx*ny*k + nx*s_n_y + i;
					if (((unsigned char*)data_r)[si])
					{
						found_n = true;
						break;
					}
					s_n_y--;
				}
				//search +Y
				int s_p_y = j;
				found_p = false;
				while (s_p_y <= int(bbox.Max().y()) &&
					s_p_y <= int(j + dy))
				{
					si = nx*ny*k + nx*s_p_y + i;
					if (((unsigned char*)data_r)[si])
					{
						found_p = true;
						break;
					}
					s_p_y++;
				}
				//found Y direction?
				if (!found_n || !found_p)
				{
					//((unsigned char*)data_r)[index] = 255;
					continue;
				}
				//search -Z
				int s_n_z = k;
				found_n = false;
				while (s_n_z >= int(bbox.Min().z()) &&
					s_n_z >= int(k - dz))
				{
					si = nx*ny*s_n_z + nx*j + i;
					if (((unsigned char*)data_r)[si])
					{
						found_n = true;
						break;
					}
					s_n_z--;
				}
				//search +Z
				int s_p_z = k;
				found_p = false;
				while (s_p_z <= int(bbox.Max().z()) &&
					s_p_z <= int(k + dz))
				{
					si = nx*ny*s_p_z + nx*j + i;
					if (((unsigned char*)data_r)[si])
					{
						found_p = true;
						break;
					}
					s_p_z++;
				}
				//found Z direction?
				if (found_p && found_n)
				{
					((unsigned char*)data_r)[index] = 255;
					//continue;
				}
			}
		}

		//if (prog_diag)
		//{
			progress++;
		//	prog_diag->Update(95 * (progress + 1) / total_prg);
		//}
	}
	//delete prog_diag;
}

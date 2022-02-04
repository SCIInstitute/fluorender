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
#include <Root.hpp>
#include <Renderview.hpp>
#include <VolumeData.hpp>
#include <VolumeGroup.hpp>
#include <Global.hpp>
#include <VolumeFactory.hpp>
#include <FLIVR/VolumeRenderer.h>
#include <FLIVR/Texture.h>
#include <Selection/VolumeSelector.h>
#include <wx/progdlg.h>

using namespace flrd;

VolumeCalculator::VolumeCalculator():
	m_root(0),
	m_view(0),
	m_selector(0),
	m_vd_a(0),
	m_vd_b(0),
	m_type(0)
{
}

VolumeCalculator::~VolumeCalculator()
{
}

void VolumeCalculator::SetVolumeA(fluo::VolumeData *vd)
{
	m_vd_a = vd;
}

void VolumeCalculator::SetVolumeB(fluo::VolumeData *vd)
{
	m_vd_b = vd;
}

fluo::VolumeData* VolumeCalculator::GetVolumeA()
{
	return m_vd_a;
}

fluo::VolumeData* VolumeCalculator::GetVolumeB()
{
	return m_vd_b;
}

fluo::VolumeData* VolumeCalculator::GetResult(bool pop)
{
	fluo::VolumeData* vd = 0;
	if (!m_vd_r.empty())
	{
		vd = m_vd_r.back();
		if (pop)
			m_vd_r.pop_back();
	}
	return vd;
}

void VolumeCalculator::CalculateSingle(int type, const std::string &prev_group, bool add)
{
	if (!m_view || !m_root)
		return;

	Calculate(type);
	fluo::VolumeData* vd = GetResult(add);
	fluo::VolumeData* vd_a = GetVolumeA();
	if (vd && vd_a)
	{
		//clipping planes
		std::vector<fluo::Plane*> *planes =
			vd_a->GetRenderer() ?
			vd_a->GetRenderer()->get_planes() : 0;
		if (planes && vd->GetRenderer())
			vd->GetRenderer()->set_planes(planes);

		if (type == 1 ||
			type == 2 ||
			type == 3 ||
			type == 4 ||
			type == 5 ||
			type == 6 ||
			type == 8 ||
			type == 9)
		{
				if (add)
				{
					//m_frame->GetDataManager()->AddVolumeData(vd);
					//vr_frame->GetDataManager()->SetVolumeDefault(vd);
					m_view->addVolumeData(vd, prev_group);

					if (type == 5 ||
						type == 6 ||
						type == 9)
					{
						vd_a->setValue(gstDisplay, false);
					}
					else if (type == 1 ||
						type == 2 ||
						type == 3 ||
						type == 4)
					{
						vd_a->setValue(gstDisplay, false);
						fluo::VolumeData* vd_b = GetVolumeB();
						if (vd_b)
							vd_b->setValue(gstDisplay, false);
					}
					//m_frame->UpdateList();
					//m_frame->UpdateTree(vd->getName());
				}
		}
		else if (type == 7)
		{
			vd_a->ReplaceData(vd);
			glbin_volf->remove(vd);
			//m_frame->GetPropView()->SetVolumeData(vd_a);
		}
		m_view->RefreshGL(5);
	}
}

void VolumeCalculator::CalculateGroup(int type, const std::string &prev_group, bool add)
{
	if (type == 5 ||
		type == 6 ||
		type == 7)
	{
		std::vector<fluo::VolumeData*> vd_list;
		if (m_selector && m_selector->GetSelectGroup())
		{
			fluo::VolumeData* vd = GetVolumeA();
			fluo::VolumeGroup* group = vd->getParentVolumeGroup();
			if (group && group->getNumChildren() > 1)
			{
				for (int i = 0; i < group->getNumChildren(); i++)
				{
					fluo::VolumeData* tmp_vd = group->getChild(i)->asVolumeData();
					if (!tmp_vd)
						continue;
					bool disp;
					tmp_vd->getValue(gstDisplay, disp);
					if (disp)
						vd_list.push_back(tmp_vd);
				}
				for (size_t i = 0; i < vd_list.size(); ++i)
				{
					SetVolumeA(vd_list[i]);
					CalculateSingle(type, prev_group, add);
				}
				SetVolumeA(vd);
			}
			else
				CalculateSingle(type, prev_group, add);
		}
		else
			CalculateSingle(type, prev_group, add);
	}
	else
		CalculateSingle(type, prev_group, add);
}

void VolumeCalculator::Calculate(int type)
{
	m_type = type;

	switch (m_type)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 8://intersection with mask
	{
		CreateVolumeResult2();
		fluo::VolumeData* vd = 0;
		if (!m_vd_r.empty())
			vd = m_vd_r.back();
		if (!vd)
			return;
		vd->Calculate(m_type, m_vd_a, m_vd_b);
		return;
	}
	case 5:
	case 6:
	case 7:
	{
		if (!m_vd_a || !m_vd_a->GetMask(false))
			return;
		CreateVolumeResult1();
		fluo::VolumeData* vd = 0;
		if (!m_vd_r.empty())
			vd = m_vd_r.back();
		if (!vd)
			return;
		vd->Calculate(m_type, m_vd_a, 0);
		return;
	}
	case 9:
	{
		if (!m_vd_a)
			return;
		CreateVolumeResult1();
		fluo::VolumeData* vd = 0;
		if (!m_vd_r.empty())
			vd = m_vd_r.back();
		if (!vd)
			return;
		FillHoles(m_threshold);
		return;
	}
	}
}

void VolumeCalculator::CreateVolumeResult1()
{
	if (!m_vd_a)
		return;

	int brick_size = m_vd_a->GetTexture()->get_build_max_tex_size();
	long res_x, res_y, res_z;
	double spc_x, spc_y, spc_z;
	m_vd_a->getValue(gstResX, res_x);
	m_vd_a->getValue(gstResY, res_y);
	m_vd_a->getValue(gstResZ, res_z);
	m_vd_a->getValue(gstSpcX, spc_x);
	m_vd_a->getValue(gstSpcY, spc_y);
	m_vd_a->getValue(gstSpcZ, spc_z);
	double max_int;
	m_vd_a->getValue(gstMaxInt, max_int);
	int bits = (max_int >255.0) ? 16 : 8;

	fluo::VolumeData* vd = glbin_volf->build(m_vd_a);
	vd->AddEmptyData(bits,
		res_x, res_y, res_z,
		spc_x, spc_y, spc_z,
		brick_size);
	vd->setValue(gstSpcFromFile, true);
	m_vd_r.push_back(vd);

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
	vd->setName(name + str_type);
}

void VolumeCalculator::CreateVolumeResult2()
{
	if (!m_vd_a || !m_vd_b)
		return;

	int brick_size = m_vd_a->GetTexture()->get_build_max_tex_size();
	long res_x_a, res_y_a, res_z_a;
	long res_x_b, res_y_b, res_z_b;
	double spc_x_a, spc_y_a, spc_z_a;
	double spc_x_b, spc_y_b, spc_z_b;
	m_vd_a->getValue(gstResX, res_x_a);
	m_vd_a->getValue(gstResY, res_y_a);
	m_vd_a->getValue(gstResZ, res_z_a);
	m_vd_a->getValue(gstSpcX, spc_x_a);
	m_vd_a->getValue(gstSpcY, spc_y_a);
	m_vd_a->getValue(gstSpcZ, spc_z_a);
	m_vd_b->getValue(gstResX, res_x_b);
	m_vd_b->getValue(gstResY, res_y_b);
	m_vd_b->getValue(gstResZ, res_z_b);
	m_vd_b->getValue(gstSpcX, spc_x_b);
	m_vd_b->getValue(gstSpcY, spc_y_b);
	m_vd_b->getValue(gstSpcZ, spc_z_b);
	double max_int_a, max_int_b;
	m_vd_a->getValue(gstMaxInt, max_int_a);
	m_vd_b->getValue(gstMaxInt, max_int_b);
	int bits = (max_int_a >255.0|| max_int_b >255.0)?16:8;
	long res_x, res_y, res_z;
	double spc_x, spc_y, spc_z;
	res_x = std::max(res_x_a, res_x_b);
	res_y = std::max(res_y_a, res_y_b);
	res_z = std::max(res_z_a, res_z_b);
	spc_x = std::max(spc_x_a, spc_x_b);
	spc_y = std::max(spc_y_a, spc_y_b);
	spc_z = std::max(spc_z_a, spc_z_b);

	fluo::VolumeData* vd = glbin_volf->build(m_vd_a);
	vd->AddEmptyData(bits,
		res_x, res_y, res_z,
		spc_x, spc_y, spc_z,
		brick_size);
	vd->setValue(gstSpcFromFile, true);
	m_vd_r.push_back(vd);

	std::string name_a = m_vd_a->getName();
	std::string name_b = m_vd_b->getName();
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
	vd->setName(name);
}

//fill holes
void VolumeCalculator::FillHoles(double thresh)
{
	if (!m_vd_a)
		return;
	fluo::VolumeData* vd = 0;
	if (!m_vd_r.empty())
		vd = m_vd_r.back();
	if (!vd)
		return;

	flvr::Texture* tex_a = m_vd_a->GetTexture();
	if (!tex_a)
		return;
	Nrrd* nrrd_a = tex_a->get_nrrd(0);
	if (!nrrd_a)
		return;
	void* data_a = nrrd_a->data;
	if (!data_a)
		return;

	flvr::Texture* tex_r = vd->GetTexture();
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
	m_vd_a->getValue(gstResX, nx);
	m_vd_a->getValue(gstResY, ny);
	m_vd_a->getValue(gstResZ, nz);
	//scale
	double int_scale;
	m_vd_a->getValue(gstIntScale, int_scale);

	wxProgressDialog *prog_diag = new wxProgressDialog(
		"FluoRender: Voxel Consolidation",
		"Consolidating... Please wait.",
		100, 0,
		wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE);
	int progress = 0;
	int total_prg = nx * 2;

	int i, j, k;
	fluo::BBox bbox;
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
				value_a = (unsigned char)((double)(((unsigned short*)data_a)[index])*int_scale / 257.0);
			if (value_a > thresh * 255)
			{
				bbox.extend(fluo::Point(i, j, k));
				((unsigned char*)data_r)[index] = 255;
			}
		}
		if (prog_diag)
		{
			progress++;
			prog_diag->Update(95 * (progress + 1) / total_prg);
		}
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

		if (prog_diag)
		{
			progress++;
			prog_diag->Update(95 * (progress + 1) / total_prg);
		}
	}
	delete prog_diag;
}

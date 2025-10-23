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
#include <VolumeCalculator.h>
#include <MainFrame.h>
#include <RenderView.h>
#include <VolumeData.h>
#include <VolumeGroup.h>
#include <CurrentObjects.h>
#include <DataManager.h>
#include <VolumePropPanel.h>
#include <VolumeDefault.h>
#include <Global.h>
#include <Names.h>
#include <Texture.h>
#include <VolumeRenderer.h>
#include <VolumeSelector.h>
#include <RenderScheduler.h>

using namespace flrd;

VolumeCalculator::VolumeCalculator():
	m_threshold(0),
	m_type(0),
	Progress()
{
}

VolumeCalculator::~VolumeCalculator()
{
}

void VolumeCalculator::SetVolumeA(const std::shared_ptr<VolumeData>& vd)
{
	m_vd_a = vd;
}

void VolumeCalculator::SetVolumeB(const std::shared_ptr<VolumeData>& vd)
{
	m_vd_b = vd;
}

std::shared_ptr<VolumeData> VolumeCalculator::GetVolumeA()
{
	return m_vd_a.lock();
}

std::shared_ptr<VolumeData> VolumeCalculator::GetVolumeB()
{
	return m_vd_b.lock();
}

std::shared_ptr<VolumeData> VolumeCalculator::GetResult(bool pop)
{
	if (!m_vd_r.empty())
	{
		auto vd = m_vd_r.back();
		if (pop)
			m_vd_r.pop_back();
		return vd;
	}
	return nullptr;
}

void VolumeCalculator::CalculateSingle(int type, const std::wstring& prev_group, bool add)
{
	bool update = false;
	bool refresh = false;

	Calculate(type);
	auto vd = GetResult(add);
	auto vd_a = GetVolumeA();
	if (vd && vd_a)
	{
		//clipping planes
		std::vector<fluo::Plane*> *planes = vd_a->GetVR() ? vd_a->GetVR()->get_planes() : 0;
		if (planes && vd->GetVR())
			vd->GetVR()->set_planes(planes);
		//transfer function
		glbin_vol_def.Copy(vd.get(), vd_a.get());

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
				glbin_data_manager.AddVolumeData(vd);
				//vr_frame->GetDataManager()->SetVolumeDefault(vd);
				if (auto cur_view = glbin_current.render_view.lock())
					cur_view->AddVolumeData(vd, prev_group);

				if (type == 5 ||
					type == 6 ||
					type == 9)
				{
					vd_a->SetDisp(false);
				}
				else if (type == 1 ||
					type == 2 ||
					type == 3 ||
					type == 4)
				{
					vd_a->SetDisp(false);
					auto vd_b = GetVolumeB();
					if (vd_b)
						vd_b->SetDisp(false);
				}
				glbin_current.SetVolumeData(vd);
				update = true;
			}
		}
		else if (type == 7)
		{
			VolumePropPanel* page = glbin_current.mainframe->FindVolumeProps(vd.get());
			vd_a->Replace(vd.get());
			page->SetVolumeData(vd_a.get());
		}
		refresh = true;
	}
	if (refresh)
	{
		if (update)
			glbin_current.mainframe->UpdateProps({ gstListCtrl, gstTreeCtrl, gstUpdateSync });
		glbin_render_scheduler_manager.requestDrawAll("Calculate refresh");
	}
}

void VolumeCalculator::CalculateGroup(int type, const std::wstring& prev_group, bool add)
{
	// If not type 5, 6, or 7, just calculate single
	if (type != 5 && type != 6 && type != 7)
	{
		CalculateSingle(type, prev_group, add);
		return;
	}

	// If group selection is not enabled, fallback to single
	if (!glbin_vol_selector.GetSelectGroup())
	{
		CalculateSingle(type, prev_group, add);
		return;
	}

	auto vd = GetVolumeA();
	auto view = glbin_current.render_view.lock();
	if (!vd || !view)
	{
		CalculateSingle(type, prev_group, add);
		return;
	}

	std::shared_ptr<VolumeGroup> group;
	for (int i = 0; i < view->GetLayerNum(); ++i)
	{
		auto layer = view->GetLayer(i);
		if (layer && layer->IsA() == 5)
		{
			auto tmp_group = std::dynamic_pointer_cast<VolumeGroup>(layer);
			for (int j = 0; j < tmp_group->GetVolumeNum(); ++j)
			{
				if (tmp_group->GetVolumeData(j) == vd)
				{
					group = tmp_group;
					break;
				}
			}
		}
		if (group) break;
	}

	if (group && group->GetVolumeNum() > 1)
	{
		std::vector<std::weak_ptr<VolumeData>> vd_list;
		for (int i = 0; i < group->GetVolumeNum(); ++i)
		{
			auto tmp_vd = group->GetVolumeData(i);
			if (tmp_vd && tmp_vd->GetDisp())
				vd_list.push_back(tmp_vd);
		}

		for (auto& weak_vd : vd_list)
		{
			SetVolumeA(weak_vd.lock());
			CalculateSingle(type, prev_group, add);
		}

		SetVolumeA(vd); // Restore original
	}
	else
	{
		CalculateSingle(type, prev_group, add);
	}
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
		if (m_vd_r.empty())
			return;
		if (auto vd = m_vd_r.back())
			vd->Calculate(m_type, m_vd_a.lock().get(), m_vd_b.lock().get());
		return;
	}
	case 5:
	case 6:
	case 7:
	{
		auto vd_a = m_vd_a.lock();
		if (!vd_a || !vd_a->GetMask(false))
			return;
		CreateVolumeResult1();
		if (m_vd_r.empty())
			return;
		if (auto vd = m_vd_r.back())
			vd->Calculate(m_type, vd_a.get(), 0);
		return;
	}
	case 9:
	{
		auto vd_a = m_vd_a.lock();
		if (!vd_a)
			return;
		CreateVolumeResult1();
		if (m_vd_r.empty())
			return;
		if (auto vd = m_vd_r.back())
			FillHoles(m_threshold);
		return;
	}
	}
}

void VolumeCalculator::CreateVolumeResult1()
{
	auto vd_a = m_vd_a.lock();
	if (!vd_a)
		return;

	int res_x, res_y, res_z;
	double spc_x, spc_y, spc_z;

	vd_a->GetResolution(res_x, res_y, res_z);
	vd_a->GetSpacings(spc_x, spc_y, spc_z);
	int brick_size = vd_a->GetTexture()->get_build_max_tex_size();

	int bits = (vd_a->GetMaxValue()>255.0) ? 16:8;
	//int bits = 8;  //it has an unknown problem with 16 bit data

	auto vd = std::make_shared<VolumeData>();
	vd->AddEmptyData(bits,
		res_x, res_y, res_z,
		spc_x, spc_y, spc_z,
		brick_size);
	vd->SetSpcFromFile(true);
	//vd->SetCurChannel(m_vd_a->GetCurChannel());
	m_vd_r.push_back(vd);

	std::wstring name = vd_a->GetName();
	std::wstring str_type;
	switch (m_type)
	{
	case 5://substraction
		str_type = L"_EXTRACTED";
		break;
	case 6:
		str_type = L"_DELETED";
		break;
	case 9:
		str_type = L"_FILLED";
		break;
	}
	vd->SetName(name + str_type);
}

void VolumeCalculator::CreateVolumeResult2()
{
	auto vd_a = m_vd_a.lock();
	auto vd_b = m_vd_b.lock();
	if (!vd_a || !vd_b)
		return;

	int res_x_a, res_y_a, res_z_a;
	int res_x_b, res_y_b, res_z_b;
	double spc_x_a, spc_y_a, spc_z_a;
	double spc_x_b, spc_y_b, spc_z_b;

	vd_a->GetResolution(res_x_a, res_y_a, res_z_a);
	vd_b->GetResolution(res_x_b, res_y_b, res_z_b);
	vd_a->GetSpacings(spc_x_a, spc_y_a, spc_z_a);
	vd_b->GetSpacings(spc_x_b, spc_y_b, spc_z_b);
	int brick_size = vd_a->GetTexture()->get_build_max_tex_size();

	int bits = (vd_a->GetMaxValue()>255.0||vd_b->GetMaxValue()>255.0) ? 16:8;
	//int bits = 8;  //it has an unknown problem with 16 bit data
	int res_x, res_y, res_z;
	double spc_x, spc_y, spc_z;

	res_x = std::max(res_x_a, res_x_b);
	res_y = std::max(res_y_a, res_y_b);
	res_z = std::max(res_z_a, res_z_b);
	spc_x = std::max(spc_x_a, spc_x_b);
	spc_y = std::max(spc_y_a, spc_y_b);
	spc_z = std::max(spc_z_a, spc_z_b);

	auto vd = std::make_shared<VolumeData>();
	vd->AddEmptyData(bits,
		res_x, res_y, res_z,
		spc_x, spc_y, spc_z,
		brick_size);
	vd->SetSpcFromFile(true);
	m_vd_r.push_back(vd);

	std::wstring name_a = vd_a->GetName();
	std::wstring name_b = vd_b->GetName();
	size_t len = 15;
	if (name_a.length() > len)
		name_a = name_a.substr(0, len);
	if (name_b.length() > len)
		name_b = name_b.substr(0, len);
	std::wstring str_type;
	switch (m_type)
	{
	case 1://substraction
		str_type = L"_SUB_";
		break;
	case 2://addition
		str_type = L"_ADD_";
		break;
	case 3://division
		str_type = L"_DIV_";
		break;
	case 4://intersection
		str_type = L"_AND_";
		break;
	}
	std::wstring name = name_a + str_type + name_b;
	vd->SetName(name);
}

//fill holes
void VolumeCalculator::FillHoles(double thresh)
{
	auto vd_a = m_vd_a.lock();
	if (!vd_a)
		return;
	VolumeData* vd = 0;
	if (!m_vd_r.empty())
		vd = m_vd_r.back().get();
	if (!vd)
		return;

	flvr::Texture* tex_a = vd_a->GetTexture();
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
	int nx, ny, nz;
	vd_a->GetResolution(nx, ny, nz);

	int progress = 0;
	int total_prg = nx * 2;
	SetProgress(0, "FluoRender is filling gaps in the volume. Please wait.");

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
				value_a = (unsigned char)((double)(((unsigned short*)data_a)[index])*vd_a->GetScalarScale() / 257.0);
			if (value_a > thresh * 255)
			{
				bbox.extend(fluo::Point(i, j, k));
				((unsigned char*)data_r)[index] = 255;
			}
		}

		progress++;
		SetProgress(100 * (progress + 1) / total_prg,
			"FluoRender is filling gaps in the volume. Please wait.");
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

		progress++;
		SetProgress(100 * (progress + 1) / total_prg,
			"FluoRender is filling gaps in the volume. Please wait.");
	}

	SetProgress(0, "");
}

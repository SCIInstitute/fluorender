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
#include <CombineList.h>
#include <Global.h>
#include <VolumeData.h>
#include <VolumeDefault.h>
#include <Texture.h>

using namespace flrd;

void CombineList::SetName(const std::wstring &name)
{
	m_name = name;
}

void CombineList::SetVolumes(const std::list<std::weak_ptr<VolumeData>>& channs)
{
	m_channs = channs;
}

std::list<std::shared_ptr<VolumeData>> CombineList::GetResults()
{
	return m_results;
}

int CombineList::Execute()
{
	m_results.clear();
	if (m_channs.empty())
		return 0;

	auto vd0 = (*m_channs.begin()).lock();
	if (!vd0)
		return 0;
	m_size = vd0->GetResolution();
	m_spacing = vd0->GetSpacing();
	m_bits = vd0->GetBits();
	int brick_size = vd0->GetTexture()->get_build_max_tex_size();
	if (m_name == L"")
		m_name = L"combined_volume";

	//red volume
	auto vd_r = std::make_shared<VolumeData>();
	vd_r->AddEmptyData(m_bits,
		m_size,
		m_spacing,
		brick_size);
	vd_r->SetSpcFromFile(true);
	vd_r->SetName(m_name + L"_CH_R");
	//green volume
	auto vd_g = std::make_shared<VolumeData>();
	vd_g->AddEmptyData(m_bits,
		m_size,
		m_spacing,
		brick_size);
	vd_g->SetSpcFromFile(true);
	vd_g->SetName(m_name + L"_CH_G");
	//blue volume
	auto vd_b = std::make_shared<VolumeData>();
	vd_b->AddEmptyData(m_bits,
		m_size,
		m_spacing,
		brick_size);
	vd_b->SetSpcFromFile(true);
	vd_b->SetName(m_name + L"_CH_B");

	//get new data
	//red volume
	flvr::Texture* tex_vd_r = vd_r->GetTexture();
	if (!tex_vd_r) return 0;
	auto comp_vd_r = tex_vd_r->get_nrrd(flvr::CompType::Data);
	if (!comp_vd_r.data) return 0;
	void* data_vd_r = comp_vd_r.data->data;
	if (!data_vd_r) return 0;
	//green volume
	flvr::Texture* tex_vd_g = vd_g->GetTexture();
	if (!tex_vd_g) return 0;
	auto comp_vd_g = tex_vd_g->get_nrrd(flvr::CompType::Data);
	if (!comp_vd_g.data) return 0;
	void* data_vd_g = comp_vd_g.data->data;
	if (!data_vd_g) return 0;
	//blue volume
	flvr::Texture* tex_vd_b = vd_b->GetTexture();
	if (!tex_vd_b) return 0;
	auto comp_vd_b = tex_vd_b->get_nrrd(flvr::CompType::Data);
	if (!comp_vd_b.data) return 0;
	void* data_vd_b = comp_vd_b.data->data;
	if (!data_vd_b) return 0;

	unsigned long long for_size = m_size.get_size_xyz();
	unsigned long long index;
	std::shared_ptr<VolumeData> volume;
	for (auto iter = m_channs.begin();
		iter != m_channs.end(); ++iter)
	{
		auto vd = iter->lock();
		if (vd->GetResolution() != m_size)
			continue;
		fluo::Color color = vd->GetColor();
		Nrrd* nrrd_iter = vd->GetVolume(false);
		if (!nrrd_iter)
			continue;
		void* data_iter = nrrd_iter->data;
		if (!data_iter)
			continue;
		for (index = 0; index < for_size; ++index)
		{
			if (m_bits == 8)
			{
				((unsigned char*)data_vd_r)[index] = Inc(
					((unsigned char*)data_vd_r)[index],
					(unsigned char)(color.r()*((unsigned char*)data_iter)[index] + 0.5));
				((unsigned char*)data_vd_g)[index] = Inc(
					((unsigned char*)data_vd_g)[index],
					(unsigned char)(color.g()*((unsigned char*)data_iter)[index] + 0.5));
				((unsigned char*)data_vd_b)[index] = Inc(
					((unsigned char*)data_vd_b)[index],
					(unsigned char)(color.b()*((unsigned char*)data_iter)[index] + 0.5));
			}
			else
			{
				((unsigned short*)data_vd_r)[index] = Inc(
					((unsigned short*)data_vd_r)[index],
					(unsigned short)(color.r()*((unsigned short*)data_iter)[index] + 0.5));
				((unsigned short*)data_vd_g)[index] = Inc(
					((unsigned short*)data_vd_g)[index],
					(unsigned short)(color.g()*((unsigned short*)data_iter)[index] + 0.5));
				((unsigned short*)data_vd_b)[index] = Inc(
					((unsigned short*)data_vd_b)[index],
					(unsigned short)(color.b()*((unsigned short*)data_iter)[index] + 0.5));
			}
		}
		if (!volume) volume = vd;
	}

	if (volume)
	{
		glbin_vol_def.Copy(vd_r.get(), volume.get());
		glbin_vol_def.Copy(vd_g.get(), volume.get());
		glbin_vol_def.Copy(vd_b.get(), volume.get());
	}
	fluo::Color red = fluo::Color(1.0, 0.0, 0.0);
	fluo::Color green = fluo::Color(0.0, 1.0, 0.0);
	fluo::Color blue = fluo::Color(0.0, 0.0, 1.0);
	vd_r->SetColor(red);
	vd_g->SetColor(green);
	vd_b->SetColor(blue);

	m_results.push_back(vd_r);
	m_results.push_back(vd_g);
	m_results.push_back(vd_b);

	return 1;
}

unsigned char CombineList::Inc(unsigned char base, unsigned char inc)
{
	double value = double(base) + double(inc);
	if (value > 255.0)
		return 255;
	else
		return base + inc;
}

unsigned short CombineList::Inc(unsigned short base, unsigned short inc)
{
	double value = double(base) + double(inc);
	if (value > 65535.0)
		return 65535;
	else
		return base + inc;
}

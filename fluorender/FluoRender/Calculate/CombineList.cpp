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
#include "CombineList.h"
#include <VolumeData.hpp>
#include <VolumeFactory.hpp>
#include <Global.hpp>
#include <FLIVR/Texture.h>

using namespace flrd;

void CombineList::SetName(const std::string &name)
{
	m_name = name;
}

void CombineList::SetVolumes(std::list<fluo::VolumeData*> &channs)
{
	m_channs = channs;
}

void CombineList::GetResults(std::list<fluo::VolumeData*> &results)
{
	results = m_results;
}

int CombineList::Execute()
{
	m_results.clear();
	if (m_channs.empty())
		return 0;

	fluo::VolumeData* fvd = *m_channs.begin();
	fvd->getValue("res x", m_resx);
	fvd->getValue("res y", m_resy);
	fvd->getValue("res z", m_resz);
	fvd->getValue("spc x", m_spcx);
	fvd->getValue("spc y", m_spcy);
	fvd->getValue("spc z", m_spcz);
	fvd->getValue("bits", m_bits);
	int brick_size = fvd->GetTexture()->get_build_max_tex_size();
	if (m_name == "")
		m_name = "combined_volume";

	//red volume
	fluo::VolumeData* vd_r = glbin_volf->build(fvd);
	vd_r->AddEmptyData(m_bits,
		m_resx, m_resy, m_resz,
		m_spcx, m_spcy, m_spcz,
		brick_size);
	vd_r->setValue("spc from file", true);
	vd_r->setName(m_name + "_CH_R");
	//green volume
	fluo::VolumeData* vd_g = glbin_volf->build(fvd);
	vd_g->AddEmptyData(m_bits,
		m_resx, m_resy, m_resz,
		m_spcx, m_spcy, m_spcz,
		brick_size);
	vd_g->setValue("spc from file", true);
	vd_g->setName(m_name + "_CH_G");
	//blue volume
	fluo::VolumeData* vd_b = glbin_volf->build(fvd);
	vd_b->AddEmptyData(m_bits,
		m_resx, m_resy, m_resz,
		m_spcx, m_spcy, m_spcz,
		brick_size);
	vd_b->setValue("spc from file", true);
	vd_b->setName(m_name + "_CH_B");

	//get new data
	//red volume
	flvr::Texture* tex_vd_r = vd_r->GetTexture();
	if (!tex_vd_r) return 0;
	Nrrd* nrrd_vd_r = tex_vd_r->get_nrrd(0);
	if (!nrrd_vd_r) return 0;
	void* data_vd_r = nrrd_vd_r->data;
	if (!data_vd_r) return 0;
	//green volume
	flvr::Texture* tex_vd_g = vd_g->GetTexture();
	if (!tex_vd_g) return 0;
	Nrrd* nrrd_vd_g = tex_vd_g->get_nrrd(0);
	if (!nrrd_vd_g) return 0;
	void* data_vd_g = nrrd_vd_g->data;
	if (!data_vd_g) return 0;
	//blue volume
	flvr::Texture* tex_vd_b = vd_b->GetTexture();
	if (!tex_vd_b) return 0;
	Nrrd* nrrd_vd_b = tex_vd_b->get_nrrd(0);
	if (!nrrd_vd_b) return 0;
	void* data_vd_b = nrrd_vd_b->data;
	if (!data_vd_b) return 0;

	unsigned long long for_size = (unsigned long long)m_resx *
		(unsigned long long)m_resy * (unsigned long long)m_resz;
	unsigned long long index;
	for (auto iter = m_channs.begin();
		iter != m_channs.end(); ++iter)
	{
		long nx, ny, nz;
		(*iter)->getValue("res x", nx);
		(*iter)->getValue("res y", ny);
		(*iter)->getValue("res z", nz);
		if (!(nx == m_resx && ny == m_resy && nz == m_resz))
			continue;
		fluo::Color color;
		(*iter)->getValue("color", color);
		Nrrd* nrrd_iter = (*iter)->GetData(false);
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
	}

	fluo::Color red = fluo::Color(1.0, 0.0, 0.0);
	fluo::Color green = fluo::Color(0.0, 1.0, 0.0);
	fluo::Color blue = fluo::Color(0.0, 0.0, 1.0);
	vd_r->setValue("color", red);
	vd_g->setValue("color", green);
	vd_b->setValue("color", blue);

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

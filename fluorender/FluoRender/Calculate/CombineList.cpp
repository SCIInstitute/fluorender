/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#include "DataManager.h"

using namespace FL;

void CombineList::SetName(wxString &name)
{
	m_name = name;
}

void CombineList::SetVolumes(std::list<VolumeData*> &channs)
{
	m_channs = channs;
}

void CombineList::GetResults(std::list<VolumeData*> &results)
{
	results = m_results;
}

int CombineList::Execute()
{
	m_results.clear();
	if (m_channs.empty())
		return 0;

	(*m_channs.begin())->GetResolution(m_resx, m_resy, m_resz);
	(*m_channs.begin())->GetSpacings(m_spcx, m_spcy, m_spcz);
	m_bits = (*m_channs.begin())->GetBits();
	if (m_name == "")
		m_name = "combined_volume";

	//red volume
	VolumeData* vd_r = new VolumeData();
	vd_r->AddEmptyData(m_bits,
		m_resx, m_resy, m_resz,
		m_spcx, m_spcy, m_spcz);
	vd_r->SetSpcFromFile(true);
	vd_r->SetName(m_name + wxString::Format("_CH_R"));
	//green volume
	VolumeData* vd_g = new VolumeData();
	vd_g->AddEmptyData(m_bits,
		m_resx, m_resy, m_resz,
		m_spcx, m_spcy, m_spcz);
	vd_g->SetSpcFromFile(true);
	vd_g->SetName(m_name + wxString::Format("_CH_G"));
	//blue volume
	VolumeData* vd_b = new VolumeData();
	vd_b->AddEmptyData(m_bits,
		m_resx, m_resy, m_resz,
		m_spcx, m_spcy, m_spcz);
	vd_b->SetSpcFromFile(true);
	vd_b->SetName(m_name + wxString::Format("_CH_B"));

	//get new data
	//red volume
	Texture* tex_vd_r = vd_r->GetTexture();
	if (!tex_vd_r) return 0;
	Nrrd* nrrd_vd_r = tex_vd_r->get_nrrd(0);
	if (!nrrd_vd_r) return 0;
	void* data_vd_r = nrrd_vd_r->data;
	if (!data_vd_r) return 0;
	//green volume
	Texture* tex_vd_g = vd_g->GetTexture();
	if (!tex_vd_g) return 0;
	Nrrd* nrrd_vd_g = tex_vd_g->get_nrrd(0);
	if (!nrrd_vd_g) return 0;
	void* data_vd_g = nrrd_vd_g->data;
	if (!data_vd_g) return 0;
	//blue volume
	Texture* tex_vd_b = vd_b->GetTexture();
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
		FLIVR::Color color = (*iter)->GetColor();
		Nrrd* nrrd_iter = (*iter)->GetVolume(false);
		if (!nrrd_iter)
			continue;
		void* data_iter = nrrd_iter->data;
		if (!data_iter)
			continue;
		for (index = 0; index < for_size; ++index)
		{
			if (m_bits == 8)
			{
				((unsigned char*)data_vd_r)[index] += (unsigned char)(color.r()*
					((unsigned char*)data_iter)[index] + 0.5);
				((unsigned char*)data_vd_g)[index] += (unsigned char)(color.g()*
					((unsigned char*)data_iter)[index] + 0.5);
				((unsigned char*)data_vd_b)[index] += (unsigned char)(color.b()*
					((unsigned char*)data_iter)[index] + 0.5);
			}
			else
			{
				((unsigned short*)data_vd_r)[index] += (unsigned short)(color.r()*
					((unsigned short*)data_iter)[index] + 0.5);
				((unsigned short*)data_vd_g)[index] += (unsigned short)(color.g()*
					((unsigned short*)data_iter)[index] + 0.5);
				((unsigned short*)data_vd_b)[index] += (unsigned short)(color.b()*
					((unsigned short*)data_iter)[index] + 0.5);
			}
		}
	}

	FLIVR::Color red = Color(1.0, 0.0, 0.0);
	FLIVR::Color green = Color(0.0, 1.0, 0.0);
	FLIVR::Color blue = Color(0.0, 0.0, 1.0);
	vd_r->SetColor(red);
	vd_g->SetColor(green);
	vd_b->SetColor(blue);


	return 1;
}
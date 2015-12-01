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
#include "CompAnalyzer.h"
#include "DataManager.h"
#include <sstream>
#include <iostream>

using namespace FL;

ComponentAnalyzer::ComponentAnalyzer(VolumeData* vd)
	: m_vd(vd)
{
}

ComponentAnalyzer::~ComponentAnalyzer()
{
}

void ComponentAnalyzer::Analyze(bool sel)
{
	if (!m_vd)
		return;

	Texture* tex = m_vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_data = tex->get_nrrd(0);
	if (!nrrd_data)
		return;
	int bits = nrrd_data->type;
	void* data_data = nrrd_data->data;
	if (!data_data)
		return;
	//get mask
	Nrrd* nrrd_mask = m_vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get label
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	//clear list and start calculating
	m_comp_list.clear();
	int ilist;
	int found;
	int nx, ny, nz;
	unsigned int id;
	double value;
	double scale;
	double delta;
	double ext;
	int i, j, k;
	m_vd->GetResolution(nx, ny, nz);
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	unsigned long long index;
	CompUList comp_ulist;
	CompUListIter iter;
	for (index = 0; index < for_size; ++index)
	{
		if (sel && !data_mask[index])
			continue;
		if (!data_label[index])
			continue;

		id = data_label[index];
		if (bits == nrrdTypeUChar)
		{
			value = ((unsigned char*)data_data)[index] / 255.0;
			scale = 255.0;
		}
		else if (bits == nrrdTypeUShort)
		{
			value = ((unsigned short*)data_data)[index] / 65535.0;
			scale = 65535.0;
		}

		if (value <= 0.0)
			continue;

		k = index / (nx*ny);
		j = index % (nx*ny);
		i = j % nx;
		j = j / nx;
		ext = GetExt(data_label, index, id, nx, ny, nz, i, j, k);

		//find in list
		iter = comp_ulist.find(id);
		if (iter == comp_ulist.end())
		{
			//not found
			CompInfo info;
			info.id = id;
			info.sumi = 1;
			info.sumd = value;
			info.ext_sumi = ext;
			info.ext_sumd = value * ext;
			info.mean = 0.0;
			info.var = 0.0;
			info.m2 = 0.0;
			delta = value - info.mean;
			info.mean += delta / info.sumi;
			info.m2 += delta * (value - info.mean);
			info.min = value;
			info.max = value;
			info.pos = FLIVR::Point(i, j, k);
			comp_ulist.insert(pair<unsigned int, CompInfo>
				(id, info));
		}
		else
		{
			iter->second.pos = FLIVR::Point((iter->second.pos * iter->second.sumi +
				FLIVR::Point(i, j, k)) / (iter->second.sumi + 1));
			//
			iter->second.sumi++;
			iter->second.sumd += value;
			iter->second.ext_sumi += ext;
			iter->second.ext_sumd += value * ext;
			//
			delta = value - iter->second.mean;
			iter->second.mean += delta / iter->second.sumi;
			iter->second.m2 += delta * (value - iter->second.mean);
			iter->second.min = value < iter->second.min ? value : iter->second.min;
			iter->second.max = value > iter->second.max ? value : iter->second.max;
		}
	}

	for (iter = comp_ulist.begin();
		iter != comp_ulist.end(); ++iter)
	{
		if (iter->second.sumi > 0)
			iter->second.var = sqrt(iter->second.m2 / (iter->second.sumi));
		iter->second.mean *= scale;
		iter->second.min *= scale;
		iter->second.max *= scale;
		m_comp_list.push_back(iter->second);
	}

	m_comp_list.sort();
	//std::sort(m_comp_list.begin(), m_comp_list.end(), CompInfo::cmp_id);
}

void ComponentAnalyzer::OutputCompList(std::string &str)
{
	ostringstream oss;
	oss << "Statistics on the selection:\n";
	oss << "A total of " <<
		m_comp_list.size() <<
		" component(s) selected\n";
	oss << "ID\tSumN\tSumI\tSurfaceN\tSurfaceI\tMean\tSigma\tMin\tMax\n";
	for (auto i = m_comp_list.begin();
		i != m_comp_list.end(); ++i)
	{
		oss << i->id << "\t";
		oss << i->sumi << "\t";
		oss << i->sumd << "\t";
		oss << i->ext_sumi << "\t";
		oss << i->ext_sumd << "\t";
		oss << i->mean << "\t";
		oss << i->var << "\t";
		oss << i->min << "\t";
		oss << i->max << "\n";
	}
	str = oss.str();
}

unsigned int ComponentAnalyzer::GetExt(unsigned int* data_label,
	unsigned long long index,
	unsigned int id,
	int nx, int ny, int nz,
	int i, int j, int k)
{
	bool surface_vox, contact_vox;
	unsigned long long indexn;
	//determine the numbers
	if (i == 0 || i == nx - 1 ||
		j == 0 || j == ny - 1 ||
		k == 0 || k == nz - 1)
	{
		//border voxel
		surface_vox = true;
		//determine contact
		contact_vox = false;
		if (i > 0)
		{
			indexn = index - 1;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				contact_vox = true;
		}
		if (!contact_vox && i < nx - 1)
		{
			indexn = index + 1;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				contact_vox = true;
		}
		if (!contact_vox && j > 0)
		{
			indexn = index - nx;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				contact_vox = true;
		}
		if (!contact_vox && j < ny - 1)
		{
			indexn = index + nx;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				contact_vox = true;
		}
		if (!contact_vox && k > 0)
		{
			indexn = index - nx*ny;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				contact_vox = true;
		}
		if (!contact_vox && k < nz - 1)
		{
			indexn = index + nx*ny;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				contact_vox = true;
		}
	}
	else
	{
		surface_vox = false;
		contact_vox = false;
		//i-1
		indexn = index - 1;
		if (data_label[indexn] == 0)
			surface_vox = true;
		if (data_label[indexn] &&
			data_label[indexn] != id)
			surface_vox = contact_vox = true;
		//i+1
		if (!surface_vox || !contact_vox)
		{
			indexn = index + 1;
			if (data_label[indexn] == 0)
				surface_vox = true;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				surface_vox = contact_vox = true;
		}
		//j-1
		if (!surface_vox || !contact_vox)
		{
			indexn = index - nx;
			if (data_label[indexn] == 0)
				surface_vox = true;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				surface_vox = contact_vox = true;
		}
		//j+1
		if (!surface_vox || !contact_vox)
		{
			indexn = index + nx;
			if (data_label[indexn] == 0)
				surface_vox = true;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				surface_vox = contact_vox = true;
		}
		//k-1
		if (!surface_vox || !contact_vox)
		{
			indexn = index - nx*ny;
			if (data_label[indexn] == 0)
				surface_vox = true;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				surface_vox = contact_vox = true;
		}
		//k+1
		if (!surface_vox || !contact_vox)
		{
			indexn = index + nx*ny;
			if (data_label[indexn] == 0)
				surface_vox = true;
			if (data_label[indexn] &&
				data_label[indexn] != id)
				surface_vox = contact_vox = true;
		}
	}

	return surface_vox ? 1 : 0;
}

bool ComponentAnalyzer::GenAnnotations(Annotations &ann)
{
	if (!m_vd)
		return false;

	Texture* tex = m_vd->GetTexture();
	if (!tex)
		return false;
	Nrrd* nrrd_data = tex->get_nrrd(0);
	if (!nrrd_data)
		return false;
	int bits = nrrd_data->type;
	double scale;
	if (bits == nrrdTypeUChar)
		scale = 255.0;
	else if (bits == nrrdTypeUShort)
		scale = 65535.0;
	double spcx, spcy, spcz;
	m_vd->GetSpacings(spcx, spcy, spcz);
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);

	if (m_comp_list.empty())
		Analyze(true);

	double total_int = 0.0;
	unsigned int sum = 0;
	std::string sinfo;
	ostringstream oss;
	for (auto i = m_comp_list.begin();
		i != m_comp_list.end(); ++i)
	{
		oss.str("");
		oss << i->sumi << "\t";
		oss << double(i->sumi)*spcx*spcy*spcz << "\t";
		oss << i->mean;
		sinfo = oss.str();
		ann.AddText(std::to_string(i->id),
			FLIVR::Point(i->pos.x()/nx, i->pos.y()/ny, i->pos.z()/nz),
			sinfo);
		total_int += i->sumd * scale;
		sum += i->sumi;
	}
	return true;
}
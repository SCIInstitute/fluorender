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
#include <limits>

using namespace FL;

ComponentAnalyzer::ComponentAnalyzer(VolumeData* vd)
	: m_vd(vd),
	m_comp_list_dirty(true)
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
	unsigned char* data_mask = 0;
	if (nrrd_mask)
		data_mask = (unsigned char*)(nrrd_mask->data);
	//get label
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	unsigned int* data_label = 0;
	if (nrrd_label)
		data_label = (unsigned int*)(nrrd_label->data);
	if (!data_mask && !data_label)
		return;

	//clear list and start calculating
	m_comp_list.clear();
	int ilist;
	int found;
	int nx, ny, nz;
	unsigned int id = 0;
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
		if (sel)
		{
			if (data_mask && !data_mask[index])
			continue;
		}
		if (data_label && !data_label[index])
			continue;

		if (data_label)
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

	m_comp_list.min = std::numeric_limits<unsigned int>::max();
	m_comp_list.max = 0;
	for (iter = comp_ulist.begin();
		iter != comp_ulist.end(); ++iter)
	{
		if (iter->second.sumi > 0)
			iter->second.var = sqrt(iter->second.m2 / (iter->second.sumi));
		iter->second.mean *= scale;
		iter->second.min *= scale;
		iter->second.max *= scale;
		m_comp_list.min = iter->second.sumi <
			m_comp_list.min ? iter->second.sumi :
			m_comp_list.min;
		m_comp_list.max = iter->second.sumi >
			m_comp_list.max ? iter->second.sumi :
			m_comp_list.max;
		m_comp_list.push_back(iter->second);
	}

	m_comp_list.sort();
	m_comp_list_dirty = false;
}

void ComponentAnalyzer::OutputFormHeader(std::string &str)
{
	str = "ID\tSumN\tSumI\tSurfaceN\tSurfaceI\tMean\tSigma\tMin\tMax\n";
}

void ComponentAnalyzer::OutputCompList(std::string &str, int verbose, std::string comp_header)
{
	ostringstream oss;
	if (verbose == 1)
	{
		oss << "Statistics on the selection:\n";
		oss << "A total of " <<
			m_comp_list.size() <<
			" component(s) selected\n";
		std::string header;
		OutputFormHeader(header);
		oss << header;
	}
	for (auto i = m_comp_list.begin();
		i != m_comp_list.end(); ++i)
	{
		if (comp_header != "")
		{
			if (i == m_comp_list.begin())
				oss << comp_header << "\t";
			else
				oss << "\t";
		}
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
	if (!data_label)
		return 0;
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

	if (m_comp_list.empty() ||
		m_comp_list_dirty)
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

bool ComponentAnalyzer::GenMultiChannels(std::list<VolumeData*>& channs, int color_type)
{
	if (!m_vd)
		return false;

	if (m_comp_list.empty() ||
		m_comp_list_dirty)
		Analyze(true);

	Texture* tex = m_vd->GetTexture();
	if (!tex)
		return false;
	Nrrd* nrrd_data = tex->get_nrrd(0);
	if (!nrrd_data)
		return false;
	int bits = 8;
	if (nrrd_data->type == nrrdTypeUChar)
		bits = 8;
	else if (nrrd_data->type == nrrdTypeUShort)
		bits = 16;
	void* data_data = nrrd_data->data;
	if (!data_data)
		return false;
	//get label
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return false;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return false;
	double spcx, spcy, spcz;
	m_vd->GetSpacings(spcx, spcy, spcz);
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	double amb, diff, spec, shine;
	m_vd->GetMaterial(amb, diff, spec, shine);

	unsigned int count = 1;
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	unsigned long long index;
	unsigned int value_label;
	for (auto i = m_comp_list.begin();
		i != m_comp_list.end(); ++i)
	{
		VolumeData* vd = new VolumeData();
		vd->AddEmptyData(bits,
			nx, ny, nz,
			spcx, spcy, spcz);
		vd->SetSpcFromFile(true);
		vd->SetName(m_vd->GetName() +
			wxString::Format("_COMP%d_SIZE%d", count++, i->sumi));

		//populate the volume
		//the actual data
		Texture* tex_vd = vd->GetTexture();
		if (!tex_vd) continue;
		Nrrd* nrrd_vd = tex_vd->get_nrrd(0);
		if (!nrrd_vd) continue;
		void* data_vd = nrrd_vd->data;
		if (!data_vd) continue;
		for (index = 0; index < for_size; ++index)
		{
			value_label = data_label[index];
			if (value_label == i->id)
			{
				if (bits == 8)
					((unsigned char*)data_vd)[index] = ((unsigned char*)data_data)[index];
				else
					((unsigned short*)data_vd)[index] = ((unsigned short*)data_data)[index];
			}
		}

		//settings
        Color c = GetColor(*i, m_vd, color_type);
		vd->SetColor(c);
		vd->SetEnableAlpha(m_vd->GetEnableAlpha());
		vd->SetShading(m_vd->GetShading());
		vd->SetShadow(false);
		//other settings
		vd->Set3DGamma(m_vd->Get3DGamma());
		vd->SetBoundary(m_vd->GetBoundary());
		vd->SetOffset(m_vd->GetOffset());
		vd->SetLeftThresh(m_vd->GetLeftThresh());
		vd->SetRightThresh(m_vd->GetRightThresh());
		vd->SetAlpha(m_vd->GetAlpha());
		vd->SetSampleRate(m_vd->GetSampleRate());
		vd->SetMaterial(amb, diff, spec, shine);

		channs.push_back(vd);
	}
	return true;
}

bool ComponentAnalyzer::GenRgbChannels(std::list<VolumeData*> &channs, int color_type)
{
	if (!m_vd)
		return false;

	if (m_comp_list.empty() ||
		m_comp_list_dirty)
		Analyze(true);

	Texture* tex = m_vd->GetTexture();
	if (!tex)
		return false;
	Nrrd* nrrd_data = tex->get_nrrd(0);
	if (!nrrd_data)
		return false;
	int bits = 8;
	if (nrrd_data->type == nrrdTypeUChar)
		bits = 8;
	else if (nrrd_data->type == nrrdTypeUShort)
		bits = 16;
	void* data_data = nrrd_data->data;
	if (!data_data)
		return false;
	//get label
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return false;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return false;
	double spcx, spcy, spcz;
	m_vd->GetSpacings(spcx, spcy, spcz);
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	double amb, diff, spec, shine;
	m_vd->GetMaterial(amb, diff, spec, shine);

	//red volume
	VolumeData* vd_r = new VolumeData();
	vd_r->AddEmptyData(bits,
		nx, ny, nz,
		spcx, spcy, spcz);
	vd_r->SetSpcFromFile(true);
	vd_r->SetName(m_vd->GetName() +
		wxString::Format("_CH_R"));
	//green volume
	VolumeData* vd_g = new VolumeData();
	vd_g->AddEmptyData(bits,
		nx, ny, nz,
		spcx, spcy, spcz);
	vd_g->SetSpcFromFile(true);
	vd_g->SetName(m_vd->GetName() +
		wxString::Format("_CH_G"));
	//blue volume
	VolumeData* vd_b = new VolumeData();
	vd_b->AddEmptyData(bits,
		nx, ny, nz,
		spcx, spcy, spcz);
	vd_b->SetSpcFromFile(true);
	vd_b->SetName(m_vd->GetName() +
		wxString::Format("_CH_B"));

	//get new data
	//red volume
	Texture* tex_vd_r = vd_r->GetTexture();
	if (!tex_vd_r) return false;
	Nrrd* nrrd_vd_r = tex_vd_r->get_nrrd(0);
	if (!nrrd_vd_r) return false;
	void* data_vd_r = nrrd_vd_r->data;
	if (!data_vd_r) return false;
	//green volume
	Texture* tex_vd_g = vd_g->GetTexture();
	if (!tex_vd_g) return false;
	Nrrd* nrrd_vd_g = tex_vd_g->get_nrrd(0);
	if (!nrrd_vd_g) return false;
	void* data_vd_g = nrrd_vd_g->data;
	if (!data_vd_g) return false;
	//blue volume
	Texture* tex_vd_b = vd_b->GetTexture();
	if (!tex_vd_b) return false;
	Nrrd* nrrd_vd_b = tex_vd_b->get_nrrd(0);
	if (!nrrd_vd_b) return false;
	void* data_vd_b = nrrd_vd_b->data;
	if (!data_vd_b) return false;

	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	unsigned long long index;
	unsigned int value_label;
	Color color;
	for (index = 0; index < for_size; ++index)
	{
		value_label = data_label[index];
		auto i = std::find(m_comp_list.begin(),
			m_comp_list.end(), CompInfo(value_label));
		if (i != m_comp_list.end())
		{
			color = GetColor(*i, m_vd, color_type);
			if (bits == 8)
			{
				double value = ((unsigned char*)data_data)[index];
				((unsigned char*)data_vd_r)[index] = (unsigned char)(color.r()*value);
				((unsigned char*)data_vd_g)[index] = (unsigned char)(color.g()*value);
				((unsigned char*)data_vd_b)[index] = (unsigned char)(color.b()*value);
			}
			else
			{
				double value = ((unsigned short*)data_data)[index];
				((unsigned short*)data_vd_r)[index] = (unsigned short)(color.r()*value);
				((unsigned short*)data_vd_g)[index] = (unsigned short)(color.g()*value);
				((unsigned short*)data_vd_b)[index] = (unsigned short)(color.b()*value);
			}
		}
	}

	FLIVR::Color red = Color(1.0, 0.0, 0.0);
	FLIVR::Color green = Color(0.0, 1.0, 0.0);
	FLIVR::Color blue = Color(0.0, 0.0, 1.0);
	vd_r->SetColor(red);
	vd_g->SetColor(green);
	vd_b->SetColor(blue);

	bool bval = m_vd->GetEnableAlpha();
	vd_r->SetEnableAlpha(bval);
	vd_g->SetEnableAlpha(bval);
	vd_b->SetEnableAlpha(bval);
	bval = m_vd->GetShading();
	vd_r->SetShading(bval);
	vd_g->SetShading(bval);
	vd_b->SetShading(bval);
	vd_r->SetShadow(false);
	vd_g->SetShadow(false);
	vd_b->SetShadow(false);
	//other settings
	double dval = m_vd->Get3DGamma();
	vd_r->Set3DGamma(dval);
	vd_g->Set3DGamma(dval);
	vd_b->Set3DGamma(dval);
	dval = m_vd->GetBoundary();
	vd_r->SetBoundary(dval);
	vd_g->SetBoundary(dval);
	vd_b->SetBoundary(dval);
	dval = m_vd->GetOffset();
	vd_r->SetOffset(dval);
	vd_g->SetOffset(dval);
	vd_b->SetOffset(dval);
	dval = m_vd->GetLeftThresh();
	vd_r->SetLeftThresh(dval);
	vd_g->SetLeftThresh(dval);
	vd_b->SetLeftThresh(dval);
	dval = m_vd->GetRightThresh();
	vd_r->SetRightThresh(dval);
	vd_g->SetRightThresh(dval);
	vd_b->SetRightThresh(dval);
	dval = m_vd->GetAlpha();
	vd_r->SetAlpha(dval);
	vd_g->SetAlpha(dval);
	vd_b->SetAlpha(dval);
	dval = m_vd->GetSampleRate();
	vd_r->SetSampleRate(dval);
	vd_g->SetSampleRate(dval);
	vd_b->SetSampleRate(dval);
	vd_r->SetMaterial(amb, diff, spec, shine);
	vd_g->SetMaterial(amb, diff, spec, shine);
	vd_b->SetMaterial(amb, diff, spec, shine);

	channs.push_back(vd_r);
	channs.push_back(vd_g);
	channs.push_back(vd_b);

	return true;
}

FLIVR::Color ComponentAnalyzer::GetColor(CompInfo &comp_info,
	VolumeData* vd, int color_type)
{
	FLIVR::Color color;
	switch (color_type)
	{
	case 1:
	default:
		color = FLIVR::Color(HSVColor(comp_info.id % 360, 1.0, 1.0));
		break;
	case 2:
		if (vd)
		{
			double value;
			if (m_comp_list.min == m_comp_list.max)
				value = 1.0;
			else
				value = double(comp_info.sumi - m_comp_list.min) /
				double(m_comp_list.max - m_comp_list.min);
			color = vd->GetColorFromColormap(value);
		}
		break;
	}
	return color;
}
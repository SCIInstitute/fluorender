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
#include "DataManager.h"
#include <VolumeData.hpp>
#include <MeshData.hpp>
#include <Annotations.hpp>
#include <Global.hpp>
#include <VolumeFactory.hpp>
#include <MeshFactory.hpp>
#include <AnnotationFactory.hpp>
#include <Formats/base_reader.h>
#include <Formats/oib_reader.h>
#include <Formats/oif_reader.h>
#include <Formats/nrrd_reader.h>
#include <Formats/tif_reader.h>
#include <Formats/nrrd_writer.h>
#include <Formats/tif_writer.h>
#include <Formats/msk_reader.h>
#include <Formats/msk_writer.h>
#include <Formats/lsm_reader.h>
#include <Formats/lbl_reader.h>
#include <Formats/pvxml_reader.h>
#include <Formats/brkxml_reader.h>
#include <Formats/imageJ_reader.h>
#include <Formats/czi_reader.h>
#include <Formats/nd2_reader.h>
#include <Formats/lif_reader.h>
#include <Formats/lof_reader.h>
#include <wx/stdpaths.h>

DataManager::DataManager() :
m_vol_exb(0.0),
	m_vol_gam(1.0),
	m_vol_of1(1.0),
	m_vol_of2(1.0),
	m_vol_lth(0.0),
	m_vol_hth(1.0),
	m_vol_lsh(0.5),
	m_vol_hsh(10.0),
	m_vol_alf(0.5),
	m_vol_spr(1.5),
	m_vol_xsp(1.0),
	m_vol_ysp(1.0),
	m_vol_zsp(2.5),
	m_vol_lum(1.0),
	m_vol_cmm(0),
	m_vol_cmi(false),
	m_vol_cmp(0),
	m_vol_cmj(0),
	m_vol_lcm(0.0),
	m_vol_hcm(1.0),
	m_vol_eap(true),
	m_vol_esh(true),
	m_vol_interp(true),
	m_vol_inv(false),
	m_vol_mip(false),
	m_vol_trp(false),
	m_vol_com(0),
	m_vol_nrd(false),
	m_vol_shw(false),
	m_vol_swi(0.0),
	m_vol_test_wiref(false),
	m_use_defaults(true),
	m_override_vox(true),
	m_ser_num(0)
{
	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = wxPathOnly(expath);
	wxString dft = expath + GETSLASH() + "default_volume_settings.dft";
	wxFileInputStream is(dft);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	double val;
	int ival;
	if (fconfig.Read("extract_boundary", &val))
		m_vol_exb = val;
	if (fconfig.Read("gamma", &val))
		m_vol_gam = val;
	if (fconfig.Read("low_offset", &val))
		m_vol_of1 = val;
	if (fconfig.Read("high_offset", &val))
		m_vol_of2 = val;
	if (fconfig.Read("low_thresholding", &val))
		m_vol_lth = val;
	if (fconfig.Read("high_thresholding", &val))
		m_vol_hth = val;
	if (fconfig.Read("low_shading", &val))
		m_vol_lsh = val;
	if (fconfig.Read("high_shading", &val))
		m_vol_hsh = val;
	if (fconfig.Read("alpha", &val))
		m_vol_alf = val;
	if (fconfig.Read("sample_rate", &val))
		m_vol_spr = val;
	if (fconfig.Read("x_spacing", &val))
		m_vol_xsp = val;
	if (fconfig.Read("y_spacing", &val))
		m_vol_ysp = val;
	if (fconfig.Read("z_spacing", &val))
		m_vol_zsp = val;
	if (fconfig.Read("luminance", &val))
		m_vol_lum = val;
	if (fconfig.Read("colormap_mode", &ival))
		m_vol_cmm = ival;
	bool bval;
	if (fconfig.Read("colormap_inv", &bval))
		m_vol_cmi = bval;
	if (fconfig.Read("colormap", &ival))
		m_vol_cmp = ival;
	if (fconfig.Read("colormap_proj", &ival))
		m_vol_cmj = ival;

	if (fconfig.Read("colormap_low", &val))
		m_vol_lcm = val;
	if (fconfig.Read("colormap_hi", &val))
		m_vol_hcm = val;

	if (fconfig.Read("enable_alpha", &bval))
		m_vol_eap = bval;
	if (fconfig.Read("enable_shading", &bval))
		m_vol_esh = bval;
	if (fconfig.Read("enable_interp", &bval))
		m_vol_interp = bval;
	if (fconfig.Read("enable_inv", &bval))
		m_vol_inv = bval;
	if (fconfig.Read("enable_mip", &bval))
		m_vol_mip = bval;
	if (fconfig.Read("enable_trp", &bval))
		m_vol_trp = bval;
	if (fconfig.Read("enable_comp", &ival))
		m_vol_com = ival;
	if (fconfig.Read("noise_rd", &bval))
		m_vol_nrd = bval;

	//shadow
	if (fconfig.Read("enable_shadow", &bval))
		m_vol_shw = bval;
	if (fconfig.Read("shadow_intensity", &val))
		m_vol_swi = val;

	//wavelength to color table
	m_vol_wav[0] = fluo::Color(1.0, 1.0, 1.0);
	m_vol_wav[1] = fluo::Color(1.0, 1.0, 1.0);
	m_vol_wav[2] = fluo::Color(1.0, 1.0, 1.0);
	m_vol_wav[3] = fluo::Color(1.0, 1.0, 1.0);

	//slice sequence
	m_sliceSequence = false;
	//read channels
	m_channSequence = false;
	//digit order
	m_digitOrder = 0;
	//compression
	m_compression = false;
	//skip brick
	m_skip_brick = false;
	//time sequence identifier
	m_timeId = "_T";
	//load mask
	m_load_mask = true;
}

DataManager::~DataManager()
{
	for (int i=0; i<(int)m_reader_list.size(); i++)
		if (m_reader_list[i])
			delete m_reader_list[i];
}

void DataManager::ClearAll()
{
	glbin_volf->removeAll();
	glbin_mshf->removeAll();
	glbin_annf->removeAll();
	for (int i=0; i<(int)m_reader_list.size(); i++)
		if (m_reader_list[i])
			delete m_reader_list[i];
	m_reader_list.clear();
}

void DataManager::SetVolumeDefault(fluo::VolumeData* vd)
{
	if (m_use_defaults)
	{
		glbin_volf->propValuesToDefault(vd);
	}
}

//set project path
//when data and project are moved, use project file's path
//if data's directory doesn't exist
void DataManager::SetProjectPath(wxString path)
{
	m_prj_path.Clear();
	m_prj_path = wxPathOnly(path);
}

fluo::VolumeData* DataManager::GetVolumeData(int index)
{
	return glbin_volf->get(index);
}

fluo::MeshData* DataManager::GetMeshData(int index)
{
	return glbin_mshf->get(index);
}

fluo::VolumeData* DataManager::GetVolumeData(const std::string &name)
{
	return glbin_volf->findFirst(name);
}

fluo::MeshData* DataManager::GetMeshData(const std::string &name)
{
	return glbin_mshf->findFirst(name);
}

int DataManager::GetVolumeIndex(const std::string &name)
{
	return glbin_volf->getIndex(name);
}

fluo::VolumeData* DataManager::GetLastVolumeData()
{
	return glbin_volf->getLast();
}

int DataManager::GetMeshIndex(const std::string &name)
{
	return glbin_mshf->getIndex(name);
}

fluo::MeshData* DataManager::GetLastMeshData()
{
	return glbin_mshf->getLast();
}

void DataManager::RemoveVolumeData(int index)
{
	glbin_volf->remove(index);
}

void DataManager::RemoveVolumeData(const std::string &name)
{
	glbin_volf->remove(name);
}

void DataManager::RemoveMeshData(int index)
{
	glbin_mshf->remove(index);
}

int DataManager::GetVolumeNum()
{
	return glbin_volf->getNum();
}

int DataManager::GetMeshNum()
{
	return glbin_mshf->getNum();
}

void DataManager::AddVolumeData(fluo::VolumeData* vd)
{
	if (!vd)
		return;

	wxString name = vd->getName();
	wxString new_name = name;

	int i;
	for (i=1; CheckNames(new_name); i++)
		new_name = name+wxString::Format("_%d", i);

	if (i>1)
		vd->setName(new_name.ToStdString());

	if (m_override_vox)
	{
		//if (m_vd_list.size() > 0)
		//{
		//	double spcx, spcy, spcz;
		//	m_vd_list[0]->GetBaseSpacings(spcx, spcy, spcz);
		//	vd->SetSpacings(spcx, spcy, spcz);
		//	vd->SetBaseSpacings(spcx, spcy, spcz);
		//	//vd->SetSpcFromFile(true);
		//}
	}
}

fluo::VolumeData* DataManager::DuplicateVolumeData(fluo::VolumeData* vd)
{
	return glbin_volf->build(vd);
	//VolumeData* vd_new = 0;

	//if (vd)
	//{
	//	vd_new = new VolumeData(*vd);
	//	AddVolumeData(vd_new);
	//}

	//return vd_new;
}

int DataManager::LoadAnnotations(wxString &filename)
{
	wxString pathname = filename;
	if (!wxFileExists(pathname))
	{
		pathname = SearchProjectPath(filename);
		if (!wxFileExists(pathname))
			return 0;
	}

	fluo::Annotations* ann = glbin_annf->build();
	ann->LoadData(pathname.ToStdWstring());

	wxString name = ann->getName();
	wxString new_name = name;
	int i;
	for (i=1; CheckNames(new_name); i++)
		new_name = name+wxString::Format("_%d", i);
	if (i>1)
		ann->setName(new_name.ToStdString());
	//m_annotation_list.push_back(ann);

	return 1;
}

void DataManager::AddAnnotations(fluo::Annotations* ann)
{
	if (!ann)
		return;

	wxString name = ann->getName();
	wxString new_name = name;

	int i;
	for (i=1; CheckNames(new_name); i++)
		new_name = name+wxString::Format("_%d", i);

	if (i>1)
		ann->setName(new_name.ToStdString());

	//m_annotation_list.push_back(ann);
}

void DataManager::RemoveAnnotations(int index)
{
	glbin_annf->remove(index);
	//Annotations* ann = m_annotation_list[index];
	//if (ann)
	//{
	//	m_annotation_list.erase(m_annotation_list.begin()+index);
	//	delete ann;
	//	ann = 0;
	//}
}

int DataManager::GetAnnotationNum()
{
	return glbin_annf->getNum();
}

fluo::Annotations* DataManager::GetAnnotations(int index)
{
	return glbin_annf->get(index);
}

fluo::Annotations* DataManager::GetAnnotations(const std::string &name)
{
	return glbin_annf->findFirst(name);
}

int DataManager::GetAnnotationIndex(const std::string &name)
{
	return glbin_annf->getIndex(name);
}

fluo::Annotations* DataManager::GetLastAnnotations()
{
	return glbin_annf->getLast();
}

bool DataManager::CheckNames(wxString &str)
{
	bool result = false;
	if (glbin_volf->findFirst(str.ToStdString()))
		result = true;
	if (glbin_mshf->findFirst(str.ToStdString()))
		result = true;
	if (glbin_annf->findFirst(str.ToStdString()))
		result = true;
	return result;
}

fluo::Color DataManager::GetColor(int c)
{
	fluo::Color result(1.0, 1.0, 1.0);
	switch (c)
	{
	case 1://red
		result = fluo::Color(1.0, 0.0, 0.0);
		break;
	case 2://green
		result = fluo::Color(0.0, 1.0, 0.0);
		break;
	case 3://blue
		result = fluo::Color(0.0, 0.0, 1.0);
		break;
	case 4://cyan
		result = fluo::Color(0.0, 1.0, 1.0);
		break;
	case 5://magenta
		result = fluo::Color(1.0, 0.0, 1.0);
		break;
	case 6://yellow
		result = fluo::Color(1.0, 1.0, 0.0);
		break;
	case 7://orange
		result = fluo::Color(1.0, 0.5, 0.0);
		break;
	case 8://white
		result = fluo::Color(1.0, 1.0, 1.0);
		break;
	}
	return result;
}

void DataManager::SetWavelengthColor(int c1, int c2, int c3, int c4)
{
	m_vol_wav[0] = GetColor(c1);
	m_vol_wav[1] = GetColor(c2);
	m_vol_wav[2] = GetColor(c3);
	m_vol_wav[3] = GetColor(c4);
}

fluo::Color DataManager::GetWavelengthColor(double wavelength)
{
	if (wavelength < 340.0)
		return fluo::Color(1.0, 1.0, 1.0);
	else if (wavelength < 440.0)
		return m_vol_wav[0];
	else if (wavelength < 500.0)
		return m_vol_wav[1];
	else if (wavelength < 600.0)
		return m_vol_wav[2];
	else if (wavelength < 750.0)
		return m_vol_wav[3];
	else
		return fluo::Color(1.0, 1.0, 1.0);
}

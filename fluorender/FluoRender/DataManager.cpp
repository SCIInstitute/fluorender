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

wxString DataManager::SearchProjectPath(wxString &filename)
{
	int i;

	wxString pathname = filename;

	if (m_prj_path == "")
		return "";
	wxString search_str;
	for (i = pathname.Length() - 1; i >= 0; i--)
	{
		if (pathname[i] == '\\' || pathname[i] == '/')
		{
			search_str.Prepend(GETSLASH());
			wxString name_temp = m_prj_path + search_str;
			if (wxFileExists(name_temp))
				return name_temp;
		}
		else
			search_str.Prepend(pathname[i]);
	}
	return "";
}

int DataManager::LoadVolumeData(wxString &filename, int type, bool withImageJ, int ch_num, int t_num)
{
	bool isURL = false;
	bool downloaded = false;
	wxString downloaded_filepath;
	bool downloaded_metadata = false;
	wxString downloaded_metadatafilepath;

	wxString pathname = filename;
	if (!wxFileExists(pathname))
	{
		pathname = SearchProjectPath(filename);
		if (!wxFileExists(pathname))
			return 0;
	}

	int i;
	int result = 0;
	BaseReader* reader = 0;

	for (i=0; i<(int)m_reader_list.size(); i++)
	{
		wstring wstr = pathname.ToStdWstring();
		if (m_reader_list[i]->Match(wstr))
		{
			reader = m_reader_list[i];
			break;
		}
	}

	int reader_return = -1;
	if (reader)
	{
		bool preprocess = false;
		if (reader->GetSliceSeq() != m_sliceSequence)
		{
			reader->SetSliceSeq(m_sliceSequence);
			preprocess = true;
		}
		if (reader->GetChannSeq() != m_channSequence)
		{
			reader->SetChannSeq(m_channSequence);
			preprocess = true;
		}
		if (reader->GetDigitOrder() != m_digitOrder)
		{
			reader->SetDigitOrder(m_digitOrder);
			preprocess = true;
		}
		if (reader->GetTimeId() != m_timeId.ToStdWstring())
		{
			wstring str_w = m_timeId.ToStdWstring();
			reader->SetTimeId(str_w);
			preprocess = true;
		}
		if (preprocess)
			reader_return = reader->Preprocess();
	}
	else
	{
		//RGB tiff
		//TODO: Loading with imageJ irrespective of the file type.
		if (withImageJ == true)
			reader = new ImageJReader();
		else {
			if (type == LOAD_TYPE_TIFF)
				reader = new TIFReader();
			else if (type == LOAD_TYPE_NRRD)
				reader = new NRRDReader();
			else if (type == LOAD_TYPE_OIB)
				reader = new OIBReader();
			else if (type == LOAD_TYPE_OIF)
				reader = new OIFReader();
			else if (type == LOAD_TYPE_LSM)
				reader = new LSMReader();
			else if (type == LOAD_TYPE_PVXML)
			{
				reader = new PVXMLReader();
				((PVXMLReader*)reader)->SetFlipX(m_pvxml_flip_x);
				((PVXMLReader*)reader)->SetFlipY(m_pvxml_flip_y);
				((PVXMLReader*)reader)->SetSeqType(m_pvxml_seq_type);
			}
			else if (type == LOAD_TYPE_BRKXML)
				reader = new BRKXMLReader();
			else if (type == LOAD_TYPE_CZI)
				reader = new CZIReader();
			else if (type == LOAD_TYPE_ND2)
				reader = new ND2Reader();
			else if (type == LOAD_TYPE_LIF)
				reader = new LIFReader();
			else if (type == LOAD_TYPE_LOF)
				reader = new LOFReader();
		}
		
		
		m_reader_list.push_back(reader);
		wstring str_w = pathname.ToStdWstring();
		reader->SetFile(str_w);
		reader->SetSliceSeq(m_sliceSequence);
		reader->SetChannSeq(m_channSequence);
		reader->SetDigitOrder(m_digitOrder);
		str_w = m_timeId.ToStdWstring();
		reader->SetTimeId(str_w);
		reader_return = reader->Preprocess();
	}

	if (reader_return > 0)
	{
		string err_str = BaseReader::GetError(reader_return);
		wxMessageBox(err_str);
		int i = (int)m_reader_list.size() - 1;		
		if (m_reader_list[i]) {
			delete m_reader_list[i];
			m_reader_list.erase(m_reader_list.begin() + (int)m_reader_list.size() - 1);
		}
		return result;
	}

	//align data for compression if vtc is not supported
	if (!GLEW_NV_texture_compression_vtc && m_compression)
	{
		reader->SetResize(1);
		reader->SetAlignment(4);
	}

	if (m_ser_num > 0)
		reader->LoadBatch(m_ser_num);
	int chan = reader->GetChanNum();
	for (i=(ch_num>=0?ch_num:0);
		i<(ch_num>=0?ch_num+1:chan); i++)
	{
		fluo::VolumeData *vd = glbin_volf->build();
		if (!vd)
			continue;

		vd->setValue(gstSkipBrick, m_skip_brick);
		Nrrd* data = reader->Convert(t_num>=0?t_num:reader->GetCurTime(), i, true);
		if (!data)
			continue;

		wxString name;
		if (type != LOAD_TYPE_BRKXML)
		{
			name = wxString(reader->GetDataName());
			if (chan > 1)
				name += wxString::Format("_Ch%d", i + 1);
		}
		else
		{
			BRKXMLReader* breader = (BRKXMLReader*)reader;
			name = reader->GetDataName();
			name = name.Mid(0, name.find_last_of(wxT('.')));
			if (ch_num > 1) name = wxT("_Ch") + wxString::Format("%i", i);
			pathname = filename;
			breader->SetCurChan(i);
			breader->SetCurTime(0);
		}

		vd->SetReader(reader);
		vd->setValue(gstCompression, m_compression);

		bool valid_spc = reader->IsSpcInfoValid();
		if (vd->LoadData(data, name.ToStdString(), pathname.ToStdWstring()))
		{
			if (m_load_mask)
			{
				//mask
				MSKReader msk_reader;
				std::wstring str = reader->GetCurMaskName(t_num >= 0 ? t_num : reader->GetCurTime(), i);
				msk_reader.SetFile(str);
				Nrrd* mask = msk_reader.Convert(0, 0, true);
				if (mask)
					vd->LoadMask(mask);
				//label mask
				LBLReader lbl_reader;
				str = reader->GetCurLabelName(t_num >= 0 ? t_num : reader->GetCurTime(), i);
				lbl_reader.SetFile(str);
				Nrrd* label = lbl_reader.Convert(0, 0, true);
				if (label)
					vd->LoadLabel(label);
			}
			if (type == LOAD_TYPE_BRKXML) ((BRKXMLReader*)reader)->SetLevel(0);
			//for 2D data
			long xres, yres, zres;
			vd->getValue(gstResX, xres);
			vd->getValue(gstResY, yres);
			vd->getValue(gstResZ, zres);
			double zspcfac = (double)std::max(xres, yres) / 256.0;
			if (zspcfac < 1.0) zspcfac = 1.0;
			vd->setValue(gstSpcFromFile, valid_spc);
			if (valid_spc)
			{
				vd->setValue(gstBaseSpcX, reader->GetXSpc());
				vd->setValue(gstBaseSpcY, reader->GetYSpc());
				if (zres == 1)
					vd->setValue(gstBaseSpcZ, reader->GetXSpc()*zspcfac);
				else
					vd->setValue(gstBaseSpcZ, reader->GetZSpc());
			}
			vd->setValue(gstIntScale, reader->GetScalarScale());
			vd->setValue(gstMaxInt, reader->GetMaxValue());
			vd->setValue(gstTime, reader->GetCurTime());
			vd->setValue(gstChannel, i);
			//++
			result++;
		}
		else
		{
			glbin_volf->remove(vd);
			continue;
		}

		SetVolumeDefault(vd);
		AddVolumeData(vd);

		//get excitation wavelength
		double wavelength = reader->GetExcitationWavelength(i);
		if (wavelength > 0.0) {
			fluo::Color col = GetWavelengthColor(wavelength);
			vd->setValue(gstColor, col);
		}
		else if (wavelength < 0.) {
			fluo::Color white(1.0, 1.0, 1.0);
			vd->setValue(gstColor, white);
		}
		else
		{
			fluo::Color white(1.0, 1.0, 1.0);
			fluo::Color red(1.0, 0.0, 0.0);
			fluo::Color green(0.0, 1.0, 0.0);
			fluo::Color blue(0.0, 0.0, 1.0);
			if (chan == 1) {
				vd->setValue(gstColor, white);
			}
			else
			{
				if (i == 0)
					vd->setValue(gstColor, red);
				else if (i == 1)
					vd->setValue(gstColor, green);
				else if (i == 2)
					vd->setValue(gstColor, blue);
				else
					vd->setValue(gstColor, white);
			}
		}

	}

	return result;
}

int DataManager::LoadMeshData(wxString &filename)
{
	wxString pathname = filename;
	if (!wxFileExists(pathname))
	{
		pathname = SearchProjectPath(filename);
		if (!wxFileExists(pathname))
			return 0;
	}

	fluo::MeshData *md = glbin_mshf->build();
	md->LoadData(pathname.ToStdWstring());

	wxString name = md->getName();
	wxString new_name = name;
	int i;
	for (i=1; CheckNames(new_name); i++)
		new_name = name+wxString::Format("_%d", i);
	if (i>1)
		md->setName(new_name.ToStdString());
	//m_md_list.push_back(md);

	return 1;
}

int DataManager::LoadMeshData(GLMmodel* mesh)
{
	if (!mesh) return 0;

	fluo::MeshData *md = glbin_mshf->build();
	md->LoadData(mesh);

	wxString name = md->getName();
	wxString new_name = name;
	int i;
	for (i=1; CheckNames(new_name); i++)
		new_name = name+wxString::Format("_%d", i);
	if (i>1)
		md->setName(new_name.ToStdString());
	//m_md_list.push_back(md);

	return 1;
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

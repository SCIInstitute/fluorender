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

#include <RenderFrameAgent.hpp>
#include <Main.h>
#include <RenderFrame.h>
#include <RenderviewPanel.h>
#include <Global.hpp>
#include <AgentFactory.hpp>
#include <VolumeFactory.hpp>
#include <MeshFactory.hpp>
#include <AnnotationFactory.hpp>
#include <MovieAgent.hpp>
#include <VolumeData.hpp>
#include <RenderviewFactory.hpp>
#include <Texture.h>
#include <TextureRenderer.h>
#include <imageJ_reader.h>
#include <tif_reader.h>
#include <nrrd_reader.h>
#include <oib_reader.h>
#include <oif_reader.h>
#include <lsm_reader.h>
#include <pvxml_reader.h>
#include <brkxml_reader.h>
#include <czi_reader.h>
#include <nd2_reader.h>
#include <lif_reader.h>
#include <lof_reader.h>
#include <msk_reader.h>
#include <lbl_reader.h>
#include <wx/string.h>
#include <wx/progdlg.h>

using namespace fluo;

RenderFrameAgent::RenderFrameAgent(RenderFrame &frame) :
	InterfaceAgent(),
	frame_(frame)
{
}

void RenderFrameAgent::setupInputs()
{

}

void RenderFrameAgent::setObject(Root* obj)
{
	InterfaceAgent::setObject(obj);
}

Root* RenderFrameAgent::getObject()
{
	return dynamic_cast<Root*>(InterfaceAgent::getObject());
}

void RenderFrameAgent::UpdateFui(const ValueCollection &names)
{
	bool update_all = names.empty();
}

std::vector<std::string> RenderFrameAgent::GetJvmArgs()
{
	std::vector<std::string> args;
	std::string jvm_path, ij_path, bioformats_path;
	getValue(gstJvmPath, jvm_path);
	getValue(gstImagejPath, ij_path);
	getValue(gstBioformatsPath, bioformats_path);
	args.push_back(jvm_path);
	args.push_back(ij_path);
	args.push_back(bioformats_path);
	return args;
}

void RenderFrameAgent::SaveProject(const std::wstring &filename)
{

}

void RenderFrameAgent::OpenProject(const std::wstring &filename)
{
	std::wstring wsval = GET_PATH(filename);
	setValue(gstProjectPath, wsval);
}

void RenderFrameAgent::StartupLoad(wxArrayString files, bool run_mov, bool with_imagej)
{
	//if (m_vrv_list[0])
	//	m_vrv_list[0]->m_glview->Init();

	if (files.Count())
	{
		wxString filename = files[0];
		wxString suffix = filename.Mid(filename.Find('.', true)).MakeLower();

		if (suffix == ".vrp")
		{
			OpenProject(files[0].ToStdWstring());
		}
		else if (suffix == ".nrrd" ||
			suffix == ".msk" ||
			suffix == ".lbl" ||
			suffix == ".tif" ||
			suffix == ".tiff" ||
			suffix == ".oib" ||
			suffix == ".oif" ||
			suffix == ".lsm" ||
			suffix == ".xml" ||
			suffix == ".vvd" ||
#ifndef _DARWIN
			suffix == ".nd2" ||
#endif
			suffix == ".czi" ||
			suffix == ".lif" ||
			suffix == ".lof")
		{
			LoadVolumes(files, with_imagej);
		}
		else if (suffix == ".obj")
		{
			LoadMeshes(files);
		}
		else if (with_imagej)
		{
			LoadVolumes(files, with_imagej);
		}
	}

	if (run_mov)
		glbin_agtf->getMovieAgent()->Run();
}

void RenderFrameAgent::LoadVolumes(wxArrayString files, bool withImageJ)
{
	int j;

	VolumeData* vd_sel = 0;
	VolumeGroup* group_sel = 0;
	Renderview* view = glbin_root->getCurrentRenderview();

	wxProgressDialog *prg_diag = 0;
	if (view)
	{
		bool streaming; getValue(gstStreamEnable, streaming);
		double gpu_size; getValue(gstGpuMemSize, gpu_size);
		double data_size; getValue(gstLargeDataSize, data_size);
		long brick_size; getValue(gstBrickSize, brick_size);
		long resp_time; getValue(gstResponseTime, resp_time);
		wxString str_streaming;
		if (streaming)
		{
			str_streaming = "Large data streaming is currently ON\n";
			str_streaming += wxString::Format("FluoRender will use %dMB GPU Memory\n", int(gpu_size));
			str_streaming += wxString::Format("Data channel larger than %dMB will be divided into bricks of %d voxels\n",
				int(data_size), brick_size);
			str_streaming += wxString::Format("System response time is %dms", resp_time);
		}
		else
			str_streaming = "Large data streaming is currently OFF";

		prg_diag = new wxProgressDialog(
			"FluoRender: Loading volume data...",
			"",
			100, 0, wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_AUTO_HIDE);

		bool enable_4d = false;

		for (j = 0; j < (int)files.Count(); j++)
		{
			wxGetApp().Yield();
			prg_diag->Update(90 * (j + 1) / (int)files.Count(),
				str_streaming);

			int ch_num = 0;
			wxString filename = files[j];
			wxString suffix = filename.Mid(filename.Find('.', true)).MakeLower();

			if (withImageJ)
				ch_num = LoadVolumeData(filename, VLT_ImageJ, true); //The type of data doesnt matter.
			else if (suffix == ".nrrd" || suffix == ".msk" || suffix == ".lbl")
				ch_num = LoadVolumeData(filename, VLT_NRRD, false);
			else if (suffix == ".tif" || suffix == ".tiff")
				ch_num = LoadVolumeData(filename, VLT_TIFF, false);
			else if (suffix == ".oib")
				ch_num = LoadVolumeData(filename, VLT_OIB, false);
			else if (suffix == ".oif")
				ch_num = LoadVolumeData(filename, VLT_OIF, false);
			else if (suffix == ".lsm")
				ch_num = LoadVolumeData(filename, VLT_LSM, false);
			else if (suffix == ".xml")
				ch_num = LoadVolumeData(filename, VLT_PVXML, false);
			else if (suffix == ".vvd")
				ch_num = LoadVolumeData(filename, VLT_BRKXML, false);
			else if (suffix == ".czi")
				ch_num = LoadVolumeData(filename, VLT_CZI, false);
			else if (suffix == ".nd2")
				ch_num = LoadVolumeData(filename, VLT_ND2, false);
			else if (suffix == ".lif")
				ch_num = LoadVolumeData(filename, VLT_LIF, false);
			else if (suffix == ".lof")
				ch_num = LoadVolumeData(filename, VLT_LOF, false);

			if (ch_num > 1)
			{
				VolumeGroup* group = view->addVolumeGroup();
				if (group)
				{
					for (int i = ch_num; i > 0; i--)
					{
						VolumeData* vd = glbin_volf->get(ch_num - i);
						if (vd)
						{
							view->addVolumeData(vd, group);
							wxString vol_name = vd->getName();
							if (vol_name.Find("_1ch") != -1 &&
								(i == 1 || i == 2))
								vd->setValue(gstDisplay, false);
							if (vol_name.Find("_2ch") != -1 && i == 1)
								vd->setValue(gstDisplay, false);

							if (i == ch_num)
							{
								vd_sel = vd;
								group_sel = group;
							}

							if (vd->GetReader() && vd->GetReader()->GetTimeNum() > 1)
								enable_4d = true;
						}
					}
					if (j > 0)
						group->setValue(gstDisplay, false);
				}
			}
			else if (ch_num == 1)
			{
				VolumeData* vd = glbin_volf->get(0);
				if (vd)
				{
					int chan_num = view->GetVolListSize();
					Color color(1.0, 1.0, 1.0);
					if (chan_num == 0)
						color = Color(1.0, 0.0, 0.0);
					else if (chan_num == 1)
						color = Color(0.0, 1.0, 0.0);
					else if (chan_num == 2)
						color = Color(0.0, 0.0, 1.0);

					bool bval;
					if (chan_num >= 0 && chan_num < 3)
						vd->setValue(gstColor, color);
					else
						vd->flipValue(gstRandomizeColor, bval);

					group_sel = view->addVolumeData(vd, group_sel);
					vd_sel = vd;

					if (vd->GetReader() && vd->GetReader()->GetTimeNum() > 1)
					{
						view->setValue(gstCurrentFrame,
							long(vd->GetReader()->GetCurTime()));
						enable_4d = true;
					}
				}
			}
			else { //TODO: other cases?

			}
		}

		//UpdateList();
		//if (vd_sel)
		//	UpdateTree(vd_sel->getName());
		//else
		//	UpdateTree();
		//v->RefreshGL(39);

		view->InitView(Renderview::INIT_BOUNDS |
			Renderview::INIT_CENTER);
		//v->m_vrv->UpdateScaleFactor(false);

		if (enable_4d)
		{
			//m_movie_view->SetTimeSeq(true);
			//m_movie_view->SetRotate(false);
			//long lval;
			//v->getValue(gstCurrentFrame, lval);
			//m_movie_view->SetCurrentTime(lval);
		}

		delete prg_diag;
	}

	//v->RefreshGL(39);//added by Takashi
}

void RenderFrameAgent::LoadMeshes(wxArrayString files)
{
	//if (!view)
	//	view = GetView(0);

	//MeshData* md_sel = 0;

	//wxProgressDialog *prg_diag = new wxProgressDialog(
	//	"FluoRender: Loading mesh data...",
	//	"Reading and processing selected mesh data. Please wait.",
	//	100, 0, wxPD_SMOOTH|wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE);

	//MeshGroup* group = 0;
	//if (files.Count() > 1)
	//	group = view->AddOrGetMGroup();

	//for (int i=0; i<(int)files.Count(); i++)
	//{
	//	prg_diag->Update(90*(i+1)/(int)files.Count());

	//	wxString filename = files[i];
	//	LoadMeshData(filename);

	//	MeshData* md = GetLastMeshData();
	//	if (view && md)
	//	{
	//		if (group)
	//		{
	//			group->insertChild(group->getNumChildren()-1, md);
	//			view->SetMeshPopDirty();
	//		}
	//		else
	//			view->AddMeshData(md);

	//		if (i==int(files.Count()-1))
	//			md_sel = md;
	//	}
	//}

	//UpdateList();
	//if (md_sel)
	//	UpdateTree(md_sel->getName());
	//else
	//	UpdateTree();

	//if (view)
	//	view->InitView(INIT_BOUNDS|INIT_CENTER);

	//delete prg_diag;
}

wxString RenderFrameAgent::SearchProjectPath(const wxString &filename)
{
	int i;

	wxString pathname = filename;

	std::wstring wsval;
	getValue(gstProjectPath, wsval);
	if (wsval.empty()) return "";
	wxString search_str;
	for (i = pathname.Length() - 1; i >= 0; i--)
	{
		if (pathname[i] == '\\' || pathname[i] == '/')
		{
			search_str.Prepend(GETSLASH());
			wxString name_temp = wsval + search_str;
			if (wxFileExists(name_temp))
				return name_temp;
		}
		else
			search_str.Prepend(pathname[i]);
	}
	return "";
}

int RenderFrameAgent::LoadVolumeData(const wxString &filename, int type, bool withImageJ, int ch_num, int t_num)
{
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

	for (i = 0; i < (int)m_reader_list.size(); i++)
	{
		std::wstring wstr = pathname.ToStdWstring();
		if (m_reader_list[i]->Match(wstr))
		{
			reader = m_reader_list[i];
			break;
		}
	}

	int reader_return = -1;
	bool bval; long lval; std::string sval; std::wstring wsval;
	if (reader)
	{
		bool preprocess = false;
		getValue(gstOpenSlices, bval);
		if (reader->GetSliceSeq() != bval)
		{
			reader->SetSliceSeq(bval);
			preprocess = true;
		}
		getValue(gstOpenChanns, bval);
		if (reader->GetChannSeq() != bval)
		{
			reader->SetChannSeq(bval);
			preprocess = true;
		}
		getValue(gstOpenDigitOrder, lval);
		if (reader->GetDigitOrder() != lval)
		{
			reader->SetDigitOrder(lval);
			preprocess = true;
		}
		getValue(gstTimeFileId, sval);
		wsval = s2ws(sval);
		if (reader->GetTimeId() != wsval)
		{
			reader->SetTimeId(wsval);
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
			if (type == VLT_TIFF)
				reader = new TIFReader();
			else if (type == VLT_NRRD)
				reader = new NRRDReader();
			else if (type == VLT_OIB)
				reader = new OIBReader();
			else if (type == VLT_OIF)
				reader = new OIFReader();
			else if (type == VLT_LSM)
				reader = new LSMReader();
			else if (type == VLT_PVXML)
			{
				reader = new PVXMLReader();
				getValue(gstPvxmlFlipX, bval);
				((PVXMLReader*)reader)->SetFlipX(bval);
				getValue(gstPvxmlFlipY, bval);
				((PVXMLReader*)reader)->SetFlipY(bval);
				getValue(gstPvxmlSeqType, lval);
				((PVXMLReader*)reader)->SetSeqType(lval);
			}
			else if (type == VLT_BRKXML)
				reader = new BRKXMLReader();
			else if (type == VLT_CZI)
				reader = new CZIReader();
			else if (type == VLT_ND2)
				reader = new ND2Reader();
			else if (type == VLT_LIF)
				reader = new LIFReader();
			else if (type == VLT_LOF)
				reader = new LOFReader();
		}

		m_reader_list.push_back(reader);
		reader->SetFile(pathname.ToStdWstring());
		getValue(gstOpenSlices, bval);
		reader->SetSliceSeq(bval);
		getValue(gstOpenChanns, bval);
		reader->SetChannSeq(bval);
		getValue(gstOpenDigitOrder, lval);
		reader->SetDigitOrder(lval);
		getValue(gstTimeFileId, sval);
		reader->SetTimeId(s2ws(sval));
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
	bool hard_comp;
	getValue(gstHardwareCompress, hard_comp);
	if (!GLEW_NV_texture_compression_vtc && hard_comp)
	{
		reader->SetResize(1);
		reader->SetAlignment(4);
	}

	getValue(gstOpenSeriesNum, lval);
	if (lval > 0)
		reader->LoadBatch(lval);
	bool skip_brick;
	getValue(gstSkipBrick, skip_brick);
	bool load_mask, load_label;
	getValue(gstLoadMask, load_mask);
	getValue(gstLoadLabel, load_label);
	int chan = reader->GetChanNum();
	for (i = (ch_num >= 0 ? ch_num : 0);
		i < (ch_num >= 0 ? ch_num + 1 : chan); i++)
	{
		VolumeData *vd = glbin_volf->build();
		if (!vd)
			continue;

		vd->setValue(gstSkipBrick, skip_brick);
		Nrrd* data = reader->Convert(t_num >= 0 ? t_num : reader->GetCurTime(), i, true);
		if (!data) continue;

		wxString name;
		if (type != VLT_BRKXML)
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
		vd->setValue(gstHardwareCompress, hard_comp);

		bool valid_spc = reader->IsSpcInfoValid();
		if (vd->LoadData(data, name.ToStdString(), pathname.ToStdWstring()))
		{
			if (load_mask)
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
			if (type == VLT_BRKXML) ((BRKXMLReader*)reader)->SetLevel(0);
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
			vd->setValue(gstTime, long(reader->GetCurTime()));
			vd->setValue(gstChannel, long(i));
			//++
			result++;
		}
		else
		{
			glbin_volf->remove(vd);
			continue;
		}

		glbin_volf->propValuesToDefault(vd);
		//AddVolumeData(vd);

		//get excitation wavelength
		double wavelength = reader->GetExcitationWavelength(i);
		if (wavelength > 0.0) {
			Color col = GetWavelengthColor(wavelength);
			vd->setValue(gstColor, col);
		}
		else if (wavelength < 0.) {
			Color white(1.0, 1.0, 1.0);
			vd->setValue(gstColor, white);
		}
		else
		{
			Color white(1.0, 1.0, 1.0);
			Color red(1.0, 0.0, 0.0);
			Color green(0.0, 1.0, 0.0);
			Color blue(0.0, 0.0, 1.0);
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

int RenderFrameAgent::LoadMeshData(const wxString &filename)
{
	wxString pathname = filename;
	if (!wxFileExists(pathname))
	{
		pathname = SearchProjectPath(filename);
		if (!wxFileExists(pathname))
			return 0;
	}

	MeshData *md = glbin_mshf->build();
	md->LoadData(pathname.ToStdWstring());

	std::string name = md->getName();
	std::string new_name = name;
	int i;
	for (i = 1; CheckNames(new_name); i++)
		new_name = name + std::to_string(i);
	if (i > 1)
		md->setName(new_name);
	//m_md_list.push_back(md);

	return 1;
}

int RenderFrameAgent::LoadMeshData(GLMmodel* mesh)
{
	if (!mesh) return 0;

	MeshData *md = glbin_mshf->build();
	md->LoadData(mesh);

	std::string name = md->getName();
	std::string new_name = name;
	int i;
	for (i = 1; CheckNames(new_name); i++)
		new_name = name + std::to_string(i);
	if (i > 1)
		md->setName(new_name);
	//m_md_list.push_back(md);

	return 1;
}

void RenderFrameAgent::SetTextureUndos()
{
	long lval;
	getValue(gstPaintHistory, lval);
	flvr::Texture::mask_undo_num_ = size_t(lval);
}

void RenderFrameAgent::SetTextureRendererSettings()
{
	bool bval;
	getValue(gstStreamEnable, bval);
	flvr::TextureRenderer::set_mem_swap(bval);
	bool use_mem_limit = false;
	GLenum error = glGetError();
	GLint mem_info[4] = { 0, 0, 0, 0 };
	glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, mem_info);
	error = glGetError();
	if (error == GL_INVALID_ENUM)
	{
		glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, mem_info);
		error = glGetError();
		if (error == GL_INVALID_ENUM)
			use_mem_limit = true;
	}
	double dval; long lval;
	getValue(gstGpuMemSize, dval);
	if (dval > mem_info[0] / 1024.0)
		use_mem_limit = true;
	flvr::TextureRenderer::set_use_mem_limit(use_mem_limit);
	flvr::TextureRenderer::set_mem_limit(use_mem_limit ? dval : mem_info[0] / 1024.0);
	flvr::TextureRenderer::set_available_mem(use_mem_limit ? dval : mem_info[0] / 1024.0);
	getValue(gstLargeDataSize, dval);
	flvr::TextureRenderer::set_large_data_size(dval);
	getValue(gstBrickSize, lval);
	flvr::TextureRenderer::set_force_brick_size(lval);
	getValue(gstResponseTime, lval);
	flvr::TextureRenderer::set_up_time(lval);
	getValue(gstUpdateOrder, lval);
	flvr::TextureRenderer::set_update_order(lval);
	flvr::TextureRenderer::set_invalidate_tex(false);
}

Color RenderFrameAgent::GetWavelengthColor(double wavelength)
{
	long lval;
	if (wavelength < 340.0)
		return Color(1.0, 1.0, 1.0);
	else if (wavelength < 440.0)
	{
		getValue(gstWaveColor1, lval);
		return GetColor(lval);
	}
	else if (wavelength < 500.0)
	{
		getValue(gstWaveColor2, lval);
		return GetColor(lval);
	}
	else if (wavelength < 600.0)
	{
		getValue(gstWaveColor3, lval);
		return GetColor(lval);
	}
	else if (wavelength < 750.0)
	{
		getValue(gstWaveColor4, lval);
		return GetColor(lval);
	}
	else
		return Color(1.0, 1.0, 1.0);
}

Color RenderFrameAgent::GetColor(int c)
{
	Color result(1.0, 1.0, 1.0);
	switch (c)
	{
	case 1://red
		result = Color(1.0, 0.0, 0.0);
		break;
	case 2://green
		result = Color(0.0, 1.0, 0.0);
		break;
	case 3://blue
		result = Color(0.0, 0.0, 1.0);
		break;
	case 4://cyan
		result = Color(0.0, 1.0, 1.0);
		break;
	case 5://magenta
		result = Color(1.0, 0.0, 1.0);
		break;
	case 6://yellow
		result = Color(1.0, 1.0, 0.0);
		break;
	case 7://orange
		result = Color(1.0, 0.5, 0.0);
		break;
	case 8://white
		result = Color(1.0, 1.0, 1.0);
		break;
	}
	return result;
}

bool RenderFrameAgent::CheckNames(const std::string &name)
{
	bool result = false;
	if (glbin_volf->findFirst(name))
		result = true;
	if (glbin_mshf->findFirst(name))
		result = true;
	if (glbin_annf->findFirst(name))
		result = true;
	return result;
}

void RenderFrameAgent::Select(const std::string &name)
{
	SearchVisitor visitor;
	visitor.setTraversalMode(NodeVisitor::TRAVERSE_CHILDREN);
	visitor.matchName(name);
	glbin_root->accept(visitor);
	ObjectList* list = visitor.getResult();
	if (list->empty())
		return;
	Node* node = dynamic_cast<Node*>(list->front());
	if (!node)
		return;

	//as node
	glbin_agtf->getOutAdjustAgent()->setObject(node);
	glbin_agtf->getClipPlaneAgent()->setObject(node);

	//as rederview
	Renderview* view = dynamic_cast<Renderview*>(node);
	VolumeGroup* vgroup = nullptr;

	std::string type(node->className());
	if (type == "VolumeData")
	{
		fluo::VolumeData* vd = node->asVolumeData();
		bool display = false;
		if (vd)
			vd->getValue(gstDisplay, display);
		if (display)
		{
			frame_.ShowVolumePropPanel(vd->getName());
		}
		else
		{
			frame_.HidePropPanel();
		}
		if (vd)
		{
			glbin_root->setRefValue(gstCurrentVolume, vd);
			glbin_agtf->getConvertAgent()->setObject(vd);
			glbin_agtf->getCountingAgent()->setObject(vd);
			glbin_agtf->getVolumePropAgent()->setObject(vd);

			visitor.reset();
			visitor.setTraversalMode(NodeVisitor::TRAVERSE_PARENTS);
			visitor.matchClassName("Renderview");
			vd->accept(visitor);
			view = visitor.getRenderview();

			visitor.reset();
			visitor.setTraversalMode(NodeVisitor::TRAVERSE_PARENTS);
			visitor.matchClassName("VolumeGroup");
			vd->accept(visitor);
			vgroup = visitor.getVolumeGroup();
		}
	}
	else if (type == "MeshData")
	{
		fluo::MeshData* md = node->asMeshData();
		bool display = false;
		if (md)
			md->getValue(gstDisplay, display);
		if (display)
		{
			frame_.ShowMeshPropPanel(md->getName());
		}
		else
		{
			frame_.HidePropPanel();
		}
		if (md)
		{
			md->setValue(gstDrawBounds, true);
			glbin_agtf->getMeshPropAgent()->setObject(md);
			glbin_agtf->getMeshTransAgent()->setObject(md);

			visitor.reset();
			visitor.setTraversalMode(NodeVisitor::TRAVERSE_PARENTS);
			visitor.matchClassName("Renderview");
			md->accept(visitor);
			view = visitor.getRenderview();
		}
	}
	else if (type == "Annotations")
	{
		fluo::Annotations* ann = node->asAnnotations();
		bool display = false;
		if (ann)
			ann->getValue(gstDisplay, display);
		if (display)
		{
			frame_.ShowAnnoPropPanel(ann->getName());
		}
		else
		{
			frame_.HidePropPanel();
		}
		if (ann)
		{
			glbin_agtf->getAnnotationPropAgent()->setObject(ann);

			visitor.reset();
			visitor.setTraversalMode(NodeVisitor::TRAVERSE_PARENTS);
			visitor.matchClassName("Renderview");
			ann->accept(visitor);
			view = visitor.getRenderview();
		}
	}
	else
	{
		frame_.HidePropPanel();
	}

	//others
	if (view)
	{
		glbin_agtf->getBrushToolAgent()->setObject(view);
		glbin_agtf->getCalculationAgent()->setObject(view);
		glbin_agtf->getClKernelAgent()->setObject(view);
		glbin_agtf->getComponentAgent()->setObject(view);
		glbin_agtf->getMeasureAgent()->setObject(view);
		glbin_agtf->getMovieAgent()->setObject(view);
		glbin_agtf->getNoiseReduceAgent()->setObject(view);
		glbin_agtf->getRecorderAgent()->setObject(view);
		glbin_agtf->getTrackAgent()->setObject(view);
	}
	if (vgroup)
	{
		glbin_agtf->getColocalAgent()->setObject(vgroup);
	}
}

void RenderFrameAgent::AddView(RenderviewPanel* vrv, bool set_gl)
{
	if (!vrv)
		return;

	fluo::Renderview* view = glbin_revf->build();
	glbin_root->addChild(view);
	glbin_root->setRefValue(gstCurrentView, view);

	std::string str_name = gstRenderviewAgent + std::to_string(RenderviewPanel::m_max_id);
	vrv->m_agent = glbin_agtf->addRenderviewAgent(str_name, *vrv);
	str_name = vrv->GetName().ToStdString();
	view->setName(str_name);

	RenderCanvas* canvas = vrv->GetCanvas();
	if (!canvas)
		return;
	canvas->m_agent = glbin_agtf->
		addRenderCanvasAgent(str_name, *canvas);
	canvas->m_agent->setObject(view);
	canvas->m_agent->setValue(gstSetGl, set_gl);
}

wxString RenderFrameAgent::ScriptDialog(const wxString& title,
	const wxString& wildcard, long style)
{
	fluo::MovieAgent* agent = glbin_agtf->getMovieAgent();
	if (agent) agent->HoldRun();
	wxString result;
	wxFileDialog *dlg = new wxFileDialog(
		&frame_, title, "", "",
		wildcard, style);
	int rval = dlg->ShowModal();
	if (rval == wxID_OK)
		result = dlg->GetPath();
	delete dlg;
	if (agent) agent->ResumeRun();
	return result;
}

void RenderFrameAgent::UpdateSettings()
{
	fluo::SettingAgent* agent = glbin_agtf->getSettingAgent();
	if (agent) agent->UpdateFui();
}
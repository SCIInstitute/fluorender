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
#include <RenderFrame.h>
#include <Global.hpp>
#include <AgentFactory.hpp>
#include <MovieAgent.hpp>
#include <VolumeData.hpp>
#include <base_reader.h>
#include <Texture.h>
#include <wx/string.h>
#include <wx/progdlg.h>

using namespace fluo;

RenderFrameAgent::RenderFrameAgent(RenderFrame &frame) :
	InterfaceAgent(),
	frame_(frame)
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

void RenderFrameAgent::UpdateAllSettings()
{
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

	fluo::VolumeData* vd_sel = 0;
	fluo::VolumeGroup* group_sel = 0;
	fluo::Renderview* view = glbin_root->getCurrentRenderview();

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
				fluo::VolumeGroup* group = v->addVolumeGroup();
				if (group)
				{
					for (int i = ch_num; i > 0; i--)
					{
						fluo::VolumeData* vd = GetVolumeData(ch_num - i);
						if (vd)
						{
							v->addVolumeData(vd, group);
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
				fluo::VolumeData* vd = GetVolumeData(0);
				if (vd)
				{
					int chan_num = v->GetVolListSize();
					fluo::Color color(1.0, 1.0, 1.0);
					if (chan_num == 0)
						color = fluo::Color(1.0, 0.0, 0.0);
					else if (chan_num == 1)
						color = fluo::Color(0.0, 1.0, 0.0);
					else if (chan_num == 2)
						color = fluo::Color(0.0, 0.0, 1.0);

					bool bval;
					if (chan_num >= 0 && chan_num < 3)
						vd->setValue(gstColor, color);
					else
						vd->flipValue(gstRandomizeColor, bval);

					group_sel = v->addVolumeData(vd, group_sel);
					vd_sel = vd;

					if (vd->GetReader() && vd->GetReader()->GetTimeNum() > 1)
					{
						v->setValue(gstCurrentFrame,
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

		v->InitView(fluo::Renderview::INIT_BOUNDS |
			fluo::Renderview::INIT_CENTER);
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

	//fluo::MeshData* md_sel = 0;

	//wxProgressDialog *prg_diag = new wxProgressDialog(
	//	"FluoRender: Loading mesh data...",
	//	"Reading and processing selected mesh data. Please wait.",
	//	100, 0, wxPD_SMOOTH|wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE);

	//fluo::MeshGroup* group = 0;
	//if (files.Count() > 1)
	//	group = view->AddOrGetMGroup();

	//for (int i=0; i<(int)files.Count(); i++)
	//{
	//	prg_diag->Update(90*(i+1)/(int)files.Count());

	//	wxString filename = files[i];
	//	LoadMeshData(filename);

	//	fluo::MeshData* md = GetLastMeshData();
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

int RenderFrameAgent::LoadVolumeData(const wxString &filename, int type, bool withImageJ, int ch_num, int t_num)
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

	for (i = 0; i < (int)m_reader_list.size(); i++)
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
	for (i = (ch_num >= 0 ? ch_num : 0);
		i < (ch_num >= 0 ? ch_num + 1 : chan); i++)
	{
		fluo::VolumeData *vd = glbin_volf->build();
		if (!vd)
			continue;

		vd->setValue(gstSkipBrick, m_skip_brick);
		Nrrd* data = reader->Convert(t_num >= 0 ? t_num : reader->GetCurTime(), i, true);
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
		vd->setValue(gstHardwareCompress, m_compression);

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

int RenderFrameAgent::LoadMeshData(const wxString &filename)
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
	for (i = 1; CheckNames(new_name); i++)
		new_name = name + wxString::Format("_%d", i);
	if (i > 1)
		md->setName(new_name.ToStdString());
	//m_md_list.push_back(md);

	return 1;
}

int RenderFrameAgent::LoadMeshData(GLMmodel* mesh)
{
	if (!mesh) return 0;

	fluo::MeshData *md = glbin_mshf->build();
	md->LoadData(mesh);

	wxString name = md->getName();
	wxString new_name = name;
	int i;
	for (i = 1; CheckNames(new_name); i++)
		new_name = name + wxString::Format("_%d", i);
	if (i > 1)
		md->setName(new_name.ToStdString());
	//m_md_list.push_back(md);

	return 1;
}

void RenderFrameAgent::SetTextureUndos()
{
	if (m_setting_dlg)
		flvr::Texture::mask_undo_num_ = (size_t)(m_setting_dlg->GetPaintHistDepth());
}

void RenderFrameAgent::SetTextureRendererSettings()
{
	if (!m_setting_dlg)
		return;

	flvr::TextureRenderer::set_mem_swap(m_setting_dlg->GetMemSwap());
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
	if (m_setting_dlg->GetGraphicsMem() > mem_info[0] / 1024.0)
		use_mem_limit = true;
	flvr::TextureRenderer::set_use_mem_limit(use_mem_limit);
	flvr::TextureRenderer::set_mem_limit(use_mem_limit ?
		m_setting_dlg->GetGraphicsMem() : mem_info[0] / 1024.0);
	flvr::TextureRenderer::set_available_mem(use_mem_limit ?
		m_setting_dlg->GetGraphicsMem() : mem_info[0] / 1024.0);
	flvr::TextureRenderer::set_large_data_size(m_setting_dlg->GetLargeDataSize());
	flvr::TextureRenderer::set_force_brick_size(m_setting_dlg->GetForceBrickSize());
	flvr::TextureRenderer::set_up_time(m_setting_dlg->GetResponseTime());
	flvr::TextureRenderer::set_update_order(m_setting_dlg->GetUpdateOrder());
	flvr::TextureRenderer::set_invalidate_tex(m_setting_dlg->GetInvalidateTex());
}


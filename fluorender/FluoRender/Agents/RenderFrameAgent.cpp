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
#include <wx/string.h>

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
	fluo::Renderview* v = 0;

	if (view)
		v = view;
	else
		v = glbin_root->getCurrentRenderview();

	wxProgressDialog *prg_diag = 0;
	if (v)
	{
		bool streaming = m_setting_dlg->GetMemSwap();
		double gpu_size = m_setting_dlg->GetGraphicsMem();
		double data_size = m_setting_dlg->GetLargeDataSize();
		int brick_size = m_setting_dlg->GetForceBrickSize();
		int resp_time = m_setting_dlg->GetResponseTime();
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

		m_data_mgr.SetSliceSequence(m_sliceSequence);
		m_data_mgr.SetChannSequence(m_channSequence);
		m_data_mgr.SetDigitOrder(m_digitOrder);
		m_data_mgr.SetSerNum(m_ser_num);
		m_data_mgr.SetCompression(m_compression);
		m_data_mgr.SetSkipBrick(m_skip_brick);
		m_data_mgr.SetTimeId(m_time_id);
		m_data_mgr.SetLoadMask(m_load_mask);
		m_setting_dlg->SetTimeId(m_time_id);

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
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_IMAGEJ, true); //The type of data doesnt matter.
			else if (suffix == ".nrrd" || suffix == ".msk" || suffix == ".lbl")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_NRRD, false);
			else if (suffix == ".tif" || suffix == ".tiff")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_TIFF, false);
			else if (suffix == ".oib")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_OIB, false);
			else if (suffix == ".oif")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_OIF, false);
			else if (suffix == ".lsm")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_LSM, false);
			else if (suffix == ".xml")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_PVXML, false);
			else if (suffix == ".vvd")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_BRKXML, false);
			else if (suffix == ".czi")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_CZI, false);
			else if (suffix == ".nd2")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_ND2, false);
			else if (suffix == ".lif")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_LIF, false);
			else if (suffix == ".lof")
				ch_num = m_data_mgr.LoadVolumeData(filename, LOAD_TYPE_LOF, false);

			if (ch_num > 1)
			{
				fluo::VolumeGroup* group = v->addVolumeGroup();
				if (group)
				{
					for (int i = ch_num; i > 0; i--)
					{
						fluo::VolumeData* vd = m_data_mgr.GetVolumeData(ch_num - i);
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
				fluo::VolumeData* vd = m_data_mgr.GetVolumeData(0);
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
	//	m_data_mgr.LoadMeshData(filename);

	//	fluo::MeshData* md = m_data_mgr.GetLastMeshData();
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


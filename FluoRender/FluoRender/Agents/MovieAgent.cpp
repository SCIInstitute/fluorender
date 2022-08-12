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

#include "MovieAgent.hpp"
//#include "MoviePanel.h"
#include "Global.hpp"
#include "AgentFactory.hpp"
#include "RecorderAgent.hpp"
#include "Root.hpp"
#include "AsyncTimer.hpp"
#include "AsyncTimerFactory.hpp"
#include "Interpolator.h"
#include "TextureRenderer.h"
#include "tiffio.h"
#include "icons.h"
#include "compatibility_utilities.h"

#include <algorithm>

using namespace fluo;

MovieAgent::MovieAgent(MoviePanel &panel) :
	InterfaceAgent(),
	panel_(panel)
{
	AsyncTimer* timer = glbin_atmf->build();
	if (timer)
	{
		std::string name = timer->getName();
		addSetValue(gstMovTimerName, name);
		timer->setFunc(std::bind(&MovieAgent::OnTimer, this));
	}
}

void MovieAgent::setupInputs()
{

}

void MovieAgent::setObject(Renderview* obj)
{
	InterfaceAgent::setObject(obj);
	std::string name = obj->getName();
	//int i = panel_.m_views_cmb->FindString(name);
	//panel_.m_views_cmb->SetSelection(i);
}

Renderview* MovieAgent::getObject()
{
	return dynamic_cast<Renderview*>(InterfaceAgent::getObject());
}

void MovieAgent::UpdateFui(const ValueCollection &names)
{
	bool update_all = names.empty();

	//update views
/*	if (update_all || FOUND_VALUE(gstNonObjectValues))
	{
		panel_.m_views_cmb->Clear();
		int sel = 0;
		for (size_t i = 0; i < glbin_root->getNumChildren(); ++i)
		{
			fluo::Renderview* view = glbin_root->getChild(i)->asRenderview();
			if (!view) continue;
			panel_.m_views_cmb->Append(view->getName());
			if (view == getObject())
				sel = i;
		}
		if (panel_.m_views_cmb->GetCount() > 0)
			panel_.m_views_cmb->Select(0);

		AddScriptToList();
		GetScriptSettings();
		glbin_agtf->getRecorderAgent()->UpdateFui();
	}

	//bool bval;
	//getValue(gstMovTimeSeqEnable, bval);
	//getValue(gstDrawCropFrame, bval);
*/}

void MovieAgent::SetProgress(double pcnt)
{
/*	pcnt = std::abs(pcnt);
	long lval;
	getValue(gstMovSliderRange, lval);
	panel_.m_progress_sldr->SetValue(pcnt * lval);
	double dval;
	getValue(gstMovLength, dval);
	dval *= pcnt;
	setValue(gstMovCurTime, dval);
	wxString st = wxString::Format("%.2f", dval);
	panel_.m_progress_text->ChangeValue(st);
*/}

void MovieAgent::Select(const std::string& name)
{
	Node* node = glbin_root->findFirstChild(name);
	if (!node)
		return;
	Renderview* view = node->asRenderview();
	if (!view)
		return;
	setObject(view);
}

int MovieAgent::GetScriptFiles(std::vector<std::string>& list)
{
	std::wstring exePath = glbin.getExecutablePath();
	std::string loc = ws2s(exePath) + GETSLASHA() + "Scripts" +
		GETSLASHA() + "*.txt";
	//wxLogNull logNo;
	//std::string file = wxFindFirstFile(loc);
	//while (!file.empty())
	//{
	//	list.push_back(file);
	//	file = wxFindNextFile();
	//}
	//std::sort(list);
	return list.size();
}

void MovieAgent::AddScriptToList()
{
/*	panel_.m_script_list->DeleteAllItems();
	wxArrayString list;
	wxString filename;
	if (GetScriptFiles(list))
	{
		for (size_t i = 0; i < list.GetCount(); ++i)
		{
			filename = wxFileNameFromPath(list[i]);
			filename = filename.BeforeLast('.');
			panel_.m_script_list->InsertItem(
				panel_.m_script_list->GetItemCount(), filename);
		}
	}
*/}

void MovieAgent::GetScriptSettings()
{
/*	bool run_script;
	getValue(gstRunScript, run_script);
	panel_.m_run_script_chk->SetValue(run_script);
	std::wstring script_file;
	getValue(gstScriptFile, script_file);
	panel_.m_script_file_text->SetValue(script_file);

	//highlight if builtin
	wxArrayString list;
	if (GetScriptFiles(list))
	{
		int idx = -1;
		for (size_t i = 0; i < list.GetCount(); ++i)
		{
			if (script_file == list[i])
			{
				idx = i;
				break;
			}
		}
		if (idx >= 0)
		{
			panel_.m_script_list->SetItemState(idx,
				wxLIST_STATE_SELECTED,
				wxLIST_STATE_SELECTED);
			//wxSize ss = m_script_list->GetItemSpacing();
			//m_script_list->ScrollList(0, ss.y*idx);
		}
	}
	//change icon
	bool bval;
	getValue(gstMovRunning, bval);
	if (run_script)
	{
		if (bval)
			panel_.m_play_btn->SetBitmap(wxGetBitmapFromMemory(pause));
		else
			panel_.m_play_btn->SetBitmap(wxGetBitmapFromMemory(playscript));
		panel_.m_notebook->SetPageText(4, "Script (Enabled)");
	}
	else
	{
		if (bval)
			panel_.m_play_btn->SetBitmap(wxGetBitmapFromMemory(pause));
		else
			panel_.m_play_btn->SetBitmap(wxGetBitmapFromMemory(play));
		panel_.m_notebook->SetPageText(4, "Script");
	}
*/}

void MovieAgent::WriteFrameToFile(int total_frames)
{
/*	wxString s_length = wxString::Format("%d", total_frames);
	int length = s_length.Length();
	wxString format = wxString::Format("_%%0%dd", length);
	std::wstring filename;
	getValue(gstMovFilename, filename);
	long lastframe;
	getValue(gstLastFrame, lastframe);
	wxString outputfilename = wxString::Format("%s" + format + "%s", filename,
		lastframe, ".tif");
	std::string filetype;
	getValue(gstMovFileType, filetype);

	//capture
	bool bmov = filetype == ".mov";
	bool bval;
	getValue(gstCaptureAlpha, bval);
	int chann = bval ? 4 : 3;
	getValue(gstCaptureFloat, bval);
	bool fp32 = bmov ? false : bval;
	long x, y, w, h;
	void* image = 0;
	getObject()->ReadPixels(chann, fp32, x, y, w, h, &image);

	std::string str_fn = outputfilename.ToStdString();
	if (bmov)
	{
		//flip vertically 
		unsigned char *flip = new unsigned char[w*h * 3];
		for (size_t yy = 0; yy < (size_t)h; yy++)
			for (size_t xx = 0; xx < (size_t)w; xx++)
				memcpy(flip + 3 * (w * yy + xx), (unsigned char*)image + chann * (w * (h - yy - 1) + xx), 3);
		bool worked = encoder_.set_frame_rgb_data(flip);
		worked = encoder_.write_video_frame(lastframe);
		if (flip)
			delete[]flip;
		if (image)
			delete[]image;
	}
	else
	{
		TIFF *out = TIFFOpen(str_fn.c_str(), "wb");
		if (!out) return;
		TIFFSetField(out, TIFFTAG_IMAGEWIDTH, w);
		TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);
		TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, chann);
		if (fp32)
		{
			TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 32);
			TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
		}
		else
		{
			TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
			//TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
		}
		TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
		TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		getValue(gstCaptureCompress, bval);
		if (bval)
			TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);

		tsize_t linebytes = chann * w * (fp32 ? 4 : 1);
		void *buf = NULL;
		buf = _TIFFmalloc(linebytes);
		TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, 0));
		for (uint32 row = 0; row < (uint32)h; row++)
		{
			if (fp32)
			{
				float* line = ((float*)image) + (h - row - 1) * chann * w;
				memcpy(buf, line, linebytes);
			}
			else
			{// check the index here, and figure out why not using h*linebytes
				unsigned char* line = ((unsigned char*)image) + (h - row - 1) * chann * w;
				memcpy(buf, line, linebytes);
			}
			if (TIFFWriteScanline(out, buf, row, 0) < 0)
				break;
		}
		TIFFClose(out);
		if (buf)
			_TIFFfree(buf);
		if (image)
			delete[]image;
	}
*/}

void MovieAgent::Stop()
{
	bool run_script;
	getValue(gstRunScript, run_script);
	//if (run_script)
	//	panel_.m_play_btn->SetBitmap(wxGetBitmapFromMemory(playscript));
	//else
	//	panel_.m_play_btn->SetBitmap(wxGetBitmapFromMemory(play));

	std::string name;
	getValue(gstMovTimerName, name);
	AsyncTimer* timer = glbin_atmf->findFirst(name);
	if (timer) timer->stop();
	setValue(gstMovRunning, false);
	setValue(gstMovRecord, false);
	encoder_.close();
	flvr::TextureRenderer::maximize_uptime_ = false;
}

void MovieAgent::SetRendering(double pcnt, bool rewind)
{
	Renderview* view = getObject();
	if (!view) return;

	setValue(gstCamLockObjEnable, false);
	//advanced options
	long page;
	getValue(gstMovCurrentPage, page);
	if (page == 1)
	{
		Interpolator *interpolator = view->GetInterpolator();
		if (interpolator && interpolator->GetLastIndex() > 0)
		{
			//bool cam_lock, mov_running;
			//getValue(gstCamLockObjEnable, cam_lock);
			//getValue(gstMovRunning, mov_running);
			//if (m_advanced_movie->GetCamLock() && m_timer.IsRunning())
			//	m_view->setValue(gstCamLockObjEnable, true);
			int end_frame = int(interpolator->GetLastT());
			view->SetParams(pcnt * end_frame);
			setValue(gstInteractive, false);
			//m_view->RefreshGL(39);
			return;
		}
	}

	//basic options
	long sf, ef;
	getValue(gstBeginFrame, sf);
	getValue(gstEndFrame, ef);
	int time = ef - sf + 1;
	time = int(sf + time * pcnt + 0.5);
	long lval;
	getValue(gstMovSeqMode, lval);
	if (lval == 1)
		view->Set4DSeqFrame(time, sf, ef, rewind);
	else if (lval == 2)
		view->Set3DBatFrame(time, sf, ef, rewind);

	//rotate animation
	std::string sval;
	double dval;
	bool bval;
	getValue(gstMovRotEnable, bval);
	if (bval)
	{
		getValue(gstMovRotAxis, lval);
		switch (lval)
		{
		case 0:
			sval = gstCamRotX;
			break;
		case 1:
			sval = gstCamRotY;
			break;
		case 2:
			sval = gstCamRotZ;
			break;
		}
		getValue(gstMovIntrpMode, lval);
		double start_ang, rot_ang;
		getValue(gstMovStartAng, start_ang);
		getValue(gstMovRotAng, rot_ang);
		if (lval == 0)
			dval = start_ang + pcnt * rot_ang;
		else if (lval == 1)
			dval = start_ang +
			(-2.0*pcnt*pcnt*pcnt + 3.0*pcnt*pcnt) * rot_ang;
		setValue(sval, dval);
	}

	setValue(gstInteractive, false);
	//m_view->RefreshGL(39);
}

void MovieAgent::Prev()
{
	bool bval;
	getValue(gstMovRunning, bval);
	if (bval)
	{
		Stop();
		return;
	}
	else
		setValue(gstMovRunning, true);

	flvr::TextureRenderer::maximize_uptime_ = true;
	//panel_.m_play_btn->SetBitmap(wxGetBitmapFromMemory(pause));
	//int slider_pos = panel_.m_progress_sldr->GetValue();
#pragma message ("get progress")
	int slider_pos = 0;
	long lval;
	getValue(gstMovSliderRange, lval);
	double len, cur_time, fps;
	getValue(gstMovLength, len);
	getValue(gstMovCurTime, cur_time);
	getValue(gstMovFps, fps);
	if (slider_pos < lval && slider_pos > 0 &&
		!(len - cur_time < 0.1 / fps ||
			cur_time > len))
	{
		TimerRun();
		return;
	}

	//basic options
	double rval[3];
	getValue(gstCamRotX, rval[0]);
	getValue(gstCamRotY, rval[1]);
	getValue(gstCamRotZ, rval[2]);
	getValue(gstMovRotAxis, lval);
	double start_ang = rval[lval];
	while (start_ang > 360.) start_ang -= 360.;
	while (start_ang < -360.) start_ang += 360.;
	if (360. - std::abs(start_ang) < 0.001)
		start_ang = 0.;
	setValue(gstMovStartAng, start_ang);
	getValue(gstMovCurrentPage, lval);
	if (lval == 1)
	{
		Interpolator *interpolator = getObject()->GetInterpolator();
		if (interpolator && interpolator->GetLastIndex() > 0)
		{
			int frames = int(interpolator->GetLastT());
			len = (double)frames / fps;
			setValue(gstMovLength, len);
			//panel_.m_movie_len_text->ChangeValue(wxString::Format("%.2f", len));
		}
	}
	SetProgress(0.);
	SetRendering(0., false);
	setValue(gstLastFrame, long(0));
	TimerRun();
}

void MovieAgent::Run()
{
	//if (m_frame->GetSettingDlg())
	//{
	//	RenderFrame::SetSaveProject(m_frame->GetSettingDlg()->GetProjSave());
	//	RenderFrame::SetSaveAlpha(m_frame->GetSettingDlg()->GetSaveAlpha());
	//	RenderFrame::SetSaveFloat(m_frame->GetSettingDlg()->GetSaveFloat());
	//}

	Rewind();
	std::wstring wfilename;
	std::string filetype;
	getValue(gstMovFilename, wfilename);
	filetype = GET_SUFFIX(ws2s(wfilename));
	if (filetype == ".mov")
	{
		//if (!m_crop)
		//{
		//	m_crop_x = 0;
		//	m_crop_y = 0;
		//	m_view->getValue(gstSizeX, m_crop_w);
		//	m_view->getValue(gstSizeY, m_crop_h);
		//}
		//bool bval = false;
		//if (m_view)
		//	m_view->getValue(gstEnlarge, bval);
		//if (bval)
		//{
		//	double scale;
		//	m_view->getValue(gstEnlargeScale, scale);
		//	m_crop_w *= scale;
		//	m_crop_h *= scale;
		//}
		long w, h;
		double fps, bitrate;
		getValue(gstCropW, w);
		getValue(gstCropH, h);
		getValue(gstMovFps, fps);
		getValue(gstMovBitrate, bitrate);
		encoder_.open(ws2s(wfilename), w, h, fps, bitrate * 1e6);
	}
	setValue(gstMovRecord, true);
	//m_filename = m_filename.SubString(0, m_filename.Len() - 5);
	//m_record = true;
	//if (m_frame->GetSettingDlg())
	//{
	//	m_frame->GetSettingDlg()->SetSaveAlpha(RenderFrame::GetSaveAlpha());
	//	m_frame->GetSettingDlg()->SetSaveFloat(RenderFrame::GetSaveFloat());
	//	if (m_frame->GetSettingDlg()->GetProjSave())
	//	{
	//		wxString new_folder;
	//		new_folder = m_filename + "_project";
	//		MkDirW(new_folder.ToStdWstring());
	//		wstring name = m_filename.ToStdWstring();
	//		name = GET_NAME(name);
	//		wxString prop_file = new_folder + GETSLASH()
	//			+ name + "_project.vrp";
	//		m_frame->SaveProject(prop_file);
	//	}
	//}

	Prev();
}

void MovieAgent::Rewind()
{
	getObject()->SetParams(0.);
	Stop();
	long lval;
	getValue(gstBeginFrame, lval);
	setValue(gstCurrentFrame, lval);
	SetProgress(0.);
	SetRendering(0., false);
}

void MovieAgent::UpFrame()
{
	//if (m_running) return;
	long cf, sf, ef;
	getValue(gstCurrentFrame, cf);
	getValue(gstBeginFrame, sf);
	getValue(gstEndFrame, ef);
	if (cf < sf)
		cf = sf;
	cf++;
	bool rewind = false;
	if (cf > ef)
	{
		cf = sf;
		rewind = true;
	}
	setValue(gstMovRewind, rewind);
	setValue(gstCurrentFrame, cf);
}

void MovieAgent::DownFrame()
{
	//if (m_running) return;
	long cf, sf, ef;
	getValue(gstCurrentFrame, cf);
	getValue(gstBeginFrame, sf);
	getValue(gstEndFrame, ef);
	if (cf > ef)
		cf = ef;
	cf--;
	if (cf < sf) cf = ef;
	setValue(gstCurrentFrame, cf);
}

void MovieAgent::TimerRun()
{
	std::string name;
	getValue(gstMovTimerName, name);
	AsyncTimer* timer = glbin_atmf->findFirst(name);
	if (timer)
	{
		double dval;
		getValue(gstMovFps, dval);
		long lval = long(1000.0 / dval + 0.5);
		timer->setValue(gstTimerInterval, lval);
		timer->restart();
	}
}

void MovieAgent::ResumeRun()
{
	bool bval;
	getValue(gstMovTimerState, bval);
	if (bval)
		TimerRun();
}

void MovieAgent::HoldRun()
{
	std::string name;
	getValue(gstMovTimerName, name);
	AsyncTimer* timer = glbin_atmf->findFirst(name);
	if (timer)
	{
		bool bval;
		timer->getValue(gstTimerRunning, bval);
		if (bval)
		{
			setValue(gstMovTimerState, true);
			timer->stop();
		}
		else
			setValue(gstMovTimerState, false);
	}
}

void MovieAgent::AutoKey()
{
	long lval;
	getValue(gstAutoKeyIndex, lval);
	//get recorder agent and gen auto key
	//if (item != -1)
	//{
	//	if (item == 0)
	//		m_advanced_movie->AutoKeyChanComb(1);
	//	else if (item == 1)
	//		m_advanced_movie->AutoKeyChanComb(2);
	//	else if (item == 2)
	//		m_advanced_movie->AutoKeyChanComb(3);

	//	m_notebook->SetSelection(1);
	//}
}

void MovieAgent::OnTimer()
{
	//get all of the progress info
	bool record;
	getValue(gstMovRecord, record);
	double fps;
	getValue(gstMovFps, fps);
	double len;
	getValue(gstMovLength, len);
	bool bval;
	getValue(gstMovDelayedStop, bval);
	if (bval)
	{
		if (record)
			WriteFrameToFile(int(fps*len + 0.5));
		changeValue(gstMovDelayedStop, false);
		Stop();
		changeValue(gstKeepEnlarge, false);
		return;
	}

	if (flvr::TextureRenderer::get_mem_swap() &&
		flvr::TextureRenderer::get_start_update_loop() &&
		!flvr::TextureRenderer::get_done_update_loop())
	{
		setValue(gstInteractive, false);
		//m_view->RefreshGL(39, false);
		return;
	}

	//move forward in time (limits FPS usability to 100 FPS)
	double cur_time;
	getValue(gstMovCurTime, cur_time);
	cur_time += 1.0 / fps;
	//frame only increments when time passes a whole number
	long sf, ef, cf, lf;
	getValue(gstBeginFrame, sf);
	getValue(gstEndFrame, ef);
	long time = ef - sf + 1;
	cf = (long)(sf + time * cur_time / len + 0.5);
	double pcnt = (double)(cf - sf) / (double)time;
	setValue(gstCurrentFrame, cf);
	SetProgress(pcnt);
	//update the rendering frame since we have advanced.
	getValue(gstLastFrame, lf);
	if (lf != cf)
	{
		//panel_.m_cur_frame_text->ChangeValue(wxString::Format("%d", cf));
		if (record)
			WriteFrameToFile(int(fps*len + 0.5));
		setValue(gstLastFrame, cf);
		SetRendering(cur_time / len, false);
	}
	if (len - cur_time < 0.1 / fps ||
		cur_time > len)
		setValue(gstMovDelayedStop, true);
}

void MovieAgent::OnMovTimeSeqEnable(Event& event)
{
	bool bval;
	long lval;
	double dval;
	getValue(gstMovTimeSeqEnable, bval);
	if (bval)
	{
		getValue(gstMovSeqMode, lval);
		if (lval == 0)
			setValue(gstMovSeqMode, long(1));
	}
	else
	{
		//panel_.m_seq_chk->SetValue(false);
		//panel_.m_bat_chk->SetValue(false);
		setValue(gstBeginFrame, long(0));
		getValue(gstMovRotAng, dval);
		setValue(gstEndFrame, long(dval));
	}
	long bf, ef;
	double fps;
	getValue(gstBeginFrame, bf);
	getValue(gstEndFrame, ef);
	getValue(gstMovFps, fps);
	dval = (ef - bf + 1) / fps;
	setValue(gstMovLength, dval);
	//panel_.m_start_frame_text->ChangeValue(wxString::Format("%d", bf));
	//panel_.m_end_frame_text->ChangeValue(wxString::Format("%d", ef));
	//panel_.m_movie_len_text->ChangeValue(wxString::Format("%.2f", dval));
}

void MovieAgent::OnMovSeqMode(Event& event)
{
	long lval;
	getValue(gstMovSeqMode, lval);
	if (lval == 1)
	{
		//panel_.m_seq_chk->SetValue(true);
		//panel_.m_bat_chk->SetValue(false);
		getObject()->Get4DSeqRange();
	}
	else if (lval == 2)
	{
		//panel_.m_seq_chk->SetValue(false);
		//panel_.m_bat_chk->SetValue(true);
		getObject()->Get3DBatRange();
	}
}

void MovieAgent::OnMovRotEnable(Event& event)
{
	bool bval;
	getValue(gstMovRotEnable, bval);
	//panel_.m_x_rd->Enable(bval);
	//panel_.m_y_rd->Enable(bval);
	//panel_.m_z_rd->Enable(bval);
	//panel_.m_degree_text->Enable(bval);
	//panel_.m_rot_int_cmb->Enable(bval);
	//panel_.m_rot_chk->SetValue(bval);
	if (!bval)
	{
		long lval;
		getValue(gstMovSeqMode, lval);
		if (lval == 0)
			setValue(gstMovTimeSeqEnable, true);
	}
}

void MovieAgent::OnMovRotAxis(Event& event)
{
	long lval;
	getValue(gstMovRotAxis, lval);
	//if (lval == 0)
	//	panel_.m_x_rd->SetValue(true);
	//else if (lval == 1)
	//	panel_.m_y_rd->SetValue(true);
	//else if (lval == 2)
	//	panel_.m_z_rd->SetValue(true);
}

void MovieAgent::OnMovRotAng(Event& event)
{
	double dval;
	getValue(gstMovRotAng, dval);
	//panel_.m_degree_text->SetValue(wxString::Format("%d", dval));
}

void MovieAgent::OnCurrentFrame(Event& event)
{
	long cf;
	getValue(gstCurrentFrame, cf);
	long sf, ef;
	getValue(gstBeginFrame, sf);
	getValue(gstEndFrame, ef);
	if (cf < sf) cf = ef;
	if (cf > ef) cf = sf;
	long time = ef - sf + 1;
	double pcnt = (double)(cf - sf) / (double)time;

	//panel_.m_cur_frame_text->ChangeValue(wxString::Format("%d", cf));
	changeValue(gstCurrentFrame, cf);
	SetProgress(pcnt);
	SetRendering(pcnt, false);
}

void MovieAgent::OnDrawCropFrame(Event& event)
{
	bool bval;
	getValue(gstDrawCropFrame, bval);
	//panel_.m_crop_chk->SetValue(bval);

	if (bval)
	{
		getObject()->CalculateCrop();
		long x, y, w, h;
		getValue(gstCropX, x); getValue(gstCropY, y);
		getValue(gstCropW, w); getValue(gstCropH, h);
		x = long(x + w / 2.0 + 0.5);
		y = long(y + h / 2.0 + 0.5);
		//panel_.m_center_x_text->ChangeValue(wxString::Format("%d", x));
		//panel_.m_center_y_text->ChangeValue(wxString::Format("%d", y));
		//panel_.m_width_text->ChangeValue(wxString::Format("%d", w));
		//panel_.m_height_text->ChangeValue(wxString::Format("%d", h));
	}

	//m_view->RefreshGL(39);
}

void MovieAgent::OnMovCurrentPage(Event& event)
{
	long lval;
	getValue(gstMovCurrentPage, lval);
	//if (panel_.m_notebook)
	//	panel_.m_notebook->SetSelection(lval);
}

void MovieAgent::OnMovLength(Event& event)
{
	double dval;
	getValue(gstMovLength, dval);
	//panel_.m_movie_len_text->SetValue(wxString::Format("%.2f", dval));
	//update fps
	long sf, ef;
	getValue(gstBeginFrame, sf);
	getValue(gstEndFrame, ef);
	double fps = double(ef - sf + 1) / dval;
	setValue(gstMovFps, fps);
}

void MovieAgent::OnMovFps(Event& event)
{
	double dval;
	getValue(gstMovFps, dval);
	//panel_.m_fps_text->SetValue(wxString::Format("%.0f", dval));

	//update length
	long sf, ef;
	getValue(gstBeginFrame, sf);
	getValue(gstEndFrame, ef);
	double len = double(ef - sf + 1) / dval;
	setValue(gstMovLength, len);
}

void MovieAgent::OnBeginFrame(Event& event)
{
	long sf, ef;
	getValue(gstBeginFrame, sf);
	getValue(gstEndFrame, ef);
	//panel_.m_start_frame_text->SetValue(wxString::Format("%d", sf));
	double fps;
	getValue(gstMovFps, fps);
	//update length
	double len = double(ef - sf + 1) / fps;
	setValue(gstMovLength, len);
}

void MovieAgent::OnEndFrame(Event& event)
{
	long sf, ef;
	getValue(gstBeginFrame, sf);
	getValue(gstEndFrame, ef);
	//panel_.m_end_frame_text->SetValue(wxString::Format("%d", ef));
	double fps;
	getValue(gstMovFps, fps);
	//update length
	double len = double(ef - sf + 1) / fps;
	setValue(gstMovLength, len);
}

void MovieAgent::OnScriptFile(Event& event)
{
	bool bval;
	getValue(gstRunScript, bval);
	//enable script if not
	if (!bval)
		setValue(gstRunScript, true);
}

void MovieAgent::OnRunScript(Event& event)
{
	bool bval;
	getValue(gstRunScript, bval);
	//panel_.m_run_script_chk->SetValue(bval);
	//if (bval)
	//{
	//	panel_.m_play_btn->SetBitmap(wxGetBitmapFromMemory(playscript));
	//	panel_.m_notebook->SetPageText(4, "Script (Enabled)");
	//}
	//else
	//{
	//	panel_.m_play_btn->SetBitmap(wxGetBitmapFromMemory(play));
	//	panel_.m_notebook->SetPageText(4, "Script");
	//}
}

void MovieAgent::OnMovCurTime(Event& event)
{
	double cur_time;
	getValue(gstMovCurTime, cur_time);
	long sldr_range;
	getValue(gstMovSliderRange, sldr_range);
	double len;
	getValue(gstMovLength, len);
	double pcnt = (cur_time / len);
	//panel_.m_progress_sldr->SetValue(sldr_range * pcnt + 0.5);
	long sf, ef, cf;
	getValue(gstBeginFrame, sf);
	getValue(gstEndFrame, ef);
	long time = ef - sf + 1;
	cf = (int)(sf + time * pcnt);
	setValue(gstCurrentFrame, cf);

	SetRendering(pcnt, false);
}

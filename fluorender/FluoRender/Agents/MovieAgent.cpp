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

#include <MovieAgent.hpp>
#include <MoviePanel.h>
#include <Global.hpp>
#include <AgentFactory.hpp>
#include <RecorderAgent.hpp>
#include <Root.hpp>
#include <AsyncTimer.hpp>
#include <AsyncTimerFactory.hpp>
#include <TextureRenderer.h>
#include <tiffio.h>
#include <img/icons.h>

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

void MovieAgent::setObject(Renderview* obj)
{
	InterfaceAgent::setObject(obj);
	std::string name = obj->getName();
	int i = panel_.m_views_cmb->FindString(name);
	panel_.m_views_cmb->SetSelection(i);
}

Renderview* MovieAgent::getObject()
{
	return dynamic_cast<Renderview*>(InterfaceAgent::getObject());
}

void MovieAgent::UpdateAllSettings()
{
	//update views
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

	bool bval;
	getValue(gstMovTimeSeqEnable, bval);
	getValue(gstDrawCropFrame, bval);

	AddScriptToList();
	GetScriptSettings();
	glbin_agtf->findFirst(gstRecorderAgent)->asRecorderAgent()->UpdateAllSettings();
}

void MovieAgent::SetProgress(double pcnt)
{
	pcnt = std::abs(pcnt);
	long lval;
	getValue(gstMovSliderRange, lval);
	panel_.m_progress_sldr->SetValue(pcnt * lval);
	double dval;
	getValue(gstMovLength, dval);
	dval *= pcnt;
	setValue(gstMovCurTime, dval);
	wxString st = wxString::Format("%.2f", dval);
	panel_.m_progress_text->ChangeValue(st);
}

int MovieAgent::GetScriptFiles(wxArrayString& list)
{
	wxString exePath = glbin.getExecutablePath();
	exePath = wxPathOnly(exePath);
	wxString loc = exePath + GETSLASH() + "Scripts" +
		GETSLASH() + "*.txt";
	wxLogNull logNo;
	wxString file = wxFindFirstFile(loc);
	while (!file.empty())
	{
		list.Add(file);
		file = wxFindNextFile();
	}
	list.Sort();
	return list.GetCount();
}

void MovieAgent::AddScriptToList()
{
	panel_.m_script_list->DeleteAllItems();
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
}

void MovieAgent::GetScriptSettings()
{
	bool run_script;
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
}

void MovieAgent::WriteFrameToFile(int total_frames)
{
	wxString s_length = wxString::Format("%d", total_frames);
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
}

void MovieAgent::Stop()
{
	bool run_script;
	getValue(gstRunScript, run_script);
	if (run_script)
		panel_.m_play_btn->SetBitmap(wxGetBitmapFromMemory(playscript));
	else
		panel_.m_play_btn->SetBitmap(wxGetBitmapFromMemory(play));

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
	setValue(gstCamLockObjEnable, false);
	//advanced options
	if (m_current_page == 1)
	{
		Interpolator *interpolator = m_frame->GetInterpolator();
		if (interpolator && interpolator->GetLastIndex() > 0)
		{
			if (m_advanced_movie->GetCamLock() && m_timer.IsRunning())
				m_view->setValue(gstCamLockObjEnable, true);
			int end_frame = int(interpolator->GetLastT());
			m_view->SetParams(pcnt * end_frame);
			m_view->setValue(gstInteractive, false);
			//m_view->RefreshGL(39);
			return;
		}
	}
	//basic options
	int time = m_end_frame - m_start_frame + 1;
	time = int(m_start_frame + time * pcnt + 0.5);

	if (m_seq_mode == 1)
	{
		m_view->Set4DSeqFrame(time, m_start_frame, m_end_frame, rewind);
	}
	else if (m_seq_mode == 2)
	{
		m_view->Set3DBatFrame(time, m_start_frame, m_end_frame, rewind);
	}

	//rotate animation
	if (m_rotate)
	{
		std::string sval;
		switch (m_rot_axis)
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
		double dval;
		m_view->getValue(sval, dval);
		if (m_rot_int_type == 0)
			dval = m_starting_rot + pcnt * m_rot_deg;
		else if (m_rot_int_type == 1)
			dval = m_starting_rot +
			(-2.0*pcnt*pcnt*pcnt + 3.0*pcnt*pcnt) * m_rot_deg;
		m_view->setValue(sval, dval);
	}

	m_view->setValue(gstInteractive, false);
	//m_view->RefreshGL(39);
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
		panel_.m_seq_chk->SetValue(false);
		panel_.m_bat_chk->SetValue(false);
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
	panel_.m_start_frame_text->ChangeValue(wxString::Format("%d", bf));
	panel_.m_end_frame_text->ChangeValue(wxString::Format("%d", ef));
	panel_.m_movie_len_text->ChangeValue(wxString::Format("%.2f", dval));
}

void MovieAgent::OnMovSeqMode(Event& event)
{
	long lval;
	getValue(gstMovSeqMode, lval);
	if (lval == 1)
	{
		panel_.m_seq_chk->SetValue(true);
		panel_.m_bat_chk->SetValue(false);
		getObject()->Get4DSeqRange();
	}
	else if (lval == 2)
	{
		panel_.m_seq_chk->SetValue(false);
		panel_.m_bat_chk->SetValue(true);
		getObject()->Get3DBatRange();
	}
}

void MovieAgent::OnMovRotEnable(Event& event)
{
	bool bval;
	getValue(gstMovRotEnable, bval);
	panel_.m_x_rd->Enable(bval);
	panel_.m_y_rd->Enable(bval);
	panel_.m_z_rd->Enable(bval);
	panel_.m_degree_text->Enable(bval);
	panel_.m_rot_chk->SetValue(bval);
}

void MovieAgent::OnMovRotAxis(Event& event)
{
	long lval;
	getValue(gstMovRotAxis, lval);
	if (lval == 0)
		panel_.m_x_rd->SetValue(true);
	else if (lval == 1)
		panel_.m_y_rd->SetValue(true);
	else if (lval == 2)
		panel_.m_z_rd->SetValue(true);
}

void MovieAgent::OnMovRotAng(Event& event)
{
	double dval;
	getValue(gstMovRotAng, dval);
	panel_.m_degree_text->SetValue(wxString::Format("%d", dval));
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

	panel_.m_cur_frame_text->ChangeValue(wxString::Format("%d", cf));
	chgValue(gstCurrentFrame, cf);
	SetProgress(pcnt);
}

void MovieAgent::OnDrawCropFrame(Event& event)
{
	bool bval;
	getValue(gstDrawCropFrame, bval);
	panel_.m_crop_chk->SetValue(bval);

	if (bval)
	{
		getObject()->CalculateCrop();
		long x, y, w, h;
		getValue(gstCropX, x); getValue(gstCropY, y);
		getValue(gstCropW, w); getValue(gstCropH, h);
		x = long(x + w / 2.0 + 0.5);
		y = long(y + h / 2.0 + 0.5);
		panel_.m_center_x_text->ChangeValue(wxString::Format("%d", x));
		panel_.m_center_y_text->ChangeValue(wxString::Format("%d", y));
		panel_.m_width_text->ChangeValue(wxString::Format("%d", w));
		panel_.m_height_text->ChangeValue(wxString::Format("%d", h));
	}

	//m_view->RefreshGL(39);
}

void MovieAgent::OnMovCurrentPage(Event& event)
{
	long lval;
	getValue(gstMovCurrentPage, lval);
	if (panel_.m_notebook)
		panel_.m_notebook->SetSelection(lval);
}

void MovieAgent::OnAutoKeyIndex(Event& event)
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
		chgValue(gstMovDelayedStop, false);
		Stop();
		chgValue(gstKeepEnlarge, false);
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
		panel_.m_cur_frame_text->ChangeValue(wxString::Format("%d", cf));
		if (record)
			WriteFrameToFile(int(fps*len + 0.5));
		setValue(gstLastFrame, cf);
		SetRendering(cur_time / len);
	}
	if (len - cur_time < 0.1 / fps ||
		cur_time > len)
		setValue(gstMovDelayedStop, true);
}


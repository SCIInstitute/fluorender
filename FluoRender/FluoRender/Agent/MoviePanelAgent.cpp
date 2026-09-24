/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#include <MoviePanelAgent.h>
#include <MoviePanel.h>
#include <Global.h>
#include <Names.h>
#include <MovieMaker.h>
#include <Root.h>
#include <RenderView.h>
#include <DataManager.h>
#include <MovieDefault.h>
#include <MainSettings.h>
#include <Interpolator.h>
#include <Directory.h>
#include <CurrentObjects.h>
#include <Coordinator.h>
#include <GlobalStates.h>
#include <ModalDlg.h>

MoviePanelAgent::MoviePanelAgent(
	MoviePanel* panel) :
	Agent(panel)
{

}

MoviePanel* MoviePanelAgent::GetPanel() const
{
	return static_cast<MoviePanel*>(GetWindow());
}

void MoviePanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;

	bool update_all = request.values.empty() || request.HasValue(gstMoviePanelAgent);
	bool bval;
	int ival;
	double dval;

	//modes
	if (update_all || request.HasValue(gstMovFps))
	{
		dval = glbin_moviemaker.GetFps();
		panel->UpdateMovFps(dval);
	}

	if (update_all || request.HasValue(gstMovLength))
	{
		dval = glbin_moviemaker.GetMovieLength();
		panel->UpdateMovLength(dval);
	}

	if (update_all || request.HasValue(gstMovViewList))
	{
		MovViewListInfo info;
		Root* root = glbin_data_manager.GetRoot();
		if (root)
		{
			for (int i = 0; i < root->GetViewNum(); i++)
			{
				auto view = root->GetView(i);
				if (view)
					info.views.push_back(view->GetName());
			}
		}
	}
	if (update_all || request.HasValue(gstMovViewIndex))
	{
		ival = glbin_mov_def.m_view_idx;
		panel->UpdateMovViewIndex(ival);
	}

	if (update_all || request.HasValue(gstMovSliderStyle))
	{
		bval = glbin_mov_def.m_slider_style;
		panel->UpdateMovSliderStyle(bval);
	}

	if (update_all || request.HasValue(gstMovProgSlider))
	{
		int cf = glbin_moviemaker.GetCurrentFrame();
		int ts = glbin_moviemaker.GetScrollThumbSize();
		int sf = glbin_moviemaker.GetClipStartFrame();
		int ef = glbin_moviemaker.GetClipEndFrame();
		panel->UpdateMovProgSlider(sf, ts, sf, ef);
	}

	if (update_all || request.HasValue(gstBeginFrame))
	{
		ival = glbin_moviemaker.GetClipStartFrame();
		panel->UpdateBeginFrame(ival);
	}

	if (update_all || request.HasValue(gstEndFrame))
	{
		ival = glbin_moviemaker.GetClipEndFrame();
		panel->UpdateEndFrame(ival);
	}

	if (update_all || request.HasValue(gstCurrentFrame))
	{
		ival = glbin_moviemaker.GetCurrentFrame();
		panel->UpdateCurrentFrame(ival);
	}

	if (update_all || request.HasValue(gstTotalFrames))
	{
		ival = glbin_moviemaker.GetFullFrameNum();
		panel->UpdateTotalFrames(ival);
	}

	if (update_all || request.HasValue(gstMovCurTime))
	{
		dval = glbin_moviemaker.GetCurrentTime();
		panel->UpdateMovCurTime(dval);
	}

	if (update_all || request.HasValue(gstMovPlay))
	{
		bool running = glbin_moviemaker.IsRunning();
		bool reverse = glbin_moviemaker.IsReverse();
		bool script = glbin_settings.m_run_script;
		panel->UpdateMovPlay(running, reverse, script);
	}

	if (update_all || request.HasValue(gstMovLoop))
	{
		bval = glbin_moviemaker.IsLoop();
		panel->UpdateMovLoop(bval);
	}

	if (update_all || request.HasValue(gstMovRotEnable))
	{
		bval = glbin_moviemaker.GetRotateEnable();
		panel->UpdateMovRotEnable(bval);
	}

	if (update_all || request.HasValue(gstMovRotAxis))
	{
		ival = glbin_moviemaker.GetRotateAxis();
		panel->UpdateMovRotAxis(ival);
	}

	if (update_all || request.HasValue(gstMovRotAng))
	{
		ival = glbin_moviemaker.GetRotateDeg();
		panel->UpdateMovRotAng(ival);
	}

	if (update_all || request.HasValue(gstMovIntrpMode))
	{
		ival = glbin_moviemaker.GetInterpolation();
		panel->UpdateMovIntrpMode(ival);
	}

	if (update_all || request.HasValue(gstMovSeqMode))
	{
		ival = glbin_moviemaker.GetSeqMode();
		panel->UpdateMovSeqMode(ival);
	}

	if (update_all || request.HasValue(gstMovSeqNum))
	{
		int scn = glbin_moviemaker.GetSeqCurNum();
		int san = glbin_moviemaker.GetSeqAllNum();
		panel->UpdateMovSeqNum(scn, san);
	}

	if (update_all || request.HasValue(gstCaptureParam))
	{
		bval = glbin_moviemaker.GetKeyframeEnable();
		panel->UpdateCaptureParam(bval);
	}

	if (update_all || request.HasValue(gstParamKeyDuration))
	{
		dval = glbin_moviemaker.GetKeyDuration();
		panel->UpdateParamKeyDuration(dval);
	}

	if (update_all || request.HasValue(gstParamList))
	{
		std::vector<KeyframeInfo> list;
		for (int i = 0; i < glbin_interpolator.GetKeyNum(); i++)
		{
			int id = glbin_interpolator.GetKeyID(i);
			int time = glbin_interpolator.GetKeyTime(i);
			int duration = glbin_interpolator.GetKeyDuration(i);
			int interp = glbin_interpolator.GetKeyType(i);
			std::wstring desc = glbin_interpolator.GetKeyDesc(i);
			list.push_back(KeyframeInfo(
				id,
				time,
				duration,
				interp,
				desc));
		}
		panel->UpdateParamList(list);
	}

	if (update_all || request.HasValue(gstParamListSelect))
	{
		dval = glbin_moviemaker.GetCurProg();
		ival = glbin_interpolator.GetKeyIndexFromTime(dval);
		panel->UpdateParamListSelect(ival);
	}

	if (update_all || request.HasValue(gstCamLockObjEnable))
	{
		bval = glbin_moviemaker.GetCamLock();
		panel->UpdateCamLockObjEnable(bval);
	}

	if (update_all || request.HasValue(gstCamLockType))
	{
		ival = glbin_moviemaker.GetCamLockType() - 1;
		panel->UpdateCamLockType(ival);
	}

	if (update_all || request.HasValue(gstPresetList))
	{
		auto names = glbin_moviemaker.GetAutoKeyTypes();
		std::vector<PresetInfo> list;
		int i = 0;
		for (auto& name : names)
		{
			list.push_back(PresetInfo(std::to_string(i), name));
			++i;
		}
		panel->UpdatePresetList(list);
	}

	if (update_all || request.HasValue(gstCropEnable))
	{
		bval = glbin_moviemaker.GetCropEnable();
		panel->UpdateCropEnable(bval);
	}

	if (update_all || request.HasValue(gstCropValues))
	{
		int x = glbin_moviemaker.GetCropX();
		int y = glbin_moviemaker.GetCropY();
		int w = glbin_moviemaker.GetCropW();
		int h = glbin_moviemaker.GetCropH();
		panel->UpdateCropValues(x, y, w, h);
	}

	if (update_all || request.HasValue(gstScalebarPos))
	{
		ival = glbin_moviemaker.GetScalebarPos();
		int x = glbin_moviemaker.GetScalebarX();
		int y = glbin_moviemaker.GetScalebarY();
		panel->UpdateScalebarPos(ival, x, y);
	}

	if (update_all || request.HasValue(gstRunScript))
	{
		bval = glbin_settings.m_run_script;
		panel->UpdateRunScript(bval);
	}

	if (update_all || request.HasValue(gstScriptFile))
	{
		std::wstring filename = glbin_settings.m_script_file;
		panel->UpdateScriptFile(filename);
	}

	if (update_all || request.HasValue(gstScriptList))
	{
		std::vector<std::wstring> names;
		if (GetScriptFiles(names))
		{
			std::vector<ScriptInfo> list;
			int i = 0;
			for (auto& name : names)
			{
				list.push_back(ScriptInfo(std::to_string(i), name));
				++i;
			}
			panel->UpdateScriptList(list);
		}
	}

	if (update_all || request.HasValue(gstScriptSelect))
	{
		std::vector<std::wstring> list;
		if (GetScriptFiles(list))
		{
			int idx = -1;
			for (size_t i = 0; i < list.size(); ++i)
			{
				if (glbin_settings.m_script_file == list[i])
				{
					idx = i;
					break;
				}
			}
			panel->UpdateScriptListSelect(idx);
		}
	}
}

void MoviePanelAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstMovFps))
		SetFps();
	if (request.HasValue(gstMovLength))
		SetMovieLength();
	if (request.HasValue(gstMovViewIndex))
		SetViewIndex();
	if (request.HasValue(gstMovSliderStyle))
		SetSliderStyle();
	if (request.HasValue(gstMovProgSlider))
		SetScrollFrame();
	if (request.HasValue(gstBeginFrame))
		SetStartFrame();
	if (request.HasValue(gstEndFrame))
		SetEndFrame();
	if (request.HasValue(gstCurrentFrame))
		SetCurrentFrame();
	if (request.HasValue(gstMovCurTime))
		SetCurrentTime();
	if (request.HasValue(gstTotalFrames))
		SetFullFrame();
	if (request.HasValue(gstMovPlay))
		Play();
	if (request.HasValue(gstMovPlayInv))
		PlayInv();
	if (request.HasValue(gstMovRewind))
		Rewind();
	if (request.HasValue(gstMovForward))
		Forward();
	if (request.HasValue(gstMovLoop))
		Loop();
	if (request.HasValue(gstMovIncFrame))
		IncFrame();
	if (request.HasValue(gstMovDecFrame))
		DecFrame();
	if (request.HasValue(gstMovSave))
		Save();
	if (request.HasValue(gstMovRotEnable))
		SetRotateEnable();
	if (request.HasValue(gstMovRotAxis))
		SetRotateAxis();
	if (request.HasValue(gstMovRotAng))
		SetRotateDeg();
	if (request.HasValue(gstMovIntrpMode))
		SetRotateInterp();
	if (request.HasValue(gstMovSeqMode))
		SetSeqMode();
	if (request.HasValue(gstMovSeqDec))
		SetSeqDec();
	if (request.HasValue(gstMovSeqInc))
		SetSeqInc();
	if (request.HasValue(gstMovSeqNum))
		SetSeqNum();
	if (request.HasValue(gstMovKeyframeNum))
		SetKeyframeNum();
	if (request.HasValue(gstCaptureParam))
		SetKeyframeMovie();
	if (request.HasValue(gstKeyDuration))
		SetKeyDuration();
	if (request.HasValue(gstKeyInterpolation))
		SetKeyInterpolation();
	if (request.HasValue(gstMovInsertKey))
		InsertKey();
	if (request.HasValue(gstCamLockObjEnable))
		SetCameraLock();
	if (request.HasValue(gstCamLockType))
		SetCameraLockType();
	if (request.HasValue(gstCamLockCtr))
		SetCameraLockCenter();
	if (request.HasValue(gstGenerateKeys))
		GenerateKeys();
	if (request.HasValue(gstCropEnable))
		SetCropEnable();
	if (request.HasValue(gstCropValues))
		SetCropValues();
	if (request.HasValue(gstScalebarPos))
		SetScalebarPos();
	if (request.HasValue(gstScalbarOffset))
		SetScalebarValues();
	if (request.HasValue(gstEnableScript))
		EnableScript();
	if (request.HasValue(gstScriptFile))
		SetScriptFile();
	if (request.HasValue(gstLoadScriptFile))
		LoadScriptFile();
	if (request.HasValue(gstSelectScriptFile))
		SelectScriptFile();
}

void MoviePanelAgent::SelectKeyframe(int id)
{

}

void MoviePanelAgent::DeleteKeyframe(int id)
{
	glbin_interpolator.RemoveKey(id);
	glbin_moviemaker.SetFullFrameNum(std::round(glbin_interpolator.GetLastT()));
	UpdateDataToUI({ gstMovLength, gstMovProgSlider, gstBeginFrame, gstEndFrame, gstCurrentFrame, gstTotalFrames, gstParamList, gstParamListSelect });
}

void MoviePanelAgent::DeleteAllKeyframes()
{
	glbin_interpolator.Clear();
	glbin_moviemaker.SetFullFrameNum(std::round(glbin_interpolator.GetLastT()));
	UpdateDataToUI({ gstMovLength, gstMovProgSlider, gstBeginFrame, gstEndFrame, gstCurrentFrame, gstTotalFrames, gstParamList, gstParamListSelect });
}

void MoviePanelAgent::SetKeyframeTime(int id, double time)
{
	glbin_interpolator.ChangeTime(id, time);
	UpdateDataToUI({ gstParamList });
}

void MoviePanelAgent::SetKeyframeDuration(int id, double duration)
{
	glbin_interpolator.ChangeDuration(id, duration);
	UpdateDataToUI({ gstParamList });
}

void MoviePanelAgent::SetKeyframeInterpolation(int id, int type)
{
	glbin_interpolator.ChangeInterpolation(id, type);
	UpdateDataToUI({ gstParamList });
}

void MoviePanelAgent::SetKeyframeDescription(int id, const std::wstring& description)
{
	glbin_interpolator.ChangeDescription(id, description);
	UpdateDataToUI({ gstParamList });
}

void MoviePanelAgent::MoveKeyframe(int sourceId, int targetId, bool before)
{
	if (before)
		glbin_interpolator.MoveKeyBefore(sourceId, targetId);
	else
		glbin_interpolator.MoveKeyAfter(sourceId, targetId);
	UpdateDataToUI({ gstParamList });
}

size_t MoviePanelAgent::GetScriptFiles(std::vector<std::wstring>& list)
{
	std::filesystem::path p = GetUserSettingsRoot();
	p /= "Scripts";
	// Iterate over the files in the "Scripts" directory
	if (!std::filesystem::exists(p) || !std::filesystem::is_directory(p))
		return 0;
	for (const auto& entry : std::filesystem::directory_iterator(p))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".txt")
		{
			list.push_back(entry.path().wstring());
		}
	}

	// Sort the list of files
	std::sort(list.begin(), list.end());
	return list.size();
}

void MoviePanelAgent::SetFps()
{
	auto panel = GetPanel();
	if (panel)
	{
		glbin_moviemaker.SetFps(panel->GetFps());
		UpdateDataToUI({ gstMovFps, gstMovLength });
	}
}

void MoviePanelAgent::SetMovieLength()
{
	auto panel = GetPanel();
	if (panel)
	{
		glbin_moviemaker.SetMovieLength(panel->GetMovieLength());
		UpdateDataToUI({ gstMovFps, gstMovLength });
	}
}

void MoviePanelAgent::SetViewIndex()
{
	auto panel = GetPanel();
	if (panel)
	{
		glbin_mov_def.m_view_idx = panel->GetViewIndex();
		//UpdateDataToUI({ gstMovViewIndex });
	}
}

void MoviePanelAgent::SetSliderStyle()
{
	glbin_mov_def.m_slider_style = !glbin_mov_def.m_slider_style;

	UpdateDataToUI({ gstMovSliderStyle });
}

void MoviePanelAgent::SetScrollFrame()
{
	auto panel = GetPanel();
	if (panel)
		return;

	int ival = panel->GetProgressScroll();

	if (glbin_moviemaker.GetCurrentFrame() == ival)
		return;
	glbin_moviemaker.SetCurrentFrame(ival);

	fluo::ValueCollection vc = { gstMovCurTime, gstCurrentFrame, gstMovSeqNum };
	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(vc, target);
}

void MoviePanelAgent::SetStartFrame()
{
	auto panel = GetPanel();
	if (panel)
		return;

	if (glbin_moviemaker.IsRunning())
		return;
	glbin_moviemaker.SetClipStartFrame(panel->GetStartFrame());

	UpdateDataToUI({ gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovFps, gstMovLength, gstMovProgSlider, gstMovSeqNum });
}

void MoviePanelAgent::SetEndFrame()
{
	auto panel = GetPanel();
	if (panel)
		return;

	if (glbin_moviemaker.IsRunning())
		return;
	glbin_moviemaker.SetClipEndFrame(panel->GetEndFrame());

	UpdateDataToUI({ gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovFps, gstMovLength, gstMovProgSlider, gstMovSeqNum });
}

void MoviePanelAgent::SetCurrentFrame()
{
	auto panel = GetPanel();
	if (panel)
		return;

	glbin_moviemaker.SetCurrentFrame(panel->GetCurrentFrame());

	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider, gstMovSeqNum };
	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(vc, target);
}

void MoviePanelAgent::SetCurrentTime()
{
	auto panel = GetPanel();
	if (panel)
		return;

	if (glbin_moviemaker.IsRunning())
		return;

	glbin_moviemaker.SetCurrentTime(panel->GetCurTime());

	fluo::ValueCollection vc = { gstCurrentFrame, gstMovProgSlider, gstMovSeqNum };
	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(vc, target);
}

void MoviePanelAgent::SetFullFrame()
{
	auto panel = GetPanel();
	if (panel)
		return;

	glbin_moviemaker.SetFullFrameNum(panel->GetFullFrame());

	UpdateDataToUI({ gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovFps, gstMovLength, gstMovProgSlider, gstMovSeqNum });
}

void MoviePanelAgent::Play()
{
	glbin_moviemaker.Play(false);

	fluo::ValueCollection vc = { gstMovPlay };
	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(vc, target);
}

void MoviePanelAgent::PlayInv()
{
	glbin_moviemaker.Play(true);

	fluo::ValueCollection vc = { gstMovPlay };
	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(vc, target);
}

void MoviePanelAgent::Rewind()
{
	glbin_moviemaker.Rewind();

	fluo::ValueCollection vc = { gstCurrentFrame, gstMovCurTime, gstMovProgSlider, gstMovPlay, gstMovSeqNum };
	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(vc, target);
}

void MoviePanelAgent::Forward()
{
	glbin_moviemaker.Forward();

	fluo::ValueCollection vc = { gstCurrentFrame, gstMovCurTime, gstMovProgSlider, gstMovPlay, gstMovSeqNum };
	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(vc, target);
}

void MoviePanelAgent::Loop()
{
	auto panel = GetPanel();
	if (panel)
		return;

	glbin_moviemaker.SetLoop(panel->GetLoop());
}

void MoviePanelAgent::IncFrame()
{
	if (glbin_moviemaker.IsRunning())
		return;
	int frame = glbin_moviemaker.GetCurrentFrame();
	frame++;
	glbin_moviemaker.SetCurrentFrame(frame);
	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider, gstCurrentFrame, gstMovSeqNum };
	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(vc, target);
}

void MoviePanelAgent::DecFrame()
{
	if (glbin_moviemaker.IsRunning())
		return;
	int frame = glbin_moviemaker.GetCurrentFrame();
	frame--;
	glbin_moviemaker.SetCurrentFrame(frame);
	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider, gstCurrentFrame, gstMovSeqNum };
	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(vc, target);
}

void MoviePanelAgent::Save()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	ModalDlg dlg(
		panel, "Save Movie Sequence",
		"", "output",
		"MP4 file (*.mp4)|*.mp4|"\
		"Tiff files(*.tif)|*.tif|"\
		"Tiff files(*.tiff)|*.tiff|"\
		"Png files(*.png)|*.png|"\
		"Jpeg files(*.jpg)|*.jpg|"\
		"Jpeg files(*.jpeg)|*.jpeg|"\
		"Jpeg2000 files(*.jp2)|*.jp2",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	SaveMovieOptions initial_options;

	initial_options.project_save = glbin_settings.m_prj_save;
	initial_options.movie_length_sec = glbin_moviemaker.GetMovieLength();
	initial_options.embed_project_files = glbin_settings.m_vrp_embed;
	initial_options.dpi = glbin_settings.m_dpi;
	initial_options.enlarge_output = initial_options.dpi > 72;
	initial_options.enlarge_scale = initial_options.dpi / 72.0;
	initial_options.compress = glbin_settings.m_save_compress;
	initial_options.save_alpha = glbin_settings.m_save_alpha;
	initial_options.save_float = glbin_settings.m_save_float;
	initial_options.bitrate = glbin_settings.m_mov_bitrate;
	initial_options.estimated_size_mb = 0.0;

	SaveMovieHook hook(initial_options);

	dlg.SetCustomizeHook(hook);
	dlg.CenterOnParent();
	int rval = dlg.ShowModal();
	if (rval == wxID_OK)
	{
		//update settings
		glbin_settings.m_prj_save = initial_options.project_save;
		glbin_settings.m_vrp_embed = initial_options.embed_project_files;
		glbin_settings.m_dpi = initial_options.dpi;
		glbin_settings.m_save_compress = initial_options.compress;
		glbin_settings.m_save_alpha = initial_options.save_alpha;
		glbin_settings.m_save_float = initial_options.save_float;
		glbin_settings.m_mov_bitrate = initial_options.bitrate;

		if (glbin_moviemaker.IsRunning())
			return;

		glbin_states.m_capture = true;
		glbin_moviemaker.SetFileName(dlg.GetPath().ToStdWstring());
		glbin_moviemaker.PlaySave();

		fluo::ValueCollection vc = { gstMovPlay };
		auto view = glbin_current.render_view.lock();
		std::set<Agent*> target{ this };
		target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
		NotifyViewUpdate(vc, target);
	}
}

void MoviePanelAgent::SetRotateEnable()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	glbin_moviemaker.SetRotateEnable(panel->GetRotateEnable());
	UpdateDataToUI({ gstCaptureParam, gstMovRotEnable, gstMovSeqMode, gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovLength, gstMovProgSlider, gstMovSeqNum });
}

void MoviePanelAgent::SetRotateAxis()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	glbin_moviemaker.SetRotateAxis(panel->GetRotateAxis());
}

void MoviePanelAgent::SetRotateDeg()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	glbin_moviemaker.SetRotateDeg(panel->GetRotateDeg());
	UpdateDataToUI({ gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovLength, gstMovProgSlider, gstMovSeqNum });
}

void MoviePanelAgent::SetRotateInterp()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	glbin_moviemaker.SetInterpolation(panel->GetRotateInterp());
	UpdateDataToUI({ gstMovIntrpMode });
}

void MoviePanelAgent::SetSeqMode()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	glbin_moviemaker.SetSeqMode(panel->GetSeqMode());
	UpdateDataToUI({ gstCaptureParam, gstMovRotEnable, gstMovSeqMode, gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovLength, gstMovProgSlider, gstMovSeqNum });
}

void MoviePanelAgent::SetSeqDec()
{
	int val = glbin_moviemaker.GetSeqCurNum();
	glbin_moviemaker.SetSeqCurNum(val - 1);
	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(
		{ gstCurrentFrame, gstMovProgSlider, gstMovSeqNum },
		target);
}

void MoviePanelAgent::SetSeqInc()
{
	int val = glbin_moviemaker.GetSeqCurNum();
	glbin_moviemaker.SetSeqCurNum(val + 1);
	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(
		{ gstCurrentFrame, gstMovProgSlider, gstMovSeqNum },
		target);
}

void MoviePanelAgent::SetSeqNum()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	glbin_moviemaker.SetSeqCurNum(panel->GetSeqNum());
	auto view = glbin_current.render_view.lock();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(
		{ gstCurrentFrame, gstMovProgSlider, gstMovSeqNum },
		target);
}

void MoviePanelAgent::SetKeyframeNum()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	int index = glbin_interpolator.GetKeyIndex(panel->GetKeyframeNum());
	double time = glbin_interpolator.GetKeyTime(index);
	glbin_moviemaker.SetCurrentFrame(time);

	auto view = glbin_moviemaker.GetView();
	if (view)
		view->SetParams(time);
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(
		{ gstCurrentFrame, gstMovProgSlider, gstMovSeqNum, gstParamListSelect },
		target);
}

void MoviePanelAgent::SetKeyframeMovie()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	glbin_moviemaker.SetKeyframeEnable(panel->GetKeyframeEnable(), true);

	auto view = glbin_moviemaker.GetView();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(
		{ gstCaptureParam, gstMovLength, gstMovProgSlider, gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovCurTime, gstMovSeqNum },
		target);
}

void MoviePanelAgent::SetKeyDuration()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	glbin_moviemaker.SetKeyDuration(panel->GetKeyDuration());
}

void MoviePanelAgent::SetKeyInterpolation()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	glbin_moviemaker.SetInterpolation(panel->GetKeyInterpolation());
	UpdateDataToUI({ gstMovIntrpMode });
}

void MoviePanelAgent::InsertKey()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	if (!glbin_moviemaker.GetKeyframeEnable())
		glbin_moviemaker.SetKeyframeEnable(true, true);

	int index = glbin_interpolator.GetKeyIndex(panel->GetKeyframeNum());
	glbin_moviemaker.InsertKey(index);

	UpdateDataToUI({ gstCaptureParam, gstMovLength, gstMovProgSlider, gstBeginFrame, gstEndFrame, gstCurrentFrame, gstTotalFrames, gstMovCurTime, gstMovSeqNum, gstParamList, gstParamListSelect });
}

void MoviePanelAgent::SetCameraLock()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	glbin_moviemaker.SetCamLock(panel->GetCameraLock());
}

void MoviePanelAgent::SetCameraLockType()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	glbin_moviemaker.SetCamLockType(panel->GetCameraLockType());
}

void MoviePanelAgent::SetCameraLockCenter()
{
	if (auto view = glbin_moviemaker.GetView())
		view->SetLockCenter();
}

void MoviePanelAgent::GenerateKeys()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	glbin_moviemaker.MakeKeys(panel->GetPresetNum());
	if (!glbin_moviemaker.GetKeyframeEnable())
		glbin_moviemaker.SetKeyframeEnable(true, true);

	UpdateDataToUI({ gstCaptureParam, gstMovLength, gstMovProgSlider, gstBeginFrame, gstEndFrame, gstCurrentFrame, gstTotalFrames, gstMovCurTime, gstMovSeqNum, gstParamList, gstParamListSelect });
}

void MoviePanelAgent::SetCropEnable()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	glbin_moviemaker.SetCropEnable(panel->GetCropEnable());
	auto view = glbin_moviemaker.GetView();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(
		{ gstCropEnable, gstCropValues }, target);
}

void MoviePanelAgent::SetCropValues()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	auto crop_info = panel->GetCropValues();
	glbin_moviemaker.SetCropValues(crop_info.x, crop_info.y, crop_info.w, crop_info.h);
	auto view = glbin_moviemaker.GetView();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(
		{ gstCropValues }, target);
}

void MoviePanelAgent::SetScalebarPos()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	glbin_moviemaker.SetScalebarPos(panel->GetScalebarPos());
	auto view = glbin_moviemaker.GetView();
	std::set<Agent*> target;
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate({ gstNull }, target);
}

void MoviePanelAgent::SetScalebarValues()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	auto offset = panel->GetScalebarOffset();
	glbin_moviemaker.SetScalebarDist(offset.x, offset.y);
	auto view = glbin_moviemaker.GetView();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(
		{ gstScalebarPos }, target);
}

void MoviePanelAgent::EnableScript()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	glbin_settings.m_run_script = panel->GetScriptEnable();
	glbin_settings.m_script_file = panel->GetScriptFileName();
	auto view = glbin_moviemaker.GetView();
	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(
		{ gstMovPlay, gstRunScript }, target);
}

void MoviePanelAgent::SetScriptFile()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	glbin_settings.m_script_file = panel->GetScriptFileName();
}

void MoviePanelAgent::LoadScriptFile()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	ModalDlg fopendlg(
		panel, "Choose a 4D script file", "", "",
		"4D script file (*.txt)|*.txt",
		wxFD_OPEN);

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		std::wstring file = fopendlg.GetPath().ToStdWstring();
		glbin_settings.m_script_file = file;
		glbin_settings.m_run_script = true;
		UpdateDataToUI({ gstMovPlay, gstRunScript });
	}
}

void MoviePanelAgent::SelectScriptFile()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	auto name = panel->GetSelScriptName();
	std::filesystem::path p = GetUserSettingsRoot();
	p = p / "Scripts" / (name + ".txt");
	std::wstring filename = p.wstring();
	glbin_settings.m_script_file = filename;
	glbin_settings.m_run_script = true;
	UpdateDataToUI({ gstMovPlay, gstRunScript, gstScriptFile });
}

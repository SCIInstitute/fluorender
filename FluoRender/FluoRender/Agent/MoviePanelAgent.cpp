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
}

void MoviePanelAgent::SelectKeyframe(int id)
{

}

void MoviePanelAgent::DeleteKeyframe(int id)
{
	glbin_interpolator.RemoveKey(id);
	UpdateDataToUI({ gstParamList });
}

void MoviePanelAgent::DeleteAllKeyframes()
{
	glbin_interpolator.Clear();
	UpdateDataToUI({ gstParamList });
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

void MoviePanelAgent::SetFullFrame(int val)
{
	glbin_moviemaker.SetFullFrameNum(val);

	FluoUpdate({ gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovFps, gstMovLength, gstMovProgSlider, gstMovSeqNum });
}

void MoviePanelAgent::SetStartFrame(int val)
{
	if (glbin_moviemaker.IsRunning())
		return;
	glbin_moviemaker.SetClipStartFrame(val);

	FluoUpdate({ gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovFps, gstMovLength, gstMovProgSlider, gstMovSeqNum });
}

void MoviePanelAgent::SetEndFrame(int val)
{
	if (glbin_moviemaker.IsRunning())
		return;
	glbin_moviemaker.SetClipEndFrame(val);

	FluoUpdate({ gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovFps, gstMovLength, gstMovProgSlider, gstMovSeqNum });
}

void MoviePanelAgent::SetCurrentFrame(int val, bool notify)
{
	//if (glbin_moviemaker.IsRunning())
	//	return;
	glbin_moviemaker.SetCurrentFrame(val);

	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider, gstMovSeqNum };
	if (notify)
		vc.insert(gstCurrentFrame);
	FluoRefresh(2, vc, { glbin_current.GetViewId() });
}

void MoviePanelAgent::SetCurrentTime(double val, bool notify)
{
	if (glbin_moviemaker.IsRunning())
		return;

	glbin_moviemaker.SetCurrentTime(val);

	fluo::ValueCollection vc = { gstCurrentFrame, gstMovProgSlider, gstMovSeqNum };
	if (notify)
		vc.insert(gstMovCurTime);
	FluoRefresh(2, vc, { glbin_current.GetViewId() });
}

void MoviePanelAgent::Play()
{
	glbin_moviemaker.Play(false);

	fluo::ValueCollection vc = { gstMovPlay };
	FluoRefresh(2, vc, { glbin_current.GetViewId() });
}

void MoviePanelAgent::PlayInv()
{
	glbin_moviemaker.Play(true);

	fluo::ValueCollection vc = { gstMovPlay };
	FluoRefresh(2, vc, { glbin_current.GetViewId() });
}

void MoviePanelAgent::Rewind()
{
	glbin_moviemaker.Rewind();

	fluo::ValueCollection vc = { gstCurrentFrame, gstMovCurTime, gstMovProgSlider, gstMovPlay, gstMovSeqNum };
	FluoRefresh(2, vc, { glbin_current.GetViewId() });
}

void MoviePanelAgent::Forward()
{
	glbin_moviemaker.Forward();

	fluo::ValueCollection vc = { gstCurrentFrame, gstMovCurTime, gstMovProgSlider, gstMovPlay, gstMovSeqNum };
	FluoRefresh(2, vc, { glbin_current.GetViewId() });
}

void MoviePanelAgent::Loop(bool val)
{
	glbin_moviemaker.SetLoop(val);
}

void MoviePanelAgent::IncFrame()
{
	if (glbin_moviemaker.IsRunning())
		return;
	int frame = glbin_moviemaker.GetCurrentFrame();
	frame++;
	glbin_moviemaker.SetCurrentFrame(frame);
	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider, gstCurrentFrame, gstMovSeqNum };
	FluoRefresh(2, vc, { glbin_current.GetViewId() });
}

void MoviePanelAgent::DecFrame()
{
	if (glbin_moviemaker.IsRunning())
		return;
	int frame = glbin_moviemaker.GetCurrentFrame();
	frame--;
	glbin_moviemaker.SetCurrentFrame(frame);
	fluo::ValueCollection vc = { gstMovCurTime, gstMovProgSlider, gstCurrentFrame, gstMovSeqNum };
	FluoRefresh(2, vc, { glbin_current.GetViewId() });
}

void MoviePanelAgent::Save(const std::wstring& filename)
{
	if (glbin_moviemaker.IsRunning())
		return;

	glbin_states.m_capture = true;
	glbin_moviemaker.SetFileName(filename);
	glbin_moviemaker.PlaySave();

	fluo::ValueCollection vc = { gstMovPlay };
	FluoRefresh(2, vc, { glbin_current.GetViewId() });
}

void MoviePanelAgent::SetKeyframeMovie(bool val)
{
	glbin_moviemaker.SetKeyframeEnable(val, true);

	FluoUpdate({ gstCaptureParam, gstMovLength, gstMovProgSlider, gstBeginFrame, gstEndFrame, gstCurrentFrame, gstMovCurTime, gstMovSeqNum });
}

void MoviePanelAgent::SetCropEnable(bool val)
{
	glbin_moviemaker.SetCropEnable(val);
	FluoRefresh(2, { gstCropEnable, gstCropValues }, { glbin_current.GetViewId() });
}

void MoviePanelAgent::SetCropValues(int x, int y, int w, int h)
{
	glbin_moviemaker.SetCropValues(x, y, w, h);
	FluoRefresh(2, { gstCropValues }, { glbin_current.GetViewId() });
}

void MoviePanelAgent::SetScalebarPos(int pos)
{
	glbin_moviemaker.SetScalebarPos(pos);
	FluoRefresh(2, { gstNull }, { glbin_current.GetViewId() });
}

void MoviePanelAgent::SetScalebarValues(int x, int y)
{
	glbin_moviemaker.SetScalebarDist(x, y);
	FluoRefresh(2, { gstScalebarPos }, { glbin_current.GetViewId() });
}


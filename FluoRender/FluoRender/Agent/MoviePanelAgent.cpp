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

MoviePanelAgent::MoviePanelAgent(
	MoviePanel* panel) :
	Agent(panel)
{

}

bool MoviePanelAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void MoviePanelAgent::Update(
	const UpdateRequest& request)
{
	if (request.dir == UpdateDir::DataToUI)
	{
		UpdateUI(request);
	}
	else if (request.dir == UpdateDir::UItoData)
	{
		UpdateData(request);
	}
}

void MoviePanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;

	if (FOUND_VALUE(gstNull))
		return;

	bool update_all = request.values.empty() || FOUND_VALUE(gstMoviePanelAgent);
	bool bval;
	int ival;
	double dval;

	//modes
	if (update_all || FOUND_VALUE(gstMovFps))
	{
		dval = glbin_moviemaker.GetFps();
		panel->UpdateMovFps(dval);
	}

	if (update_all || FOUND_VALUE(gstMovLength))
	{
		dval = glbin_moviemaker.GetMovieLength();
		panel->UpdateMovLength(dval);
	}

	if (update_all || FOUND_VALUE(gstMovViewList))
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
	if (update_all || FOUND_VALUE(gstMovViewIndex))
	{
		ival = glbin_mov_def.m_view_idx;
		panel->UpdateMovViewIndex(ival);
	}

	if (update_all || FOUND_VALUE(gstMovSliderStyle))
	{
		bval = glbin_mov_def.m_slider_style;
		panel->UpdateMovSliderStyle(bval);
	}

	if (update_all || FOUND_VALUE(gstMovProgSlider))
	{
		int cf = glbin_moviemaker.GetCurrentFrame();
		int ts = glbin_moviemaker.GetScrollThumbSize();
		int sf = glbin_moviemaker.GetClipStartFrame();
		int ef = glbin_moviemaker.GetClipEndFrame();
		panel->UpdateMovProgSlider(sf, ts, sf, ef);
	}

	if (update_all || FOUND_VALUE(gstBeginFrame))
	{
		ival = glbin_moviemaker.GetClipStartFrame();
		panel->UpdateBeginFrame(ival);
	}

	if (update_all || FOUND_VALUE(gstEndFrame))
	{
		ival = glbin_moviemaker.GetClipEndFrame();
		panel->UpdateEndFrame(ival);
	}

	if (update_all || FOUND_VALUE(gstCurrentFrame))
	{
		ival = glbin_moviemaker.GetCurrentFrame();
		panel->UpdateCurrentFrame(ival);
	}

	if (update_all || FOUND_VALUE(gstTotalFrames))
	{
		ival = glbin_moviemaker.GetFullFrameNum();
		panel->UpdateTotalFrames(ival);
	}

	if (update_all || FOUND_VALUE(gstMovCurTime))
	{
		dval = glbin_moviemaker.GetCurrentTime();
		panel->UpdateMovCurTime(dval);
	}

	if (update_all || FOUND_VALUE(gstMovPlay))
	{
		bool running = glbin_moviemaker.IsRunning();
		bool reverse = glbin_moviemaker.IsReverse();
		bool script = glbin_settings.m_run_script;
		panel->UpdateMovPlay(running, reverse, script);
	}

	if (update_all || FOUND_VALUE(gstMovLoop))
	{
		bval = glbin_moviemaker.IsLoop();
		panel->UpdateMovLoop(bval);
	}

	if (update_all || FOUND_VALUE(gstMovRotEnable))
	{
		bval = glbin_moviemaker.GetRotateEnable();
		panel->UpdateMovRotEnable(bval);
	}

	if (update_all || FOUND_VALUE(gstMovRotAxis))
	{
		ival = glbin_moviemaker.GetRotateAxis();
		panel->UpdateMovRotAxis(ival);
	}

	if (update_all || FOUND_VALUE(gstMovRotAng))
	{
		ival = glbin_moviemaker.GetRotateDeg();
		panel->UpdateMovRotAng(ival);
	}

	if (update_all || FOUND_VALUE(gstMovIntrpMode))
	{
		ival = glbin_moviemaker.GetInterpolation();
		panel->UpdateMovIntrpMode(ival);
	}

	if (update_all || FOUND_VALUE(gstMovSeqMode))
	{
		ival = glbin_moviemaker.GetSeqMode();
		panel->UpdateMovSeqMode(ival);
	}

	if (update_all || FOUND_VALUE(gstMovSeqNum))
	{
		int scn = glbin_moviemaker.GetSeqCurNum();
		int san = glbin_moviemaker.GetSeqAllNum();
		panel->UpdateMovSeqNum(scn, san);
	}

	if (update_all || FOUND_VALUE(gstCaptureParam))
	{
		bval = glbin_moviemaker.GetKeyframeEnable();
		panel->UpdateCaptureParam(bval);
	}

	if (update_all || FOUND_VALUE(gstParamKeyDuration))
	{
		dval = glbin_moviemaker.GetKeyDuration();
		panel->UpdateParamKeyDuration(dval);
	}

	if (update_all || FOUND_VALUE(gstParamList))
		panel->UpdateParamList();

	if (update_all || FOUND_VALUE(gstParamListSelect))
	{
		dval = glbin_moviemaker.GetCurProg();
		ival = glbin_interpolator.GetKeyIndexFromTime(dval);
		panel->UpdateParamListSelect(ival);
	}

	if (update_all || FOUND_VALUE(gstCamLockObjEnable))
	{
		bval = glbin_moviemaker.GetCamLock();
		panel->UpdateCamLockObjEnable(bval);
	}

	if (update_all || FOUND_VALUE(gstCamLockType))
	{
		ival = glbin_moviemaker.GetCamLockType() - 1;
		panel->UpdateCamLockType(ival);
	}

	if (update_all || FOUND_VALUE(gstCropEnable))
	{
		bval = glbin_moviemaker.GetCropEnable();
		panel->UpdateCropEnable(bval);
	}

	if (update_all || FOUND_VALUE(gstCropValues))
	{
		int x = glbin_moviemaker.GetCropX();
		int y = glbin_moviemaker.GetCropY();
		int w = glbin_moviemaker.GetCropW();
		int h = glbin_moviemaker.GetCropH();
		panel->UpdateCropValues(x, y, w, h);
	}

	if (update_all || FOUND_VALUE(gstScalebarPos))
	{
		ival = glbin_moviemaker.GetScalebarPos();
		int x = glbin_moviemaker.GetScalebarX();
		int y = glbin_moviemaker.GetScalebarY();
		panel->UpdateScalebarPos(ival, x, y);
	}

	if (update_all || FOUND_VALUE(gstRunScript))
	{
		bval = glbin_settings.m_run_script;
		panel->UpdateRunScript(bval);
	}

	if (update_all || FOUND_VALUE(gstScriptFile))
	{
		std::wstring filename = glbin_settings.m_script_file;
		panel->UpdateScriptFile(filename);
	}

	if (update_all || FOUND_VALUE(gstScriptList))
	{
		std::vector<std::wstring> list;
		if (GetScriptFiles(list))
			panel->UpdateScriptList(list);
	}

	if (update_all || FOUND_VALUE(gstScriptSelect))
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


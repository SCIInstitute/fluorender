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

#include <RenderviewAgent.hpp>
//#include <RenderviewPanel.h>
#include <AgentFactory.hpp>
#include <RenderFrameAgent.hpp>
#include <ClipPlaneAgent.hpp>
#include <compatibility.h>

using namespace fluo;

#pragma message ("replace dummy dialog")
class RenderviewPanel : public wxWindow
{
public:
	RenderviewPanel() {}
	~RenderviewPanel() {}
};

RenderviewAgent::RenderviewAgent(RenderviewPanel &panel) :
	InterfaceAgent(),
	panel_(panel)
{
	setupInputs();
}

void RenderviewAgent::setupInputs()
{
	inputs_ = ValueCollection
	{
		//top settings
		MixMethod,
		DrawInfo,
		DrawCamCtr,
		DrawLegend,
		DrawColormap,
		DrawScaleBar,
		DrawScaleBarText,
		BgColor,
		Aov,
		Perspective,
		Free,
		FullScreen,
		//left settings
		DepthAtten,
		DaInt,
		//right settings
		PinRotCtr,
		ScaleFactor121,
		ScaleFactor,
		ScaleMode,
		//bottom settings
		GearedEnable,
		CamRotX,
		CamRotY,
		CamRotZ,
		CamRotQ,
		CamRotZeroQ
	};
}

void RenderviewAgent::setObject(Renderview* view)
{
	InterfaceAgent::setObject(view);
}

Renderview* RenderviewAgent::getObject()
{
	return dynamic_cast<Renderview*>(InterfaceAgent::getObject());
}

void RenderviewAgent::UpdateFui(const ValueCollection &names)
{
	bool update_all = names.empty();
}

void RenderviewAgent::SaveProject(const std::wstring & dir, const std::wstring &filename)
{
	wxString cap_file = dir + GETSLASH() + filename;
	updateValue(gstCaptureFile, cap_file.ToStdWstring());
	updateValue(gstCapture, true);
	bool bval;
	glbin_root->getValue(gstSaveProjectEnable, bval);
	if (bval)
	{
		wxString new_folder;
		new_folder = cap_file + "_project";
		MkDirW(new_folder.ToStdWstring());
		wxString prop_file = new_folder + GETSLASH() + filename + "_project.vrp";
		glbin_agtf->getRenderFrameAgent()->SaveProject(prop_file.ToStdWstring());
	}
}

bool RenderviewAgent::GetClipHold()
{
	bool bval = false;
	fluo::ClipPlaneAgent* agent = glbin_agtf->getClipPlaneAgent();
	if (agent)
	{
		agent->getValue(gstClipHold, bval);
	}
	return false;
}

//void RenderviewAgent::OnBoundsChanged(Event& event)
//{
//	Renderview* view = getObject();
//	if (!view) return;
//	view->InitView(Renderview::INIT_BOUNDS | Renderview::INIT_CENTER);
//}
//
//void RenderviewAgent::OnSceneChanged(Event& event)
//{
//	Renderview* view = getObject();
//	if (!view) return;
//	view->PopMeshList();
//	view->PopVolumeList();
//}

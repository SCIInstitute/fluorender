/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2020 Scientific Computing and Imaging Institute,
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
#ifndef _SCRIPTPROC_H_
#define _SCRIPTPROC_H_

#include <Tracking/VolCache.h>
#include <Tracking/Cell.h>
#include <wx/string.h>
#include <wx/fileconf.h>

class VRenderFrame;
class VRenderView;
class VRenderGLView;
class VolumeData;
namespace fls
{
	class ScriptProc
	{
	public:
		ScriptProc();
		~ScriptProc();

		void SetFrame(VRenderFrame* frame) { m_frame = frame; }
		void SetVrv(VRenderView* vrv) { m_vrv = vrv; }
		void SetView(VRenderGLView* view) { m_view = view; }

		//run 4d script
		//index: 0-pre-change; 1-post-change
		void Run4DScript(int index, wxString &scriptname);

	private:
		VRenderFrame* m_frame;
		VRenderView* m_vrv;
		VRenderGLView *m_view;

		//file path for script
		wxString m_script_output;
		//selected labels
		CelpList m_sel_labels;

	private:
		void RunNoiseReduction(int index, wxFileConfig &fconfig);
		void RunSelectionTracking(int index, wxFileConfig &fconfig);
		void RunSparseTracking(int index, wxFileConfig &fconfig);
		void RunRandomColors(int index, wxFileConfig &fconfig);
		void RunFetchMask(int index, wxFileConfig &fconfig);
		void RunSaveMask(int index, wxFileConfig &fconfig);
		void RunSaveVolume(int index, wxFileConfig &fconfig);
		void RunCalculate(int index, wxFileConfig &fconfig);
		void RunOpenCL(int index, wxFileConfig &fconfig);
		void RunCompAnalysis(int index, wxFileConfig &fconfig);
		void RunGenerateComp(int index, wxFileConfig &fconfig);
		void RunRulerProfile(int index, wxFileConfig &fconfig);
		void RunAddCells(int index, wxFileConfig &fconfig);
		void RunLinkCells(int index, wxFileConfig &fconfig);
		void RunUnlinkCells(int index, wxFileConfig &fconfig);

		//read/delete volume cache
		//for sparse tracking
		void ReadVolCache(VolCache& vol_cache);
		void DelVolCache(VolCache& vol_cache);
	};
}
#endif//_SCRIPTPROC_H_

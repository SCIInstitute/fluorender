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

#include <Flobject/Group.hpp>
#include <Tracking/VolCache.h>
#include <Tracking/Cell.h>
#include <wx/string.h>
#include <wx/fileconf.h>
#include <vector>

class VRenderFrame;
class VRenderView;
class VRenderGLView;
class VolumeData;
namespace flrd
{
	class ScriptProc
	{
	public:
		enum TimeMask
		{
			TM_NONE = 0,
			TM_PRE = 1 << 0,
			TM_POST = 1 << 1,
			TM_ALL = 0x7FFFFFFF
		};
		enum FrameMask
		{
			FM_NONE = 0,
			FM_FIRST = 1 << 0,
			FM_LAST = 0x40000000,
			FM_ALL = 0x7FFFFFFF
		};
		ScriptProc();
		~ScriptProc();

		void SetFrame(VRenderFrame* frame) { m_frame = frame; }
		void SetVrv(VRenderView* vrv) { m_vrv = vrv; }
		void SetView(VRenderGLView* view) { m_view = view; }

		//run 4d script
		//index: 0-pre-change; 1-post-change
		void Run4DScript(TimeMask tm, wxString &scriptname);

		void ClearResults() { m_output->removeAllChildren(); }

	private:
		VRenderFrame* m_frame;
		VRenderView* m_vrv;
		VRenderGLView *m_view;

		wxString m_type;
		TimeMask m_time_mask;
		wxFileConfig *m_fconfig;

		//selected labels
		CelpList m_sel_labels;

		//output
		fluo::ref_ptr<fluo::Group> m_output;

	private:
		bool TimeCondition();
		bool GetVolumes(std::vector<VolumeData*> &list);
		void UpdateTraceDlg();
		void RunNoiseReduction();
		void RunPreTracking();
		void RunPostTracking();
		void RunMaskTracking();
		void RunRandomColors();
		void RunFetchMask();
		void RunClearMask();
		void RunSaveMask();
		void RunSaveVolume();
		void RunCalculate();
		void RunOpenCL();
		void RunCompAnalysis();
		void RunGenerateComp();
		void RunRulerProfile();
		void RunAddCells();
		void RunLinkCells();
		void RunUnlinkCells();
		void RunBackgroundStat();

		void ExportCompAnalysis();

		//read/delete volume cache
		void ReadVolCache(VolCache& vol_cache);
		void DelVolCache(VolCache& vol_cache);
	};
}
#endif//_SCRIPTPROC_H_

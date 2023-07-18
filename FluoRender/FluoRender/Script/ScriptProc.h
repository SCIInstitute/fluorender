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
			TM_REWIND = 0x8000,
			TM_ALL_PRE = 0x2AAA5555,
			TM_ALL_POST = 0x55552AAA,
			TM_FIRST_PRE = 1,
			TM_FIRST_POST = 2,
			TM_FIRST_BOTH = 3,
			TM_LAST_PRE = 0x20000000,
			TM_LAST_POST = 0x40000000,
			TM_LAST_BOTH = 0x60000000,
			TM_ALL_PRE_FIRST_BOTH = 0x2AAA5557,
			TM_ALL_POST_FIRST_BOTH = 0x55552AAB,
			TM_ALL_PRE_LAST_BOTH = 0x6AAA5555,
			TM_ALL_POST_LAST_BOTH = 0x75552AAA,
			TM_ALL_PRE_REWIND = 0x2AAAD555,
			TM_ALL_POST_REWIND = 0x5555AAAA,
			TM_ALL = 0x7FFF7FFF
		};
		ScriptProc();
		~ScriptProc();

		void SetFrame(VRenderFrame* frame) { m_frame = frame; }
		void SetView(VRenderGLView* view) { m_view = view; }
		void SetBreak(bool bval) { m_break = bval; }
		bool GetBreak() { return m_break; }
		void SetBreakCount(int val = 0) { m_break_count = val; }

		//run 4d script
		//return 0:failure; 1:normal; 2:break
		int Run4DScript(TimeMask tm, wxString &scriptname, bool rewind);

		void ClearResults() { m_output->removeAllChildren(); }

	private:
		VRenderFrame* m_frame;
		VRenderGLView *m_view;

		wxString m_type;
		TimeMask m_time_mask;
		wxFileConfig *m_fconfig;
		wxString m_fconfig_name;
		bool m_rewind;
		bool m_break;
		int m_break_count;

		//selected labels
		CelpList m_sel_labels;

		//output
		//structure: output(group)--time(group)--channel(group)--script_command(group)
		//bacg_stat(group)--single_node(node)--mean(value), etc
		//comp_analysis(group)--comp_id(node)--size(value), pos(value), etc
		fluo::ref_ptr<fluo::Group> m_output;

	private:
		bool TimeCondition();
		bool GetVolumes(std::vector<VolumeData*> &list);
		void UpdateTraceDlg();
		int TimeMode(std::string &str);
		int GetTimeNum();
		wxString GetInputFile(const wxString &str, const wxString &subd);
		wxString GetSavePath(const wxString &str, const wxString &ext, bool rep = true);
		wxString GetDataDir(const wxString &ext);
		wxString GetConfigFile(const wxString& str, const wxString& ext, const wxString& type, int mode);//mode-0 open;1-save
		int GetItems(const wxString& str, std::vector<std::string>& items);
		bool GetRegistrationTransform(fluo::Point& transl, fluo::Point& center, fluo::Point& euler, int sn);//sn: smooth frame number
		
		void RunNoiseReduction();
		void RunPreTracking();
		void RunPostTracking();
		void RunMaskTracking();
		void RunRandomColors();
		void RunCompSelect();
		void RunCompEdit();
		void RunFetchMask();
		void RunClearMask();
		void RunSaveMask();
		void RunSaveVolume();
		void RunCalculate();
		void RunOpenCL();
		void RunCompAnalysis();
		void RunCompRuler();
		void RunGenerateComp();
		void RunRulerProfile();
		void RunRoi();
		void RunRoiDff();
		void RunAddCells();
		void RunLinkCells();
		void RunUnlinkCells();
		void RunBackgroundStat();
		void RunRegistration();
		void RunCameraPoints();
		void RunRulerInfo();
		void RunRulerTransform();
		void RunRulerSpeed();
		void RunGenerateWalk();
		//break
		bool RunBreak();
		//python
		void RunPython();
		void RunDlcVideoAnalyze();
		void RunDlcGetRulers();
		void RunDlcCreateProj();
		void RunDlcLabel();

		void ExportInfo();
		void ExportTemplate();
		void ExportSpreadsheet();

		void ChangeData();
		void ChangeScript();
		void LoadProject();

		//read/delete volume cache
		void ReadVolCacheData(VolCache& vol_cache);
		void ReadVolCacheDataMask(VolCache& vol_cache);
		void ReadVolCacheDataLabel(VolCache& vol_cache);
		void DelVolCacheData(VolCache& vol_cache);
		void DelVolCacheDataLabel(VolCache& vol_cache);
	};
}
#endif//_SCRIPTPROC_H_

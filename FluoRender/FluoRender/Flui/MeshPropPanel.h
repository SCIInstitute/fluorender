/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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
#ifndef _MESHPROPPANEL_H_
#define _MESHPROPPANEL_H_

#include <PropPanel.h>

class MeshData;
class MeshGroup;
class wxSingleSlider;
class wxUndoableColorPicker;
class wxColourPickerEvent;
class wxUndoableToolbar;
class MeshPropPanel: public PropPanel
{
	enum
	{
		//toolbar
		ID_OutlineChk = 0,
		ID_SyncGroupChk,
		ID_LegendChk,
		ID_ResetDefault,
		ID_SaveDefault
	};

public:
	MeshPropPanel(MainFrame* frame,
		wxWindow* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "MeshPropPanel");
	~MeshPropPanel();

	void SetMeshData(MeshData* md);
	MeshData* GetMeshData();
	void SetMeshGroup(MeshGroup* mg);
	MeshGroup* GetMeshGroup();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

	void EnableShadowDir(bool);
	void SetShadowDir(double, bool);

private:
	MeshGroup* m_group;
	MeshData* m_md;
	bool m_sync_group;

	wxUndoableToolbar* m_options_toolbar;

	wxTextCtrl *m_color_text;
	wxUndoableColorPicker* m_color_btn;

	wxCheckBox* m_alpha_chk;
	wxSingleSlider *m_alpha_sldr;
	wxTextCtrl* m_alpha_text;

	wxCheckBox *m_shading_chk;
	wxSingleSlider *m_shading_sldr;
	wxTextCtrl* m_shading_text;
	wxSingleSlider *m_shine_sldr;
	wxTextCtrl* m_shine_text;

	wxCheckBox* m_shadow_chk;
	wxSingleSlider* m_shadow_sldr;
	wxTextCtrl* m_shadow_text;
	wxCheckBox* m_shadow_dir_chk;
	wxSingleSlider* m_shadow_dir_sldr;
	wxTextCtrl* m_shadow_dir_text;

	wxCheckBox* m_scale_chk;
	wxSingleSlider *m_scale_sldr;
	wxTextCtrl* m_scale_text;

private:
	void SetOutline();
	void SetSyncGroup();
	void SetLegend();
	void SaveDefault();
	void ResetDefault();

	void OnOptions(wxCommandEvent& event);

	void OnColorChange(const wxColor& c);
	void OnColorTextChange(wxCommandEvent& event);
	void OnColorTextFocus(wxMouseEvent& event);
	void OnColorBtn(wxColourPickerEvent& event);

	void OnAlphaCheck(wxCommandEvent& event);
	void OnAlphaChange(wxScrollEvent & event);
	void OnAlphaText(wxCommandEvent& event);

	void OnShadingCheck(wxCommandEvent& event);
	void OnShadingChange(wxScrollEvent & event);
	void OnShadingText(wxCommandEvent& event);
	void OnShineChange(wxScrollEvent & event);
	void OnShineText(wxCommandEvent& event);

	void OnShadowCheck(wxCommandEvent& event);
	void OnShadowChange(wxScrollEvent& event);
	void OnShadowText(wxCommandEvent& event);

	void OnShadowDirCheck(wxCommandEvent& event);
	void OnShadowDirChange(wxScrollEvent& event);
	void OnShadowDirText(wxCommandEvent& event);

	void OnScaleCheck(wxCommandEvent& event);
	void OnScaleChange(wxScrollEvent & event);
	void OnScaleText(wxCommandEvent& event);
};

#endif//_MESHPROPPANEL_H_

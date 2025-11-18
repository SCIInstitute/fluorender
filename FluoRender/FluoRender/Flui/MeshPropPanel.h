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

class RenderView;
class MeshData;
class wxSingleSlider;
class wxUndoableColorPicker;
class wxColourPickerEvent;
class MeshPropPanel: public PropPanel
{
public:
	MeshPropPanel(MainFrame* frame,
		wxWindow* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "MeshPropPanel");
	~MeshPropPanel();

	void SetView(RenderView* view);
	void SetMeshData(MeshData* md);
	MeshData* GetMeshData();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

	void EnableShadowDir(bool);
	void SetShadowDir(double, bool);

private:
	RenderView* m_view;
	MeshData* m_md;

	wxTextCtrl *m_color_text;
	wxUndoableColorPicker* m_color_btn;

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

	wxSingleSlider *m_scale_sldr;
	wxTextCtrl* m_scale_text;

private:
	void OnColorChange(const wxColor& c);
	void OnColorTextChange(wxCommandEvent& event);
	void OnColorTextFocus(wxMouseEvent& event);
	void OnColorBtn(wxColourPickerEvent& event);

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

	void OnScaleChange(wxScrollEvent & event);
	void OnScaleText(wxCommandEvent& event);
};

#endif//_MESHPROPPANEL_H_

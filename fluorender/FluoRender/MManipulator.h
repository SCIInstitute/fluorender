/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#include "DataManager.h"
#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/spinbutt.h>
#include "compatibility.h"

#ifndef _MMANIPULATOR_H_
#define _MMANIPULATOR_H_

class MManipulator: public wxPanel
{
	enum
	{
		ID_XTransText = wxID_HIGHEST+301,
		ID_XTransSpin,
		ID_YTransText,
		ID_YTransSpin,
		ID_ZTransText,
		ID_ZTransSpin,
		ID_XRotText,
		ID_XRotSpin,
		ID_YRotText,
		ID_YRotSpin,
		ID_ZRotText,
		ID_ZRotSpin,
		ID_XScalText,
		ID_XScalSpin,
		ID_YScalText,
		ID_YScalSpin,
		ID_ZScalText,
		ID_ZScalSpin
	};

public:
	MManipulator(wxWindow* frame, wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "MManipulator");
	~MManipulator();

	void SetMeshData(MeshData* md);
	MeshData* GetMeshData();
	void RefreshVRenderViews();
	void GetData();
	void UpdateData();

private:
	wxWindow* m_frame;
	MeshData* m_md;

	wxStaticText* m_trans_st;
	wxStaticText* m_x_trans_st;
	wxTextCtrl* m_x_trans_text;
	wxSpinButton* m_x_trans_spin;
	wxStaticText* m_y_trans_st;
	wxTextCtrl* m_y_trans_text;
	wxSpinButton* m_y_trans_spin;
	wxStaticText* m_z_trans_st;
	wxTextCtrl* m_z_trans_text;
	wxSpinButton* m_z_trans_spin;

	wxStaticText* m_rot_st;
	wxStaticText* m_x_rot_st;
	wxTextCtrl* m_x_rot_text;
	wxSpinButton* m_x_rot_spin;
	wxStaticText* m_y_rot_st;
	wxTextCtrl* m_y_rot_text;
	wxSpinButton* m_y_rot_spin;
	wxStaticText* m_z_rot_st;
	wxTextCtrl* m_z_rot_text;
	wxSpinButton* m_z_rot_spin;

	wxStaticText* m_scl_st;
	wxStaticText* m_x_scl_st;
	wxTextCtrl* m_x_scl_text;
	wxSpinButton* m_x_scl_spin;
	wxStaticText* m_y_scl_st;
	wxTextCtrl* m_y_scl_text;
	wxSpinButton* m_y_scl_spin;
	wxStaticText* m_z_scl_st;
	wxTextCtrl* m_z_scl_text;
	wxSpinButton* m_z_scl_spin;

private:
	void OnSpinUp(wxSpinEvent& event);
	void OnSpinDown(wxSpinEvent& event);
	void OnValueEnter(wxCommandEvent& event);

	DECLARE_EVENT_TABLE();
};

#endif//_MMANIPULATOR_H_

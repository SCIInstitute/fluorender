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

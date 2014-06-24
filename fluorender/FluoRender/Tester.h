#include <wx/wx.h>
#include <wx/dialog.h>

#ifndef _TESTER_H_
#define _TESTER_H_

class TesterDlg : public wxDialog
{
	enum
	{
		ID_P1Slider = wxID_HIGHEST+1501,
		ID_P1Text,
		ID_P1Check,
		ID_P2Slider,
		ID_P2Text,
		ID_P2Check,
		ID_P3Slider,
		ID_P3Text,
		ID_P3Check,
		ID_P4Slider,
		ID_P4Text,
		ID_P4Check,
		//all control
		ID_AllCheck,
		//buttons
		ID_B1Btn
	};

public:
	TesterDlg(wxWindow* frame,
		wxWindow* parent);
	~TesterDlg();

	//values
	double m_p1;
	double m_p2;
	double m_p3;
	double m_p4;

	//sliders
	wxSlider* m_p1_sldr;
	wxSlider* m_p2_sldr;
	wxSlider* m_p3_sldr;
	wxSlider* m_p4_sldr;

	//text boxes
	wxTextCtrl* m_p1_text;
	wxTextCtrl* m_p2_text;
	wxTextCtrl* m_p3_text;
	wxTextCtrl* m_p4_text;

	//check boxes
	wxCheckBox *m_p1_chck;
	wxCheckBox *m_p2_chck;
	wxCheckBox *m_p3_chck;
	wxCheckBox *m_p4_chck;

	//all control
	wxCheckBox *m_all_chk;

	//buttons
	wxButton *m_b1_btn;

private:
	wxWindow* m_frame;

	//sliders
	void OnP1Change(wxScrollEvent &event);
	void OnP2Change(wxScrollEvent &event);
	void OnP3Change(wxScrollEvent &event);
	void OnP4Change(wxScrollEvent &event);

	//check boxes
	void OnP1Check(wxCommandEvent& event);
	void OnP2Check(wxCommandEvent& event);
	void OnP3Check(wxCommandEvent& event);
	void OnP4Check(wxCommandEvent& event);

	//all control
	void OnAllCheck(wxCommandEvent& event);

	//buttons
	void OnB1(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif//_TESTER_H_

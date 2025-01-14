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
#ifndef _TESTER_H_
#define _TESTER_H_

#include <PropPanel.h>

class wxSingleSlider;
class TesterDlg : public PropPanel
{
	//enum
	//{
	//	ID_P1Slider = ID_TESTER,
	//	ID_P1Text,
	//	ID_P1Check,
	//	ID_P2Slider,
	//	ID_P2Text,
	//	ID_P2Check,
	//	ID_P3Slider,
	//	ID_P3Text,
	//	ID_P3Check,
	//	ID_P4Slider,
	//	ID_P4Text,
	//	ID_P4Check,
	//	//all control
	//	ID_AllCheck,
	//	//buttons
	//	ID_B1Btn
	//};

public:
	TesterDlg(MainFrame* frame);
	~TesterDlg();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

	//values
	double m_p1;
	double m_p2;
	double m_p3;
	double m_p4;

	//sliders
	wxSingleSlider* m_p1_sldr;
	wxSingleSlider* m_p2_sldr;
	wxSingleSlider* m_p3_sldr;
	wxSingleSlider* m_p4_sldr;

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
	//sliders
	void OnP1Change(wxScrollEvent& event);
	void OnP2Change(wxScrollEvent& event);
	void OnP3Change(wxScrollEvent& event);
	void OnP4Change(wxScrollEvent& event);

	//check boxes
	void OnP1Check(wxCommandEvent& event);
	void OnP2Check(wxCommandEvent& event);
	void OnP3Check(wxCommandEvent& event);
	void OnP4Check(wxCommandEvent& event);

	//all control
	void OnAllCheck(wxCommandEvent& event);

	//buttons
	void OnB1(wxCommandEvent& event);
};

#endif//_TESTER_H_

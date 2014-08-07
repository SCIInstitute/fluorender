#include "ColocalizationDlg.h"
#include "VRenderFrame.h"
#include <wx/valnum.h>

BEGIN_EVENT_TABLE(ColocalizationDlg, wxPanel)
	EVT_BUTTON(ID_CalcLoadABtn, ColocalizationDlg::OnLoadA)
	EVT_BUTTON(ID_CalcLoadBBtn, ColocalizationDlg::OnLoadB)
	EVT_COMMAND_SCROLL(ID_MinSizeSldr, ColocalizationDlg::OnMinSizeChange)
	EVT_TEXT(ID_MinSizeText, ColocalizationDlg::OnMinSizeText)
	EVT_COMMAND_SCROLL(ID_MaxSizeSldr, ColocalizationDlg::OnMaxSizeChange)
	EVT_TEXT(ID_MaxSizeText, ColocalizationDlg::OnMaxSizeText)
	EVT_CHECKBOX(ID_BrushSelectBothChk, ColocalizationDlg::OnSelectBothChk)
	EVT_BUTTON(ID_CalcColocalizationBtn, ColocalizationDlg::OnColocalizationBtn)
END_EVENT_TABLE()

ColocalizationDlg::ColocalizationDlg(wxWindow* frame,
	wxWindow* parent) :
wxPanel(parent, wxID_ANY,
wxPoint(500, 150), wxSize(400, 165),
0, "ColocalizationDlg"),
m_frame(parent),
m_view(0),
m_vol_a(0),
m_vol_b(0)
{
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;

	//operand A
	wxBoxSizer *sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Volume A:",
		wxDefaultPosition, wxSize(75, 20));
	m_calc_load_a_btn = new wxButton(this, ID_CalcLoadABtn, "Load",
		wxDefaultPosition,wxSize(75, 20));
	m_calc_a_text = new wxTextCtrl(this, ID_CalcAText, "",
		wxDefaultPosition, wxSize(200, 20));
	sizer_1->Add(10, 10);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_calc_load_a_btn, 0, wxALIGN_CENTER);
	sizer_1->Add(m_calc_a_text, 1, wxEXPAND|wxALIGN_CENTER);
	sizer_1->Add(10, 10);
	//operand B
	wxBoxSizer *sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Volume B:",
		wxDefaultPosition, wxSize(75, 20));
	m_calc_load_b_btn = new wxButton(this, ID_CalcLoadBBtn, "Load",
		wxDefaultPosition, wxSize(75, 20));
	m_calc_b_text = new wxTextCtrl(this, ID_CalcBText, "",
		wxDefaultPosition, wxSize(75, 20));
	sizer_2->Add(10, 10);
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->Add(m_calc_load_b_btn, 0, wxALIGN_CENTER);
	sizer_2->Add(m_calc_b_text, 1, wxEXPAND|wxALIGN_CENTER);
	sizer_2->Add(10, 10);
	//min size
	wxBoxSizer *sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Min Size:",
		wxDefaultPosition, wxSize(75, 20));
	m_min_size_sldr = new wxSlider(this, ID_MinSizeSldr, 10, 0, 100,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_min_size_text = new wxTextCtrl(this, ID_MinSizeText, "10",
		wxDefaultPosition, wxSize(75, 20), 0, vald_int);
	sizer_3->Add(10, 10);
	sizer_3->Add(st, 0, wxALIGN_CENTER);
	sizer_3->Add(m_min_size_sldr, 1, wxEXPAND|wxALIGN_CENTER);
	sizer_3->Add(m_min_size_text, 0, wxALIGN_CENTER);
	sizer_3->Add(10, 10);
	//max size
	wxBoxSizer *sizer_4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Max Size:",
		wxDefaultPosition, wxSize(75, 20));
	m_max_size_sldr = new wxSlider(this, ID_MaxSizeSldr, 101, 1, 101,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_max_size_text = new wxTextCtrl(this, ID_MaxSizeText, "Ignored",
		wxDefaultPosition, wxSize(75, 20), 0, vald_int);
	sizer_4->Add(10, 10);
	sizer_4->Add(st, 0, wxALIGN_CENTER);
	sizer_4->Add(m_max_size_sldr, 1, wxEXPAND|wxALIGN_CENTER);
	sizer_4->Add(m_max_size_text, 0, wxALIGN_CENTER);
	sizer_4->Add(10, 10);
	//select and calculate
	wxBoxSizer *sizer_5 = new wxBoxSizer(wxHORIZONTAL);
	m_select_both_chk = new wxCheckBox(this, ID_BrushSelectBothChk, "Select Both: ",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	m_colocalization_btn = new wxButton(this, ID_CalcColocalizationBtn, "Colocalization",
		wxDefaultPosition, wxDefaultSize);
	sizer_5->Add(10, 10);
	sizer_5->Add(m_select_both_chk, 0, wxALIGN_CENTER);
	sizer_5->AddStretchSpacer(1);
	sizer_5->Add(m_colocalization_btn, 0, wxALIGN_CENTER);
	sizer_5->Add(10, 10);

	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_1, 0, wxEXPAND|wxALIGN_CENTER);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_2, 0, wxEXPAND|wxALIGN_CENTER);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_3, 0, wxEXPAND|wxALIGN_CENTER);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_4, 0, wxEXPAND|wxALIGN_CENTER);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_5, 0, wxEXPAND|wxALIGN_CENTER);
	
	SetSizer(sizerV);
	Layout();
}

ColocalizationDlg::~ColocalizationDlg()
{
}

void ColocalizationDlg::GetSettings(VRenderView* vrv)
{
	m_view = vrv;

	if (!m_view)
		return;

	bool bval;

	bval = vrv->GetSelectBoth();
	m_select_both_chk->SetValue(bval);

}

void ColocalizationDlg::SetVolumeA(VolumeData* vd)
{
	if (!vd)
		return;

	m_vol_a = vd;
	m_calc_a_text->SetValue(m_vol_a->GetName());
}

void ColocalizationDlg::SetVolumeB(VolumeData* vd)
{
	if (!vd)
		return;

	m_vol_b = vd;
	m_calc_b_text->SetValue(m_vol_b->GetName());

	m_select_both_chk->SetValue(true);
	wxCommandEvent event;
	OnSelectBothChk(event);
}

void ColocalizationDlg::OnLoadA(wxCommandEvent &event)
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		m_vol_a = vr_frame->GetCurSelVol();
		if (m_vol_a)
			m_calc_a_text->SetValue(m_vol_a->GetName());
	}
}

void ColocalizationDlg::OnLoadB(wxCommandEvent &event)
{
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
	{
		m_vol_b = vr_frame->GetCurSelVol();
		if (m_vol_b)
			m_calc_b_text->SetValue(m_vol_b->GetName());

		m_select_both_chk->SetValue(true);
		wxCommandEvent event;
		OnSelectBothChk(event);
	}
}

void ColocalizationDlg::OnMinSizeChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	m_min_size_text->SetValue(str);
}

void ColocalizationDlg::OnMinSizeText(wxCommandEvent &event)
{
	wxString str = m_min_size_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_min_size_sldr->SetValue(ival);
}

void ColocalizationDlg::OnMaxSizeChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival*100);
	if (ival == 101)
		str = "Ignored";
	m_max_size_text->SetValue(str);
}

void ColocalizationDlg::OnMaxSizeText(wxCommandEvent &event)
{
	wxString str = m_max_size_text->GetValue();
	long ival;
	if (str == "Ignored")
		ival = 10100;
	else
		str.ToLong(&ival);
	m_max_size_sldr->SetValue(ival/100);
}

void ColocalizationDlg::OnSelectBothChk(wxCommandEvent &event)
{
	bool select_both = m_select_both_chk->GetValue();

	if (m_view)
	{
		m_view->SetSelectBoth(select_both);
		if (select_both)
		{
			m_view->SetVolumeA(m_vol_a);
			m_view->SetVolumeB(m_vol_b);
		}
	}
}

void ColocalizationDlg::OnColocalizationBtn(wxCommandEvent &event)
{
	if (!m_view || !m_vol_a || !m_vol_b)
		return;

	bool select = true;
	wxString str = m_min_size_text->GetValue();
	long ival;
	str.ToLong(&ival);
	double min_voxels = ival;
	str = m_max_size_text->GetValue();
	if (str == "Ignored")
		ival = -1;
	else
		str.ToLong(&ival);
	double max_voxels = ival;
	double thresh = 0.0;
	double falloff = 1.0;

	//save masks
	m_vol_a->GetVR()->return_mask();
	m_vol_b->GetVR()->return_mask();

	//volume a
	m_view->GetVolumeSelector()->SetVolume(m_vol_a);
	m_view->CompAnalysis(min_voxels, max_voxels, thresh, falloff, select, true, false);

	m_view->m_glview->ForceDraw();

	//volume b
	m_view->GetVolumeSelector()->SetVolume(m_vol_b);
	m_view->CompAnalysis(min_voxels, max_voxels, thresh, falloff, select, true, false);

	m_view->m_glview->ForceDraw();

	m_vol_a->SetDisp(false);
	m_vol_b->SetDisp(false);

	//volume c
	m_view->GetVolumeCalculator()->SetVolumeA(m_vol_a);
	m_view->GetVolumeCalculator()->SetVolumeB(m_vol_b);
	m_view->Calculate(8);
	VolumeData* vd = m_view->GetVolumeCalculator()->GetResult();
	if (vd)
	{
		select = false;
		m_view->GetVolumeSelector()->SetVolume(vd);
		m_view->CompAnalysis(min_voxels, max_voxels, thresh, falloff, select, true, false);
	}

}
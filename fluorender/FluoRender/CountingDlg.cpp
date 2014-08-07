#include "CountingDlg.h"
#include "VRenderFrame.h"
#include <wx/valnum.h>

BEGIN_EVENT_TABLE(CountingDlg, wxPanel)
	//component analyzer
	EVT_COMMAND_SCROLL(ID_CAThreshSldr, CountingDlg::OnCAThreshChange)
	EVT_TEXT(ID_CAThreshText, CountingDlg::OnCAThreshText)
	EVT_CHECKBOX(ID_CAIgnoreMaxChk, CountingDlg::OnCAIgnoreMaxChk)
	EVT_BUTTON(ID_CAAnalyzeBtn, CountingDlg::OnCAAnalyzeBtn)
	EVT_BUTTON(ID_CAMultiChannBtn, CountingDlg::OnCAMultiChannBtn)
	EVT_BUTTON(ID_CARandomColorBtn, CountingDlg::OnCARandomColorBtn)
	EVT_BUTTON(ID_CAAnnotationsBtn, CountingDlg::OnCAAnnotationsBtn)
END_EVENT_TABLE()

CountingDlg::CountingDlg(wxWindow *frame, wxWindow*parent)
: wxPanel(parent, wxID_ANY,
wxPoint(500, 150), wxSize(400, 150),
0, "CountingDlg"),
m_frame(parent),
m_view(0),
m_group(0),
m_vol(0),
m_max_value(255.0),
m_dft_thresh(0.0)
{
	//validator: floating point 1
	wxFloatingPointValidator<double> vald_fp1(1);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;

	//component analyzer
	//threshold of ccl
	wxBoxSizer *sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Threshold:",
		wxDefaultPosition, wxSize(75, 20));
	m_ca_thresh_sldr = new wxSlider(this, ID_CAThreshSldr, 0, 0, 2550,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_ca_thresh_text = new wxTextCtrl(this, ID_CAThreshText, "0.0",
		wxDefaultPosition, wxSize(40, 20), 0, vald_fp1);
	sizer_1->Add(st, 0, wxALIGN_CENTER);
	sizer_1->Add(m_ca_thresh_sldr, 1, wxALIGN_CENTER|wxEXPAND);
	sizer_1->Add(m_ca_thresh_text, 0, wxALIGN_CENTER);
	m_ca_analyze_btn = new wxButton(this, ID_CAAnalyzeBtn, "Analyze",
		wxDefaultPosition, wxSize(-1, 23));
	sizer_1->Add(m_ca_analyze_btn, 0, wxALIGN_CENTER);
	//size of ccl
	wxBoxSizer *sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	m_ca_select_only_chk = new wxCheckBox(this, ID_CASelectOnlyChk, "Selct. Only",
		wxDefaultPosition, wxSize(75, 20));
	sizer_2->Add(m_ca_select_only_chk, 0, wxALIGN_CENTER);
	sizer_2->AddStretchSpacer();
	st = new wxStaticText(this, 0, "Min:",
		wxDefaultPosition, wxSize(40, 15));
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	m_ca_min_text = new wxTextCtrl(this, ID_CAMinText, "0",
		wxDefaultPosition, wxSize(40, 20), 0, vald_int);
	sizer_2->Add(m_ca_min_text, 0, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "vx",
		wxDefaultPosition, wxSize(15, 15));
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->AddStretchSpacer();
	st = new wxStaticText(this, 0, "Max:",
		wxDefaultPosition, wxSize(40, 15));
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	m_ca_max_text = new wxTextCtrl(this, ID_CAMaxText, "1000",
		wxDefaultPosition, wxSize(40, 20), 0, vald_int);
	sizer_2->Add(m_ca_max_text, 0, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "vx",
		wxDefaultPosition, wxSize(15, 15));
	sizer_2->Add(st, 0, wxALIGN_CENTER);
	sizer_2->AddStretchSpacer();
	m_ca_ignore_max_chk = new wxCheckBox(this, ID_CAIgnoreMaxChk, "Ignore Max");
	sizer_2->Add(m_ca_ignore_max_chk, 0, wxALIGN_CENTER);
	//text result
	wxBoxSizer *sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Components:  ");
	sizer_3->Add(st, 0, wxALIGN_CENTER);
	m_ca_comps_text = new wxTextCtrl(this, ID_CACompsText, "0",
		wxDefaultPosition, wxSize(60, 20), wxTE_READONLY);
	sizer_3->Add(m_ca_comps_text, 2, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "Total Volume:  ");
	sizer_3->Add(st, 0, wxALIGN_CENTER);
	m_ca_volume_text = new wxTextCtrl(this, ID_CAVolumeText, "0",
		wxDefaultPosition, wxSize(60, 20), wxTE_READONLY);
	sizer_3->Add(m_ca_volume_text, 2, wxALIGN_CENTER);
	st = new wxStaticText(this, 0, "vx",
		wxDefaultPosition, wxSize(15, 15));
	sizer_3->Add(st, 0, wxALIGN_CENTER);
	sizer_3->Add(5, 5);
	m_ca_vol_unit_text = new wxTextCtrl(this, ID_CAVolUnitText, "0",
		wxDefaultPosition, wxSize(90, 20), wxTE_READONLY);
	sizer_3->Add(m_ca_vol_unit_text, 3, wxALIGN_CENTER);
	//export
	wxBoxSizer *sizer_4 = new wxBoxSizer(wxHORIZONTAL);
	sizer_4->AddStretchSpacer();
	st = new wxStaticText(this, 0, "Export:  ");
	sizer_4->Add(st, 0, wxALIGN_CENTER);
	m_ca_multi_chann_btn = new wxButton(this, ID_CAMultiChannBtn, "Multi-Channels",
		wxDefaultPosition, wxSize(-1, 23));
	m_ca_random_color_btn = new wxButton(this, ID_CARandomColorBtn, "Random Colors",
		wxDefaultPosition, wxSize(-1, 23));
	m_ca_annotations_btn = new wxButton(this, ID_CAAnnotationsBtn, "Show Annotations",
		wxDefaultPosition, wxSize(-1, 23));
	sizer_4->Add(m_ca_multi_chann_btn, 0, wxALIGN_CENTER);
	sizer_4->Add(m_ca_random_color_btn, 0, wxALIGN_CENTER);
	sizer_4->Add(m_ca_annotations_btn, 0, wxALIGN_CENTER);


	//all controls
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_1, 0, wxEXPAND|wxALIGN_CENTER);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_2, 0, wxEXPAND|wxALIGN_CENTER);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_3, 0, wxEXPAND|wxALIGN_CENTER);
	sizerV->Add(10, 10);
	sizerV->Add(sizer_4, 0, wxEXPAND|wxALIGN_CENTER);

	SetSizer(sizerV);
	Layout();

	LoadDefault();
}

CountingDlg::~CountingDlg()
{
}

//load default
void CountingDlg::LoadDefault()
{
#ifdef _DARWIN
    
    wxString dft = wxString(getenv("HOME")) + "/Fluorender.settings/default_brush_settings.dft";
    std::ifstream tmp(dft);
    if (!tmp.good())
        dft = "FluoRender.app/Contents/Resources/default_brush_settings.dft";
    else
        tmp.close();
#else
    wxString dft = "default_brush_settings.dft";
#endif
	wxFileInputStream is(dft);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	wxString str;
	int ival;
	bool bval;

	//component analyzer
	//selected only
	if (fconfig.Read("ca_select_only", &bval))
		m_ca_select_only_chk->SetValue(bval);
	//min voxel
	if (fconfig.Read("ca_min", &ival))
	{
		str = wxString::Format("%d", ival);
		m_ca_min_text->SetValue(str);
	}
	//max voxel
	if (fconfig.Read("ca_max", &ival))
	{
		str = wxString::Format("%d", ival);
		m_ca_max_text->SetValue(str);
	}
	//ignore max
	if (fconfig.Read("ca_ignore_max", &bval))
	{
		m_ca_ignore_max_chk->SetValue(bval);
		if (bval)
			m_ca_max_text->Disable();
		else
			m_ca_max_text->Enable();
	}
	//thresh
	if (fconfig.Read("ca_thresh", &m_dft_thresh))
	{
		str = wxString::Format("%.1f", m_dft_thresh*m_max_value);
		m_ca_thresh_sldr->SetRange(0, int(m_max_value*10.0));
		m_ca_thresh_text->SetValue(str);
	}
}

void CountingDlg::GetSettings(VRenderView* vrv)
{
	if (!vrv)
		return;

	VolumeData* sel_vol = 0;
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
		sel_vol = vr_frame->GetCurSelVol();

	m_view = vrv;

	//threshold range
	if (sel_vol)
	{
		m_max_value = sel_vol->GetMaxValue();
		//threshold
		m_ca_thresh_sldr->SetRange(0, int(m_max_value*10.0));
		m_ca_thresh_sldr->SetValue(int(m_dft_thresh*m_max_value*10.0+0.5));
		m_ca_thresh_text->ChangeValue(wxString::Format("%.1f", m_dft_thresh*m_max_value));
	}
}

//component analyze
void CountingDlg::OnCAThreshChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%.1f", double(ival)/10.0);
	m_ca_thresh_text->SetValue(str);
}

void CountingDlg::OnCAThreshText(wxCommandEvent &event)
{
	wxString str = m_ca_thresh_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_dft_thresh = val/m_max_value;
	m_ca_thresh_sldr->SetValue(int(val*10.0+0.5));

	//change mask threshold
	VolumeData* sel_vol = 0;
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
		sel_vol = vr_frame->GetCurSelVol();
	if (sel_vol)
		sel_vol->SetMaskThreshold(m_dft_thresh);
	vr_frame->RefreshVRenderViews();
}

void CountingDlg::OnCAAnalyzeBtn(wxCommandEvent &event)
{
	if (m_view)
	{
		bool select = m_ca_select_only_chk->GetValue();
		double min_voxels, max_voxels;
		wxString str = m_ca_min_text->GetValue();
		str.ToDouble(&min_voxels);
		str = m_ca_max_text->GetValue();
		str.ToDouble(&max_voxels);
		bool ignore_max = m_ca_ignore_max_chk->GetValue();

		int comps = m_view->CompAnalysis(min_voxels, ignore_max?-1.0:max_voxels, m_dft_thresh, 1.0, select, true, false);
		int volume = m_view->GetVolumeSelector()->GetVolumeNum();
		//change mask threshold
		VolumeData* sel_vol = 0;
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
			sel_vol = vr_frame->GetCurSelVol();
		if (sel_vol)
			sel_vol->SetUseMaskThreshold(true);
		m_ca_comps_text->SetValue(wxString::Format("%d", comps));
		m_ca_volume_text->SetValue(wxString::Format("%d", volume));
		if (sel_vol)
		{
			double spcx, spcy, spcz;
			sel_vol->GetSpacings(spcx, spcy, spcz);
			double vol_unit = volume*spcx*spcy*spcz;
			wxString vol_unit_text;
			vol_unit_text = wxString::Format("%.0f", vol_unit);
			vol_unit_text += " ";
			switch (m_view->m_glview->m_sb_unit)
			{
			case 0:
				vol_unit_text += L"nm\u00B3";
				break;
			case 1:
			default:
				vol_unit_text += L"\u03BCm\u00B3";
				break;
			case 2:
				vol_unit_text += L"mm\u00B3";
				break;
			}

			m_ca_vol_unit_text->SetValue(vol_unit_text);
		}
		if (vr_frame)
			vr_frame->RefreshVRenderViews();
	}
}

void CountingDlg::OnCAIgnoreMaxChk(wxCommandEvent &event)
{
	if (m_ca_ignore_max_chk->GetValue())
		m_ca_max_text->Disable();
	else
		m_ca_max_text->Enable();
}

void CountingDlg::OnCAMultiChannBtn(wxCommandEvent &event)
{
	if (m_view)
	{
		bool select = m_ca_select_only_chk->GetValue();
		m_view->CompExport(0, select);
	}
}

void CountingDlg::OnCARandomColorBtn(wxCommandEvent &event)
{
	if (m_view)
	{
		bool select = m_ca_select_only_chk->GetValue();
		m_view->CompExport(1, select);
	}
}

void CountingDlg::OnCAAnnotationsBtn(wxCommandEvent &event)
{
	if (m_view)
		m_view->ShowAnnotations();
}


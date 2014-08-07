#include "NoiseCancellingDlg.h"
#include "VRenderFrame.h"
#include <wx/valnum.h>

BEGIN_EVENT_TABLE(NoiseCancellingDlg, wxPanel)
	EVT_COMMAND_SCROLL(ID_ThresholdSldr, NoiseCancellingDlg::OnThresholdChange)
	EVT_TEXT(ID_ThresholdText, NoiseCancellingDlg::OnThresholdText)
	EVT_COMMAND_SCROLL(ID_VoxelSldr, NoiseCancellingDlg::OnVoxelChange)
	EVT_TEXT(ID_VoxelText, NoiseCancellingDlg::OnVoxelText)
	EVT_BUTTON(ID_PreviewBtn, NoiseCancellingDlg::OnPreviewBtn)
	EVT_BUTTON(ID_EraseBtn, NoiseCancellingDlg::OnEraseBtn)
	EVT_CHECKBOX(ID_EnhanceSelChk, NoiseCancellingDlg::OnEnhanceSelChk)
END_EVENT_TABLE()

NoiseCancellingDlg::NoiseCancellingDlg(wxWindow *frame, wxWindow *parent)
: wxPanel(parent, wxID_ANY,
			wxPoint(500, 150), wxSize(400, 150),
			0, "NoiseCancellingDlg"),
			m_frame(parent),
			m_view(0),
			m_group(0),
			m_vol(0),
			m_max_value(255.0),
			m_dft_thresh(0.0),
			m_previewed(false)
{
	//validator: floating point 1
	wxFloatingPointValidator<double> vald_fp1(1);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;

	//group1
	wxBoxSizer *sizer1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Threshold:",
		wxDefaultPosition, wxSize(75, 23));
	m_threshold_sldr = new wxSlider(this, ID_ThresholdSldr, 0, 0, 2550,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_threshold_text = new wxTextCtrl(this, ID_ThresholdText, "0.0",
		wxDefaultPosition, wxSize(40, 20), 0, vald_fp1);
	m_preview_btn = new wxButton(this, ID_PreviewBtn, "Preview",
		wxDefaultPosition, wxSize(60, 23));
	sizer1->Add(st, 0, wxALIGN_CENTER);
	sizer1->Add(m_threshold_sldr, 1, wxEXPAND|wxALIGN_CENTER);
	sizer1->Add(m_threshold_text, 0, wxALIGN_CENTER);
	sizer1->Add(5, 5);
	sizer1->Add(m_preview_btn, 0, wxALIGN_CENTER);

	//group2
	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Voxel Size:",
		wxDefaultPosition, wxSize(75, 23));
	m_voxel_sldr = new wxSlider(this, ID_VoxelSldr, 1, 1, 500,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_voxel_text = new wxTextCtrl(this, ID_VoxelText, "1",
		wxDefaultPosition, wxSize(40, 20), 0, vald_int);
	m_erase_btn = new wxButton(this, ID_EraseBtn, "Erase",
		wxDefaultPosition, wxSize(60, 23));
	sizer2->Add(st, 0, wxALIGN_CENTER);
	sizer2->Add(m_voxel_sldr, 1, wxEXPAND|wxALIGN_CENTER);
	sizer2->Add(m_voxel_text, 0, wxALIGN_CENTER);
	sizer2->Add(5, 5);
	sizer2->Add(m_erase_btn, 0, wxALIGN_CENTER);

	//group3
	wxBoxSizer *sizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_enhance_sel_chk = new wxCheckBox(this, ID_EnhanceSelChk, "Enhance selection",
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	sizer3->Add(m_enhance_sel_chk, 0, wxALIGN_CENTER);

	//group4
	wxBoxSizer *sizer4 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0,
		"Check this option if selections are too dim. It is equivalent to\n"\
		"adjusting the Equalization values in the Output Adjustment panel.",
		wxDefaultPosition, wxDefaultSize);
	sizer4->Add(st, 0, wxALIGN_CENTER);

	//all controls
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(sizer1, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer2, 0, wxEXPAND);
	sizerV->Add(10, 10);
	sizerV->Add(sizer3, 0, wxEXPAND);
	sizerV->Add(sizer4, 0, wxEXPAND);

	SetSizer(sizerV);
	Layout();
}

NoiseCancellingDlg::~NoiseCancellingDlg()
{
}

void NoiseCancellingDlg::GetSettings(VRenderView* vrv)
{
	if (!vrv)
		return;

	VolumeData* sel_vol = 0;
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
		sel_vol = vr_frame->GetCurSelVol();
	else
		return;

	m_view = vrv;

	if (vr_frame->GetBrushToolDlg())
	{
		m_dft_thresh = vr_frame->GetBrushToolDlg()->GetDftNRThresh();
		m_dft_size = vr_frame->GetBrushToolDlg()->GetDftNRSize();
	}

	//threshold range
	if (sel_vol)
	{
		m_max_value = sel_vol->GetMaxValue();
		//threshold
		m_threshold_sldr->SetRange(0, int(m_max_value*10.0));
		m_threshold_sldr->SetValue(int(m_dft_thresh*m_max_value*10.0+0.5));
		m_threshold_text->ChangeValue(wxString::Format("%.1f", m_dft_thresh*m_max_value));
		//voxel
		int nx, ny, nz;
		sel_vol->GetResolution(nx, ny, nz);
		m_voxel_sldr->SetRange(1, nx);
		m_voxel_sldr->SetValue(int(m_dft_size));
		m_voxel_text->ChangeValue(wxString::Format("%d", int(m_dft_size)));
		m_previewed = false;
	}
}

//threshold
void NoiseCancellingDlg::OnThresholdChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%.1f", double (ival)/10.0);
	m_threshold_text->SetValue(str);
}

void NoiseCancellingDlg::OnThresholdText(wxCommandEvent &event)
{
	wxString str = m_threshold_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_dft_thresh = val/m_max_value;
	m_threshold_sldr->SetValue(int(val*10.0+0.5));

	//change mask threshold
	VolumeData* sel_vol = 0;
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
		sel_vol = vr_frame->GetCurSelVol();
	if (sel_vol)
		sel_vol->SetMaskThreshold(m_dft_thresh);
	vr_frame->RefreshVRenderViews();
}

//voxel size
void NoiseCancellingDlg::OnVoxelChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	m_voxel_text->SetValue(str);
}

void NoiseCancellingDlg::OnVoxelText(wxCommandEvent &event)
{
	wxString str = m_voxel_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_dft_size = ival;
	m_voxel_sldr->SetValue(ival);
}

void NoiseCancellingDlg::OnPreviewBtn(wxCommandEvent &event)
{
	if (m_view)
	{
		//change mask threshold
		VolumeData* sel_vol = 0;
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
		if (vr_frame)
			sel_vol = vr_frame->GetCurSelVol();
		if (sel_vol)
			sel_vol->SetUseMaskThreshold(true);
		if (vr_frame)
		{
			if (vr_frame->GetBrushToolDlg())
			{
				vr_frame->GetBrushToolDlg()->SetDftNRThresh(m_dft_thresh);
				vr_frame->GetBrushToolDlg()->SetDftNRSize(m_dft_size);
			}
			vr_frame->RefreshVRenderViews();
		}
		m_previewed = true;
		OnEnhanceSelChk(event);
	}
}

void NoiseCancellingDlg::OnEraseBtn(wxCommandEvent &event)
{
	VRenderFrame* frame = (VRenderFrame*)m_frame;
	if (frame && frame->GetTree())
		frame->GetTree()->BrushErase();
}

void NoiseCancellingDlg::OnEnhanceSelChk(wxCommandEvent &event)
{
	if (!m_previewed)
		return;

	bool enhance = m_enhance_sel_chk->GetValue();

	VolumeData* sel_vol = 0;
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (vr_frame)
		sel_vol = vr_frame->GetCurSelVol();
	if (enhance && sel_vol)
	{
		Color mask_color = sel_vol->GetMaskColor();
		double hdr_r = 0.0;
		double hdr_g = 0.0;
		double hdr_b = 0.0;
		if (mask_color.r() > 0.0)
			hdr_r = 0.4;
		if (mask_color.g() > 0.0)
			hdr_g = 0.4;
		if (mask_color.b() > 0.0)
			hdr_b = 0.4;
		Color hdr_color = Color(hdr_r, hdr_g, hdr_b);
		m_hdr = sel_vol->GetHdr();
		sel_vol->SetHdr(hdr_color);
	}
	else if (!enhance && sel_vol)
	{
		sel_vol->SetHdr(m_hdr);
	}
	if (vr_frame)
		vr_frame->RefreshVRenderViews();
}

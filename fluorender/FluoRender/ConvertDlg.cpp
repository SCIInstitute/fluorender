#include "ConvertDlg.h"
#include "VRenderFrame.h"
#include "DataManager.h"
#include <wx/valnum.h>
#include "Converters/VolumeMeshConv.h"

BEGIN_EVENT_TABLE(ConvertDlg, wxPanel)
	//convert from volume to mesh
	EVT_COMMAND_SCROLL(ID_CnvVolMeshThreshSldr, ConvertDlg::OnCnvVolMeshThreshChange)
	EVT_TEXT(ID_CnvVolMeshThreshText, ConvertDlg::OnCnvVolMeshThreshText)
	EVT_COMMAND_SCROLL(ID_CnvVolMeshDownsampleSldr, ConvertDlg::OnCnvVolMeshDownsampleChange)
	EVT_TEXT(ID_CnvVolMeshDownsampleText, ConvertDlg::OnCnvVolMeshDownsampleText)
	EVT_COMMAND_SCROLL(ID_CnvVolMeshDownsampleZSldr, ConvertDlg::OnCnvVolMeshDownsampleZChange)
	EVT_TEXT(ID_CnvVolMeshDownsampleZText, ConvertDlg::OnCnvVolMeshDownsampleZText)
	EVT_BUTTON(ID_CnvVolMeshConvertBtn, ConvertDlg::OnCnvVolMeshConvert)
END_EVENT_TABLE()

ConvertDlg::ConvertDlg(wxWindow *frame, wxWindow *parent) :
wxPanel(parent, wxID_ANY,
	wxPoint(520, 170), wxSize(400, 200),
	0, "ConvertDlg"),
	m_frame(parent)
{
	//validator: floating point 2
	wxFloatingPointValidator<double> vald_fp2(2);
	//validator: integer
	wxIntegerValidator<unsigned int> vald_int;

	wxStaticText *st = 0;

	//group1
	//convert from volume to mesh
	wxBoxSizer *group1 = new wxStaticBoxSizer(
		new wxStaticBox(this, wxID_ANY, "Volume to Polygon Mesh"),
		wxVERTICAL);
	//threshold slider and text
	wxBoxSizer *sizer11 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Threshold:",
		wxDefaultPosition, wxSize(75, 23));
	m_cnv_vol_mesh_thresh_sldr = new wxSlider(this, ID_CnvVolMeshThreshSldr, 30, 1, 99,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cnv_vol_mesh_thresh_text = new wxTextCtrl(this, ID_CnvVolMeshThreshText, "0.30",
		wxDefaultPosition, wxSize(40, 23), 0, vald_fp2);
	sizer11->Add(st, 0, wxALIGN_CENTER);
	sizer11->Add(10, 10);
	sizer11->Add(m_cnv_vol_mesh_thresh_sldr, 1, wxEXPAND|wxALIGN_CENTER);
	sizer11->Add(m_cnv_vol_mesh_thresh_text, 0, wxALIGN_CENTER);
	sizer11->Add(15, 15);
	//downsampling slider and text
	wxBoxSizer *sizer12 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Downsmp. XY:",
		wxDefaultPosition, wxSize(75, 23));
	m_cnv_vol_mesh_downsample_sldr = new wxSlider(this, ID_CnvVolMeshDownsampleSldr, 2, 1, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cnv_vol_mesh_downsample_text = new wxTextCtrl(this, ID_CnvVolMeshDownsampleText, "2",
		wxDefaultPosition, wxSize(40, 23), 0, vald_int);
	sizer12->Add(st, 0, wxALIGN_CENTER);
	sizer12->Add(10, 10);
	sizer12->Add(m_cnv_vol_mesh_downsample_sldr, 1, wxEXPAND|wxALIGN_CENTER);
	sizer12->Add(m_cnv_vol_mesh_downsample_text, 0, wxALIGN_CENTER);
	sizer12->Add(15, 15);
	//downsampling in z slider and text
	wxBoxSizer *sizer13 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Downsmp. Z:",
		wxDefaultPosition, wxSize(75, 23));
	m_cnv_vol_mesh_downsample_z_sldr = new wxSlider(this, ID_CnvVolMeshDownsampleZSldr, 1, 1, 10,
		wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_cnv_vol_mesh_downsample_z_text = new wxTextCtrl(this, ID_CnvVolMeshDownsampleZText, "1",
		wxDefaultPosition, wxSize(40, 23), 0, vald_int);
	sizer13->Add(st, 0, wxALIGN_CENTER);
	sizer13->Add(10, 10);
	sizer13->Add(m_cnv_vol_mesh_downsample_z_sldr, 1, wxEXPAND|wxALIGN_CENTER);
	sizer13->Add(m_cnv_vol_mesh_downsample_z_text, 0, wxALIGN_CENTER);
	sizer13->Add(15, 15);
	//check options and convert button
	wxBoxSizer *sizer14 = new wxBoxSizer(wxHORIZONTAL);
	m_cnv_vol_mesh_usetransf_chk = new wxCheckBox(this, ID_CnvVolMeshUsetransfChk, "Use transfer function",
		wxDefaultPosition, wxSize(-1, 23));
	m_cnv_vol_mesh_selected_chk = new wxCheckBox(this, ID_CnvVolMeshSelectedChk, "Selected Only",
		wxDefaultPosition, wxSize(-1, 23));
	m_cnv_vol_mesh_selected_chk->SetValue(true);
	m_cnv_vol_mesh_weld_chk = new wxCheckBox(this, ID_CnvVolMeshWeldChk, "Weld vertices",
		wxDefaultPosition, wxSize(-1, 23));
	m_cnv_vol_mesh_weld_chk->SetValue(true);
	sizer14->Add(m_cnv_vol_mesh_usetransf_chk, 0, wxALIGN_CENTER);
	sizer14->Add(m_cnv_vol_mesh_selected_chk, 0, wxALIGN_CENTER);
	sizer14->Add(m_cnv_vol_mesh_weld_chk, 0, wxALIGN_CENTER);
	//button
	wxBoxSizer *sizer15 = new wxBoxSizer(wxHORIZONTAL);
	m_cnv_vol_mesh_convert_btn = new wxButton(this, ID_CnvVolMeshConvertBtn, "Convert",
		wxDefaultPosition, wxSize(-1, 23));
	sizer15->AddStretchSpacer();
	sizer15->Add(m_cnv_vol_mesh_convert_btn, 0, wxALIGN_CENTER);
	
	//group1
	group1->Add(5, 5);
	group1->Add(sizer11, 0, wxEXPAND|wxALIGN_CENTER);
	group1->Add(5, 5);
	group1->Add(sizer12, 0, wxEXPAND|wxALIGN_CENTER);
	group1->Add(5, 5);
	group1->Add(sizer13, 0, wxEXPAND|wxALIGN_CENTER);
	group1->Add(5, 5);
	group1->Add(sizer14, 0, wxEXPAND|wxALIGN_CENTER);
	group1->Add(5, 5);
	group1->Add(sizer15, 0, wxEXPAND|wxALIGN_CENTER);
	group1->Add(5, 5);

	//all controls
	wxBoxSizer *sizerV = new wxBoxSizer(wxVERTICAL);
	sizerV->Add(10, 10);
	sizerV->Add(group1, 0, wxEXPAND);

	SetSizer(sizerV);
	Layout();
}

ConvertDlg::~ConvertDlg()
{
}

//threshold
void ConvertDlg::OnCnvVolMeshThreshChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	double val = double(ival)/100.0;
	wxString str = wxString::Format("%.2f", val);
	m_cnv_vol_mesh_thresh_text->SetValue(str);
}

void ConvertDlg::OnCnvVolMeshThreshText(wxCommandEvent &event)
{
	wxString str = m_cnv_vol_mesh_thresh_text->GetValue();
	double val;
	str.ToDouble(&val);
	m_cnv_vol_mesh_thresh_sldr->SetValue(int(val*100.0+0.5));
}

//downsampling
void ConvertDlg::OnCnvVolMeshDownsampleChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	m_cnv_vol_mesh_downsample_text->SetValue(str);
}

void ConvertDlg::OnCnvVolMeshDownsampleText(wxCommandEvent &event)
{
	wxString str = m_cnv_vol_mesh_downsample_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_cnv_vol_mesh_downsample_sldr->SetValue(ival);
}

//downsampling Z
void ConvertDlg::OnCnvVolMeshDownsampleZChange(wxScrollEvent &event)
{
	int ival = event.GetPosition();
	wxString str = wxString::Format("%d", ival);
	m_cnv_vol_mesh_downsample_z_text->SetValue(str);
}

void ConvertDlg::OnCnvVolMeshDownsampleZText(wxCommandEvent &event)
{
	wxString str = m_cnv_vol_mesh_downsample_z_text->GetValue();
	long ival;
	str.ToLong(&ival);
	m_cnv_vol_mesh_downsample_z_sldr->SetValue(ival);
}

void ConvertDlg::OnCnvVolMeshConvert(wxCommandEvent& event)
{
	VolumeData* sel_vol = 0;
	VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
	if (!vr_frame)
		return;

	sel_vol = vr_frame->GetCurSelVol();

	if (!sel_vol)
		return;

	wxProgressDialog *prog_diag = new wxProgressDialog(
		"FluoRender: Convert volume to polygon data",
		"Converting... Please wait.",
		100, 0,
		wxPD_SMOOTH|wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE);
	int progress = 0;

	progress = 50;
	prog_diag->Update(progress);

	VolumeMeshConv converter;
	converter.SetVolume(sel_vol->GetTexture()->get_nrrd(0));
	double x, y, z;
	sel_vol->GetSpacings(x, y, z);
	converter.SetVolumeSpacings(x, y, z);
	converter.SetMaxValue(sel_vol->GetMaxValue());
	wxString str;
	//get iso value
	str = m_cnv_vol_mesh_thresh_text->GetValue();
	double iso_value;
	str.ToDouble(&iso_value);
	converter.SetIsoValue(iso_value);
	//get downsampling
	str = m_cnv_vol_mesh_downsample_text->GetValue();
	long downsample;
	str.ToLong(&downsample);
	converter.SetDownsample(downsample);
	//get downsampling Z
	str = m_cnv_vol_mesh_downsample_z_text->GetValue();
	str.ToLong(&downsample);
	converter.SetDownsampleZ(downsample);
	//get use transfer function
	if (m_cnv_vol_mesh_usetransf_chk->GetValue())
	{
		double gamma, lo_thresh, hi_thresh, offset, gm_thresh;
		gamma = sel_vol->Get3DGamma();
		lo_thresh = sel_vol->GetLeftThresh();
		hi_thresh = sel_vol->GetRightThresh();
		offset = sel_vol->GetOffset();
		gm_thresh = sel_vol->GetBoundary();
		converter.SetVolumeTransfer(gamma, lo_thresh, hi_thresh, offset, gm_thresh);
		converter.SetVolumeUseTrans(true);
	}
	else
		converter.SetVolumeUseTrans(false);
	//get use selection
	if (m_cnv_vol_mesh_selected_chk->GetValue())
	{
		sel_vol->GetVR()->return_mask();
		converter.SetVolumeMask(sel_vol->GetTexture()->get_nrrd(sel_vol->GetTexture()->nmask()));
		converter.SetVolumeUseMask(true);
	}
	else
		converter.SetVolumeUseMask(false);
	//start converting
	converter.Convert();
	GLMmodel* mesh = converter.GetMesh();

	progress = 90;
	prog_diag->Update(progress);

	if (mesh)
	{
		if (m_cnv_vol_mesh_weld_chk->GetValue())
			glmWeld(mesh, Min(x, Min(y, z)*0.001));
		DataManager* mgr = vr_frame->GetDataManager();
		mgr->LoadMeshData(mesh);
		MeshData* md = mgr->GetLastMeshData();
		if (md && vr_frame->GetView(0))
		{
			vr_frame->GetView(0)->AddMeshData(md);
			vr_frame->GetView(0)->RefreshGL();
		}
		vr_frame->UpdateList();
		vr_frame->UpdateTree();
	}

	delete prog_diag;

}

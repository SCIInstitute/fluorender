#include "APropView.h"
#include "VRenderFrame.h"
#include <wx/valnum.h>

BEGIN_EVENT_TABLE(APropView, wxPanel)
	EVT_BUTTON(ID_MemoUpdateBtn, APropView::OnMemoUpdateBtn)
END_EVENT_TABLE()

APropView::APropView(wxWindow* frame, wxWindow* parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
wxPanel(parent, id, pos, size, style, name),
m_frame(frame),
m_ann(0),
m_vrv(0)
{
	wxBoxSizer* sizer_v1 = new wxBoxSizer(wxVERTICAL);
	wxStaticText* st = 0;

	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Memo:",
		wxDefaultPosition, wxDefaultSize);
	sizer_1->Add(10, 10);
	sizer_1->Add(st);

	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	m_memo_text = new wxTextCtrl(this, ID_MemoText, "",
		wxDefaultPosition, wxSize(400, 100), wxTE_MULTILINE);
	sizer_2->Add(10, 10);
	sizer_2->Add(m_memo_text, 1, wxEXPAND);

	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	m_memo_update_btn = new wxButton(this, ID_MemoUpdateBtn, "Update");
	sizer_3->Add(330, 10);
	sizer_3->Add(m_memo_update_btn, 0, wxALIGN_CENTER);

	sizer_v1->Add(sizer_1, 0, wxALIGN_LEFT);
	sizer_v1->Add(sizer_2, 1, wxALIGN_LEFT);
	sizer_v1->Add(sizer_3, 0, wxALIGN_LEFT);

	SetSizer(sizer_v1);
	Layout();
}

APropView::~APropView()
{
}

void APropView::GetSettings()
{
	if (!m_ann)
		return;

	wxString memo = m_ann->GetMemo();
	m_memo_text->SetValue(memo);
	if (m_ann->GetMemoRO())
	{
		m_memo_text->SetEditable(false);
		m_memo_update_btn->Disable();
	}
	else
	{
		m_memo_text->SetEditable(true);
		m_memo_update_btn->Enable();
	}
}

void APropView::SetAnnotations(Annotations* ann, VRenderView* vrv)
{
	m_ann = ann;
	m_vrv = vrv;

	GetSettings();
}

Annotations* APropView::GetAnnotations()
{
	return m_ann;
}

void APropView::RefreshVRenderViews(bool tree)
{
	VRenderFrame* vrender_frame = (VRenderFrame*)m_frame;
	if (vrender_frame)
		vrender_frame->RefreshVRenderViews(tree);
}

void APropView::OnMemoUpdateBtn(wxCommandEvent& event)
{
	if (m_ann)
	{
		wxString memo = m_memo_text->GetValue();
                std::string str = memo.ToStdString();
		m_ann->SetMemo(str);
	}
}

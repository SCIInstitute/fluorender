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
#include <AnnotatPropPanel.h>
#include <MainFrame.h>
#include <AnnotData.h>
#include <wx/valnum.h>

AnnotatPropPanel::AnnotatPropPanel(MainFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	PropPanel(frame, parent, pos, size, style, name),
	m_ann(0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);
	SetDoubleBuffered(true);

	wxBoxSizer* sizer_v1 = new wxBoxSizer(wxVERTICAL);
	wxStaticText* st = 0;

	wxBoxSizer* sizer_1 = new wxBoxSizer(wxHORIZONTAL);
	st = new wxStaticText(this, 0, "Memo:",
		wxDefaultPosition, wxDefaultSize);
	sizer_1->Add(10, 10);
	sizer_1->Add(st);

	wxBoxSizer* sizer_2 = new wxBoxSizer(wxHORIZONTAL);
	m_memo_text = new wxTextCtrl(this, wxID_ANY, "",
		wxDefaultPosition, FromDIP(wxSize(400, 100)), wxTE_MULTILINE);
	sizer_2->Add(10, 10);
	sizer_2->Add(m_memo_text, 1, wxEXPAND);

	wxBoxSizer* sizer_3 = new wxBoxSizer(wxHORIZONTAL);
	m_memo_update_btn = new wxButton(this, wxID_ANY, "Update");
	m_memo_update_btn->Bind(wxEVT_BUTTON, &AnnotatPropPanel::OnMemoUpdateBtn, this);
	sizer_3->Add(330, 10);
	sizer_3->Add(m_memo_update_btn, 0, wxALIGN_CENTER);

	sizer_v1->Add(sizer_1, 0, wxALIGN_LEFT);
	sizer_v1->Add(sizer_2, 1, wxALIGN_LEFT);
	sizer_v1->Add(sizer_3, 0, wxALIGN_LEFT);

	SetSizer(sizer_v1);
	Layout();
	SetAutoLayout(true);
	SetScrollRate(10, 10);
}

AnnotatPropPanel::~AnnotatPropPanel()
{
}

void AnnotatPropPanel::FluoUpdate(const fluo::ValueCollection& values)
{
	if (!m_ann)
		return;

	wxString memo = m_ann->GetMemo();
	m_memo_text->ChangeValue(memo);
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

void AnnotatPropPanel::SetAnnotData(AnnotData* ann)
{
	m_ann = ann;

	FluoUpdate();
}

AnnotData* AnnotatPropPanel::GetAnnotData()
{
	return m_ann;
}

void AnnotatPropPanel::OnMemoUpdateBtn(wxCommandEvent& event)
{
	if (m_ann)
	{
		wxString memo = m_memo_text->GetValue();
		std::wstring str = memo.ToStdWstring();
		m_ann->SetMemo(str);
	}
}

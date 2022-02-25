/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#include <AnnotationPropPanel.h>
#include <RenderFrame.h>
#include <Global.hpp>
#include <AgentFactory.hpp>
#include <Annotations.hpp>
#include <wx/valnum.h>

BEGIN_EVENT_TABLE(AnnotationPropPanel, wxPanel)
	EVT_BUTTON(ID_MemoUpdateBtn, AnnotationPropPanel::OnMemoUpdateBtn)
END_EVENT_TABLE()

AnnotationPropPanel::AnnotationPropPanel(RenderFrame* frame,
	wxWindow* parent,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	wxPanel(parent, wxID_ANY, pos, size, style, name),
	m_frame(frame),
	m_ann(0)
{
	// temporarily block events during constructor:
	wxEventBlocker blocker(this);

	m_agent = glbin_agtf->getOrAddAnnotationPropAgent(gstAnnotationPropAgent, *this);

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

AnnotationPropPanel::~AnnotationPropPanel()
{
}

void AnnotationPropPanel::AssociateAnnotations(fluo::Annotations* ann)
{
	m_agent->setObject(ann);
}

void AnnotationPropPanel::GetSettings()
{
	if (!m_ann)
		return;

	std::string memo;
	m_ann->getValue(gstMemo, memo);
	m_memo_text->SetValue(memo);
	bool bval;
	m_ann->getValue(gstMemoRo, bval);
	if (bval)
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

void AnnotationPropPanel::SetAnnotations(fluo::Annotations* ann)
{
	m_ann = ann;

	GetSettings();
}

fluo::Annotations* AnnotationPropPanel::GetAnnotations()
{
	return m_ann;
}

void AnnotationPropPanel::RefreshVRenderViews(bool tree)
{
	if (m_frame)
		m_frame->RefreshVRenderViews(tree);
}

void AnnotationPropPanel::OnMemoUpdateBtn(wxCommandEvent& event)
{
	if (m_ann)
	{
		wxString memo = m_memo_text->GetValue();
		std::string str = memo.ToStdString();
		m_ann->updValue(gstMemo, str);
	}
}

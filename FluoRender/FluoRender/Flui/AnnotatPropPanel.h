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
#ifndef _ANNOTATPROPPANEL_H_
#define _ANNOTATPROPPANEL_H_

#include <PropPanel.h>

class AnnotData;
class AnnotatPropPanel : public PropPanel
{
public:
	AnnotatPropPanel(MainFrame* frame,
		wxWindow* parent,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "AnnotatPropPanel");
	~AnnotatPropPanel();

	void SetAnnotData(AnnotData* ann);
	AnnotData* GetAnnotData();

	virtual void FluoUpdate(const fluo::ValueCollection& vc = {});

private:
	AnnotData* m_ann;

	wxTextCtrl* m_memo_text;
	wxButton* m_memo_update_btn;

	//memo
	void OnMemoUpdateBtn(wxCommandEvent& event);
};

#endif//_ANNOTATPROPPANEL_H_

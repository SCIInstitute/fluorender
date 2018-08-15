/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#ifndef _APROPVIEW_H_
#define _APROPVIEW_H_

#include "DataManager.h"
#include "VRenderView.h"
#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/glcanvas.h>
#include <wx/clrpicker.h>
#include <wx/slider.h>

using namespace std;

class APropView : public wxPanel
{
	enum
	{
		ID_MemoText = ID_APROP_VIEW,
		ID_MemoUpdateBtn
	};

public:
	APropView(wxWindow* frame, wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxString& name = "APropView");
	~APropView();

	void SetAnnotations(Annotations* ann, VRenderView* vrv);
	Annotations* GetAnnotations();
	void RefreshVRenderViews(bool tree=false);

	void GetSettings();

private:
	wxWindow* m_frame;
	Annotations* m_ann;
	VRenderView* m_vrv;

	wxTextCtrl* m_memo_text;
	wxButton* m_memo_update_btn;

	//memo
	void OnMemoUpdateBtn(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};

#endif//_APROPVIEW_H_

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
#ifndef _LISTPANEL_H_
#define _LISTPANEL_H_

#include <wx/wx.h>
#include <wx/dataview.h>
#include <Fui/ListModel.h>

namespace FUI
{
	class ListPanel : public wxPanel
	{
	public:
		enum
		{
			ID_AddToView = ID_LIST_PANEL2,
			ID_Rename,
			ID_Save,
			ID_Bake,
			ID_SaveMask,
			ID_Delete,
			ID_DeleteAll
		};
		ListPanel(wxWindow* frame,
			wxWindow* parent,
			wxWindowID id,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = "ListPanel");
		~ListPanel();

	private:
		wxWindow* m_frame;
		wxToolBar* m_toolbar;

		wxDataViewCtrl* m_list_ctrl;
		ListModel* m_list_model;

		DECLARE_EVENT_TABLE()
	};
}

#endif//_LISTPANEL_H_
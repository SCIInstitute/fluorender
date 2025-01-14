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
#ifndef _WXUNDOABLETOOLBAR_H_
#define _WXUNDOABLETOOLBAR_H_

#include <Undoable.h>
#include <wx/toolbar.h>

typedef std::vector<bool> UTBData;
class wxUndoableToolbar : public wxToolBar, public Undoable
{
public:
	wxUndoableToolbar(
		wxWindow *parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxTB_HORIZONTAL,
		const wxString& name = "wxUndoableToolbar");

	virtual void ToggleTool(int id, bool val);
	wxToolBarToolBase* AddToolWithHelp(int toolid,
		const wxString& label,
		const wxBitmapBundle& bitmap,
		const wxString& shortHelp = wxEmptyString,
		wxItemKind kind = wxITEM_NORMAL)
	{
		wxToolBarToolBase* result = wxToolBar::AddTool(toolid, label, bitmap, shortHelp, kind);
		SetToolLongHelp(toolid, shortHelp);
		return result;
	}

private:
	int id_;

private:
	void OnChange(wxCommandEvent& event);

private:
	virtual void replace(double t);
	virtual void push(double t);
	virtual void update();
	UTBData get_data();
};

#endif//_WXUNDOABLETOOLBAR_H_

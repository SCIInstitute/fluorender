﻿/*
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
#ifndef _WXUNDOABLETEXTCTRL_H_
#define _WXUNDOABLETEXTCTRL_H_

#include <Undoable.h>
#include <wx/textctrl.h>

class wxUndoableTextCtrl : public wxTextCtrl, public Undoable
{
public:
	wxUndoableTextCtrl(
		wxWindow *parent,
		wxWindowID id,
		const wxString& value,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxValidator& val = wxDefaultValidator,
		const wxString& name = "wxUndoableTextCtrl");

	virtual void SetValue(const wxString& value);
	virtual void ChangeValue(const wxString& value);

private:
	void OnChange(wxCommandEvent& event);

private:
	virtual void replace(double t);
	virtual void push(double t);
	virtual void update();

	virtual double get_time_span()
	{
		return 3;//allow more time for text input
	}
};

#endif//_WXUNDOABLETEXTCTRL_H_

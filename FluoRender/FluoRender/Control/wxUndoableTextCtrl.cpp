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
#include <wxUndoableTextCtrl.h>

wxUndoableTextCtrl::wxUndoableTextCtrl(
	wxWindow *parent,
	wxWindowID id,
	const wxString& label,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxValidator& val,
	const wxString& name):
	wxTextCtrl(parent, id, label, pos, size, style, val, name),
	Undoable()
{
	Bind(wxEVT_TEXT, &wxUndoableTextCtrl::OnChange, this);
}

void wxUndoableTextCtrl::SetValue(const wxString& val)
{
	wxString val_ = GetValue();
	if (val_ != val || stack_.empty())
	{
		wxTextCtrl::SetValue(val);
		double t;
		if (time_sample(t))
			push(t);
		else
			replace(t);
	}
}

void wxUndoableTextCtrl::ChangeValue(const wxString& val)
{
	wxString val_ = GetValue();
	if (val_ != val || stack_.empty())
	{
		wxTextCtrl::ChangeValue(val);
		double t;
		if (time_sample(t))
			push(t);
		else
			replace(t);
	}
}

void wxUndoableTextCtrl::OnChange(wxCommandEvent& event)
{
	if (event.GetString() != "update")
	{
		double t;
		if (time_sample(t))
			push(t);
		else
			replace(t);
	}
	event.Skip();
}

void wxUndoableTextCtrl::replace(double t)
{
	if (stack_.empty())
		return;
	wxString val_ = GetValue();
	stack_[stack_pointer_] = std::pair<double, wxString>(t, val_);
}

void wxUndoableTextCtrl::push(double t)
{
	size_t size = stack_.size();
	wxString val_ = GetValue();
	wxString val;
	if (size)
		val = std::any_cast<wxString>(stack_[stack_pointer_].second);
	if (!size || val_ != val)
	{
		if (!size ||
			stack_pointer_ == size - 1)
			stack_.push_back(std::pair<double, wxString>(t, val_));
		else
			stack_.insert(stack_.begin() + stack_pointer_, std::pair<double, wxString>(t, val_));
		stack_pointer_++;
	}
}

void wxUndoableTextCtrl::update()
{
	wxString val = std::any_cast<wxString>(stack_[stack_pointer_].second);
	wxTextCtrl::ChangeValue(val);

	wxCommandEvent e(wxEVT_TEXT, GetId());
	e.SetEventObject(this);
	e.SetString("update");
	ProcessWindowEvent(e);
	wxPostEvent(GetParent(), e);
}

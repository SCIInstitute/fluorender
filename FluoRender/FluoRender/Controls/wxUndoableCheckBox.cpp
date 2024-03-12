/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#include "wxUndoableCheckBox.h"
#include <Debug.h>

wxUndoableCheckBox::wxUndoableCheckBox(
	wxWindow *parent,
	wxWindowID id,
	const wxString& label,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxValidator& val,
	const wxString& name):
	wxCheckBox(parent, id, label, pos, size, style, val, name),
	Undoable()
{
	Bind(wxEVT_CHECKBOX, &wxUndoableCheckBox::OnChange, this);
}

void wxUndoableCheckBox::SetValue(bool val)
{
	bool val_ = GetValue();
	if (val_ != val || stack_.empty())
	{
		wxCheckBox::SetValue(val);
		double t;
		if (time_sample(t))
			push(t);
		else
			replace(t);
	}
}

void wxUndoableCheckBox::OnChange(wxCommandEvent& event)
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

void wxUndoableCheckBox::replace(double t)
{
	if (stack_.empty())
		return;
	bool val_ = wxCheckBox::GetValue();
	stack_[stack_pointer_] = std::pair<double, bool>(t, val_);
}

void wxUndoableCheckBox::push(double t)
{
	size_t size = stack_.size();
	bool val_ = wxCheckBox::GetValue();
	bool val = size ? std::any_cast<bool>(stack_[stack_pointer_].second) : false;
	if (!size || val_ != val)
	{
		if (!size ||
			stack_pointer_ == size - 1)
			stack_.push_back(std::pair<double, bool>(t, val_));
		else
			stack_.insert(stack_.begin() + stack_pointer_, std::pair<double, bool>(t, val_));
		stack_pointer_++;
		//DBGPRINT(L"\tsize:%d,pointer:%d,last:(%f, %d)\n",
		//	stack_.size(), stack_pointer_, stack_.back().first,
		//	std::any_cast<bool>(stack_.back().second));
	}
}

void wxUndoableCheckBox::update()
{
	bool val = std::any_cast<bool>(stack_[stack_pointer_].second);
	wxCheckBox::SetValue(val);

	wxCommandEvent e(wxEVT_CHECKBOX, GetId());
	e.SetEventObject(this);
	e.SetString("update");
	ProcessWindowEvent(e);
	wxPostEvent(GetParent(), e);
}

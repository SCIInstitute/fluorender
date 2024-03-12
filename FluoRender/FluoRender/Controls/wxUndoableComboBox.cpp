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
#include "wxUndoableComboBox.h"

wxUndoableComboBox::wxUndoableComboBox(
	wxWindow *parent,
	wxWindowID id,
	const wxString& value,
	const wxPoint& pos,
	const wxSize& size,
	int n,
	const wxString choices[],
	long style,
	const wxValidator& val,
	const wxString& name):
	wxComboBox(parent, id, value, pos, size, n, choices, style, val, name),
	Undoable()
{
	Bind(wxEVT_COMBOBOX, &wxUndoableComboBox::OnChange, this);
}

void wxUndoableComboBox::SetSelection(int val)
{
	int val_ = GetSelection();
	if (val_ != val || stack_.empty())
	{
		wxComboBox::SetSelection(val);
		double t;
		if (time_sample(t))
			push(t);
		else
			replace(t);
	}
}

void wxUndoableComboBox::OnChange(wxCommandEvent& event)
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

void wxUndoableComboBox::replace(double t)
{
	if (stack_.empty())
		return;
	int val_ = GetSelection();
	stack_[stack_pointer_] = std::pair<double, int>(t, val_);
}

void wxUndoableComboBox::push(double t)
{
	size_t size = stack_.size();
	int val_ = GetSelection();
	int val = size ? std::any_cast<int>(stack_[stack_pointer_].second) : 0;
	if (!size || val_ != val)
	{
		if (!size ||
			stack_pointer_ == size - 1)
			stack_.push_back(std::pair<double, int>(t, val_));
		else
			stack_.insert(stack_.begin() + stack_pointer_, std::pair<double, int>(t, val_));
		stack_pointer_++;
	}
}

void wxUndoableComboBox::update()
{
	int val = std::any_cast<int>(stack_[stack_pointer_].second);
	wxComboBox::SetSelection(val);

	wxCommandEvent e(wxEVT_COMBOBOX, GetId());
	e.SetEventObject(this);
	e.SetString("update");
	ProcessWindowEvent(e);
	wxPostEvent(GetParent(), e);
}

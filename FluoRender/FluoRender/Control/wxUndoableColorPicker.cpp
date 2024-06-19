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
#include "wxUndoableColorPicker.h"

wxUndoableColorPicker::wxUndoableColorPicker(
	wxWindow *parent,
	wxWindowID id,
	const wxColor& color,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxValidator& val,
	const wxString& name):
	wxColourPickerCtrl(parent, id, color, pos, size, style, val, name),
	Undoable()
{
	Bind(wxEVT_COLOURPICKER_CHANGED, &wxUndoableColorPicker::OnChange, this);
}

void wxUndoableColorPicker::SetValue(const wxColor& color)
{
	wxColor color_ = GetColour();
	if (color_ != color || stack_.empty())
	{
		SetColour(color);
		double t;
		if (time_sample(t))
			push(t);
		else
			replace(t);
	}
}

void wxUndoableColorPicker::OnChange(wxColourPickerEvent& event)
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

void wxUndoableColorPicker::replace(double t)
{
	if (stack_.empty())
		return;
	wxColor color_ = GetColour();
	stack_[stack_pointer_] = std::pair<double, wxColor>(t, color_);
}

void wxUndoableColorPicker::push(double t)
{
	size_t size = stack_.size();
	wxColor color_ = GetColour();
	wxColor color = size ? std::any_cast<wxColor>(stack_[stack_pointer_].second) : *wxWHITE;
	if (!size || color_ != color)
	{
		if (!size ||
			stack_pointer_ == size - 1)
			stack_.push_back(std::pair<double, wxColor>(t, color_));
		else
			stack_.insert(stack_.begin() + stack_pointer_, std::pair<double, wxColor>(t, color_));
		stack_pointer_++;
	}
}

void wxUndoableColorPicker::update()
{
	wxColor color = std::any_cast<wxColor>(stack_[stack_pointer_].second);
	SetColour(color);

	wxColourPickerEvent e(this, GetId(), color);
	e.SetString("update");
	ProcessWindowEvent(e);
	wxPostEvent(GetParent(), e);
}

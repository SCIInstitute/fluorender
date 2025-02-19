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
#include <wxUndoableToolbar.h>
#include <Debug.h>

wxUndoableToolbar::wxUndoableToolbar(
	wxWindow *parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name):
	wxToolBar(parent, id, pos, size, style, name),
	Undoable(),
	id_(-1)
{
	Bind(wxEVT_TOOL, &wxUndoableToolbar::OnChange, this);
}

void wxUndoableToolbar::ToggleTool(int id, bool val)
{
	id_ = id;
	bool val_ = GetToolState(id);
	if (val_ != val || stack_.empty())
	{
		wxToolBar::ToggleTool(id, val);
		double t;
		if (time_sample(t))
			push(t);
		else
			replace(t);
	}
}

void wxUndoableToolbar::OnChange(wxCommandEvent& event)
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

void wxUndoableToolbar::replace(double t)
{
	if (stack_.empty())
		return;
	UTBData val_ = get_data();
	stack_[stack_pointer_] = std::pair<double, UTBData>(t, val_);
}

void wxUndoableToolbar::push(double t)
{
	size_t size = stack_.size();
	UTBData val_ = get_data();
	UTBData val;
	if (size)
		val = std::any_cast<UTBData>(stack_[stack_pointer_].second);
	if (!size || val_ != val)
	{
		if (!size ||
			stack_pointer_ == size - 1)
			stack_.push_back(std::pair<double, UTBData>(t, val_));
		else
			stack_.insert(stack_.begin() + stack_pointer_,
				std::pair<double, UTBData>(t, val_));
		stack_pointer_++;
		//DBGPRINT(L"\tsize:%d,pointer:%d,last:(%f, %d)\n",
		//	stack_.size(), stack_pointer_, stack_.back().first,
		//	std::any_cast<bool>(stack_.back().second));
	}
}

void wxUndoableToolbar::update()
{
	UTBData val = std::any_cast<UTBData>(stack_[stack_pointer_].second);
	for (size_t i = 0; i < val.size(); ++i)
	{
		if (val[i] != GetToolState(i))
		{
			wxToolBar::ToggleTool(i, val[i]);
			id_ = i;
		}
	}

	wxCommandEvent e(wxEVT_TOOL, id_);
	e.SetEventObject(this);
	e.SetString("update");
	ProcessWindowEvent(e);
	wxPostEvent(GetParent(), e);
}

UTBData wxUndoableToolbar::get_data()
{
	UTBData data;
	int n = GetToolsCount();
	for (size_t i = 0; i < n; ++i)
		data.push_back(GetToolState(i));
	return data;
}
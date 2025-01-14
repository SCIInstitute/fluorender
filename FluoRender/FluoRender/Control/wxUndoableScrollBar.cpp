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
#include <wxUndoableScrollBar.h>
#include <Utils.h>
#include <Debug.h>

wxUndoableScrollBar::wxUndoableScrollBar(
	wxWindow *parent,
	wxWindowID id,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxValidator& val,
	const wxString& name):
	wxScrollBar(parent, id, pos, size, style, val, name),
	Undoable(),
	timer_(this),
	track_(false)
{
	Bind(wxEVT_LEFT_DOWN, &wxUndoableScrollBar::OnLeftDown, this);
	Bind(wxEVT_LEFT_UP, &wxUndoableScrollBar::OnLeftUp, this);
	Bind(wxEVT_SCROLL_THUMBTRACK, &wxUndoableScrollBar::OnTrack, this);
	Bind(wxEVT_SCROLL_LINEDOWN, &wxUndoableScrollBar::OnLineDown, this);
	Bind(wxEVT_SCROLL_LINEUP, &wxUndoableScrollBar::OnLineUp, this);
	Bind(wxEVT_SCROLL_PAGEUP, &wxUndoableScrollBar::OnScrollPageUp, this);
	Bind(wxEVT_SCROLL_PAGEDOWN, &wxUndoableScrollBar::OnScrollPageDown, this);
}

void wxUndoableScrollBar::SetMode(int val)
{
	mode_ = val;
	switch (mode_)
	{
	case 0://normal
		if (timer_.IsRunning())
			timer_.Stop();
		SetThumbPosition2(value_);
		Unbind(wxEVT_TIMER, &wxUndoableScrollBar::OnTimer, this);
		Unbind(wxEVT_SCROLL_THUMBRELEASE, &wxUndoableScrollBar::OnRelease, this);
		break;
	case 1://jog
		SetThumbPosition2(value_);
		Bind(wxEVT_TIMER, &wxUndoableScrollBar::OnTimer, this);
		Bind(wxEVT_SCROLL_THUMBRELEASE, &wxUndoableScrollBar::OnRelease, this);
		break;
	}
}

void wxUndoableScrollBar::SetScrollbar2(int position, int thumbSize, int low, int high, int pageSize)
{
	value_ = position;
	low_ = low;
	high_ = high > low ? high : low + 1;
	int p;
	if (mode_ == 0)
		p = position - low;
	else
		p = (high - low) / 2;
	SetScrollbar(p, thumbSize, high - low + thumbSize, pageSize);
}

void wxUndoableScrollBar::SetThumbPosition2(int val)
{
	if (mode_ == 0)
	{
		SetThumbPosition(val - low_);
	}
	else if (mode_ == 1)
	{
		if (track_)
			return;
		SetThumbPosition(
			(GetRange() - GetThumbSize()) / 2);
	}
}

void wxUndoableScrollBar::SetValue(int val)
{
	int old = value_;
	value_ = fluo::RotateClamp2(val, low_, high_);
	bool changed = old != value_;

	if (changed)
		SetThumbPosition2(value_);

	if (changed || stack_.empty())
	{
		double t;
		if (time_sample(t))
			push(t);
		else
			replace(t);
	}

	if (changed)
	{
		wxCommandEvent e(wxEVT_SCROLL_CHANGED, GetId());
		e.SetEventObject(this);
		e.SetString("update");
		ProcessWindowEvent(e);
		wxPostEvent(GetParent(), e);
	}
}

void wxUndoableScrollBar::ChangeValue(int val)
{
	int old = value_;
	value_ = fluo::RotateClamp2(val, low_, high_);
	bool changed = old != value_;

	if (changed)
		SetThumbPosition2(value_);

	if (changed || stack_.empty())
	{
		double t;
		if (time_sample(t))
			push(t);
		else
			replace(t);
	}
}

int wxUndoableScrollBar::GetValue()
{
	return value_;
}

void wxUndoableScrollBar::OnTimer(wxTimerEvent& event)
{
	if (mode_ == 1)
	{
		int pos = GetThumbPosition();
		int range = GetRange() - GetThumbSize();
		int mid = range / 2;
		int dist = pos - mid;
		double val = value_;
		double f = double(dist) / (mid / 3);
		val += f * f * f;

		SetValue(std::round(val));
	}
	event.Skip();
}

void wxUndoableScrollBar::OnLeftDown(wxMouseEvent& event)
{
	//SetFocus();
	//if (!HasCapture())
	//	CaptureMouse();
	event.Skip();
}

void wxUndoableScrollBar::OnLeftUp(wxMouseEvent& event)
{
	//if (HasCapture())
	//	ReleaseMouse();
	event.Skip();
}

void wxUndoableScrollBar::OnTrack(wxScrollEvent& event)
{
	if (mode_ == 0)
		SetValue(GetThumbPosition() + low_);
	else if (mode_ == 1)
	{
		track_ = true;
		if (!timer_.IsRunning())
			timer_.Start(50);
	}
	event.Skip();
}

void wxUndoableScrollBar::OnRelease(wxScrollEvent& event)
{
	track_ = false;
	timer_.Stop();
	SetThumbPosition2(value_);
	event.Skip();
}

void wxUndoableScrollBar::OnLineDown(wxScrollEvent& event)
{
	SetValue(value_ + 1);
	event.Skip();
}

void wxUndoableScrollBar::OnLineUp(wxScrollEvent& event)
{
	SetValue(value_ - 1);
	event.Skip();
}

void wxUndoableScrollBar::OnScrollPageDown(wxScrollEvent& event)
{
	SetValue(value_ + GetPageSize());
	event.Skip();
}

void wxUndoableScrollBar::OnScrollPageUp(wxScrollEvent& event)
{
	SetValue(value_ - GetPageSize());
	event.Skip();
}

void wxUndoableScrollBar::replace(double t)
{
	if (stack_.empty())
		return;
	stack_[stack_pointer_] = std::pair<double, int>(t, value_);
}

void wxUndoableScrollBar::push(double t)
{
	size_t size = stack_.size();
	int val = size ? std::any_cast<int>(stack_[stack_pointer_].second) : false;
	if (!size || value_ != val)
	{
		if (!size ||
			stack_pointer_ == size - 1)
			stack_.push_back(std::pair<double, int>(t, value_));
		else
			stack_.insert(stack_.begin() + stack_pointer_, std::pair<double, int>(t, value_));
		stack_pointer_++;
		//DBGPRINT(L"\tsize:%d,pointer:%d,last:(%f, %d)\n",
		//	stack_.size(), stack_pointer_, stack_.back().first,
		//	std::any_cast<int>(stack_.back().second));
	}
}

void wxUndoableScrollBar::update()
{
	int val = std::any_cast<int>(stack_[stack_pointer_].second);
	SetValue(val);
}

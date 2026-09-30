/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#include <wxNumTextCtrl.h>

wxBEGIN_EVENT_TABLE(wxNumTextCtrl, wxTextCtrl)
EVT_TIMER(wxID_ANY, wxNumTextCtrl::OnTimer)
EVT_KILL_FOCUS(wxNumTextCtrl::OnKillFocus)
wxEND_EVENT_TABLE()

wxNumTextCtrl::wxNumTextCtrl(
	wxWindow* parent,
	wxWindowID id,
	const wxString& value,
	const wxPoint& pos,
	const wxSize& size,
	long style) :
	wxTextCtrl(parent, id, value, pos, size, style),
	timer_(this)
{
}

wxNumTextCtrl::~wxNumTextCtrl()
{
	timer_.Stop();
}

void wxNumTextCtrl::ChangeValue(const wxString& value)
{
	// Normal behavior when not editing
	if (!HasFocus())
	{
		has_pending_ = false;
		timer_.Stop();

		wxTextCtrl::ChangeValue(value);
		return;
	}

	// User is editing.
	// Remember latest value and delay update.
	pending_value_ = value;
	has_pending_ = true;

	timer_.StartOnce(DelayMs);
}

void wxNumTextCtrl::OnTimer(wxTimerEvent&)
{
	// User still editing.
	if (HasFocus())
	{
		timer_.StartOnce(DelayMs);
		return;
	}

	ApplyPendingValue();
}

void wxNumTextCtrl::OnKillFocus(wxFocusEvent& event)
{
	ApplyPendingValue();

	event.Skip();
}

void wxNumTextCtrl::ApplyPendingValue()
{
	if (!has_pending_)
		return;

	has_pending_ = false;

	wxTextCtrl::ChangeValue(pending_value_);
}
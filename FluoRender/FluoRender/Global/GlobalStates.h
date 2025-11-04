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
#ifndef _GLOBALSTATES_H_
#define _GLOBALSTATES_H_

#include <string>

namespace flrd
{
	enum class SelectMode : int;
	enum class RulerMode : int;
}
enum class InteractiveMode : int;
class GlobalStates
{
public:
	GlobalStates();
	~GlobalStates();

	bool ClipDisplayChanged();

	void SetModal(bool bval = true);

	//freehand tool states
	bool ToggleBrushMode(flrd::SelectMode mode);
	bool ToggleRulerMode(flrd::RulerMode mode);
	bool ToggleIntMode(InteractiveMode mode);
	bool ToggleMagnet(bool redist);
	bool QueryShowBrush();

public:
	bool m_mouse_in_clip_plane_panel = false;
	bool m_mouse_in_aov_slider = false;
	bool m_clip_display = false;	//show clipping planes in view

	bool m_freehand_tool_from_kb = false;	//keyboard shortcuts for freehand tools
	
	bool m_modal_shown = false;	//a modal dialog is currently shown, disable keyboard shortcuts

	std::string m_status_str;		//string to show on main statusbar

	bool m_benchmark = false;	//benchmark mode

	bool m_capture = false;	//capture mode, used for movie making
};
#endif//_GLOBALSTATES_H_
/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#include <Input.hpp>
#include <Names.hpp>

// Default constructor
//
Flinput::Flinput()
{
	//mouse
	addValue(gstMouseLeftDown, bool(false));
	addValue(gstMouseRightDown, bool(false));
	addValue(gstMouseMiddleDown, bool(false));
	addValue(gstMouseLeftUp, bool(false));
	addValue(gstMouseRightUp, bool(false));
	addValue(gstMouseMiddleUp, bool(false));
	addValue(gstMouseLeftHold, bool(false));
	addValue(gstMouseRightHold, bool(false));
	addValue(gstMouseMiddleHold, bool(false));
	addValue(gstMouseDrag, bool(false));
	addValue(gstMouseWheel, long(0));
	//keys
	//fn
	addValue(gstKbF5Down, bool(false));
	addValue(gstKbF5Up, bool(false));
	addValue(gstKbF5Hold, bool(false));
	addValue(gstKbAltDown, bool(false));
	addValue(gstKbAltUp, bool(false));
	addValue(gstKbAltHold, bool(false));
	addValue(gstKbCtrlDown, bool(false));
	addValue(gstKbCtrlUp, bool(false));
	addValue(gstKbCtrlHold, bool(false));
	addValue(gstKbShiftDown, bool(false));
	addValue(gstKbShiftUp, bool(false));
	addValue(gstKbShiftHold, bool(false));
	addValue(gstKbReturnDown, bool(false));
	addValue(gstKbReturnUp, bool(false));
	addValue(gstKbReturnHold, bool(false));
	addValue(gstKbSpaceDown, bool(false));
	addValue(gstKbSpaceUp, bool(false));
	addValue(gstKbSpaceHold, bool(false));
	addValue(gstKbLeftDown, bool(false));
	addValue(gstKbLeftUp, bool(false));
	addValue(gstKbLeftHold, bool(false));
	addValue(gstKbRightDown, bool(false));
	addValue(gstKbRightUp, bool(false));
	addValue(gstKbRightHold, bool(false));
	addValue(gstKbUpDown, bool(false));
	addValue(gstKbUpUp, bool(false));
	addValue(gstKbUpHold, bool(false));
	addValue(gstKbDownDown, bool(false));
	addValue(gstKbDownUp, bool(false));
	addValue(gstKbDownHold, bool(false));
	addValue(gstKbADown, bool(false));
	addValue(gstKbAUp, bool(false));
	addValue(gstKbAHold, bool(false));
	addValue(gstKbCDown, bool(false));
	addValue(gstKbCUp, bool(false));
	addValue(gstKbCHold, bool(false));
	addValue(gstKbDDown, bool(false));
	addValue(gstKbDUp, bool(false));
	addValue(gstKbDHold, bool(false));
	addValue(gstKbFDown, bool(false));
	addValue(gstKbFUp, bool(false));
	addValue(gstKbFHold, bool(false));
	addValue(gstKbLDown, bool(false));
	addValue(gstKbLUp, bool(false));
	addValue(gstKbLHold, bool(false));
	addValue(gstKbMDown, bool(false));
	addValue(gstKbMUp, bool(false));
	addValue(gstKbMHold, bool(false));
	addValue(gstKbNDown, bool(false));
	addValue(gstKbNUp, bool(false));
	addValue(gstKbNHold, bool(false));
	addValue(gstKbRDown, bool(false));
	addValue(gstKbRUp, bool(false));
	addValue(gstKbRHold, bool(false));
	addValue(gstKbSDown, bool(false));
	addValue(gstKbSUp, bool(false));
	addValue(gstKbSHold, bool(false));
	addValue(gstKbVDown, bool(false));
	addValue(gstKbVUp, bool(false));
	addValue(gstKbVHold, bool(false));
	addValue(gstKbWDown, bool(false));
	addValue(gstKbWUp, bool(false));
	addValue(gstKbWHold, bool(false));
	addValue(gstKbXDown, bool(false));
	addValue(gstKbXUp, bool(false));
	addValue(gstKbXHold, bool(false));
	addValue(gstKbZDown, bool(false));
	addValue(gstKbZUp, bool(false));
	addValue(gstKbZHold, bool(false));
	addValue(gstKbLbrktDown, bool(false));
	addValue(gstKbLbrktUp, bool(false));
	addValue(gstKbLbrktHold, bool(false));
	addValue(gstKbRbrktDown, bool(false));
	addValue(gstKbRbrktUp, bool(false));
	addValue(gstKbRbrktHold, bool(false));
	addValue(gstKbBslshDown, bool(false));
	addValue(gstKbBslshUp, bool(false));
	addValue(gstKbBslshHold, bool(false));
}

Flinput::Flinput(const Flinput& data, const fluo::CopyOp& copyop, bool copy_values) :
	Node(data, copyop, copy_values)
{
}

// Destructor
//
Flinput::~Flinput()
{
}

void Flinput::Update()
{
#pragma message ("get mouse and keyboard states")
/*
	//mouse
	wxMouseState ms = wxGetMouseState();
	updateState(gstMouseLeft, ms.LeftIsDown());
	updateState(gstMouseRight, ms.RightIsDown());
	updateState(gstMouseMiddle, ms.MiddleIsDown());
	//keys
	//fn
	updateState(gstKbF5, wxGetKeyState(WXK_F5));
	//control keys
	updateState(gstKbAlt, wxGetKeyState(WXK_ALT));
	updateState(gstKbCtrl, wxGetKeyState(WXK_CONTROL));
	updateState(gstKbShift, wxGetKeyState(WXK_SHIFT));
	updateState(gstKbReturn, wxGetKeyState(WXK_RETURN));
	//space
	updateState(gstKbSpace, wxGetKeyState(WXK_SPACE));
	//arrows
	updateState(gstKbLeft, wxGetKeyState(WXK_LEFT));
	updateState(gstKbRight, wxGetKeyState(WXK_RIGHT));
	updateState(gstKbUp, wxGetKeyState(WXK_UP));
	updateState(gstKbDown, wxGetKeyState(WXK_DOWN));
	//alphabet
	updateState(gstKbA, wxGetKeyState(wxKeyCode('A')));
	updateState(gstKbC, wxGetKeyState(wxKeyCode('C')));
	updateState(gstKbD, wxGetKeyState(wxKeyCode('D')));
	updateState(gstKbF, wxGetKeyState(wxKeyCode('F')));
	updateState(gstKbL, wxGetKeyState(wxKeyCode('L')));
	updateState(gstKbM, wxGetKeyState(wxKeyCode('M')));
	updateState(gstKbN, wxGetKeyState(wxKeyCode('N')));
	updateState(gstKbR, wxGetKeyState(wxKeyCode('R')));
	updateState(gstKbS, wxGetKeyState(wxKeyCode('S')));
	updateState(gstKbV, wxGetKeyState(wxKeyCode('V')));
	updateState(gstKbW, wxGetKeyState(wxKeyCode('W')));
	updateState(gstKbX, wxGetKeyState(wxKeyCode('X')));
	updateState(gstKbZ, wxGetKeyState(wxKeyCode('Z')));
	//puctuation marks
	updateState(gstKbLbrkt, wxGetKeyState(wxKeyCode('[')));
	updateState(gstKbRbrkt, wxGetKeyState(wxKeyCode(']')));
	updateState(gstKbBslsh, wxGetKeyState(wxKeyCode('\\')));
	*/
}

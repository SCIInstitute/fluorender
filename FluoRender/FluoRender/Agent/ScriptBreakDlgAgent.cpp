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

#include <ScriptBreakDlgAgent.h>
#include <ScriptBreakDlg.h>
#include <Global.h>
#include <Names.h>
#include <ScriptProc.h>

ScriptBreakDlgAgent::ScriptBreakDlgAgent(
	ScriptBreakDlg* dlg) :
	Agent(dlg)
{

}

bool ScriptBreakDlgAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void ScriptBreakDlgAgent::Update(
	const UpdateRequest& request)
{
	if (request.dir == UpdateDir::DataToUI)
	{
		UpdateUI(request);
	}
	else if (request.dir == UpdateDir::UItoData)
	{
		UpdateData(request);
	}
}

void ScriptBreakDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	//update user interface
	if (FOUND_VALUE(gstNull))
		return;

	bool update_all = request.values.empty();

	if (update_all || FOUND_VALUE(gstScriptBreakTitle))
	{
		auto str = glbin_script_proc.GetTitle();
		dlg->SetLabel(str);
	}

	if (update_all || FOUND_VALUE(gstScriptBreakInfo))
	{
		auto str = glbin_script_proc.GetInfo();
		dlg->UpdateScriptBreakInfo(str);
	}
}

void ScriptBreakDlgAgent::UpdateData(const UpdateRequest& request)
{

}

ScriptBreakDlg* ScriptBreakDlgAgent::GetDialog() const
{
	return static_cast<ScriptBreakDlg*>(GetWindow());
}


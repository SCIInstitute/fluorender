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

#include <OclDlgAgent.h>
#include <OclDlg.h>
#include <Global.h>
#include <Names.h>
#include <Directory.h>
#include <KernelExecutor.h>

OclDlgAgent::OclDlgAgent(
	OclDlg* dlg) :
	Agent(dlg)
{

}

bool OclDlgAgent::Accept(
	const UpdateRequest& request) const
{
	return true;
}

void OclDlgAgent::Update(
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

void OclDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	//update user interface
	if (FOUND_VALUE(gstNull))
		return;
	bool update_all = request.values.empty();

	if (update_all || FOUND_VALUE(gstKernelList))
	{
		std::vector<std::wstring> list;
		if (GetKernelList(list))
			dlg->UpdateKernelList(list);
	}

	if (update_all || FOUND_VALUE(gstKernelListSelect))
	{
		int idx = glbin_kernel_executor.GetFileIndex();
		dlg->UpdateKernelListSelect(idx);
	}
}

void OclDlgAgent::UpdateData(const UpdateRequest& request)
{

}

OclDlg* OclDlgAgent::GetDialog() const
{
	return static_cast<OclDlg*>(GetWindow());
}

bool OclDlgAgent::GetKernelList(std::vector<std::wstring>& list)
{
	std::filesystem::path p = GetUserSettingsRoot();
	p /= "CL_code";
	std::vector<std::string> list;
	// Iterate over the files in the "Scripts" directory
	if (!std::filesystem::exists(p) || !std::filesystem::is_directory(p))
		return false;
	for (const auto& entry : std::filesystem::directory_iterator(p))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".cl")
		{
			list.push_back(entry.path().stem().wstring());
		}
	}
	if (list.empty())
		return false;
	// Sort the list of files
	std::sort(list.begin(), list.end());
	return true;
}

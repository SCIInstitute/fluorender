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
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <RenderView.h>
#include <DataManager.h>
#include <Coordinator.h>
#include <ModalDlg.h>

OclDlgAgent::OclDlgAgent(
	OclDlg* dlg) :
	Agent(dlg)
{

}

void OclDlgAgent::UpdateUI(const UpdateRequest& request)
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	bool update_all = request.values.empty();

	if (update_all || request.HasValue(gstKernelList))
	{
		std::vector<std::wstring> list;
		if (GetKernelList(list))
			dlg->UpdateKernelList(list);
	}

	if (update_all || request.HasValue(gstKernelListSelect))
	{
		int idx = glbin_kernel_executor.GetFileIndex();
		dlg->UpdateKernelListSelect(idx);
	}

	if (update_all || request.HasValue(gstKernelIterations))
	{
		int ival = glbin_kernel_executor.GetRepeat();
		dlg->UpdateKernelIterations(ival);
	}
}

void OclDlgAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstKernelExecute))
		Execute();
	if (request.HasValue(gstKernelSave))
		Save();
	if (request.HasValue(gstKernelSaveAs))
		SaveAs();
	if (request.HasValue(gstKernelIterations))
		SetIterations();
}

OclDlg* OclDlgAgent::GetDialog() const
{
	return static_cast<OclDlg*>(GetWindow());
}

bool OclDlgAgent::GetKernelList(std::vector<std::wstring>& list)
{
	std::filesystem::path p = GetUserSettingsRoot();
	p /= "CL_code";

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

void OclDlgAgent::Execute()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto vd = glbin_current.vol_data.lock();
	auto view = glbin_current.render_view.lock();
	if (!vd || !view)
		return;


	//get volume currently selected
	const std::wstring vd_name = vd->GetName();
	bool dup = vd_name.find(L"_CL") == std::wstring::npos;

	//get cl code
	auto code = dlg->GetCode();
	glbin_kernel_executor.SetCode(code);
	glbin_kernel_executor.SetVolume(vd);
	glbin_kernel_executor.SetDuplicate(dup);
	glbin_kernel_executor.SetProgress(0, "Running volume filter.");
	glbin_kernel_executor.Execute();
	glbin_kernel_executor.SetRange(0, 100);
	glbin_kernel_executor.SetProgress(0, "");

	dlg->UpdateOutputText(glbin_kernel_executor.GetInfo(), true);

	//add result for rendering
	if (dup)
	{
		auto vd_r = glbin_kernel_executor.GetResult(true);
		if (!vd_r)
			return;
		glbin_data_manager.AddVolumeData(vd_r);
		view->AddVolumeData(vd_r);
		vd->SetDisp(false);
		glbin_current.SetVolumeData(vd_r);
	}

	fluo::ValueCollection vc;
	if (dup)
		vc.insert({ gstListCtrl, gstTreeCtrl, gstUpdateSync, gstCurrentSelect, gstVolumePropPanel });
	else
		vc.insert({ gstNull });

	std::set<Agent*> target{ this };
	target.insert(glbin_coordinator.FindRenderCanvasAgent(view));
	NotifyViewUpdate(vc, target);
}

void OclDlgAgent::Save()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto filename = dlg->GetKernelFileName();
	if (filename.empty())
		SaveAs();
	else
		Save();

}

void OclDlgAgent::SaveAs()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	ModalDlg fopendlg(
		dlg, "Choose an filter file",
		"", "", "Filter file|*.cl;*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	int rval = fopendlg.ShowModal();
	if (rval == wxID_OK)
	{
		auto filename = fopendlg.GetPath().ToStdWstring();
		if (dlg->SaveKernelFile(filename))
		{
			dlg->UpdateKernelFileName(filename);
			std::filesystem::path p(filename);
			std::wstring fn = p.filename().wstring();
			p = GetUserSettingsRoot();
			p = p / "CL_code" / fn;
			fn = p.wstring();
			dlg->SaveKernelFile(fn);
			//fn = p.stem().string();
			//m_kernel_list->InsertItem(m_kernel_list->GetItemCount(), fn);
			UpdateDataToUI({ gstKernelList });
		}
	}
}

void OclDlgAgent::SetIterations()
{
	auto dlg = GetDialog();
	if (dlg)
	{
		glbin_kernel_executor.SetRepeat(dlg->GetIterations() - 1);
		UpdateDataToUI({ gstKernelIterations });
	}
}

void OclDlgAgent::SetCode()
{
	auto dlg = GetDialog();
	if (!dlg)
		return;

	auto filename = dlg->GetKernelFile();
	std::filesystem::path p = GetUserSettingsRoot();
	p = p / "CL_code" / (filename + L".cl");
	filename = p.wstring();
	dlg->LoadFile(filename);

	//get cl code
	auto code = dlg->GetCode();
	glbin_kernel_executor.SetCode(code);
	glbin_kernel_executor.SetFileIndex(dlg->GetKernelFileIndex());
}

void OclDlgAgent::SetFileIndex()
{

}

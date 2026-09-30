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
#include <DragDropFileAgent.h>
#include <DragDrop.h>
#include <Global.h>
#include <Project.h>
#include <DataManager.h>

DragDropFileAgent::DragDropFileAgent(
	DnDFile* dlg) :
	Agent(dlg)
{

}

void DragDropFileAgent::UpdateUI(const UpdateRequest& request)
{
}

void DragDropFileAgent::UpdateData(const UpdateRequest& request)
{
	if (request.HasValue(gstDragDropFile))
		DropFiles();
}

DnDFile* DragDropFileAgent::GetDnDFile() const
{
	return static_cast<DnDFile*>(GetOwner());
}

void DragDropFileAgent::DropFiles()
{
	auto dnd = GetDnDFile();
	if (!dnd)
		return;
	auto filenames = dnd->GetFilenames();

	std::wstring filename = filenames[0];
	std::filesystem::path p(filename);
	std::wstring suffix = p.extension().wstring();
	std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);

	if (suffix == L".vrp")
	{
		glbin_project.Open(filename);
	}
	else if (suffix == L".nrrd" ||
		suffix == L".msk" ||
		suffix == L".lbl" ||
		suffix == L".tif" ||
		suffix == L".tiff" ||
		suffix == L".png" ||
		suffix == L".jpg" ||
		suffix == L".jpeg" ||
		suffix == L".jp2" ||
		suffix == L".oib" ||
		suffix == L".oif" ||
		suffix == L".lsm" ||
		suffix == L".xml" ||
		suffix == L".vvd" ||
		suffix == L".nd2" ||
		suffix == L".czi" ||
		suffix == L".lif" ||
		suffix == L".lof" ||
		suffix == L".mp4" ||
		suffix == L".m4v" ||
		suffix == L".mov" ||
		suffix == L".avi" ||
		suffix == L".wmv" ||
		suffix == L".dcm" ||
		suffix == L".dicom")
	{
		glbin_data_manager.LoadVolumes(filenames, false);
	}
	else if (suffix == L".obj")
	{
		glbin_data_manager.LoadMeshFiles(filenames);
	}
	else
	{
		glbin_data_manager.LoadVolumes(filenames, true);
	}
}
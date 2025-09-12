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
#include <DragDrop.h>
#include <Global.h>
#include <MainFrame.h>
#include <Project.h>
#include <DataManager.h>

DnDFile::DnDFile(wxWindow *frame, wxWindow *view)
: m_frame(frame),
m_view(view)
{
}

DnDFile::~DnDFile()
{
}

wxDragResult DnDFile::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
	return wxDragCopy;
}

bool DnDFile::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames)
{
	if (filenames.Count())
	{
		std::vector<std::wstring> std_filenames;
		std_filenames.reserve(filenames.size()); // Reserve space for efficiency

		for (const auto& filename : filenames) {
			std_filenames.push_back(filename.ToStdWstring());
		}

		MainFrame* vr_frame = (MainFrame*)m_frame;
		if (vr_frame)
		{
			std::wstring filename = std_filenames[0];
			std::filesystem::path p(filename);
			std::wstring suffix = p.extension().wstring();
			std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);

			if (suffix == L".vrp")
			{
				if (m_view)
				{
					wxMessageBox("For project files, drag and drop to regions other than render views.");
					return false;
				}
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
				glbin_data_manager.LoadVolumes(std_filenames, false);
			}
			else if (suffix == L".obj")
			{
				glbin_data_manager.LoadMeshes(std_filenames);
			}
			else
			{
				glbin_data_manager.LoadVolumes(std_filenames, true);
			}
		}
	}

	return true;
}

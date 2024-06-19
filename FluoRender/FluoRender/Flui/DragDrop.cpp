/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
		MainFrame* vr_frame = (MainFrame*)m_frame;
		if (vr_frame)
		{
			wxString filename = filenames[0];
			wxString suffix = filename.Mid(filename.Find('.', true)).MakeLower();

			if (suffix == ".vrp")
			{
				if (m_view)
				{
					wxMessageBox("For project files, drag and drop to regions other than render views.");
					return false;
				}
				glbin_project.Open(filename);
			}
			else if (suffix == ".nrrd" ||
					 suffix == ".msk" ||
					 suffix == ".lbl" ||
					 suffix == ".tif" ||
					 suffix == ".tiff" ||
					 suffix == ".oib" ||
					 suffix == ".oif" ||
					 suffix == ".lsm" ||
					 suffix == ".xml" ||
					 suffix == ".vvd" ||
					 suffix == ".nd2" ||
					 suffix == ".czi" ||
					 suffix == ".lif" ||
					 suffix == ".lof" ||
					 suffix == ".mp4" ||
					 suffix == ".m4v" ||
					 suffix == ".mov" ||
					 suffix == ".avi" ||
					 suffix == ".wmv")
			{
				glbin_data_manager.LoadVolumes(filenames, false);
			}
			else if (suffix == ".obj")
			{
				glbin_data_manager.LoadMeshes(filenames);
			}
			else
			{
				glbin_data_manager.LoadVolumes(filenames, true);
			}
		}
	}

	return true;
}

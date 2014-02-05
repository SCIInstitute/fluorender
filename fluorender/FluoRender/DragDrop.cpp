#include "DragDrop.h"
#include "VRenderFrame.h"
#include "VRenderView.h"

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
		VRenderFrame* vr_frame = (VRenderFrame*)m_frame;
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
				vr_frame->OpenProject(filename);
			}
			else if (suffix == ".nrrd" ||
					 suffix == ".tif" ||
					 suffix == ".tiff" ||
					 suffix == ".oib" ||
					 suffix == ".oif" ||
					 suffix == ".lsm" ||
					 suffix == ".xml")
			{
				vr_frame->LoadVolumes(filenames, (VRenderView*)m_view);
			}
			else if (suffix == ".obj")
			{
				vr_frame->LoadMeshes(filenames, (VRenderView*)m_view);
			}
		}
	}

	return true;
}

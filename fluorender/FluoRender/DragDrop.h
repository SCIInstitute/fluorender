#include <wx/wx.h>
#include <wx/dnd.h>

#ifndef _DRAGDROP_H_
#define _DRAGDROP_H_

class DnDFile : public wxFileDropTarget
{
public:
	DnDFile(wxWindow *frame, wxWindow *view = 0);
	~DnDFile();

	virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def);
	virtual bool OnDropFiles(wxCoord x, wxCoord y,
		const wxArrayString& filenames);

private:
	wxWindow *m_frame;
	wxWindow *m_view;
};

#endif//_DRAGDROP_H_
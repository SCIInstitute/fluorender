#ifndef PNG_RESOURCE_H
#define PNG_RESOURCE_H

#include <wx/wx.h>
#include "../compatibility.h"

namespace PNG_RES
{
	wxBitmap* CreateBitmapFromPngResource(const wxString& t_name);
	bool LoadDataFromResource(char*& t_data, DWORD& t_dataSize, const wxString& t_name);
	wxBitmap* GetBitmapFromMemory(const char* t_data, const DWORD t_size);

}

#endif//PNG_RESOURCE_H

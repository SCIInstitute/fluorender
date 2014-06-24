#ifndef PNG_RESOURCE_H
#define PNG_RESOURCE_H

#include <wx/wx.h>
#include "../compatibility.h"
#include "wx/mstream.h"

#define wxGetBitmapFromMemory(name) _wxGetBitmapFromMemory(name ## _png, sizeof(name ## _png))

inline wxBitmap _wxGetBitmapFromMemory(const unsigned char *data, int length)
{
	  wxLogNull logNo;
      wxMemoryInputStream is(data, length);
         return wxBitmap(wxImage(is, wxBITMAP_TYPE_ANY, -1), -1);
}

#endif//PNG_RESOURCE_H

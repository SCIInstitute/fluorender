/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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

#ifndef _DATAVIEWCOLORRENDERER_H_
#define _DATAVIEWCOLORRENDERER_H_

#include <Types/Color.h>
#include <wx/dataview.h>
#include <wx/brush.h>
#include <wx/dc.h>
#include <wx/variant.h>
#include <iostream>
#include <sstream>

namespace FUI
{
	class DataViewColorRenderer : public wxDataViewCustomRenderer
	{
	public:
		explicit DataViewColorRenderer(wxDataViewCellMode mode) :
			wxDataViewCustomRenderer("string"/*GetDefaultType()*/, mode, wxALIGN_LEFT)
		{}

		virtual bool Render(wxRect rect, wxDC* dc, int state) override
		{
			if (color_.IsOk())
			{
				wxBrush brush(color_, wxBRUSHSTYLE_SOLID);
				dc->SetBrush(brush);
				dc->SetPen(*wxBLACK_PEN);
			}
			else
			{
				wxBrush brush(color_, wxBRUSHSTYLE_CROSSDIAG_HATCH);
				dc->SetBrush(brush);
				//dc->SetPen(*wxBLACK_PEN);
			}
			rect.Deflate(1);
			dc->DrawRectangle(rect);
			return true;
		}

		//virtual bool ActivateCell(const wxRect& cell,
		//	wxDataViewModel *model,
		//	const wxDataViewItem &item,
		//	unsigned int col,
		//	const wxMouseEvent *mouseEvent) override
		//{
		//	wxString position;
		//	if (mouseEvent)
		//		position = wxString::Format("via mouse at %d, %d", mouseEvent->m_x, mouseEvent->m_y);
		//	else
		//		position = "from keyboard";
		//	return false;
		//}

		virtual wxSize GetSize() const override
		{
			return wxSize(20, 20);
		}

		virtual bool SetValue(const wxVariant &value) override
		{
			if (!value.IsNull())
			{
				std::string str = value.GetString().ToStdString();
				std::istringstream iss(str);
				FLTYPE::Color color;
				iss >> color;
				color_ = wxColor((unsigned char)(color.r() * 255 + 0.5),
					(unsigned char)(color.g() * 255 + 0.5),
					(unsigned char)(color.b() * 255 + 0.5));
			}
			else
			{
				color_ = wxColor();
			}
			return true;
		}

		virtual bool GetValue(wxVariant &value) const override
		{
			value << color_;
			return true;
		}

		virtual wxString GetAccessibleDescription() const override
		{
			return color_.GetAsString();
		}

		virtual bool HasEditorCtrl() const override
		{
			return false;
		}

	private:
		wxColor color_;
	};
}

#endif//_DATAVIEWCOLORRENDERER_H_
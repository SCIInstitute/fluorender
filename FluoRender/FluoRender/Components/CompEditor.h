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
#ifndef FL_CompEditor_h
#define FL_CompEditor_h

#include <vector>
#include <Cell.h>
#include <wx/string.h>

class RenderCanvas;
class VolumeData;
namespace flrd
{
	class VolCache;
	//class ComponentAnalyzer;
	class ComponentEditor
	{
	public:
		ComponentEditor();
		~ComponentEditor();

		void SetView(RenderCanvas* view)
		{
			m_view = view;
		}
		RenderCanvas* GetView()
		{
			return m_view;
		}
		void SetVolume(VolumeData* vd)
		{
			m_vd = vd;
		}
		VolumeData* GetVolume()
		{
			return m_vd;
		}
		wxString GetOutput();

		void Clean(int mode);
		void NewId(unsigned int id, bool id_empty, bool append, bool track);
		void Replace(unsigned int id, bool id_empty);
		void Replace(unsigned int id, bool id_empty, CelpList &list);
		void Combine();
		void Combine(CelpList &list);

	private:
		VolumeData* m_vd;
		RenderCanvas* m_view;
		wxString m_output;

	private:
		//read/delete volume cache from file
		void ReadVolCache(VolCache& vol_cache);
		void DelVolCache(VolCache& vol_cache);

	};
}
#endif//FL_CompEditor_h
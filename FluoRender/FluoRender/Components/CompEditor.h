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
#include <wx/colour.h>

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

		void SetId(unsigned int id, bool id_empty)
		{
			m_id = id;
			m_id_empty = id_empty;
		}
		fluo::Color GetColor();
		wxColor GetWxColor();
		wxString GetOutput();

		void Clean(int mode);
		void NewId(bool append, bool track);
		void Replace();
		void Replace(CelpList &list);
		void Combine();
		void Combine(CelpList &list);

	private:
		wxString m_output;
		unsigned int m_id;
		bool m_id_empty;

	private:
		//read/delete volume cache from file
		void ReadVolCache(VolCache& vol_cache);
		void DelVolCache(VolCache& vol_cache);

	};
}
#endif//FL_CompEditor_h
/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2014 Scientific Computing and Imaging Institute,
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
#ifndef FL_CompAnalyzer_h
#define FL_CompAnalyzer_h

#include <string>
#include "CompGraph.h"

class VolumeData;
class Annotations;

namespace FL
{
	class ComponentAnalyzer
	{
	public:
		ComponentAnalyzer(VolumeData* vd=0);
		~ComponentAnalyzer();

		void SetVolume(VolumeData* vd)
		{
			m_comp_list_dirty = m_vd != vd;
			m_vd = vd;
		}
		VolumeData* GetVolume()
		{ return m_vd; }

		void Analyze(bool sel);
		void MatchBricks(bool sel);

		size_t GetListSize();
		void OutputFormHeader(std::string &str);
		//print out results
		//verbose: 0-clean output; 1-extra info
		void OutputCompList(std::string &str, int verbose, std::string comp_header="");
		void OutputCompListTxt(std::string &filename, int verbose, std::string comp_header = "");
		bool GenAnnotations(Annotations &ann);
		//color_type: 1-id-based; 2-size-based
		bool GenMultiChannels(std::list<VolumeData*> &channs, int color_type);
		bool GenRgbChannels(std::list<VolumeData*> &channs, int color_type);

	private:
		VolumeData* m_vd;
		CompList m_comp_list;
		bool m_comp_list_dirty;

		//comp graph
		CompGraph m_comp_graph;

	private:
		unsigned long long GetKey(unsigned int id, unsigned int brick_id)
		{
			unsigned long long temp = brick_id;
			return (temp << 32) | id;
		}
		unsigned int GetExt(unsigned int* data_label,
			unsigned long long index,
			unsigned int id,
			int nx, int ny, int nz,
			int i, int j, int k);
		FLIVR::Color GetColor(CompInfo &comp_info, VolumeData* vd, int color_type);
	};
}
#endif//FL_CompAnalyzer_h
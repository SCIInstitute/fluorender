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

#include <FLIVR/Point.h>
#include <string>
#include <list>
#include <boost/unordered_map.hpp>

class VolumeData;
class Annotations;
namespace FL
{
	class ComponentAnalyzer
	{
	public:
		ComponentAnalyzer(VolumeData* vd);
		~ComponentAnalyzer();

		void SetVolume(VolumeData* vd)
		{ m_vd = vd; }
		VolumeData* GetVolume()
		{ return m_vd; }

		void Analyze(bool sel);
		void OutputCompList(std::string &str);
		bool GenAnnotations(Annotations &ann);

	private:
		VolumeData* m_vd;
		struct CompInfo
		{
			unsigned int id;
			unsigned int sumi;
			double sumd;
			unsigned int ext_sumi;
			double ext_sumd;
			double mean;
			double var;
			double m2;
			double min;
			double max;
			FLIVR::Point pos;

			bool operator<(const CompInfo &info2)
			{
				return id < info2.id;
			}
		};
		typedef boost::unordered_map<unsigned int, CompInfo> CompUList;
		typedef CompUList::iterator CompUListIter;
		typedef std::list<CompInfo> CompList;
		typedef CompList::iterator CompListIter;
		CompList m_comp_list;

	private:
		unsigned int GetExt(unsigned int* data_label,
			unsigned long long index,
			unsigned int id,
			int nx, int ny, int nz,
			int i, int j, int k);
	};
}
#endif//FL_CompAnalyzer_h
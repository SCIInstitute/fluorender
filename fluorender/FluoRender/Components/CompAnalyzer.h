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
#include <vector>
#include <boost/signals2.hpp>
#include "CompGraph.h"
#include "DataManager.h"

class VolumeData;
class Annotations;

namespace FL
{
#define SIZE_LIMIT 10

	class ComponentAnalyzer
	{
	public:
		ComponentAnalyzer(VolumeData* vd=0);
		~ComponentAnalyzer();

		bool GetAnalyzed()
		{ return m_analyzed; }
		void SetVolume(VolumeData* vd)
		{
			m_comp_list_dirty = m_vd != vd;
			m_vd = vd;
		}
		VolumeData* GetVolume()
		{ return m_vd; }

		void SetCoVolumes(std::vector<VolumeData*> &list)
		{
			m_vd_list = list;
		}
		void AddCoVolume(VolumeData* vd)
		{
			m_vd_list.push_back(vd);
		}
		void ClearCoVolumes()
		{
			m_vd_list.clear();
		}
		CompList* GetCompList()
		{
			return &m_comp_list;
		}
		CompGraph* GetCompGraph()
		{
			return &m_comp_graph;
		}

		void Analyze(bool sel, bool consistent, bool colocal=false);
		void MatchBricks(bool sel);
		void UpdateMaxCompSize(bool);
		void MakeColorConsistent();

		size_t GetCompSize();
		size_t GetListSize();
		void OutputFormHeader(std::string &str);
		//print out results
		//verbose: 0-clean output; 1-extra info
		void OutputCompListStream(std::ostream &stream, int verbose, std::string comp_header = "");
		void OutputCompListStr(std::string &str, int verbose, std::string comp_header="");
		void OutputCompListFile(std::string &filename, int verbose, std::string comp_header = "");
		bool GenAnnotations(Annotations &ann, bool consistent);
		//color_type: 1-id-based; 2-size-based
		bool GenMultiChannels(std::list<VolumeData*> &channs, int color_type, bool consistent);
		bool GenRgbChannels(std::list<VolumeData*> &channs, int color_type, bool consistent);

		//update progress
		boost::signals2::signal<void()> m_sig_progress;

	private:
		bool m_analyzed;//if used
		VolumeData* m_vd;//main volume
		bool m_colocal;
		std::vector<VolumeData*> m_vd_list;//list of volumes for colocalization analysis

		//output components
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

		bool GetColor(unsigned int id, int brick_id,
			VolumeData* vd, int color_type,
			FLIVR::Color &color);
		int GetColocalization(size_t bid,
			unsigned int bi,
			unsigned int bj,
			unsigned int bk,
			std::vector<unsigned int> &sumi,
			std::vector<double> &sumd);

		//replace id to make color consistent
		void ReplaceId(unsigned int base_id, pCompInfo &info);
		//get nonconflict color
		unsigned int GetNonconflictId(unsigned int id,
			int nx, int ny, int nz,
			FLIVR::TextureBrick* b,
			unsigned int* data);
	};
}
#endif//FL_CompAnalyzer_h
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
#ifndef FL_CompAnalyzer_h
#define FL_CompAnalyzer_h

#include <string>
#include <vector>
#include <set>
#include <boost/signals2.hpp>
#include <Tracking/Cell.h>
#include "DataManager.h"

class VolumeData;
class Annotations;

namespace fls
{
	struct CompGroup
	{
		VolumeData* vd;//associated volume
		bool dirty;
		CelpList celps;
		CellGraph graph;//links comps in multibrick volume

		CompGroup() :
			vd(0), dirty(true)
		{}
	};
	class ComponentAnalyzer
	{
	public:
		ComponentAnalyzer(VolumeData* vd=0);
		~ComponentAnalyzer();

		unsigned int GetSizeLimit()
		{ return m_slimit; }
		void SetSizeLimit(unsigned int size)
		{ m_slimit = size; }
		bool GetAnalyzed()
		{ return m_analyzed; }
		void SetVolume(VolumeData* vd)
		{
			if (!vd)
				return;
			CompGroup* compgroup = FindCompGroup(vd);
			if (!compgroup)
				compgroup = AddCompGroup(vd);
			m_compgroup = compgroup;
		}
		VolumeData* GetVolume()
		{
			if (m_compgroup)
				return m_compgroup->vd;
			return 0;
		}

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
		CelpList* GetCelpList()
		{
			if (m_compgroup)
				return &(m_compgroup->celps);
			return 0;
		}
		CellGraph* GetCellGraph()
		{
			if (m_compgroup)
				return &(m_compgroup->graph);
			return 0;
		}
		int GetCompGroupSize()
		{
			return m_comp_groups.size();
		}
		CompGroup* GetCompGroup(int i)
		{
			if (i >= 0 && i < m_comp_groups.size())
				return &(m_comp_groups[i]);
			return 0;
		}
		int GetBrickNum()
		{
			return m_bn;
		}

		void Analyze(bool sel, bool consistent, bool colocal=false);
		void MatchBricks(bool sel);
		void UpdateMaxCompSize(bool);
		void MakeColorConsistent();

		size_t GetCompSize();
		size_t GetListSize();
		void GetCompsPoint(fluo::Point& p, std::set<unsigned long long> &ids);

		void OutputFormHeader(std::string &str);
		//print out results
		//verbose: 0-clean output; 1-extra info
		void OutputCompListStream(std::ostream &stream, int verbose, std::string comp_header = "");
		void OutputCompListStr(std::string &str, int verbose, std::string comp_header="");
		void OutputCompListFile(std::string &filename, int verbose, std::string comp_header = "");
		bool GenAnnotations(Annotations &ann, bool consistent, int type);
		//color_type: 1-id-based; 2-size-based
		bool GenMultiChannels(std::list<VolumeData*> &channs, int color_type, bool consistent);
		bool GenRgbChannels(std::list<VolumeData*> &channs, int color_type, bool consistent);

		//update progress
		boost::signals2::signal<void()> m_sig_progress;

	private:
		bool m_analyzed;//if used
		int m_bn;
		bool m_colocal;
		unsigned int m_slimit;//size limit for connecting components
		std::vector<VolumeData*> m_vd_list;//list of volumes for colocalization analysis

		std::vector<CompGroup> m_comp_groups;//each analyzed volume can have comp results saved
		CompGroup* m_compgroup;//current group

	private:
		unsigned int GetExt(unsigned int* data_label,
			unsigned long long index,
			unsigned int id,
			int nx, int ny, int nz,
			int i, int j, int k);

		bool GetColor(unsigned int id, int brick_id,
			VolumeData* vd, int color_type,
			fluo::Color &color);
		int GetColocalization(size_t bid,
			unsigned int bi,
			unsigned int bj,
			unsigned int bk,
			std::vector<unsigned int> &sumi,
			std::vector<double> &sumd);

		//replace id to make color consistent
		void ReplaceId(unsigned int base_id, Celp &info);
		//get nonconflict color
		unsigned int GetNonconflictId(unsigned int id,
			int nx, int ny, int nz,
			flvr::TextureBrick* b,
			unsigned int* data);

		//comp groups
		CompGroup* FindCompGroup(VolumeData* vd)
		{
			for (size_t i = 0; i < m_comp_groups.size(); ++i)
			{
				if (m_comp_groups[i].vd == vd)
					return &(m_comp_groups[i]);
			}
			return 0;
		}
		CompGroup* AddCompGroup(VolumeData* vd)
		{
			if (!vd)
				return 0;
			m_comp_groups.push_back(CompGroup());
			CompGroup *compgroup = &(m_comp_groups.back());
			compgroup->vd = vd;
			return compgroup;
		}
	};
}
#endif//FL_CompAnalyzer_h
/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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

#include <Progress.h>
#include <Cell.h>
#include <Vector.h>
#include <Point.h>
#include <functional>
#include <string>
#include <vector>
#include <set>
#include <memory>

class VolumeData;
namespace flvr
{
	class TextureBrick;
}
namespace flrd
{
	class RulerList;
	struct CompGroup
	{
		std::weak_ptr<VolumeData> vd;//associated volume
		bool dirty;
		CelpList celps;
		CellGraph graph;//links comps in multibrick volume

		CompGroup() :
			dirty(true)
		{}

		void Clear()
		{
			celps.clear();
			graph.clear();
			dirty = true;
		}
	};

	typedef std::function<void()> CompAnalyzerFunc;

	class ComponentAnalyzer : public Progress
	{
	public:
		ComponentAnalyzer();
		~ComponentAnalyzer();

		bool GetAnalyzed()
		{ return m_analyzed; }

		void SetUseSel(bool val) { m_use_sel = val; }
		bool GetUseSel() { return m_use_sel; }
		unsigned int GetSizeLimit() { return m_slimit; }
		void SetSizeLimit(unsigned int size) { m_slimit = size; }
		void SetColocal(bool val) { m_colocal = val; }
		bool GetColocal() { return m_colocal; }
		void SetConsistent(bool val) { m_consistent = val; }
		bool GetConsistent() { return m_consistent; }
		void SetColorType(int val) { m_color_type = val; }
		int GetColorType() { return m_color_type; }
		void SetChannelType(int val) { m_channel_type = val; }
		int GetChannelType() { return m_channel_type; }
		void SetAnnotType(int val) { m_annot_type = val; }
		int GetAnnotType() { return m_annot_type; }
		void SetUseDistNeighbor(bool val) { m_use_dist_neighbor = val; }
		bool GetUseDistNeighbor() { return m_use_dist_neighbor; }
		void SetDistNeighborNum(int val) { m_dist_neighbor_num = val; }
		int GetDistNeighborNum() { return m_dist_neighbor_num; }
		void SetUseDistAllchan(bool val) { m_use_dist_allchan = val; }
		bool GetUseDistAllchan() { return m_use_dist_allchan; }

		void SetVolume(const std::shared_ptr<VolumeData>& vd);
		std::shared_ptr<VolumeData> GetVolume();

		void SetCoVolumes(const std::vector<std::weak_ptr<VolumeData>>& list);
		void AddCoVolume(const std::shared_ptr<VolumeData>& vd);
		void ClearCoVolumes();
		CelpList* GetCelpList();
		CellGraph* GetCellGraph();
		int GetCompGroupSize();
		CompGroup* GetCompGroup(int i);
		int GetBrickNum();
		//count
		void SetUseMin(bool val) { m_use_min = val; }
		void SetUseMax(bool val) { m_use_max = val; }
		bool GetUseMin() { return m_use_min; }
		bool GetUseMax() { return m_use_max; }
		void SetMinNum(unsigned int num) { m_min_num = num; }
		void SetMaxNum(unsigned int num) { m_max_num = num; }
		int GetMinNum() { return m_min_num; }
		int GetMaxNum() { return m_max_num; }
		size_t GetCount() { return m_count; }
		size_t GetVox() { return m_vox; }
		double GetSize() { return m_size; }

		void Analyze();
		void MatchBricks(bool sel);
		void UpdateMaxCompSize(bool);
		void MakeColorConsistent();
		void Count();
		void ClearCompGroup();

		size_t GetCompSize();
		size_t GetListSize();
		void GetCompsPoint(fluo::Point& p, std::set<unsigned long long> &ids);

		void OutputFormHeader(std::string &str);
		//print out results
		//verbose: 0-clean output; 1-extra info
		void OutputCompListStream(std::ostream &stream, int verbose, const std::string& comp_header = "");
		void OutputCompListStr(std::string &str, int verbose, const std::string& comp_header="");
		void OutputCompListFile(const std::wstring &filename, int verbose, const std::string& comp_header = "");

		bool OutputChannels();
		bool OutputMultiChannels(std::vector<std::shared_ptr<VolumeData>> &channs);
		bool OutputRgbChannels(std::vector<std::shared_ptr<VolumeData>> &channs);
		bool OutputAnnotData();

		//distance
		void OutputDistance(std::ostream &stream);
		size_t GetDistMatSize();

		//align
		bool GetRulerListFromCelp(RulerList& list);

		//list
		void SetSelectedIds(const std::vector<unsigned int>& ids,
			const std::vector<unsigned int>& bids);
		bool GetSelectedCelp(CelpList& cl, bool links = false);
		bool GetAllCelp(CelpList& cl, bool links = false);
		bool GetCelpFromIds(CelpList& cl, const std::vector<unsigned long long>& ids, bool links = false);

		//update progress
		CompAnalyzerFunc m_sig_progress;

	private:
		bool m_use_sel;//use mask instead of data
		bool m_analyzed;//if used
		unsigned int m_slimit;//size limit for connecting components
		bool m_colocal;
		bool m_consistent;
		int m_channel_type;//channel type: 1-multichannel; 2-rgb channel
		int m_color_type;//color_type: 1-id-based; 2-size-based
		int m_annot_type;//annot type: 1-id; 2:serianl number

		//distance
		bool m_use_dist_neighbor;
		int m_dist_neighbor_num;
		bool m_use_dist_allchan;

		int m_bn;

		std::vector<std::weak_ptr<VolumeData>> m_vd_list;//list of volumes for colocalization analysis
		std::vector<CompGroup> m_comp_groups;//each analyzed volume can have comp results saved
		CompGroup* m_compgroup;//current group

		//selection
		std::vector<unsigned int> m_sel_ids;
		std::vector<unsigned int> m_sel_bids;

		//count
		bool m_use_min;
		bool m_use_max;
		unsigned int m_min_num;
		unsigned int m_max_num;
		//result
		size_t m_count;
		size_t m_vox;
		double m_size;

	private:
		unsigned int GetExt(unsigned int* data_label,
			unsigned long long index,
			unsigned int id,
			const fluo::Vector& size,
			const fluo::Point& p);

		bool GetColor(unsigned int id,
			int brick_id,
			VolumeData* vd,
			fluo::Color &color);
		int GetColocalization(size_t bid,
			const fluo::Point& p,
			std::vector<unsigned int> &sumi,
			std::vector<double> &sumd);

		//replace id to make color consistent
		void ReplaceId(unsigned int base_id, Celp &info);
		//get nonconflict color
		unsigned int GetNonconflictId(unsigned int id,
			const fluo::Vector& size,
			flvr::TextureBrick* b,
			unsigned int* data);

		//comp groups
		CompGroup* FindCompGroup(const std::shared_ptr<VolumeData>& vd);
		CompGroup* AddCompGroup(const std::shared_ptr<VolumeData>& vd);

		//get list
		void FindCelps(CelpList& list,
			CelpListIter& it, bool links = false);
	};

}
#endif//FL_CompAnalyzer_h
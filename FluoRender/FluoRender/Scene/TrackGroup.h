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
#ifndef _TRACK_GROUP_H_
#define _TRACK_GROUP_H_

#include <TreeLayer.h>
#include <vector>
#include <unordered_map>

namespace flrd
{
	class Vertex;
	typedef std::shared_ptr<Vertex> Verp;
	typedef std::unordered_map<unsigned int, Verp> VertexList;
	class Ruler;
	class RulerList;
	typedef std::vector<Ruler*>::iterator RulerListIter;
	class EntryParams;
	class TrackMap;
	typedef std::shared_ptr<TrackMap> pTrackMap;
	class CelpList;
}
class TrackGroup : public TreeLayer
{
public:
	TrackGroup();
	virtual ~TrackGroup();

	//reset counter
	static void ResetID()
	{
		m_num = 0;
	}
	static void SetID(int id)
	{
		m_num = id;
	}
	static int GetID()
	{
		return m_num;
	}

	flrd::pTrackMap GetTrackMap()
	{
		return m_track_map;
	}

	std::wstring GetPath() {return m_data_path;}
	void SetCurTime(int time);
	int GetCurTime();
	void SetPrvTime(int time);
	int GetPrvTime();
	//ghost num
	void SetGhostNum(int num);
	int GetGhostNum() {return m_ghost_num;}
	void SetDrawTail(bool draw);
	bool GetDrawTail() {return m_draw_tail;}
	void SetDrawLead(bool draw);
	bool GetDrawLead() {return m_draw_lead;}
	//cells size filter
	void SetCellSize(int size) {m_cell_size = size;}
	int GetSizeSize() {return m_cell_size;}
	//uncertainty filter
	void SetUncertainLow(int value) { m_uncertain_low = value; }
	int GetUncertainLow() { return m_uncertain_low; }

	//get information
	void GetLinkLists(size_t frame,
		flrd::VertexList &in_orphan_list,
		flrd::VertexList &out_orphan_list,
		flrd::VertexList &in_multi_list,
		flrd::VertexList &out_multi_list);

	//for selective drawing
	void ClearCellList();
	void UpdateCellList(flrd::CelpList &cur_sel_list);
	flrd::CelpList &GetCellList();
	bool FindCell(unsigned int id);

	//rulers
	bool GetMappedRulers(flrd::RulerList &rulers);

	//i/o
	void Clear();
	bool Load(const std::wstring &filename);
	bool Save(const std::wstring &filename);

	//draw
	unsigned int Draw(std::vector<float> &verts, int shuffle);

	//pattern search
/*	typedef struct
	{
		int div;
		int conv;
	} Patterns;
	//type: 1-diamond; 2-branching
	bool FindPattern(int type, unsigned int id, int time);*/

private:
	static int m_num;
	std::wstring m_data_path;
	//for selective drawing
	int m_cur_time;
	int m_prv_time;
	int m_ghost_num;
	bool m_draw_tail;
	bool m_draw_lead;
	int m_cell_size;
	int m_uncertain_low;

	flrd::pTrackMap m_track_map;
	std::unique_ptr<flrd::CelpList> m_cell_list;

	//edges (in a vector of drawable)
	unsigned int GetMappedEdges(
		flrd::CelpList &sel_list1, flrd::CelpList &sel_list2,
		std::vector<float> &verts,
		size_t frame1, size_t frame2,
		int shuffle=0);
	//rulers
	bool GetMappedRulers(
		flrd::CelpList &sel_list1, flrd::CelpList &sel_list2,
		flrd::RulerList &rulers,
		size_t frame1, size_t frame2);
	flrd::RulerListIter FindRulerFromList(unsigned int id, flrd::RulerList &list);
};


#endif//_TRACK_GROUP_H_
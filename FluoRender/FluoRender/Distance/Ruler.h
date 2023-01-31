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
#ifndef FL_Ruler_h
#define FL_Ruler_h

#include <memory>
#include <vector>
#include <set>
#include <unordered_map>
#include <Types/Point.h>
#include <Types/Color.h>
#include <Types/Transform.h>
#include <wx/string.h>

namespace flrd
{
	class Ruler;
	class ProfileBin
	{
	public:
		ProfileBin() :
			m_pixels(0), m_accum(0.0) {}
		~ProfileBin() {}
		int m_pixels;
		double m_accum;
		double dist;//dist from start
	};

	class RulerPoint;
	typedef std::shared_ptr<RulerPoint> pRulerPoint;
	typedef std::vector<pRulerPoint> RulerBranch;
	typedef std::vector<std::pair<size_t, fluo::Point>> TimePoint;
	typedef TimePoint::iterator TimePointIter;
	typedef std::unordered_map<size_t, size_t> TimePointIndex;
	typedef TimePointIndex::iterator TimePointIndexIter;

	class RulerPoint
	{
	public:
		RulerPoint():
			m_locked(false),
			m_id(0)
		{}
		RulerPoint(bool locked):
			m_locked(locked),
			m_id(0)
		{}
		RulerPoint(fluo::Point& p, size_t t) :
			m_locked(false),
			m_id(0)
		{
			SetPoint(p, t);
		}
		RulerPoint(fluo::Point& p, bool locked, size_t t) :
			m_locked(locked),
			m_id(0)
		{
			SetPoint(p, t);
		}
		RulerPoint(fluo::Point& p, unsigned int id, size_t t) :
			m_locked(false),
			m_id(id)
		{
			SetPoint(p, t);
		}
		RulerPoint(fluo::Point& p, unsigned int id, std::set<unsigned int> bid, size_t t) :
			m_locked(false),
			m_id(id),
			m_bid(bid)
		{
			SetPoint(p, t);
		}
		RulerPoint(fluo::Point& p, unsigned int id, bool locked, size_t t) :
			m_locked(locked),
			m_id(id)
		{
			SetPoint(p, t);
		}

		size_t GetTimeNum() { return m_tp.size(); }
		bool GetTimeAndPoint(size_t i, size_t& t, fluo::Point& p)
		{
			if (i < m_tp.size())
			{
				t = m_tp[i].first;
				p = m_tp[i].second;
				return true;
			}
			return false;
		}
		void SetPoint(const fluo::Point& p, size_t t)
		{
			TimePointIndexIter i = m_index.find(t);
			if (i == m_index.end())
			{
				m_tp.push_back(std::make_pair(t, p));
				m_index.insert(std::make_pair(t, m_tp.size() - 1));
			}
			else
			{
				m_tp[i->second].second = p;
			}
		}
		fluo::Point GetPoint(size_t t)
		{
			TimePointIndexIter i = m_index.find(t);
			if (i == m_index.end())
			{
				if (m_tp.empty())
					return fluo::Point();
				return m_tp[get_prev(t)].second;
			}
			else
				return m_tp[i->second].second;
		}
		void ScalePoint(double sx, double sy, double sz, size_t t)
		{
			TimePointIndexIter i = m_index.find(t);
			if (i == m_index.end())
			{
				if (m_tp.empty())
					return;
				m_tp[get_prev(t)].second.scale(sx, sy, sz);
			}
			else
			{
				m_tp[i->second].second.scale(sx, sy, sz);
			}
		}
		void DisplacePoint(fluo::Vector& dp, size_t t)
		{
			TimePointIndexIter i = m_index.find(t);
			if (i == m_index.end())
			{
				if (m_tp.empty())
					return;
				fluo::Point op = m_tp[get_prev(t)].second + dp;
				SetPoint(op, t);
			}
			else
				m_tp[i->second].second += dp;
		}

		void SetLocked(bool locked = true)
		{
			m_locked = locked;
		}
		bool GetLocked()
		{
			return m_locked;
		}
		bool ToggleLocked()
		{
			m_locked = !m_locked;
			return m_locked;
		}

		void SetBid(std::set<unsigned int> &bid)
		{
			m_bid = bid;
		}
		bool MatchId(unsigned int id)
		{
			if (id == m_id)
				return true;
			if (m_bid.find(id) != m_bid.end())
				return true;
			return false;
		}
		void SetId(unsigned int id)
		{
			m_id = id;
		}
		unsigned int GetId()
		{
			return m_id;
		}

		friend class Ruler;

	private:
		TimePoint m_tp;//points over time (t, point)
		TimePointIndex m_index;//lookup for m_tp (t, index to m_tp)

		bool m_locked;
		unsigned int m_id;//from comp
		std::set<unsigned int> m_bid;//merged ids from multiple bricks

		size_t get_prev(size_t t)
		{
			if (m_tp.empty())
				return 0;
			size_t rt = 0;
			size_t rt2 = std::numeric_limits<unsigned int>::max();
			for (auto i = m_tp.rbegin();
				i != m_tp.rend(); ++i)
			{
				if (i->first == t - 1)
				{
					rt = i->first;
					break;
				}
				if (i->first < t && i->first > rt)
					rt = i->first;
				if (i->first > t && i->first < rt2)
					rt2 = i->first;
			}
			auto i = m_index.find(rt);
			if (i != m_index.end())
				return i->second;
			else
			{
				//find other dir
				i = m_index.find(rt2);
				if (i != m_index.end())
					return i->second;
			}
			return 0;
		}
	};

	class RulerList;
	class Ruler
	{
	public:
		Ruler();
		virtual ~Ruler();

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

		//name
		wxString GetName()
		{
			return m_name;
		}
		void SetName(wxString name)
		{
			m_name = name;
		}
		unsigned int Id()
		{
			return m_id;
		}
		void Id(unsigned int id)
		{
			m_id = id;
		}
		unsigned int Group()
		{
			return m_group;
		}
		void Group(unsigned int group)
		{
			m_group = group;
		}

		//data
		int GetNumBranch();
		int GetNumPoint();
		int GetNumBranchPoint(int nbranch);
		RulerPoint* GetRulerPoint(int index);
		RulerPoint* GetLastRulerPoint();
		pRulerPoint GetPRulerPoint(int index);
		RulerPoint* GetRulerPoint(int nbranch, int index);
		pRulerPoint GetPRulerPoint(int nbranch, int index);
		pRulerPoint FindPRulerPoint(fluo::Point& point);
		pRulerPoint FindNearestPRulerPoint(fluo::Point& point, size_t &ri, size_t &rj);
		pRulerPoint FindBranchPRulerPoint(fluo::Point& point, size_t& ri, size_t& rj);
		fluo::Point GetPoint(int index)
		{
			RulerPoint* p = GetRulerPoint(index);
			if (p)
				return p->GetPoint(m_work_time);
			return fluo::Point();
		}
		bool GetPoint(int index, fluo::Point& point)
		{
			RulerPoint* p = GetRulerPoint(index);
			if (p)
			{
				point = p->GetPoint(m_work_time);
				return true;
			}
			return false;
		}
		fluo::Point GetPointTransformed(int index)
		{
			fluo::Point p, tfp;
			if (GetPoint(index, p))
				m_tform.project(p, tfp);
			return tfp;
		}
		fluo::Point GetPoint(int nbranch, int index)
		{
			RulerPoint* p = GetRulerPoint(nbranch, index);
			if (p)
				return p->GetPoint(m_work_time);
			return fluo::Point();
		}
		void SetPoint(int index, const fluo::Point& point)
		{
			RulerPoint* p = GetRulerPoint(index);
			if (p)
				p->SetPoint(point, m_work_time);
		}
		int GetRulerType();
		void SetRulerType(int type);
		bool GetFinished();
		void SetFinished();
		double GetLength();
		double GetLengthObject(double spcx, double spcy, double spcz);
		double GetAngle();
		void Scale(double spcx, double spcy, double spcz);

		bool AddPoint(fluo::Point &point);
		bool AddPointAfterId(fluo::Point &point, unsigned int id,
			std::set<unsigned int> &cid, std::set<unsigned int> &bid);
		void SetTransform(const fluo::Transform &tform);
		fluo::Transform GetTransform();
		bool AddBranch(pRulerPoint point);
		void DeletePoint(pRulerPoint &point);
		void Prune(int len);

		void Clear();
		void Reverse();

		//display functions
		void SetDisp(bool disp)
		{
			m_disp = disp;
		}
		void ToggleDisp()
		{
			m_disp = !m_disp;
		}
		bool GetDisp()
		{
			return m_disp;
		}

		//work time
		void SetWorkTime(size_t t)
		{
			m_work_time = t;
		}
		size_t GetWorkTime()
		{
			return m_work_time;
		}
		//transient
		void SetTransient(bool bval)
		{
			m_transient = bval;
		}
		bool GetTransient()
		{
			return m_transient;
		}
		void SetTransTime(size_t t)
		{
			m_trans_time = t;
		}
		size_t GetTransTime()
		{
			return m_trans_time;
		}

		//extra info
		void AddInfoNames(wxString &str)
		{
			m_info_names += str;
		}
		void SetInfoNames(wxString &str)
		{
			m_info_names = str;
		}
		wxString &GetInfoNames()
		{
			return m_info_names;
		}
		void AddInfoValues(wxString &str)
		{
			m_info_values += str;
		}
		void SetInfoValues(wxString &str)
		{
			m_info_values = str;
		}
		wxString &GetInfoValues()
		{
			return m_info_values;
		}
		wxString GetDelInfoValues(wxString del = ",");
		wxString GetPosValues();
		wxString GetPosNames();

		//profile
		void SetInfoProfile(wxString &str)
		{
			m_info_profile = str;
		}
		wxString &GetInfoProfile()
		{
			return m_info_profile;
		}
		std::vector<ProfileBin> *GetProfile()
		{
			return &m_profile;
		}
		void SaveProfile(wxString &filename);
		double GetProfileMaxValue();
		void GetProfileMaxValue(double &val, double &dist);//return max value and its distance on ruler
		void SetScalarScale(double dval)
		{
			m_scale = dval;
		}
		double GetScalarScale()
		{
			return m_scale;
		}

		//color
		void SetColor(fluo::Color& color)
		{
			m_color = color; m_use_color = true;
		}
		bool GetUseColor()
		{
			return m_use_color;
		}
		fluo::Color &GetColor()
		{
			return m_color;
		}

		//brush size
		void SetBrushSize(double size)
		{
			m_brush_size = size;
		}
		double GetBrushSize()
		{
			return m_brush_size;
		}

		void FinishEllipse(fluo::Vector view);
		fluo::Point GetCenter();

		void SetMeanInt(double val)
		{
			m_mean_int = val;
		}
		double GetMeanInt()
		{
			return m_mean_int;
		}

	private:
		static int m_num;
		wxString m_name;
		unsigned int m_id;
		unsigned int m_group;//group number
		int m_ruler_type;	//0: 2 point; 1: multi point; 2:locator; 3: probe;
							//4: protractor; 5: ellipse
		bool m_finished;
		std::vector<RulerBranch> m_ruler;
		bool m_disp;
		fluo::Transform m_tform;
		//a profile
		wxString m_info_profile;
		std::vector<ProfileBin> m_profile;
		//mean intensity in roi
		double m_mean_int;
		double m_scale;
		//color
		bool m_use_color;
		fluo::Color m_color;

		//work time
		size_t m_work_time;
		//transient
		bool m_transient;
		size_t m_trans_time;

		//extra info
		wxString m_info_names;
		wxString m_info_values;

		//brush size if brush is used along with the ruler
		double m_brush_size;
	};

	class RulerList : public std::vector<Ruler*>
	{
	public:
		int GetGroupNum(std::vector<unsigned int> &groups)
		{
			for (auto iter = this->begin();
				iter != this->end(); ++iter)
			{
				unsigned int group = (*iter)->Group();
				if (std::find(groups.begin(), groups.end(), group) == groups.end())
					groups.push_back(group);
			}
			return groups.size();
		}

		int GetGroupNumAndCount(std::vector<unsigned int> &groups,
			std::vector<int> &counts)
		{
			for (auto iter = this->begin();
				iter != this->end(); ++iter)
			{
				unsigned int group = (*iter)->Group();
				auto ir = std::find(groups.begin(), groups.end(), group);
				if (ir == groups.end())
				{
					groups.push_back(group);
					counts.push_back(1);
				}
				else
				{
					int index = std::distance(groups.begin(), ir);
					counts[index]++;
				}
			}
			return groups.size();
		}
	};
	typedef RulerList::iterator RulerListIter;
}
#endif//FL_Ruler_h

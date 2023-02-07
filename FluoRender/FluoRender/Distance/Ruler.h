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
#include <map>
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
	typedef std::map<size_t, fluo::Point> TimePoint;
	typedef TimePoint::iterator TimePointIter;
	//typedef std::map<size_t, size_t> TimePointIndex;
	//typedef TimePointIndex::iterator TimePointIndexIter;

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
			size_t c = 0;
			for (auto &it : m_tp)
			{
				if (c == i)
				{
					t = it.first;
					p = it.second;
					return true;
				}
				c++;
			}
			return false;
		}
		void SetPoint(const fluo::Point& p, size_t t)
		{
			fluo::Point* op = get_point(t);
			if (op)
				*op = p;
			else
				m_tp.insert(std::make_pair(t, p));
		}
		fluo::Point GetPoint(size_t t, int interp)//interp:0-step; 1-linear; 2-smooth
		{
			fluo::Point* op = get_point(t);
			if (op)
				return* op;

			//not found
			switch (interp)
			{
			case 0:
			default:
				//step
				return get_point_step(t);
			case 1:
				return get_point_linear(t);
			case 2:
				return get_point_smooth(t);
			}
		}
		void ScalePoint(double sx, double sy, double sz)
		{
			//scale all points
			for (auto &i : m_tp)
				i.second.scale(sx, sy, sz);
		}
		void DisplacePoint(fluo::Vector& dp, size_t t, int interp)
		{
			fluo::Point* op = get_point(t);
			if (op)
				*op += dp;
			else
			{
				fluo::Point np = GetPoint(t, interp) + dp;
				SetPoint(np, t);
			}
		}
		void DeletePoint(size_t t)
		{
			if (m_tp.size() == 1)
				return;
			auto i = m_tp.find(t);
			if (i != m_tp.end())
				m_tp.erase(i);
		}
		void DeleteAllPoint(size_t t, int interp)
		{
			if (m_tp.size() == 1)
				return;
			fluo::Point p = GetPoint(t, interp);
			m_tp.clear();
			SetPoint(p, 0);
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
		//TimePointIndex m_index;//lookup for m_tp (t, index to m_tp)

		bool m_locked;
		unsigned int m_id;//from comp
		std::set<unsigned int> m_bid;//merged ids from multiple bricks

		//no interpolation
		fluo::Point* get_point(size_t t)
		{
			auto i = m_tp.find(t);
			if (i == m_tp.end())
				return 0;
			return &(i->second);
		}
		//interpolated point
		fluo::Point get_point_step(size_t t)
		{
			size_t pt, nt;
			int r = get_nb(t, pt, nt);
			switch (r)
			{
			case 0:
			default:
				return fluo::Point();
			case 1:
			case 3:
				t = pt;
				break;
			case 2:
				t = nt;
				break;
			}
			fluo::Point* op = get_point(t);
			if (op)
				return *op;
			return fluo::Point();
		}
		fluo::Point get_point_linear(size_t t)
		{
			size_t pt, nt;
			int r = get_nb(t, pt, nt);
			switch (r)
			{
			case 0:
			default:
				return fluo::Point();
			case 1:
				t = pt;
				break;
			case 2:
				t = nt;
				break;
			case 3:
			{
				fluo::Point* p0 = get_point(pt);
				fluo::Point* p1 = get_point(nt);
				if (!p0 || !p1)
					return fluo::Point();
				double d = nt - pt;
				if (d == 0.0)
					return fluo::Point();
				double w0, w1;
				w0 = (nt - t) / d;
				w1 = (t - pt) / d;
				return fluo::Point(w0 * (*p0) + w1 * (*p1));
			}
			}
			fluo::Point* op = get_point(t);
			if (op)
				return *op;
			return fluo::Point();
		}
		fluo::Point get_point_smooth(size_t t)
		{
			size_t pt, nt;
			int r = get_nb(t, pt, nt);
			switch (r)
			{
			case 0:
			default:
				return fluo::Point();
			case 1:
				t = pt;
				break;
			case 2:
				t = nt;
				break;
			case 3:
			{
				fluo::Point* p0 = get_point(pt);
				fluo::Point* p1 = get_point(nt);
				if (!p0 || !p1)
					return fluo::Point();
				double d = nt - pt;
				if (d == 0.0)
					return fluo::Point();
				double w0, w1;
				w1 = (t - pt) / d;
				w0 = -2.0 * w1 * w1 * w1 + 3.0 * w1 * w1;
				return fluo::Point((*p0) + w0 * ((*p1) - (*p0)));
			}
			}
			fluo::Point* op = get_point(t);
			if (op)
				return *op;
			return fluo::Point();
		}

		//return 0-not found (can't be); 1-found prt; 2-found nxt; 3-found both;
		int get_nb(size_t t, size_t& prt, size_t& nxt)//get previous and next
		{
			if (m_tp.empty())
				return false;

			prt = -1; nxt = -1;//init
			for (auto& i : m_tp)
			{
				size_t iv = i.first;
				if (iv < t)
					prt = iv;
				if (iv > t)
					nxt = iv;
				if (prt != -1 && nxt != -1)
					break;
			}
			return int(prt != -1) | (int(nxt != -1) << 1);
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
				return p->GetPoint(m_work_time, m_interp);
			return fluo::Point();
		}
		bool GetPoint(int index, fluo::Point& point)
		{
			RulerPoint* p = GetRulerPoint(index);
			if (p)
			{
				point = p->GetPoint(m_work_time, m_interp);
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
				return p->GetPoint(m_work_time, m_interp);
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

		//interp mode
		void SetInterp(int mode)
		{
			m_interp = mode;
		}
		int GetInterp()
		{
			return m_interp;
		}
		//delete key
		void DeleteKey()
		{
			for (size_t i = 0; i < GetNumPoint(); ++i)
			{
				RulerPoint* p = GetRulerPoint(i);
				if (!p)
					continue;
				p->DeletePoint(m_work_time);
			}
		}
		void DeleteAllKey()
		{
			for (size_t i = 0; i < GetNumPoint(); ++i)
			{
				RulerPoint* p = GetRulerPoint(i);
				if (!p)
					continue;
				p->DeleteAllPoint(m_work_time, m_interp);
			}
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

		//interp mode for time interpolation
		int m_interp;
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

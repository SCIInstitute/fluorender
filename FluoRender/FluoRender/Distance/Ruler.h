/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#include <Point.h>
#include <Color.h>
#include <Transform.h>
#include <BBox.h>
#include <memory>
#include <vector>
#include <set>
#include <map>
#include <string>

namespace flrd
{
	enum class RulerMode : int
	{
		None,
		Line,		//0: 2 point
		Polyline,	//1: multi point
		Locator,	//2: locator
		Probe,		//3: probe
		Protractor,	//4: protractor
		Ellipse		//5: ellipse
	};
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
	typedef std::vector<std::shared_ptr<RulerPoint>> RulerBranch;
	typedef std::map<size_t, fluo::Point> TimePoint;

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
		std::pair<size_t, fluo::Point> GetTimeAndPoint(size_t i)
		{
			size_t c = 0;
			size_t t = 0;
			fluo::Point p;
			for (auto &it : m_tp)
			{
				if (c == i)
				{
					t = it.first;
					p = it.second;
					return {t, p};
				}
				c++;
			}
			return {-1, fluo::Point()};
		}
		void SetPoint(const fluo::Point& p, size_t t)
		{
			m_tp[t] = p;
		}
		fluo::Point GetPoint(size_t t, int interp)//interp:0-step; 1-linear; 2-smooth
		{
			auto i = m_tp.find(t);
			if (i != m_tp.end())
				return i->second;
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
		void ScalePoint(const fluo::Vector& scale)
		{
			//scale all points
			for (auto &[i, p] : m_tp)
				p.scale(scale);
		}
		void DisplacePoint(fluo::Vector& dp, size_t t, int interp)
		{
			auto op = m_tp.find(t);
			if (op != m_tp.end())
				op->second += dp;
			else
			{
				auto np = GetPoint(t, interp) + dp;
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
			auto p = GetPoint(t, interp);
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
		fluo::Point get_point(size_t t)
		{
			auto i = m_tp.find(t);
			if (i == m_tp.end())
				return fluo::Point();
			return i->second;
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
			return get_point(t);
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
				auto p0 = get_point(pt);
				auto p1 = get_point(nt);
				double d = static_cast<double>(nt - pt);
				if (d == 0.0)
					return fluo::Point();
				double w0, w1;
				w0 = (nt - t) / d;
				w1 = (t - pt) / d;
				return fluo::Point(w0 * p0 + w1 * p1);
			}
			}
			return get_point(t);
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
				auto p0 = get_point(pt);
				auto p1 = get_point(nt);
				double d = static_cast<double>(nt - pt);
				if (d == 0.0)
					return fluo::Point();
				double w0, w1;
				w1 = (t - pt) / d;
				w0 = -2.0 * w1 * w1 * w1 + 3.0 * w1 * w1;
				return fluo::Point(p0 + w0 * (p1 - p0));
			}
			}
			return get_point(t);
		}

		//return 0-not found (can't be); 1-found prt; 2-found nxt; 3-found both;
		int get_nb(size_t t, size_t& prt, size_t& nxt)//get previous and next
		{
			if (m_tp.empty())
				return 0;

			prt = -1; nxt = -1;//init
			for (auto& i : m_tp)
			{
				size_t iv = i.first;
				if (iv < t)
					prt = iv;
				if (iv > t)
				{
					nxt = iv;
					break;
				}
			}
			return int(prt != -1) | (int(nxt != -1) << 1);
		}
	};

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
		static int GetIdCount()
		{
			return m_num;
		}

		//name
		std::wstring GetName()
		{
			return m_name;
		}
		void SetName(const std::wstring& name)
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

		//bounding box
		fluo::BBox GetBounds();

		//data
		int GetNumBranch();
		int GetNumPoint();
		int GetNumBranchPoint(int nbranch);
		std::shared_ptr<RulerPoint> GetLastRulerPoint();
		std::shared_ptr<RulerPoint> GetRulerPoint(int index);
		std::shared_ptr<RulerPoint> GetRulerPoint(int nbranch, int index);
		std::shared_ptr<RulerPoint> FindRulerPoint(fluo::Point& point);
		std::shared_ptr<RulerPoint> FindNearestRulerPoint(fluo::Point& point, size_t &ri, size_t &rj);
		std::shared_ptr<RulerPoint> FindBranchRulerPoint(fluo::Point& point, size_t& ri, size_t& rj);
		fluo::Point GetPoint(int index)
		{
			auto p = GetRulerPoint(index);
			if (p)
				return p->GetPoint(m_work_time, m_interp);
			return fluo::Point();
		}
		bool GetPoint(int index, fluo::Point& point)
		{
			auto p = GetRulerPoint(index);
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
			auto p = GetRulerPoint(nbranch, index);
			if (p)
				return p->GetPoint(m_work_time, m_interp);
			return fluo::Point();
		}
		void SetPoint(int index, const fluo::Point& point)
		{
			auto p = GetRulerPoint(index);
			if (p)
				p->SetPoint(point, m_work_time);
		}
		RulerMode GetRulerMode();
		void SetRulerMode(RulerMode mode);
		bool GetFinished();
		void SetFinished();
		double GetLength();
		double GetLengthObject(const fluo::Vector& scale);
		double GetAngle();
		void Scale(const fluo::Vector& scale);

		bool AddPoint(fluo::Point &point);
		bool AddPointAfterId(fluo::Point &point, unsigned int id,
			std::set<unsigned int> &cid, std::set<unsigned int> &bid);
		void SetTransform(const fluo::Transform &tform);
		fluo::Transform GetTransform();
		bool AddBranch(const std::shared_ptr<RulerPoint>& point);
		void DeletePoint(const std::shared_ptr<RulerPoint>& point);
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
		//display
		void SetDisplay(int type, bool bval)
		{
			if (type >= 0 && type < 3)
				m_disp_part[type] = bval;
		}
		bool GetDisplay(int type)
		{
			if (type >= 0 && type < 3)
				return m_disp_part[type];
			return true;
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
		void AddInfoNames(const std::string &str)
		{
			m_info_names += str;
		}
		void SetInfoNames(const std::string &str)
		{
			m_info_names = str;
		}
		std::string GetInfoNames()
		{
			return m_info_names;
		}
		void AddInfoValues(const std::string &str)
		{
			m_info_values += str;
		}
		void SetInfoValues(const std::string &str)
		{
			m_info_values = str;
		}
		std::string GetInfoValues()
		{
			return m_info_values;
		}
		std::string GetDelInfoValues(const std::string& del = ",");
		std::string GetPosValues();
		std::string GetPosNames();

		//profile
		void SetPaintIntensity(int ival, double dval);//use intensity value from paint instead of profile
		void SetInfoProfile(const std::string &str)
		{
			m_info_profile = str;
		}
		std::string GetInfoProfile()
		{
			return m_info_profile;
		}
		std::vector<ProfileBin>& GetProfile()
		{
			return m_profile;
		}
		void SaveProfile(const std::string &filename);
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
				auto p = GetRulerPoint(static_cast<int>(i));
				if (!p)
					continue;
				p->DeletePoint(m_work_time);
			}
		}
		void DeleteAllKey()
		{
			for (size_t i = 0; i < GetNumPoint(); ++i)
			{
				auto p = GetRulerPoint(static_cast<int>(i));
				if (!p)
					continue;
				p->DeleteAllPoint(m_work_time, m_interp);
			}
		}

	private:
		static int m_num;
		std::wstring m_name;
		unsigned int m_id;
		unsigned int m_group;//group number
		RulerMode m_mode;
		bool m_finished;
		std::vector<RulerBranch> m_ruler;
		bool m_disp;
		fluo::Transform m_tform;
		//a profile
		std::string m_info_profile;
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
		//display
		bool m_disp_part[3];//point, line, name

		//extra info
		std::string m_info_names;
		std::string m_info_values;

		//brush size if brush is used along with the ruler
		double m_brush_size;

		//interp mode for time interpolation
		int m_interp;
	};

}
#endif//FL_Ruler_h

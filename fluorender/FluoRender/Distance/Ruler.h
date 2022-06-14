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
	};

	class RulerPoint;
	typedef std::shared_ptr<RulerPoint> pRulerPoint;
	typedef std::vector<pRulerPoint> RulerBranch;

	class RulerPoint
	{
	public:
		RulerPoint():
			m_locked(false),
			m_id(0)
		{}
		RulerPoint(fluo::Point& p):
			m_p(p),
			m_locked(false),
			m_id(0)
		{}
		RulerPoint(bool locked):
			m_locked(locked),
			m_id(0)
		{}
		RulerPoint(fluo::Point& p, bool locked):
			m_p(p),
			m_locked(locked),
			m_id(0)
		{}
		RulerPoint(fluo::Point& p, unsigned int id):
			m_p(p),
			m_locked(false),
			m_id(id)
		{}
		RulerPoint(fluo::Point& p, unsigned int id, std::set<unsigned int> bid) :
			m_p(p),
			m_locked(false),
			m_id(id),
			m_bid(bid)
		{}
		RulerPoint(fluo::Point& p, unsigned int id, bool locked):
			m_p(p),
			m_locked(locked),
			m_id(id)
		{}

		void SetPoint(fluo::Point& p)
		{
			m_p = p;
		}
		fluo::Point GetPoint()
		{
			return m_p;
		}
		void ScalePoint(double sx, double sy, double sz)
		{
			m_p.scale(sx, sy, sz);
		}
		void DisplacePoint(fluo::Vector& dp)
		{
			m_p += dp;
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
		fluo::Point m_p;
		bool m_locked;
		unsigned int m_id;//from comp
		std::set<unsigned int> m_bid;//merged ids from multiple bricks
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
		fluo::Point GetPointTransformed(int index)
		{
			fluo::Point tfp;
			RulerPoint* p = GetPoint(index);
			if (p)
				m_tform.project(p->GetPoint(), tfp);
			return tfp;
		}
		RulerPoint* GetPoint(int index);
		RulerPoint* GetLastPoint();
		pRulerPoint GetPPoint(int index);
		int GetNumBranchPoint(int nbranch);
		RulerPoint* GetPoint(int nbranch, int index);
		pRulerPoint GetPPoint(int nbranch, int index);
		pRulerPoint FindPoint(fluo::Point& point);
		pRulerPoint FindNearestPoint(fluo::Point& point, size_t &ri, size_t &rj);
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

		//time-dependent
		void SetTimeDep(bool time_dep)
		{
			m_time_dep = time_dep;
		}
		bool GetTimeDep()
		{
			return m_time_dep;
		}
		void SetTime(int time)
		{
			m_time = time;
		}
		int GetTime()
		{
			return m_time;
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
		//color
		bool m_use_color;
		fluo::Color m_color;

		//time-dependent
		bool m_time_dep;
		int m_time;

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

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
#include <FLIVR/Point.h>
#include <FLIVR/Color.h>
#include <FLIVR/Transform.h>
#include <wx/string.h>

using namespace FLIVR;

namespace FL
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
		RulerPoint(Point& p):
			m_p(p),
			m_locked(false),
			m_id(0)
		{}
		RulerPoint(bool locked):
			m_locked(locked),
			m_id(0)
		{}
		RulerPoint(Point& p, bool locked):
			m_p(p),
			m_locked(locked),
			m_id(0)
		{}
		RulerPoint(Point& p, unsigned int id):
			m_p(p),
			m_locked(false),
			m_id(id)
		{}
		RulerPoint(Point& p, unsigned int id, bool locked):
			m_p(p),
			m_locked(locked),
			m_id(id)
		{}

		void SetPoint(Point& p)
		{
			m_p = p;
		}
		Point GetPoint()
		{
			return m_p;
		}
		void ScalePoint(double sx, double sy, double sz)
		{
			m_p.scale(sx, sy, sz);
		}
		void DisplacePoint(Vector& dp)
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

		friend class Ruler;

	private:
		Point m_p;
		bool m_locked;
		unsigned int m_id;//from comp
	};

	typedef std::vector<Ruler*> RulerList;
	typedef std::vector<Ruler*>::iterator RulerListIter;

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

		//data
		int GetNumBranch();
		int GetNumPoint();
		RulerPoint* GetPoint(int index);
		RulerPoint* GetLastPoint();
		pRulerPoint GetPPoint(int index);
		int GetNumBranchPoint(int nbranch);
		RulerPoint* GetPoint(int nbranch, int index);
		pRulerPoint FindPoint(Point& point);
		int GetRulerType();
		void SetRulerType(int type);
		bool GetFinished();
		void SetFinished();
		double GetLength();
		double GetLengthObject(double spcx, double spcy, double spcz);
		double GetAngle();
		void Scale(double spcx, double spcy, double spcz);

		bool AddPoint(Point &point);
		bool AddPointAfterId(Point &point, unsigned int id, unsigned int cid);
		void SetTransform(Transform *tform);
		bool AddBranch(pRulerPoint point);

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
		void SetColor(Color& color)
		{
			m_color = color; m_use_color = true;
		}
		bool GetUseColor()
		{
			return m_use_color;
		}
		Color &GetColor()
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

		void FinishEllipse(Vector view);
		Point GetCenter();

	private:
		static int m_num;
		wxString m_name;
		unsigned int m_id;
		int m_ruler_type;	//0: 2 point; 1: multi point; 2:locator; 3: probe;
							//4: protractor; 5: ellipse
		bool m_finished;
		std::vector<RulerBranch> m_ruler;
		bool m_disp;
		Transform *m_tform;
		//a profile
		wxString m_info_profile;
		std::vector<ProfileBin> m_profile;
		//color
		bool m_use_color;
		Color m_color;

		//time-dependent
		bool m_time_dep;
		int m_time;

		//extra info
		wxString m_info_names;
		wxString m_info_values;

		//brush size if brush is used along with the ruler
		double m_brush_size;
	};

}
#endif//FL_Ruler_h

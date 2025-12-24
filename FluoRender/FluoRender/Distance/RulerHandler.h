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

#ifndef _RulerHandler_H_
#define _RulerHandler_H_

#include <Point.h>
#include <Plane.h>
#include <string>
#include <algorithm>
#include <set>
#include <memory>

class RenderView;
class VolumeData;

namespace flrd
{
	enum class RulerMode : int;
	class Ruler;
	class RulerPoint;
	class RulerList;
	class WalkCycle;
	class RulerHandler
	{
	public:
		RulerHandler();
		~RulerHandler();

		//set selected ruler list
		void SetSelRulers(const std::set<int>& sel_list) { m_sel_list = sel_list; }
		std::set<int> GetSelRulers() { return m_sel_list; }
		//handle group
		void NewGroup();
		void SetGroup(unsigned int group) { m_group = group; }
		unsigned int GetGroup() { return m_group; }

		void GroupRulers(const std::set<int>& rulers);

		double GetVolumeBgInt();

		Ruler* GetRuler(const std::wstring& name);

		Ruler* GetRuler(size_t i);

		int GetRulerIndex();

		void GetRulerList(const std::set<int>& rulers, flrd::RulerList& list);

		void SetRulerMode(RulerMode val) { m_mode = val; }

		RulerMode GetRulerMode() { return m_mode; }

		void SetEdited(bool val) { m_edited = val; }
		bool GetEdited() { return m_edited; }

		//display
		void SetDisplay(bool bval, const std::set<int>& list);
		void ToggleDisplay(const std::set<int>& list);
		void ToggleGroupDisp();
		//display part
		void SetDisplay(bool bval, const std::set<int>& rulers, int type);//0:point; 1:line; 2:name

		//search
		bool FindEditingRuler(double mx, double my);
		bool FindClosestRulerPoint(double mx, double my);
		bool FindClosestRulerBranch(double mx, double my);
		bool FindClosestRulerBranchPoint(double mx, double my);

		void SetPoint(const std::shared_ptr<RulerPoint>& point) { m_point = point; }
		RulerPoint* GetPoint() { return m_point.get(); }
		void DeletePoint();

		RulerPoint* GetEllipsePoint(int index);
		bool CompleteEllipse(int mode);

		void FinishRuler();
		bool GetRulerFinished();

		Ruler* AddRuler(fluo::Point& p, size_t t);
		void AddRulerPoint(fluo::Point &p);
		void AddRulerPointAfterId(fluo::Point &p, unsigned int id, std::set<unsigned int> &cid, std::set<unsigned int> &bid);
		bool GetMouseDist(int mx, int my, double dist);
		void AddRulerPoint(int mx, int my, int branch);//branch:0-na; 1-polyline; 2-pencil
		void AddPaintRulerPoint();
		bool MoveRuler(int mx, int my);
		bool EditPoint(int mx, int my, bool alt);
		void Flip(const std::set<int>& rulers);
		void AddAverage(const std::set<int>& rulers);
		bool GetAutoRelax();
		void Relax();
		void Relax(const std::set<int>& rulers);
		void Prune(const std::set<int>& rulers);
		void Prune(int idx, int len);
		void Profile(const std::set<int>& rulers);
		int Profile(Ruler* ruler);
		int Profile(int index);
		int ProfileAll();
		void Distance(const std::set<int>& rulers, const std::wstring& filename);
		void Project(const std::set<int>& rulers, const std::wstring& filename);

		//transient over time
		void SetTransient(bool bval, const std::set<int>& rulers);

		//key
		void SetInterp(int ival, const std::set<int>& rulers);
		void DeleteKey(const std::set<int>& rulers);
		void DeleteAllKeys(const std::set<int>& rulers);

		int Roi(Ruler* ruler);
		int Roi(int index);
		int RoiAll();

		//stroke for magnet
		bool MagStrokeEmpty() { return m_mag_stroke.empty(); }
		void ApplyMagPoint();
		void ApplyMagStroke();
		void InitMagStrokeLength(int n);
		void InitMagRulerLength();
		fluo::Point GetPointOnMagStroke(int i);
		fluo::Point GetPointOnMagStroke(double d);
		void ClearMagStroke();
		void AddMagStrokePoint(int mx, int my);

		void DeleteSelection(const std::set<int> &sel);
		void DeleteAll(bool cur_time);

		std::wstring PrintRulers(bool h);//h-if prints hierarchy

		void SetFsize(int ival) { m_fsize = ival; }
		void SetSampleType(int type) { m_sample_type = type; }
		void SetStepLength(double dval) { m_step_length = dval; }

		//magnet mode
		bool GetRedistLength() { return m_redist_len; }
		void SetRedistLength(bool val) { m_redist_len = val; }

		//get time points where keys exist
		bool GetKeyFrames(std::set<size_t>& kf);
		size_t GetRulerPointNum();
		bool GetRulerPointNames(std::vector<std::wstring>& names);
		bool GetRulerPointNames(std::vector<std::string>& names);
		bool GetRulerPointCoords(std::vector<double>& coords);

		//generate walk
		void GenerateWalk(size_t nl, double dir, WalkCycle& cycle);

		//get values for gradient
		std::pair<bool, fluo::Point> GetCenter();
		std::pair<bool, double> GetRadius();
		std::pair<bool, std::pair<fluo::Plane, fluo::Plane>> GetLinearPlanes();

	private:
		std::set<int> m_sel_list;
		unsigned int m_group;
		Ruler* m_mag_ruler;
		size_t m_mag_branch;
		size_t m_mag_branch_point;
		bool m_redist_len;
		RulerMode m_mode;
		bool m_edited;

		//find moving distance
		fluo::Point m_mouse;//mouse position
		//get point
		std::shared_ptr<RulerPoint> m_point;
		int m_pindex;//index of point in ruler

		//stroke for the magnet tool
		int m_magx, m_magy;//2d coords for the starting point for searching closest ruler point
		std::vector<fluo::Point> m_mag_stroke;
		std::vector<double> m_mag_stroke_len;//interval lengths on stroke
		double m_mag_stroke_int;//interval length on ruler
		std::vector<double> m_mag_ruler_len;//interval lengths on ruler before editing

		//simple data sampler
		int m_sample_type;//0-nn; 1-bilinear
		double m_step_length;//for sampling along a ruler
		void* m_data;
		size_t m_nx, m_ny, m_nz, m_bits, m_fsize;//box filter
		double m_scale;
		bool valid()
		{
			if (!m_data) return false;
			if (!m_nx || !m_ny || !m_nz) return false;
			return true;
		}
		void clampxyz(double &x, double &y, double &z)
		{
			x = std::trunc(std::clamp(x, 0.0, double(m_nx - 1)));
			y = std::trunc(std::clamp(y, 0.0, double(m_ny - 1)));
			z = std::trunc(std::clamp(z, 0.0, double(m_nz - 1)));
		}
		bool excldxyz(double x, double y, double z)
		{
			return (x >= 0 && x < m_nx&&
				y >= 0 && y < m_ny&&
				z >= 0 && z < m_nz);
		}
		double get_data(double x, double y, double z)
		{
			switch (m_sample_type)
			{
			case 0:
				return get_data_nn(x, y, z);
			case 1:
			default:
				return get_data_bl(x, y, z);
			}
		}
		double get_data_nn(double x, double y, double z)
		{
			//clampxyz(x, y, z);
			if (!excldxyz(x, y, z))
				return 0;
			unsigned long long index =
				static_cast<unsigned long long>(z) * m_nx * m_ny +
				static_cast<unsigned long long>(y) * m_nx +
				static_cast<unsigned long long>(x);
			if (m_bits == 8)
				return double(((unsigned char*)m_data)[index]) / 255.0;
			else
				return double(((unsigned short*)m_data)[index]) / 65535.0;
		}
		void xyz2ijkt(
			double x, double y, double z,
			int &i, int &j, int &k,
			double &tx, double &ty, double &tz)
		{
			double id = x - 0.5;
			double jd = y - 0.5;
			double kd = z - 0.5;
			i = id >= 0.0 ? int(id) : int(id) - 1;
			j = jd >= 0.0 ? int(jd) : int(jd) - 1;
			k = kd >= 0.0 ? int(kd) : int(kd) - 1;
			tx = id - i;
			ty = jd - j;
			tz = kd - k;
		}
		double lerp(double t, double q0, double q1)
		{
			return (1.0 - t) * q0 + t * q1;
		}
		double bilerp(double tx, double ty,
			double q00, double q01, double q10, double q11)
		{
			double r1 = lerp(tx, q00, q10);
			double r2 = lerp(tx, q01, q11);
			return lerp(ty, r1, r2);
		}
		bool ijk(int &i, int &j, int &k)
		{
			if (i < 0) i = 0;
			if (i >= m_nx) i = static_cast<int>(m_nx - 1);
			if (j < 0) j = 0;
			if (j >= m_ny) j = static_cast<int>(m_ny - 1);
			if (k < 0) k = 0;
			if (k >= m_nz) k = static_cast<int>(m_nz - 1);
			return true;
		}
		double get_data_bl(double x, double y, double z)
		{
			//clampxyz(x, y, z);
			if (!excldxyz(x, y, z))
				return 0;
			int i, j, k;
			double tx, ty, tz;
			xyz2ijkt(x, y, z, i, j, k, tx, ty, tz);
			double q[4] = { 0 };
			int count = 0;
			int in, jn;
			unsigned long long index;
			for (int ii = 0; ii < 2; ++ii)
			for (int jj = 0; jj < 2; ++jj)
			{
				in = i + ii;
				jn = j + jj;
				if (ijk(in, jn, k))
				{
					index = (unsigned long long)m_nx*m_ny*k +
						(unsigned long long)m_nx*jn +
						(unsigned long long)in;
					if (m_bits == 8)
						q[count] = double(((unsigned char*)m_data)[index]) / 255.0;
					else
						q[count] = double(((unsigned short*)m_data)[index]) / 65535.0;
				}
				count++;
			}

			return bilerp(tx, ty,
				q[0], q[1], q[2], q[3]);
		}
		double get_filtered_data(double x, double y, double z)
		{
			if (m_fsize > 1)
			{
				size_t count = 0;
				double sum = 0;
				double val;
				int lub = static_cast<int>(m_fsize / 2 + m_fsize % 2);
				double r = double(m_fsize) / 2.0; r *= r;
				double dx, dy;
				for (int ii = -lub; ii <= lub; ++ii)
				for (int jj = -lub; jj <= lub; ++jj)
				//for (int kk = -lb; kk <= ub; ++kk)
				{
					dx = double(ii) + 0.5; dx *= dx;
					dy = double(jj) + 0.5; dy *= dy;
					if (dx + dy > r + fluo::Epsilon()) continue;
					val = get_data(x + ii, y + jj, z);
					if (val > 0)
					{
						sum += val;
						count++;
					}
				}
				if (count)
					return sum / count;
			}
			else
			{
				return get_data(x, y, z);
			}
			return 0;
		}

		//snap ruler points to magnet stroke
		RulerPoint* get_closest_point(fluo::Point& p);
	};

}
#endif//_RulerHandler_H_
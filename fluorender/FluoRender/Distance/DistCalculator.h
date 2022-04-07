/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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
#ifndef FL_DistCalculator_h
#define FL_DistCalculator_h

#include <Ruler.h>
#include <Relax.h>
#include <Tracking/Cell.h>

namespace fluo
{
	class VolumeData;
}
namespace flrd
{
	class DistCalculator
	{
	public:
		DistCalculator();
		~DistCalculator();

		void SetRuler(Ruler* ruler)
		{
			if (ruler != m_ruler)
			{
				m_ruler = ruler;
				m_init = false;
			}
			m_relax.SetRuler(ruler);
		}
		Ruler* GetRuler()
		{
			return m_ruler;
		}
		void SetCelpList(CelpList* list)
		{
			if (list != m_celps)
			{
				m_celps = list;
				m_init = false;
			}
		}
		CelpList* GetCelpList()
		{
			return m_celps;
		}
		void SetVolume(fluo::VolumeData* vd)
		{
			m_vd = vd;
			m_relax.SetVolume(vd);
		}

		void SetF1(double val)
		{
			m_f1 = val;
		}
		void SetInfr(double val)
		{
			m_infr = val;
		}

		void CenterRuler(int type, bool init, int iter=1);
		void Project();

	private:
		int m_type;//0:no data; 1:volume; 2:mask; 3:comps
		bool m_init;
		//components
		CelpList *m_celps;
		//volume
		fluo::VolumeData *m_vd;
		//Ruler
		Ruler *m_ruler;
		//spring properties
		double m_rest;
		double m_f1;
		double m_f2;
		double m_f3;
		double m_infr;//range of influence

		struct SpringNode
		{
			RulerPoint *p;
			double prevd;
			double nextd;
			double dist;//total distance
		};
		std::vector<SpringNode> m_spring;
		std::vector<fluo::Point> m_cloud;

		//volume relax
		Relax m_relax;

	private:
		void BuildSpring();
		void BuildCloud();
		double GetRestDist();
		void UpdateSpringNode(int idx);
		void UpdateSpringDist();
		void SpringProject(fluo::Point &p0, fluo::Point &pp);
	};
}
#endif//FL_CompAnalyzer_h
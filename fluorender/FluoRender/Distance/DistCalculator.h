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
#ifndef FL_DistCalculator_h
#define FL_DistCalculator_h

#include "CompGraph.h"
#include "DataManager.h"

namespace FL
{
	class DistCalculator
	{
	public:
		DistCalculator();
		~DistCalculator();

		void SetCompList(CompList* list)
		{
			if (list != m_comp_list)
			{
				m_comp_list = list;
				m_init = false;
			}
		}
		CompList* GetCompList()
		{
			return m_comp_list;
		}
		void SetRuler(Ruler* ruler)
		{
			if (ruler != m_ruler)
			{
				m_ruler = ruler;
				m_init = false;
			}
		}
		Ruler* GetRuler()
		{
			return m_ruler;
		}
		void SetF1(double val)
		{
			m_f1 = val;
		}

		void CenterRuler(bool init, int iter=1);

	private:
		bool m_init;
		//components
		CompList *m_comp_list;
		//Ruler
		Ruler *m_ruler;
		//spring properties
		double m_rest;
		double m_f1;
		double m_f2;
		double m_f3;

		struct SpringNode
		{
			Point p;
			double prevd;
			double nextd;
		};
		std::vector<SpringNode> m_spring;
		std::vector<Point> m_cloud;

	private:
		void BuildSpring();
		void BuildCloud();
		double GetRestDist();
		void UpdateSpringNode(int idx);
		void UpdateRuler();
	};
}
#endif//FL_CompAnalyzer_h
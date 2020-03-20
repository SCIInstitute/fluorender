/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2020 Scientific Computing and Imaging Institute,
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
#ifndef FL_Relax_h
#define FL_Relax_h

#include <Distance/Ruler.h>
#include <FLIVR/KernelProgram.h>
#include <FLIVR/VolKernel.h>
#include <FLIVR/Point.h>
#include <vector>

using namespace std;

class VolumeData;
namespace FL
{
	class Relax
	{
	public:
		Relax();
		~Relax();

		void SetUseMask(bool use_mask)
		{
			m_use_mask = use_mask;
		}
		bool GetUseMask()
		{
			return m_use_mask;
		}
		void SetVolume(VolumeData* vd)
		{
			m_vd = vd;
		}
		void SetRuler(Ruler* ruler)
		{
			if (ruler != m_ruler)
			{
				m_ruler = ruler;
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
		void SetInfr(double val)
		{
			m_infr = val;
		}

		bool Compute();

	private:
		bool m_use_mask;//use mask instead of data
		bool m_use_int;//use intensity values as weights
		VolumeData *m_vd;
		//Ruler
		Ruler *m_ruler;
		//spring properties
		double m_rest;
		double m_f1;
		double m_f2;
		double m_f3;
		double m_infr;//range of influence
		//spring
		unsigned int m_snum;//total point number
		std::vector<float> m_spoints;//x, y, z for each point
		std::vector<unsigned int> m_slock;//lock for each point
		std::vector<float> m_sdist;//prevd for each point

	private:
		void BuildSpring();
	};

}
#endif//FL_Relax_h
